
#include<iostream>
#include<array>
#include<vector>
#include<sstream>
#include<algorithm>
#include "GameBoard.h"
#include "Move.h"
#include "Tables.h"

    GameBoard::GameBoard() {
        setFromFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        irreversibleNumber = 0;
    }

    bool GameBoard::setFromFen(const std::string& FEN) {
        if (FEN.find("startpos") == 0) {
            return setFromFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        }
        std::istringstream fenStream(FEN);
        std::string piecePlacement, activeColor, castlingRights, enPassant, halfmoveClock, fullmoveNumber;
        if (!(fenStream >> piecePlacement >> activeColor >> castlingRights >> enPassant >> halfmoveClock >> fullmoveNumber)) {
            return false; 
        }

        int row = 7, col = 0;
        for (char ch : piecePlacement) {
            if (ch == '/') {
                if (col != 8) return false; 
                col = 0;
                row--;
                if (row < 0) return false;
            }
            else if (isdigit(ch)) {
                col += ch - '0'; 
                if (col > 8) return false; 
            }
            else if (strchr("PNBRQKpnbrqk", ch)) {
                col++; 
                if (col > 8) return false; 
            }
            else {
                return false; 
            }
        }
        if (row != 0 || col != 8) return false; 

        if (activeColor != "w" && activeColor != "b") return false;
        if (castlingRights != "-" && castlingRights.find_first_not_of("KQkq") != std::string::npos) return false;
        if (enPassant != "-" && (enPassant.size() != 2 || enPassant[0] < 'a' || enPassant[0] > 'h' || enPassant[1] < '1' || enPassant[1] > '6')) {
            return false;
        }

        if (!std::all_of(halfmoveClock.begin(), halfmoveClock.end(), ::isdigit)) return false;
        if (!std::all_of(fullmoveNumber.begin(), fullmoveNumber.end(), ::isdigit) || std::stoi(fullmoveNumber) <= 0) {
            return false;
        }

        positionHistory.clear();

        // validation pass, reset the position
        whites[PAWN] = whites[KNIGHT] = whites[BISHOP] = whites[ROOK] = whites[QUEEN] = whites[KING] =
            blacks[PAWN] = blacks[KNIGHT] = blacks[BISHOP] = blacks[ROOK] = blacks[QUEEN] = blacks[KING] =
            whitePieces = blackPieces = occupied = 0;
        for (int i = 0; i < 64; i++) {
            whitePieceArray[i] = EMPTY;
            blackPieceArray[i] = EMPTY;
        }
        turn = true; 
        wOO = wOOO = bOO = bOOO = false; 
        enPassantTarget = 0x0ULL; 
        ply50MoveRule = 0; 
        plyCount = 1; 

        // actually process the fen 
        row = 7;
        col = 0;
        for (int index = 0; index < piecePlacement.size(); index++) {
            if (piecePlacement[index] == '/') {
                col = 0; row--;
            }
            else if (isdigit(piecePlacement[index])) {
                col += piecePlacement[index] - '0';
            }
            else {
                Bitboard bb = bit << (col + row * 8);
                switch (piecePlacement[index]) {
                case 'P': whitePieceArray[row * 8 + col] = PAWN; whites[PAWN] |= bb; break;
                case 'N': whitePieceArray[row * 8 + col] = KNIGHT; whites[KNIGHT] |= bb; break;
                case 'B': whitePieceArray[row * 8 + col] = BISHOP; whites[BISHOP] |= bb; break;
                case 'R': whitePieceArray[row * 8 + col] = ROOK; whites[ROOK] |= bb; break;
                case 'Q': whitePieceArray[row * 8 + col] = QUEEN; whites[QUEEN] |= bb; break;
                case 'K': whitePieceArray[row * 8 + col] = KING; whites[KING] |= bb; wKingPos = row * 8 + col; break;
                case 'p': blackPieceArray[row * 8 + col] = PAWN; blacks[PAWN] |= bb; break;
                case 'n': blackPieceArray[row * 8 + col] = KNIGHT; blacks[KNIGHT] |= bb; break;
                case 'b': blackPieceArray[row * 8 + col] = BISHOP; blacks[BISHOP] |= bb; break;
                case 'r': blackPieceArray[row * 8 + col] = ROOK; blacks[ROOK] |= bb; break;
                case 'q': blackPieceArray[row * 8 + col] = QUEEN; blacks[QUEEN] |= bb; break;
                case 'k': blackPieceArray[row * 8 + col] = KING; blacks[KING] |= bb; bKingPos = row * 8 + col; break;
                default: break;
                }
                col++;
            }
        }
        whitePieces = whites[PAWN] | whites[KNIGHT] | whites[BISHOP] | whites[ROOK] | whites[QUEEN] | whites[KING];
        blackPieces = blacks[PAWN] | blacks[KNIGHT] | blacks[BISHOP] | blacks[ROOK] | blacks[QUEEN] | blacks[KING];
        occupied = whitePieces | blackPieces;

        turn = (activeColor == "w");
        wOO = castlingRights.find('K') != std::string::npos;
        wOOO = castlingRights.find('Q') != std::string::npos;
        bOO = castlingRights.find('k') != std::string::npos;
        bOOO = castlingRights.find('q') != std::string::npos;

        if (enPassant != "-") {
            int file = enPassant[0] - 'a';
            int rank = enPassant[1] - '1';
            enPassantTarget = 1ULL << (rank * 8 + file);
        }
        else {
            enPassantTarget = 0x0ULL;
        }

        ply50MoveRule = std::stoi(halfmoveClock);
        plyCount = (std::stoi(fullmoveNumber) - 1) * 2 + (turn ? 0 : 1);

        unsigned long lsb;
        if (turn) {
            Bitboard rqChecks = RookMagicBitboards[wKingPos][((occupied & RawRookAttacks[wKingPos]) * RookMagics[wKingPos] >> RookShifts[wKingPos])] & (blacks[ROOK] | blacks[QUEEN]);
            Bitboard bqChecks = BishopMagicBitboards[wKingPos][((occupied & RawBishopAttacks[wKingPos]) * BishopMagics[wKingPos] >> BishopShifts[wKingPos])] & (blacks[BISHOP] | blacks[QUEEN]);
            Bitboard kChecks = KnightBitboards[wKingPos] & blacks[KNIGHT];
            Bitboard pChecks = (((whites[KING] << 7) & not_hFile) | ((whites[KING] << 9) & not_aFile)) & blacks[PAWN];
            checksFrom = rqChecks | bqChecks | kChecks | pChecks;
        }
        else {
            Bitboard rqChecks = RookMagicBitboards[bKingPos][((occupied & RawRookAttacks[bKingPos]) * RookMagics[bKingPos] >> RookShifts[bKingPos])] & (whites[ROOK] | whites[QUEEN]);
            Bitboard bqChecks = BishopMagicBitboards[bKingPos][((occupied & RawBishopAttacks[bKingPos]) * BishopMagics[bKingPos] >> BishopShifts[bKingPos])] & (whites[BISHOP] | whites[QUEEN]);
            Bitboard kChecks = KnightBitboards[bKingPos] & whites[KNIGHT];
            Bitboard pChecks = (((blacks[KING] >> 7) & not_aFile) | ((blacks[KING] >> 9) & not_hFile)) & whites[PAWN];
            checksFrom = rqChecks | bqChecks | kChecks | pChecks;
        }
        positionHistory.push_back(computeZobristKey());

        return true; 
    }

    /*GameBoard::GameBoard(const std::string& FEN) {
        int wKingPos, bKingPos;
        whites[PAWN] = whites[KNIGHT] = whites[BISHOP] = whites[ROOK] = whites[QUEEN] = whites[KING] = blacks[PAWN] = blacks[KNIGHT] = blacks[BISHOP] = blacks[ROOK] = blacks[QUEEN] = blacks[KING] = whitePieces = blackPieces = occupied = 0;
        for (int i = 0; i < 64; i++) {
            whitePieceArray[i] = EMPTY;
            blackPieceArray[i] = EMPTY;
        }
        int index = 0;
        int row = 7;
        int col = 0;
        while (FEN[index] != ' ') {
            if (FEN[index] == '/') {
                col = 0; row--;
            }
            else if (isdigit(FEN[index])) {
                col += FEN[index] - '0';
            }
            else {
                Bitboard bb = bit << (col + row * 8);
                switch (FEN[index])
                {
                case 'P': {
                    whitePieceArray[row * 8 + col] = PAWN;
                    whites[PAWN] |= bb;
                    break;
                }
                case 'N': {
                    whitePieceArray[row * 8 + col] = KNIGHT;
                    whites[KNIGHT] |= bb;
                    break;
                }
                case 'B': {
                    whitePieceArray[row * 8 + col] = BISHOP;
                    whites[BISHOP] |= bb;
                    break;
                }
                case 'R': {
                    whitePieceArray[row * 8 + col] = ROOK;
                    whites[ROOK] |= bb;
                    break;
                }
                case 'Q': {
                    whitePieceArray[row * 8 + col] = QUEEN;
                    whites[QUEEN] |= bb;
                    break;
                }
                case 'K': {
                    whitePieceArray[row * 8 + col] = KING;
                    whites[KING] |= bb;
                    wKingPos = row * 8 + col;
                    break;
                }
                case 'p': {
                    blackPieceArray[row * 8 + col] = PAWN;
                    blacks[PAWN] |= bb;
                    break;
                }
                case 'n': {
                    blackPieceArray[row * 8 + col] = KNIGHT;
                    blacks[KNIGHT] |= bb;
                    break;
                }
                case 'b': {
                    blackPieceArray[row * 8 + col] = BISHOP;
                    blacks[BISHOP] |= bb;
                    break;
                }
                case 'r': {
                    blackPieceArray[row * 8 + col] = ROOK;
                    blacks[ROOK] |= bb;
                    break;
                }
                case 'q': {
                    blackPieceArray[row * 8 + col] = QUEEN;
                    blacks[QUEEN] |= bb;
                    break;
                }
                case 'k': {
                    blackPieceArray[row * 8 + col] = KING;
                    blacks[KING] |= bb;
                    bKingPos = row * 8 + col;
                    break;
                }
                default: break;
                }
                col++;
            }
            index++;
        }
        whitePieces = whites[PAWN] | whites[KNIGHT] | whites[BISHOP] | whites[ROOK] | whites[QUEEN] | whites[KING];
        blackPieces = blacks[PAWN] | blacks[KNIGHT] | blacks[BISHOP] | blacks[ROOK] | blacks[QUEEN] | blacks[KING];
        occupied = whitePieces | blackPieces;
        index++;
        switch (FEN[index])
        {
        case 'w': turn = true; break;
        case 'b': turn = false; break;
        default: break;
        }
        index += 2;
        wOO = wOOO = bOO = bOOO = false;
        while (FEN[index] != ' ') {
            switch (FEN[index])
            {
            case 'k': bOO = true; break;
            case 'q': bOOO = true; break;
            case 'K': wOO = true; break;
            case 'Q': wOOO = true; break;
            default:
                break;
            }
            index++;
        }
        index++;
        if (FEN[index] != '-') {
            int file = FEN[index] - 'a';
            index++;
            int rank = FEN[index] - '1';
            enPassantTarget = 1ULL << (rank * 8 + file);
        }
        else enPassantTarget = 0x0ULL;
        index += 2;
        int hm2 = FEN[index] - '0';
        int hm1 = 0;
        if (isdigit(FEN[index + 1])) {
            hm1 = hm2;
            index++;
            int hm2 = FEN[index] - '0';
        }
        ply50MoveRule = hm1 * 10 + hm2;
        index += 2;
        hm2 = FEN[index] - '0';
        hm1 = 0;
        if (isdigit(FEN[index + 1])) {
            hm1 = hm2;
            index++;
            int hm2 = FEN[index] - '0';
        }
        plyCount = (hm1 * 10 + hm2) * 2;
        if (!turn) plyCount++;
        unsigned long lsb;
        if (turn) {
            Bitboard rqChecks = RookMagicBitboards[wKingPos][((occupied & RawRookAttacks[wKingPos]) * RookMagics[wKingPos] >> RookShifts[wKingPos])] & (blacks[ROOK] | blacks[QUEEN]);
            Bitboard bqChecks = BishopMagicBitboards[wKingPos][((occupied & RawBishopAttacks[wKingPos]) * BishopMagics[wKingPos] >> BishopShifts[wKingPos])] & (blacks[BISHOP] | blacks[QUEEN]);
            Bitboard kChecks = KnightBitboards[wKingPos] & blacks[KNIGHT];
            Bitboard pChecks = (((whites[KING] << 7) & not_hFile) | ((whites[KING] << 9) & not_aFile)) & blacks[PAWN];
            checksFrom = rqChecks | bqChecks | kChecks | pChecks;
        }
        else {
            Bitboard rqChecks = RookMagicBitboards[bKingPos][((occupied & RawRookAttacks[bKingPos]) * RookMagics[bKingPos] >> RookShifts[bKingPos])] & (whites[ROOK] | whites[QUEEN]);
            Bitboard bqChecks = BishopMagicBitboards[bKingPos][((occupied & RawBishopAttacks[bKingPos]) * BishopMagics[bKingPos] >> BishopShifts[bKingPos])] & (whites[BISHOP] | whites[QUEEN]);
            Bitboard kChecks = KnightBitboards[bKingPos] & whites[KNIGHT];
            Bitboard pChecks = (((blacks[KING] >> 7) & not_aFile) | ((blacks[KING] >> 9) & not_hFile)) & whites[PAWN];
            checksFrom = rqChecks | bqChecks | kChecks | pChecks;
        }
        positionHistory.push_back(computeZobristKey());
    }*/

    Bitboard GameBoard::wpPushes() {
        return ((whites[PAWN] & not_sevenRank) << 8) & ~occupied;
    }
    Bitboard GameBoard::wpDoublePushes() {
        Bitboard bb = whites[PAWN] & secondRank;
        bb = (bb << 8) & ~occupied;
        bb = (bb << 8) & ~occupied;
        return bb;
    }
    Bitboard GameBoard::wpRightCaptures() {
        return ((whites[PAWN] & not_hFile & not_sevenRank) << 9) & blackPieces;
    }
    Bitboard GameBoard::wpLeftCaptures() {
        return ((whites[PAWN] & not_aFile & not_sevenRank) << 7) & blackPieces;
    }
    Bitboard GameBoard::wpPromotions() {
        return ((whites[PAWN] & sevenRank) << 8) & ~occupied;
    }
    Bitboard GameBoard::wpRightPromotions() {
        return ((whites[PAWN] & sevenRank & not_hFile) << 9) & blackPieces;
    }
    Bitboard GameBoard::wpLeftPromotions() {
        return ((whites[PAWN] & sevenRank & not_aFile) << 7) & blackPieces;
    }

    Bitboard GameBoard::bpPushes() {
        return ((blacks[PAWN] & not_secondRank) >> 8) & ~occupied;
    }
    Bitboard GameBoard::bpDoublePushes() {
        Bitboard bb = blacks[PAWN] & sevenRank;
        bb = (bb >> 8) & ~occupied;
        bb = (bb >> 8) & ~occupied;
        return bb;
    }
    Bitboard GameBoard::bpRightCaptures() {
        return ((blacks[PAWN] & not_aFile & not_secondRank) >> 9) & whitePieces;
    }
    Bitboard GameBoard::bpLeftCaptures() {
        return ((blacks[PAWN] & not_hFile & not_secondRank) >> 7) & whitePieces;
    }
    Bitboard GameBoard::bpPromotions() {
        return ((blacks[PAWN] & secondRank) >> 8) & ~occupied;
    }
    Bitboard GameBoard::bpRightPromotions() {
        return ((blacks[PAWN] & secondRank & not_aFile) >> 9) & whitePieces;
    }
    Bitboard GameBoard::bpLeftPromotions() {
        return ((blacks[PAWN] & secondRank & not_hFile) >> 7) & whitePieces;
    }

    Bitboard GameBoard::singleWhitePawnAttacks(unsigned long index) {
        return ((bit << index + 7) & not_hFile) | ((bit << index + 9) & not_aFile);
    }
    Bitboard GameBoard::singleBlackPawnAttacks(unsigned long index) {
        return ((bit << index - 7) & not_aFile) | ((bit << index - 9) & not_hFile);
    }

    Bitboard GameBoard::wDiscoveredFrom(unsigned long moved) {
        if (!((bit << moved) & (whiteKingBishopMoves | whiteKingRookMoves))) return 0x0ULL;
        return (((RookMagicBitboards[wKingPos][(occupied & RawRookAttacks[wKingPos]) * RookMagics[wKingPos] >> RookShifts[wKingPos]]) & (blacks[ROOK] | blacks[QUEEN]))
            | ((BishopMagicBitboards[wKingPos][(occupied & RawBishopAttacks[wKingPos]) * BishopMagics[wKingPos] >> BishopShifts[wKingPos]]) & (blacks[BISHOP] | blacks[QUEEN])))
            & ~checksFrom;
    }
    Bitboard GameBoard::bDiscoveredFrom(unsigned long moved) {
        if (!((bit << moved) & (blackKingBishopMoves | blackKingRookMoves))) return 0x0ULL;
        return ((RookMagicBitboards[bKingPos][(occupied & RawRookAttacks[bKingPos]) * RookMagics[bKingPos] >> RookShifts[bKingPos]]) & (whites[ROOK] | whites[QUEEN])
            | (BishopMagicBitboards[bKingPos][(occupied & RawBishopAttacks[bKingPos]) * BishopMagics[bKingPos] >> BishopShifts[bKingPos]]) & (whites[BISHOP] | whites[QUEEN]))
            & ~checksFrom;
    }

    void GameBoard::printBoard() const {
        std::cout << "\n";
        for (int r = 7; r >= 0; r--) {
            std::cout << r + 1 << "\t";
            for (int c = 0; c < 8; c++) {
                if (whitePieceArray[r * 8 + c] != EMPTY) {
                    switch (whitePieceArray[r * 8 + c])
                    {
                    case PAWN: std::cout << "P"; break;
                    case KNIGHT: std::cout << "N"; break;
                    case BISHOP: std::cout << "B"; break;
                    case ROOK: std::cout << "R"; break;
                    case QUEEN: std::cout << "Q"; break;
                    case KING: std::cout << "K"; break;
                    default:
                        break;
                    }
                    //std::cout << static_cast<int>(whitePieceArray[r * 8 + c]);
                }
                if (blackPieceArray[r * 8 + c] != EMPTY) {
                    switch (blackPieceArray[r * 8 + c])
                    {
                    case PAWN: std::cout << "p"; break;
                    case KNIGHT: std::cout << "n"; break;
                    case BISHOP: std::cout << "b"; break;
                    case ROOK: std::cout << "r"; break;
                    case QUEEN: std::cout << "q"; break;
                    case KING: std::cout << "k"; break;
                    default:
                        break;
                    }
                    //std::cout << static_cast<int>(blackPieceArray[r * 8 + c]);
                }
                if (whitePieceArray[r * 8 + c] == EMPTY && blackPieceArray[r * 8 + c] == EMPTY) {
                    std::cout << ".";
                }
                std::cout << "   ";
            }
            std::cout << "\n\n";
        }
        std::cout << "\n\ta   b   c   d   e   f   g   h\n";
        if (checksFrom) {
            std::cout << "Checkers :";
            Bitboard checks = checksFrom;
            unsigned long lsb;
            while (checks) {
                _BitScanForward64(&lsb, checks);
                checks &= checks - 1;
                std::cout << " " << notation_from_index(lsb);
            }
            std::cout << "\n";
        }
        std::string move = turn ? "White to move" : "Black to move";
        std::cout << move << std::endl;
    }
    void GameBoard::printBoardFromBlack() const {
        std::cout << "\n";
        for (int r = 0; r < 8; r++) {
            std::cout << r + 1 << "\t";  
            for (int c = 7; c >= 0; c--) {
                if (whitePieceArray[r * 8 + c] != EMPTY) {
                    switch (whitePieceArray[r * 8 + c]) {
                    case PAWN: std::cout << "P"; break;
                    case KNIGHT: std::cout << "N"; break;
                    case BISHOP: std::cout << "B"; break;
                    case ROOK: std::cout << "R"; break;
                    case QUEEN: std::cout << "Q"; break;
                    case KING: std::cout << "K"; break;
                    default: break;
                    }
                }
                if (blackPieceArray[r * 8 + c] != EMPTY) {
                    switch (blackPieceArray[r * 8 + c]) {
                    case PAWN: std::cout << "p"; break;
                    case KNIGHT: std::cout << "n"; break;
                    case BISHOP: std::cout << "b"; break;
                    case ROOK: std::cout << "r"; break;
                    case QUEEN: std::cout << "q"; break;
                    case KING: std::cout << "k"; break;
                    default: break;
                    }
                }
                if (whitePieceArray[r * 8 + c] == EMPTY && blackPieceArray[r * 8 + c] == EMPTY) {
                    std::cout << ".";
                }
                std::cout << "   ";
            }
            std::cout << "\n\n";
        }

        std::cout << "\n\th   g   f   e   d   c   b   a\n";

        if (checksFrom) {
            std::cout << "Checkers :";
            Bitboard checks = checksFrom;
            unsigned long lsb;
            while (checks) {
                _BitScanForward64(&lsb, checks);
                checks &= checks - 1;
                std::cout << " " << notation_from_index(lsb);
            }
            std::cout << "\n";
        }

        std::string move = turn ? "White to move" : "Black to move";
        std::cout << move << std::endl;
    }
    void GameBoard::smartPrint(bool turn) const {
        if (turn) printBoard();
        else printBoardFromBlack();
    }

    uint64_t GameBoard::computeZobristKey() const {
        uint64_t key = 0;
        for (int i = 0; i < 64; ++i) {
            key ^= zobristTable[0][whitePieceArray[i]][i];
            key ^= zobristTable[0][blackPieceArray[i] + 7][i];
        }
        if (turn) key ^= zobristTurn;
        return key;
    }

    uint64_t GameBoard::reserveKey() const {
        uint64_t key = 0;
        for (int i = 0; i < 64; ++i) {
            key ^= zobristTable[1][whitePieceArray[i]][i];
            key ^= zobristTable[1][blackPieceArray[i] + 7][i];
        }
        if (turn) key ^= zobristTurn;
        return key;
    }

    Bitboard GameBoard::whiteAttacks() {
        Bitboard attacks = 0x0ULL;
        attacks |= (whites[PAWN] & not_aFile) << 7;
        attacks |= (whites[PAWN] & not_hFile) << 9;
        unsigned long lsb;
        Bitboard currentPieces = whites[KNIGHT];
        while (currentPieces) {
            _BitScanForward64(&lsb, currentPieces);
            attacks |= KnightBitboards[lsb];
            currentPieces &= currentPieces - 1;
        }
        currentPieces = whites[BISHOP] | whites[QUEEN];
        while (currentPieces) {
            _BitScanForward64(&lsb, currentPieces);
            attacks |= BishopMagicBitboards[lsb][(occupied & RawBishopAttacks[lsb]) * BishopMagics[lsb] >> BishopShifts[lsb]];
            currentPieces &= currentPieces - 1;
        }
        currentPieces = whites[ROOK] | whites[QUEEN];
        while (currentPieces) {
            _BitScanForward64(&lsb, currentPieces);
            attacks |= RookMagicBitboards[lsb][(occupied & RawRookAttacks[lsb]) * RookMagics[lsb] >> RookShifts[lsb]];
            currentPieces &= currentPieces - 1;
        }
        attacks |= KingBitboards[wKingPos];
        return attacks;
    }
    Bitboard GameBoard::blackAttacks() {
        Bitboard attacks = 0x0ULL;
        attacks |= (blacks[PAWN] & not_hFile) >> 7;
        attacks |= (blacks[PAWN] & not_aFile) >> 9;
        unsigned long lsb;
        Bitboard currentPieces = blacks[KNIGHT];
        while (currentPieces) {
            _BitScanForward64(&lsb, currentPieces);
            attacks |= KnightBitboards[lsb];
            currentPieces &= currentPieces - 1;
        }
        currentPieces = blacks[BISHOP] | blacks[QUEEN];
        while (currentPieces) {
            _BitScanForward64(&lsb, currentPieces);
            attacks |= BishopMagicBitboards[lsb][(occupied & RawBishopAttacks[lsb]) * BishopMagics[lsb] >> BishopShifts[lsb]];
            currentPieces &= currentPieces - 1;
        }
        currentPieces = blacks[ROOK] | blacks[QUEEN];
        while (currentPieces) {
            _BitScanForward64(&lsb, currentPieces);
            attacks |= RookMagicBitboards[lsb][(occupied & RawRookAttacks[lsb]) * RookMagics[lsb] >> RookShifts[lsb]];
            currentPieces &= currentPieces - 1;
        }
        attacks |= KingBitboards[bKingPos];
        return attacks;
    }

    // mask is a block check bath, or all squares if not in check
    void GameBoard::generateWhitePromotions(std::vector<Move>& moves, Bitboard mask) {
        unsigned long lsb;
        Bitboard simpleCheck;
        Bitboard discoveredCheck;
        Bitboard currentMoves;

        currentMoves = wpRightPromotions() & mask;
        while (currentMoves) {
            _BitScanForward64(&lsb, currentMoves);
            occupied &= ~(bit << (lsb - 9));
            if (!(wDiscoveredFrom(lsb - 9) & ~(bit << lsb))) {
                discoveredCheck = bDiscoveredFrom(lsb - 9);

                simpleCheck = ((BishopMagicBitboards[bKingPos][((RawBishopAttacks[bKingPos] & occupied) * BishopMagics[bKingPos]) >> BishopShifts[bKingPos]]) | (blackKingRookMoves)) & (bit << lsb);
                moves.push_back(Move(lsb, lsb - 9, blackPieceArray[lsb], QUEEN, false, false, false, simpleCheck && discoveredCheck, simpleCheck | discoveredCheck));

                simpleCheck = ((blackKingRookMoves) & (bit << lsb));
                moves.push_back(Move(lsb, lsb - 9, blackPieceArray[lsb], ROOK, false, false, false, simpleCheck && discoveredCheck, simpleCheck | discoveredCheck));

                simpleCheck = (BishopMagicBitboards[bKingPos][((RawBishopAttacks[bKingPos] & occupied) * BishopMagics[bKingPos]) >> BishopShifts[bKingPos]] & (bit << lsb));
                moves.push_back(Move(lsb, lsb - 9, blackPieceArray[lsb], BISHOP, false, false, false, simpleCheck && discoveredCheck, simpleCheck | discoveredCheck));

                simpleCheck = (KnightBitboards[bKingPos] & (bit << lsb));
                moves.push_back(Move(lsb, lsb - 9, blackPieceArray[lsb], KNIGHT, false, false, false, simpleCheck && discoveredCheck, simpleCheck | discoveredCheck));
            }
            occupied |= bit << (lsb - 9);
            currentMoves &= currentMoves - 1;
        }

        currentMoves = wpLeftPromotions() & mask;
        while (currentMoves) {
            _BitScanForward64(&lsb, currentMoves);
            occupied &= ~(bit << (lsb - 7));
            if (!(wDiscoveredFrom(lsb - 7) & ~(bit << lsb))) {
                discoveredCheck = bDiscoveredFrom(lsb - 7);

                simpleCheck = ((BishopMagicBitboards[bKingPos][((RawBishopAttacks[bKingPos] & occupied) * BishopMagics[bKingPos]) >> BishopShifts[bKingPos]]) | (blackKingRookMoves)) & (bit << lsb);
                moves.push_back(Move(lsb, lsb - 7, blackPieceArray[lsb], QUEEN, false, false, false, simpleCheck && discoveredCheck, simpleCheck | discoveredCheck));

                simpleCheck = ((blackKingRookMoves) & (bit << lsb));
                moves.push_back(Move(lsb, lsb - 7, blackPieceArray[lsb], ROOK, false, false, false, simpleCheck && discoveredCheck, simpleCheck | discoveredCheck));

                simpleCheck = (BishopMagicBitboards[bKingPos][((RawBishopAttacks[bKingPos] & occupied) * BishopMagics[bKingPos]) >> BishopShifts[bKingPos]] & (bit << lsb));
                moves.push_back(Move(lsb, lsb - 7, blackPieceArray[lsb], BISHOP, false, false, false, simpleCheck && discoveredCheck, simpleCheck | discoveredCheck));

                simpleCheck = (KnightBitboards[bKingPos] & (bit << lsb));
                moves.push_back(Move(lsb, lsb - 7, blackPieceArray[lsb], KNIGHT, false, false, false, simpleCheck && discoveredCheck, simpleCheck | discoveredCheck));
            }
            occupied |= bit << (lsb - 7);
            currentMoves &= currentMoves - 1;
        }

        currentMoves = wpPromotions() & mask;
        while (currentMoves) {
            _BitScanForward64(&lsb, currentMoves);
            occupied &= ~(bit << (lsb - 8));
            occupied |= bit << lsb;
            if (!wDiscoveredFrom(lsb - 8)) {

                discoveredCheck = bDiscoveredFrom(lsb - 8);

                simpleCheck = (RookMagicBitboards[bKingPos][((RawRookAttacks[bKingPos] & occupied) * RookMagics[bKingPos]) >> RookShifts[bKingPos]] | (blackKingBishopMoves)) & (bit << lsb);
                moves.push_back(Move(lsb, lsb - 8, EMPTY, QUEEN, false, false, false, simpleCheck && discoveredCheck, simpleCheck | discoveredCheck));

                simpleCheck = RookMagicBitboards[bKingPos][((RawRookAttacks[bKingPos] & occupied) * RookMagics[bKingPos]) >> RookShifts[bKingPos]] & (bit << lsb);
                moves.push_back(Move(lsb, lsb - 8, EMPTY, ROOK, false, false, false, simpleCheck && discoveredCheck, simpleCheck | discoveredCheck));

                simpleCheck = ((blackKingBishopMoves) & (bit << lsb));
                moves.push_back(Move(lsb, lsb - 8, EMPTY, BISHOP, false, false, false, simpleCheck && discoveredCheck, simpleCheck | discoveredCheck));

                simpleCheck = (KnightBitboards[bKingPos] & (bit << lsb));
                moves.push_back(Move(lsb, lsb - 8, EMPTY, KNIGHT, false, false, false, simpleCheck && discoveredCheck, simpleCheck | discoveredCheck));
            }
            occupied &= ~(bit << lsb);
            occupied |= bit << (lsb - 8);
            currentMoves &= currentMoves - 1;
        }
    }
    void GameBoard::generateBlackPromotions(std::vector<Move>& moves, Bitboard mask) {
        unsigned long lsb;
        Bitboard discoveredCheck;
        Bitboard simpleCheck;
        Bitboard currentMoves;

        currentMoves = bpRightPromotions() & mask;
        while (currentMoves) {
            _BitScanForward64(&lsb, currentMoves);
            occupied &= ~(bit << (lsb + 9));
            if (!(bDiscoveredFrom(lsb + 9) & ~(bit << lsb))) {
                discoveredCheck = wDiscoveredFrom(lsb + 9);

                simpleCheck = ((BishopMagicBitboards[wKingPos][(occupied & RawBishopAttacks[wKingPos]) * BishopMagics[wKingPos] >> BishopShifts[wKingPos]]) | (whiteKingRookMoves)) & (bit << lsb);
                moves.push_back(Move(lsb, lsb + 9, whitePieceArray[lsb], QUEEN, false, false, false, simpleCheck && discoveredCheck, simpleCheck | discoveredCheck));

                simpleCheck = ((whiteKingRookMoves) & (bit << lsb));
                moves.push_back(Move(lsb, lsb + 9, whitePieceArray[lsb], ROOK, false, false, false, simpleCheck && discoveredCheck, simpleCheck | discoveredCheck));

                simpleCheck = (BishopMagicBitboards[wKingPos][(occupied & RawBishopAttacks[wKingPos]) * BishopMagics[wKingPos] >> BishopShifts[wKingPos]]) & (bit << lsb);
                moves.push_back(Move(lsb, lsb + 9, whitePieceArray[lsb], BISHOP, false, false, false, simpleCheck && discoveredCheck, simpleCheck | discoveredCheck));

                simpleCheck = KnightBitboards[wKingPos] & (bit << lsb);
                moves.push_back(Move(lsb, lsb + 9, whitePieceArray[lsb], KNIGHT, false, false, false, simpleCheck && discoveredCheck, simpleCheck | discoveredCheck));
            }
            occupied |= bit << (lsb + 9);
            currentMoves &= currentMoves - 1;
        }

        currentMoves = bpLeftPromotions() & mask;
        while (currentMoves) {
            _BitScanForward64(&lsb, currentMoves);
            occupied &= ~(bit << (lsb + 7));
            if (!(bDiscoveredFrom(lsb + 7) & ~(bit << lsb))) {
                discoveredCheck = wDiscoveredFrom(lsb + 7);

                simpleCheck = ((BishopMagicBitboards[wKingPos][(occupied & RawBishopAttacks[wKingPos]) * BishopMagics[wKingPos] >> BishopShifts[wKingPos]]) | (whiteKingRookMoves)) & (bit << lsb);
                moves.push_back(Move(lsb, lsb + 7, whitePieceArray[lsb], QUEEN, false, false, false, simpleCheck && discoveredCheck, simpleCheck | discoveredCheck));

                simpleCheck = ((whiteKingRookMoves) & (bit << lsb));
                moves.push_back(Move(lsb, lsb + 7, whitePieceArray[lsb], ROOK, false, false, false, simpleCheck && discoveredCheck, simpleCheck | discoveredCheck));

                simpleCheck = (BishopMagicBitboards[wKingPos][(occupied & RawBishopAttacks[wKingPos]) * BishopMagics[wKingPos] >> BishopShifts[wKingPos]]) & (bit << lsb);
                moves.push_back(Move(lsb, lsb + 7, whitePieceArray[lsb], BISHOP, false, false, false, simpleCheck && discoveredCheck, simpleCheck | discoveredCheck));

                simpleCheck = KnightBitboards[wKingPos] & (bit << lsb);
                moves.push_back(Move(lsb, lsb + 7, whitePieceArray[lsb], KNIGHT, false, false, false, simpleCheck && discoveredCheck, simpleCheck | discoveredCheck));
            }
            occupied |= bit << (lsb + 7);
            currentMoves &= currentMoves - 1;
        }

        currentMoves = bpPromotions() & mask;
        while (currentMoves) {
            _BitScanForward64(&lsb, currentMoves);
            occupied &= ~(bit << (lsb + 8));
            occupied |= bit << lsb;
            if (!bDiscoveredFrom(lsb + 8)) {
                discoveredCheck = wDiscoveredFrom(lsb + 8);

                simpleCheck = ((RookMagicBitboards[wKingPos][(occupied & RawRookAttacks[wKingPos]) * RookMagics[wKingPos] >> RookShifts[wKingPos]]) | (whiteKingBishopMoves)) & (bit << lsb);
                moves.push_back(Move(lsb, lsb + 8, EMPTY, QUEEN, false, false, false, simpleCheck && discoveredCheck, simpleCheck | discoveredCheck));

                simpleCheck = ((RookMagicBitboards[wKingPos][(occupied & RawRookAttacks[wKingPos]) * RookMagics[wKingPos] >> RookShifts[wKingPos]]) & (bit << lsb));
                moves.push_back(Move(lsb, lsb + 8, EMPTY, ROOK, false, false, false, simpleCheck && discoveredCheck, simpleCheck | discoveredCheck));

                simpleCheck = ((whiteKingBishopMoves) & (bit << lsb));
                moves.push_back(Move(lsb, lsb + 8, EMPTY, BISHOP, false, false, false, simpleCheck && discoveredCheck, simpleCheck | discoveredCheck));

                simpleCheck = (KnightBitboards[wKingPos] & (bit << lsb));
                moves.push_back(Move(lsb, lsb + 8, EMPTY, KNIGHT, false, false, false, simpleCheck && discoveredCheck, simpleCheck | discoveredCheck));
            }
            occupied &= ~(bit << lsb);
            occupied |= bit << (lsb + 8);
            currentMoves &= currentMoves - 1;
        }

    }

    void GameBoard::generateWhiteMoves(std::vector<Move>& moves, Bitboard mask) {
        unsigned long lsb;
        Bitboard simpleCheck;
        Bitboard discoveredCheck;
        Bitboard currentMoves, currentPieces;
        Bitboard blockers;
        unsigned long pieceIndex;

        currentMoves = wpRightCaptures() & mask;
        while (currentMoves) {
            _BitScanForward64(&lsb, currentMoves);
            occupied &= ~(bit << (lsb - 9));
            if (!(wDiscoveredFrom(lsb - 9) & ~(bit << lsb))) {
                simpleCheck = singleBlackPawnAttacks(bKingPos) & (bit << lsb);
                discoveredCheck = bDiscoveredFrom(lsb - 9);
                moves.push_back(Move(lsb, lsb - 9, blackPieceArray[lsb], EMPTY, false, false, false, simpleCheck && discoveredCheck, simpleCheck | discoveredCheck));
            }
            occupied |= bit << (lsb - 9);
            currentMoves &= currentMoves - 1;
        }

        currentMoves = wpLeftCaptures() & mask;
        while (currentMoves) {
            _BitScanForward64(&lsb, currentMoves);
            occupied &= ~(bit << (lsb - 7));
            if (!(wDiscoveredFrom(lsb - 7) & ~(bit << lsb))) {
                simpleCheck = singleBlackPawnAttacks(bKingPos) & (bit << lsb);
                discoveredCheck = bDiscoveredFrom(lsb - 7);
                moves.push_back(Move(lsb, lsb - 7, blackPieceArray[lsb], EMPTY, false, false, false, simpleCheck && discoveredCheck, simpleCheck | discoveredCheck));
            }
            occupied |= bit << (lsb - 7);
            currentMoves &= currentMoves - 1;
        }

        currentPieces = whites[QUEEN];
        while (currentPieces) {
            _BitScanForward64(&pieceIndex, currentPieces);
            blockers = occupied & RawRookAttacks[pieceIndex];
            currentMoves = RookMagicBitboards[pieceIndex][blockers * RookMagics[pieceIndex] >> RookShifts[pieceIndex]];
            blockers = occupied & RawBishopAttacks[pieceIndex];
            currentMoves |= BishopMagicBitboards[pieceIndex][blockers * BishopMagics[pieceIndex] >> BishopShifts[pieceIndex]];
            currentMoves &= ~whitePieces;
            currentMoves &= mask;
            occupied &= ~(bit << pieceIndex);
            discoveredCheck = wDiscoveredFrom(pieceIndex);
            if (discoveredCheck) {
                _BitScanForward64(&lsb, discoveredCheck);
                currentMoves &= BlockCheckPath[wKingPos][lsb];
            }
            occupied |= (bit << pieceIndex);
            while (currentMoves) {
                _BitScanForward64(&lsb, currentMoves);
                simpleCheck = (blackKingRookMoves | blackKingBishopMoves) & (bit << lsb);
                moves.push_back(Move(lsb, pieceIndex, blackPieceArray[lsb], EMPTY, false, false, false, false, simpleCheck));
                currentMoves &= currentMoves - 1;
            }
            currentPieces &= currentPieces - 1;
        }

        currentPieces = whites[ROOK];
        while (currentPieces) {
            _BitScanForward64(&pieceIndex, currentPieces);
            blockers = occupied & RawRookAttacks[pieceIndex];
            currentMoves = RookMagicBitboards[pieceIndex][blockers * RookMagics[pieceIndex] >> RookShifts[pieceIndex]] & ~whitePieces;
            currentMoves &= mask;
            occupied &= ~(bit << pieceIndex);
            discoveredCheck = wDiscoveredFrom(pieceIndex);
            if (discoveredCheck) {
                _BitScanForward64(&lsb, discoveredCheck);
                currentMoves &= BlockCheckPath[wKingPos][lsb];
            }
            while (currentMoves) {
                _BitScanForward64(&lsb, currentMoves);
                simpleCheck = blackKingRookMoves & (bit << lsb);
                discoveredCheck = bDiscoveredFrom(pieceIndex);
                moves.push_back(Move(lsb, pieceIndex, blackPieceArray[lsb], EMPTY, false, false, false, simpleCheck && discoveredCheck, simpleCheck | discoveredCheck));
                currentMoves &= currentMoves - 1;
            }
            occupied |= (bit << pieceIndex);
            currentPieces &= currentPieces - 1;
        }

        currentPieces = whites[BISHOP];
        while (currentPieces) {
            _BitScanForward64(&pieceIndex, currentPieces);
            blockers = occupied & RawBishopAttacks[pieceIndex];
            currentMoves = BishopMagicBitboards[pieceIndex][blockers * BishopMagics[pieceIndex] >> BishopShifts[pieceIndex]] & ~whitePieces;
            currentMoves &= mask;
            occupied &= ~(bit << pieceIndex);
            discoveredCheck = wDiscoveredFrom(pieceIndex);
            if (discoveredCheck) {
                _BitScanForward64(&lsb, discoveredCheck);
                currentMoves &= BlockCheckPath[wKingPos][lsb];
            }
            while (currentMoves) {
                _BitScanForward64(&lsb, currentMoves);
                simpleCheck = blackKingBishopMoves & (bit << lsb);
                discoveredCheck = bDiscoveredFrom(pieceIndex);
                moves.push_back(Move(lsb, pieceIndex, blackPieceArray[lsb], EMPTY, false, false, false, simpleCheck && discoveredCheck, simpleCheck | discoveredCheck));
                currentMoves &= currentMoves - 1;
            }
            occupied |= (bit << pieceIndex);
            currentPieces &= currentPieces - 1;
        }

        currentPieces = whites[KNIGHT];
        while (currentPieces) {
            _BitScanForward64(&pieceIndex, currentPieces);
            currentMoves = (KnightBitboards[pieceIndex] & ~whitePieces) & mask;
            occupied &= ~(bit << pieceIndex);
            if (!wDiscoveredFrom(pieceIndex)) {
                while (currentMoves) {
                    _BitScanForward64(&lsb, currentMoves);
                    simpleCheck = KnightBitboards[bKingPos] & (bit << lsb);
                    discoveredCheck = bDiscoveredFrom(pieceIndex);
                    moves.push_back(Move(lsb, pieceIndex, blackPieceArray[lsb], EMPTY, false, false, false, simpleCheck && discoveredCheck, simpleCheck | discoveredCheck));
                    currentMoves &= currentMoves - 1;
                }
            }
            occupied |= (bit << pieceIndex);
            currentPieces &= currentPieces - 1;
        }

        if ((enPassantTarget >> 8) & mask) {
            if ((enPassantTarget >> 7) & whites[PAWN] & not_aFile) {
                _BitScanForward64(&lsb, enPassantTarget);
                occupied &= ~((enPassantTarget >> 7) | (enPassantTarget >> 8));
                occupied |= enPassantTarget;
                if (!wDiscoveredFrom(lsb - 7) && !wDiscoveredFrom(lsb - 8)) {
                    simpleCheck = singleBlackPawnAttacks(bKingPos) & (bit << lsb);
                    discoveredCheck = bDiscoveredFrom(lsb - 7) | bDiscoveredFrom(lsb - 8);
                    bool isDoubleDiscoveredCheck = bDiscoveredFrom(lsb - 7) && bDiscoveredFrom(lsb - 8);
                    moves.push_back(Move(lsb, lsb - 7, PAWN, EMPTY, false, false, true, (simpleCheck&& discoveredCheck) || isDoubleDiscoveredCheck, simpleCheck | discoveredCheck));
                }
                occupied |= (enPassantTarget >> 7) | (enPassantTarget >> 8);
                occupied &= ~enPassantTarget;
            }
            if ((enPassantTarget >> 9) & whites[PAWN] & not_hFile) {
                _BitScanForward64(&lsb, enPassantTarget);
                occupied &= ~((enPassantTarget >> 9) | (enPassantTarget >> 8));
                occupied |= enPassantTarget;
                if (!wDiscoveredFrom(lsb - 9) && !wDiscoveredFrom(lsb - 8)) {
                    simpleCheck = singleBlackPawnAttacks(bKingPos) & (bit << lsb);
                    discoveredCheck = bDiscoveredFrom(lsb - 9) | bDiscoveredFrom(lsb - 8);
                    bool isDoubleDiscoveredCheck = bDiscoveredFrom(lsb - 9) && bDiscoveredFrom(lsb - 8);
                    moves.push_back(Move(lsb, lsb - 9, PAWN, EMPTY, false, false, true, (simpleCheck&& discoveredCheck) || isDoubleDiscoveredCheck, simpleCheck | discoveredCheck));
                }
                occupied |= (enPassantTarget >> 9) | (enPassantTarget >> 8);
                occupied &= ~enPassantTarget;
            }
        }

        currentMoves = wpPushes() & mask;
        while (currentMoves) {
            _BitScanForward64(&lsb, currentMoves);
            occupied &= ~(bit << (lsb - 8));
            occupied |= bit << lsb;
            if (!wDiscoveredFrom(lsb - 8)) {
                simpleCheck = singleBlackPawnAttacks(bKingPos) & (bit << lsb);
                discoveredCheck = bDiscoveredFrom(lsb - 8);
                moves.push_back(Move(lsb, lsb - 8, EMPTY, EMPTY, false, false, false, simpleCheck && discoveredCheck, simpleCheck | discoveredCheck));
            }
            occupied &= ~(bit << lsb);
            occupied |= bit << (lsb - 8);
            currentMoves &= currentMoves - 1;
        }

        currentMoves = wpDoublePushes() & mask;
        while (currentMoves) {
            _BitScanForward64(&lsb, currentMoves);
            occupied &= ~(bit << (lsb - 16));
            occupied |= bit << lsb;
            if (!wDiscoveredFrom(lsb - 16)) {
                simpleCheck = singleBlackPawnAttacks(bKingPos) & (bit << lsb);
                discoveredCheck = bDiscoveredFrom(lsb - 16);
                moves.push_back(Move(lsb, lsb - 16, EMPTY, EMPTY, false, true, false, simpleCheck && discoveredCheck, simpleCheck | discoveredCheck));
            }
            occupied &= ~(bit << lsb);
            occupied |= bit << (lsb - 16);
            currentMoves &= currentMoves - 1;
        }

    }
    void GameBoard::generateBlackMoves(std::vector<Move>& moves, Bitboard mask) {
        unsigned long lsb;
        Bitboard discoveredCheck;
        Bitboard simpleCheck;
        Bitboard currentMoves, currentPieces;
        Bitboard blockers;
        unsigned long pieceIndex;

        currentMoves = bpRightCaptures() & mask;
        while (currentMoves) {
            _BitScanForward64(&lsb, currentMoves);
            occupied &= ~(bit << (lsb + 9));
            if (!(bDiscoveredFrom(lsb + 9) & ~(bit << lsb))) {
                simpleCheck = singleWhitePawnAttacks(wKingPos) & (bit << lsb);
                discoveredCheck = wDiscoveredFrom(lsb + 9);
                moves.push_back(Move(lsb, lsb + 9, whitePieceArray[lsb], EMPTY, false, false, false, simpleCheck && discoveredCheck, simpleCheck | discoveredCheck));
            }
            occupied |= bit << (lsb + 9);
            currentMoves &= currentMoves - 1;
        }

        currentMoves = bpLeftCaptures() & mask;
        while (currentMoves) {
            _BitScanForward64(&lsb, currentMoves);
            occupied &= ~(bit << (lsb + 7));
            if (!(bDiscoveredFrom(lsb + 7) & ~(bit << lsb))) {
                simpleCheck = singleWhitePawnAttacks(wKingPos) & (bit << lsb);
                discoveredCheck = wDiscoveredFrom(lsb + 7);
                moves.push_back(Move(lsb, lsb + 7, whitePieceArray[lsb], EMPTY, false, false, false, simpleCheck && discoveredCheck, simpleCheck | discoveredCheck));
            }
            occupied |= bit << (lsb + 7);
            currentMoves &= currentMoves - 1;
        }

        currentPieces = blacks[QUEEN];
        while (currentPieces) {
            _BitScanForward64(&pieceIndex, currentPieces);
            blockers = occupied & RawRookAttacks[pieceIndex];
            currentMoves = RookMagicBitboards[pieceIndex][blockers * RookMagics[pieceIndex] >> RookShifts[pieceIndex]];
            blockers = occupied & RawBishopAttacks[pieceIndex];
            currentMoves |= BishopMagicBitboards[pieceIndex][blockers * BishopMagics[pieceIndex] >> BishopShifts[pieceIndex]];
            currentMoves &= ~blackPieces;
            currentMoves &= mask;
            occupied &= ~(bit << pieceIndex);
            discoveredCheck = bDiscoveredFrom(pieceIndex);
            if (discoveredCheck) {
                _BitScanForward64(&lsb, discoveredCheck);
                currentMoves &= BlockCheckPath[bKingPos][lsb];
            }
            occupied |= (bit << pieceIndex);
            while (currentMoves) {
                _BitScanForward64(&lsb, currentMoves);
                simpleCheck = (whiteKingBishopMoves | whiteKingRookMoves) & (bit << lsb);
                moves.push_back(Move(lsb, pieceIndex, whitePieceArray[lsb], EMPTY, false, false, false, false, simpleCheck));
                currentMoves &= currentMoves - 1;
            }
            currentPieces &= currentPieces - 1;
        }

        currentPieces = blacks[ROOK];
        while (currentPieces) {
            _BitScanForward64(&pieceIndex, currentPieces);
            blockers = occupied & RawRookAttacks[pieceIndex];
            currentMoves = RookMagicBitboards[pieceIndex][blockers * RookMagics[pieceIndex] >> RookShifts[pieceIndex]] & ~blackPieces;
            currentMoves &= mask;
            occupied &= ~(bit << pieceIndex);
            discoveredCheck = bDiscoveredFrom(pieceIndex);
            if (discoveredCheck) {
                _BitScanForward64(&lsb, discoveredCheck);
                currentMoves &= BlockCheckPath[bKingPos][lsb];
            }
            while (currentMoves) {
                _BitScanForward64(&lsb, currentMoves);
                simpleCheck = whiteKingRookMoves & (bit << lsb);
                discoveredCheck = wDiscoveredFrom(pieceIndex);
                moves.push_back(Move(lsb, pieceIndex, whitePieceArray[lsb], EMPTY, false, false, false, simpleCheck && discoveredCheck, simpleCheck | discoveredCheck));
                currentMoves &= currentMoves - 1;
            }
            occupied |= (bit << pieceIndex);
            currentPieces &= currentPieces - 1;
        }

        currentPieces = blacks[KNIGHT];
        while (currentPieces) {
            _BitScanForward64(&pieceIndex, currentPieces);
            currentMoves = KnightBitboards[pieceIndex] & ~blackPieces;
            currentMoves &= mask;
            occupied &= ~(bit << pieceIndex);
            if (!bDiscoveredFrom(pieceIndex)) {
                while (currentMoves) {
                    _BitScanForward64(&lsb, currentMoves);
                    simpleCheck = KnightBitboards[wKingPos] & (bit << lsb);
                    discoveredCheck = wDiscoveredFrom(pieceIndex);
                    moves.push_back(Move(lsb, pieceIndex, whitePieceArray[lsb], EMPTY, false, false, false, simpleCheck && discoveredCheck, simpleCheck | discoveredCheck));
                    currentMoves &= currentMoves - 1;
                }
            }
            occupied |= (bit << pieceIndex);
            currentPieces &= currentPieces - 1;
        }

        currentPieces = blacks[BISHOP];
        while (currentPieces) {
            _BitScanForward64(&pieceIndex, currentPieces);
            blockers = occupied & RawBishopAttacks[pieceIndex];
            currentMoves = BishopMagicBitboards[pieceIndex][blockers * BishopMagics[pieceIndex] >> BishopShifts[pieceIndex]] & ~blackPieces;
            currentMoves &= mask;
            occupied &= ~(bit << pieceIndex);
            discoveredCheck = bDiscoveredFrom(pieceIndex);
            if (discoveredCheck) {
                _BitScanForward64(&lsb, discoveredCheck);
                currentMoves &= BlockCheckPath[bKingPos][lsb];
            }
            while (currentMoves) {
                _BitScanForward64(&lsb, currentMoves);
                simpleCheck = whiteKingBishopMoves & (bit << lsb);
                discoveredCheck = wDiscoveredFrom(pieceIndex);
                moves.push_back(Move(lsb, pieceIndex, whitePieceArray[lsb], EMPTY, false, false, false, simpleCheck && discoveredCheck, simpleCheck | discoveredCheck));
                currentMoves &= currentMoves - 1;
            }
            occupied |= (bit << pieceIndex);
            currentPieces &= currentPieces - 1;
        }

        currentMoves = bpPushes() & mask;
        while (currentMoves) {
            _BitScanForward64(&lsb, currentMoves);
            occupied &= ~(bit << (lsb + 8));
            occupied |= bit << lsb;
            if (!bDiscoveredFrom(lsb + 8)) {
                simpleCheck = singleWhitePawnAttacks(wKingPos) & (bit << lsb);
                discoveredCheck = wDiscoveredFrom(lsb + 8);
                moves.push_back(Move(lsb, lsb + 8, EMPTY, EMPTY, false, false, false, simpleCheck && discoveredCheck, simpleCheck | discoveredCheck));
            }
            occupied &= ~(bit << lsb);
            occupied |= bit << (lsb + 8);
            currentMoves &= currentMoves - 1;
        }

        currentMoves = bpDoublePushes() & mask;
        while (currentMoves) {
            _BitScanForward64(&lsb, currentMoves);
            occupied &= ~(bit << (lsb + 16));
            occupied |= bit << lsb;
            if (!bDiscoveredFrom(lsb + 16)) {
                simpleCheck = singleWhitePawnAttacks(wKingPos) & (bit << lsb);
                discoveredCheck = wDiscoveredFrom(lsb + 16);
                moves.push_back(Move(lsb, lsb + 16, EMPTY, EMPTY, false, true, false, simpleCheck && discoveredCheck, simpleCheck | discoveredCheck));
            }
            occupied &= ~(bit << lsb);
            occupied |= bit << (lsb + 16);
            currentMoves &= currentMoves - 1;
        }

        if ((enPassantTarget << 8) & mask) {
            if ((enPassantTarget << 7) & blacks[PAWN] & not_hFile) {
                _BitScanForward64(&lsb, enPassantTarget);
                occupied &= ~((enPassantTarget << 7) | (enPassantTarget << 8));
                occupied |= enPassantTarget;
                if (!bDiscoveredFrom(lsb + 7) && !bDiscoveredFrom(lsb + 8)) {
                    simpleCheck = singleWhitePawnAttacks(wKingPos) & (bit << lsb);
                    discoveredCheck = wDiscoveredFrom(lsb + 7) | wDiscoveredFrom(lsb + 8);
                    bool isDoubleDiscoveredCheck = wDiscoveredFrom(lsb + 7) && wDiscoveredFrom(lsb + 8);
                    moves.push_back(Move(lsb, lsb + 7, PAWN, EMPTY, false, false, true, (simpleCheck && discoveredCheck) || isDoubleDiscoveredCheck, simpleCheck | discoveredCheck));
                }
                occupied |= (enPassantTarget << 7) | (enPassantTarget << 8);
                occupied &= ~enPassantTarget;
            }
            if ((enPassantTarget << 9) & blacks[PAWN] & not_aFile) {
                _BitScanForward64(&lsb, enPassantTarget);
                occupied &= ~((enPassantTarget << 9) | (enPassantTarget << 8));
                occupied |= enPassantTarget;
                if (!bDiscoveredFrom(lsb + 9) && !bDiscoveredFrom(lsb + 8)) {
                    simpleCheck = singleWhitePawnAttacks(wKingPos) & (bit << lsb);
                    discoveredCheck = wDiscoveredFrom(lsb + 9) | wDiscoveredFrom(lsb + 8);
                    bool isDoubleDiscoveredCheck = wDiscoveredFrom(lsb + 9) && wDiscoveredFrom(lsb + 8);
                    moves.push_back(Move(lsb, lsb + 9, PAWN, EMPTY, false, false, true, (simpleCheck && discoveredCheck) || isDoubleDiscoveredCheck, simpleCheck | discoveredCheck));
                }
                occupied |= (enPassantTarget << 9) | (enPassantTarget << 8);
                occupied &= ~enPassantTarget;
            }
        }
    }

    void GameBoard::generateWhiteKingMoves(std::vector<Move>& moves, bool capturesOnly) {
        Bitboard currentMoves, nonCheckMoves, checkMoves, discoveredCheck;
        unsigned long lsb;

        occupied &= ~whites[KING];
        currentMoves = KingBitboards[wKingPos] & ~(blackAttacks() | whitePieces);
        if (capturesOnly) currentMoves &= blackPieces;

        discoveredCheck = bDiscoveredFrom(wKingPos);
        if (discoveredCheck) {
            _BitScanForward64(&lsb, discoveredCheck);
            nonCheckMoves = currentMoves & BlockCheckPath[bKingPos][lsb];
        }
        else nonCheckMoves = currentMoves;
        checkMoves = currentMoves & ~nonCheckMoves;

        while (nonCheckMoves) {
            _BitScanForward64(&lsb, nonCheckMoves);
            moves.push_back(Move(lsb, wKingPos, blackPieceArray[lsb], EMPTY, false, false, false, false, 0));
            nonCheckMoves &= nonCheckMoves - 1;
        }
        while (checkMoves) {
            _BitScanForward64(&lsb, checkMoves);
            moves.push_back(Move(lsb, wKingPos, blackPieceArray[lsb], EMPTY, false, false, false, false, discoveredCheck));
            checkMoves &= checkMoves - 1;
        }
        occupied |= whites[KING];
    }
    void GameBoard::generateBlackKingMoves(std::vector<Move>& moves, bool capturesOnly) {
        Bitboard currentMoves, nonCheckMoves, checkMoves, discoveredCheck;
        unsigned long lsb;

        occupied &= ~(blacks[KING]);
        currentMoves = KingBitboards[bKingPos] & ~(whiteAttacks() | blackPieces);
        if (capturesOnly) currentMoves &= whitePieces;

        discoveredCheck = wDiscoveredFrom(bKingPos);
        if (discoveredCheck) {
            _BitScanForward64(&lsb, discoveredCheck);
            nonCheckMoves = currentMoves & BlockCheckPath[wKingPos][lsb];
        }
        else nonCheckMoves = currentMoves;
        checkMoves = currentMoves & ~nonCheckMoves;

        while (checkMoves) {
            _BitScanForward64(&lsb, checkMoves);
            moves.push_back(Move(lsb, bKingPos, whitePieceArray[lsb], EMPTY, false, false, false, false, discoveredCheck));
            checkMoves &= checkMoves - 1;
        }
        while (nonCheckMoves) {
            _BitScanForward64(&lsb, nonCheckMoves);
            moves.push_back(Move(lsb, bKingPos, whitePieceArray[lsb], EMPTY, false, false, false, false, 0));
            nonCheckMoves &= nonCheckMoves - 1;
        }
        occupied |= blacks[KING];

    }

    void GameBoard::generateWhiteCastles(std::vector<Move>& moves) {
        if (wOO) {
            if (!(wOOPlaces & (blackAttacks() | occupied))) {
                occupied &= ~(bit << 4);
                if ((RookMagicBitboards[5][(occupied & RawRookAttacks[5]) * RookMagics[5] >> RookShifts[5]]) & blacks[KING]) {
                    moves.push_back(Move(6, 4, EMPTY, EMPTY, true, false, false, false, bit << 5));
                }
                else {
                    moves.push_back(Move(6, 4, EMPTY, EMPTY, true, false, false, false, 0));
                }
                occupied |= (bit << 4);
            }
        }
        if (wOOO) {
            if (!(wOOOPlacesExt & occupied) && !(wOOOPlaces & blackAttacks())) {
                occupied &= ~(bit << 4);
                if ((RookMagicBitboards[3][(occupied & RawRookAttacks[3]) * RookMagics[3] >> RookShifts[3]]) & blacks[KING]) {
                    moves.push_back(Move(2, 4, EMPTY, EMPTY, true, false, false, false, bit << 3));
                }
                else {
                    moves.push_back(Move(2, 4, EMPTY, EMPTY, true, false, false, false, 0));
                }
                occupied |= (bit << 4);
            }
        }
    }
    void GameBoard::generateBlackCastles(std::vector<Move>& moves) {
        if (bOO) {
            if (!(bOOPlaces & (whiteAttacks() | occupied))) {
                occupied &= ~(bit << 60);
                if ((RookMagicBitboards[61][(occupied & RawRookAttacks[61]) * RookMagics[61] >> RookShifts[61]]) & whites[KING]) {
                    moves.push_back(Move(62, 60, EMPTY, EMPTY, true, false, false, false, bit << 61));
                }
                else {
                    moves.push_back(Move(62, 60, EMPTY, EMPTY, true, false, false, false, 0));
                }
                occupied |= (bit << 60);
            }
        }
        if (bOOO) {
            if (!(bOOOPlacesExt & occupied) && !(bOOOPlaces & whiteAttacks())) {
                occupied &= ~(bit << 60);
                if ((RookMagicBitboards[59][(occupied & RawRookAttacks[59]) * RookMagics[59] >> RookShifts[59]]) & whites[KING]) {
                    moves.push_back(Move(58, 60, EMPTY, EMPTY, true, false, false, false, bit << 59));
                }
                else {
                    moves.push_back(Move(58, 60, EMPTY, EMPTY, true, false, false, false, 0));
                }
                occupied |= (bit << 60);
            }
        }
    }

    std::vector<Move> GameBoard::allMoves() {
        std::vector<Move> moves;

        // are used to detect discovered checks
        _BitScanForward64(&bKingPos, blacks[KING]);
        _BitScanForward64(&wKingPos, whites[KING]);
        whiteKingRookMoves = RookMagicBitboards[wKingPos][(occupied & RawRookAttacks[wKingPos]) * RookMagics[wKingPos] >> RookShifts[wKingPos]];
        whiteKingBishopMoves = BishopMagicBitboards[wKingPos][(occupied & RawBishopAttacks[wKingPos]) * BishopMagics[wKingPos] >> BishopShifts[wKingPos]];
        blackKingRookMoves = RookMagicBitboards[bKingPos][(occupied & RawRookAttacks[bKingPos]) * RookMagics[bKingPos] >> RookShifts[bKingPos]];
        blackKingBishopMoves = BishopMagicBitboards[bKingPos][(occupied & RawBishopAttacks[bKingPos]) * BishopMagics[bKingPos] >> BishopShifts[bKingPos]];

        if (turn) {
            if (!checksFrom) {
                generateWhiteMoves(moves, 0xFFFFFFFFFFFFFFFFULL);
                generateWhitePromotions(moves, 0xFFFFFFFFFFFFFFFFULL);
            }
            else if (!inDoubleCheck) {
                unsigned long lsb;
                _BitScanForward64(&lsb, checksFrom);
                generateWhiteMoves(moves, BlockCheckPath[wKingPos][lsb]);
                generateWhitePromotions(moves, BlockCheckPath[wKingPos][lsb]);
            }

            generateWhiteKingMoves(moves, false);

            if (!checksFrom) {
                generateWhiteCastles(moves);
            }

            return moves;
        }

        if (!checksFrom) {
            generateBlackMoves(moves, 0xFFFFFFFFFFFFFFFFULL);
            generateBlackPromotions(moves, 0xFFFFFFFFFFFFFFFFULL);
        }
        else if (!inDoubleCheck) {
            unsigned long lsb;
            _BitScanForward64(&lsb, checksFrom);
            generateBlackMoves(moves, BlockCheckPath[bKingPos][lsb]);
            generateBlackPromotions(moves, BlockCheckPath[bKingPos][lsb]);
        }

        generateBlackKingMoves(moves, false);

        if (!checksFrom) {
            generateBlackCastles(moves);
        }

        return moves;

    }
    std::vector<Move> GameBoard::noisyMoves() {
        if (checksFrom) return allMoves();

        std::vector<Move> moves;

        // are used to detect discovered checks
        _BitScanForward64(&bKingPos, blacks[KING]);
        _BitScanForward64(&wKingPos, whites[KING]);
        whiteKingRookMoves = RookMagicBitboards[wKingPos][(occupied & RawRookAttacks[wKingPos]) * RookMagics[wKingPos] >> RookShifts[wKingPos]];
        whiteKingBishopMoves = BishopMagicBitboards[wKingPos][(occupied & RawBishopAttacks[wKingPos]) * BishopMagics[wKingPos] >> BishopShifts[wKingPos]];
        blackKingRookMoves = RookMagicBitboards[bKingPos][(occupied & RawRookAttacks[bKingPos]) * RookMagics[bKingPos] >> RookShifts[bKingPos]];
        blackKingBishopMoves = BishopMagicBitboards[bKingPos][(occupied & RawBishopAttacks[bKingPos]) * BishopMagics[bKingPos] >> BishopShifts[bKingPos]];

        if (turn) {
            generateWhitePromotions(moves, 0xFFFFFFFFFFFFFFFFULL);

            generateWhiteMoves(moves, 0xFFFFFFFFFFFFFFFFULL & blackPieces);

            generateWhiteKingMoves(moves, true);

            return moves;
        }

        generateBlackPromotions(moves, 0xFFFFFFFFFFFFFFFFULL);

        generateBlackMoves(moves, 0xFFFFFFFFFFFFFFFFULL & whitePieces);

        generateBlackKingMoves(moves, true);

        return moves;
    }

    void GameBoard::doMove(Move& m) {
        plyCount++;

        // store "last" properties
        m.lastInDoubleCheck = inDoubleCheck;
        m.lastCheckedFrom = checksFrom;
        m.lastPly50 = ply50MoveRule;
        if (enPassantTarget) {
            unsigned long index;
            _BitScanForward64(&index, enPassantTarget);
            m.lastEnPassantTargetIndex = index;
        }
        else m.lastEnPassantTargetIndex = 64;

        inDoubleCheck = m.doubleCheck;
        checksFrom = m.checks;

        if (turn) { // white move
            if (whitePieceArray[m.start] == PAWN || m.capture != EMPTY) {
                ply50MoveRule = 0;
                irreversibleNumber++;
            }
            else ply50MoveRule++;

            whitePieces &= ~(bit << m.start);
            whitePieces |= (bit << m.end);
            if (m.doublePush) {
                enPassantTarget = bit << (m.start + 8);
            }
            else enPassantTarget = 0x0ULL;

            // moving a rook only, king movement and castling rights are done in the last section : 
            if (m.castle) {
                if (m.end == 2) {
                    whites[ROOK] &= ~bit;
                    whites[ROOK] |= 0x8ULL;
                    whitePieces &= ~bit;
                    whitePieces |= 0x8ULL;
                    whitePieceArray[0] = EMPTY;
                    whitePieceArray[3] = ROOK;
                }
                else {
                    whites[ROOK] &= ~0x80ULL;
                    whites[ROOK] |= 0x20ULL;
                    whitePieces &= ~0x80ULL;
                    whitePieces |= 0x20ULL;
                    whitePieceArray[7] = EMPTY;
                    whitePieceArray[5] = ROOK;
                }
            }
            // capturing with en passant :
            else if (m.enPassant) {
                blackPieces &= ~(bit << (m.end - 8));
                blacks[PAWN] &= ~(bit << (m.end - 8));
                blackPieceArray[m.end - 8] = EMPTY;
            }
            // other capturing  if any :
            else if (m.capture != EMPTY) {
                blackPieces &= ~(bit << m.end);
                blackPieceArray[m.end] = EMPTY;
                blacks[m.capture] &= ~(bit << m.end);
                if (m.capture == ROOK) {
                    if (m.end == 56 && bOOO) {
                        bOOO = false;
                        m.bOOOch = true;
                    }
                    else if (m.end == 63 && bOO) {
                        bOO = false;
                        m.bOOch = true;
                    }
                }
            }
            // promotion or "moving" a piece itself :
            // includes castling rights correction
            if (m.promotion == EMPTY) {
                whites[whitePieceArray[m.start]] &= ~(bit << m.start);
                whites[whitePieceArray[m.start]] |= (bit << m.end);
                if (whitePieceArray[m.start] == KING) {
                    if (wOO) {
                        wOO = false;
                        m.wOOch = true;
                    }
                    if (wOOO) {
                        wOOO = false;
                        m.wOOOch = true;
                    }
                }
                else if (whitePieceArray[m.start] == ROOK) {
                    if (m.start == 0 && wOOO) {
                        wOOO = false;
                        m.wOOOch = true;
                    }
                    else if (m.start == 7 && wOO) {
                        wOO = false;
                        m.wOOch = true;
                    }
                }
                whitePieceArray[m.end] = whitePieceArray[m.start];
            }
            else {
                whites[PAWN] &= ~(bit << m.start);
                whites[m.promotion] |= (bit << m.end);
                whitePieceArray[m.end] = m.promotion;
            }
            whitePieceArray[m.start] = EMPTY;
        }
        else { // black move
            if (blackPieceArray[m.start] == PAWN || m.capture != EMPTY) {
                ply50MoveRule = 0;
                irreversibleNumber++;
            }
            else ply50MoveRule++;

            blackPieces &= ~(bit << m.start);
            blackPieces |= (bit << m.end);
            if (m.doublePush) {
                enPassantTarget = bit << (m.start - 8);
            }
            else enPassantTarget = 0x0ULL;

            // moving a rook only, king movement and castling rights are done in the last section : 
            if (m.castle) {
                if (m.end == 58) {
                    blacks[ROOK] &= ~0x0100000000000000ULL;
                    blacks[ROOK] |= 0x0800000000000000ULL;
                    blackPieces &= ~0x0100000000000000ULL;
                    blackPieces |= 0x0800000000000000ULL;
                    blackPieceArray[56] = EMPTY;
                    blackPieceArray[59] = ROOK;
                }
                else {
                    blacks[ROOK] &= ~0x8000000000000000ULL;
                    blacks[ROOK] |= 0x2000000000000000ULL;
                    blackPieces &= ~0x8000000000000000ULL;
                    blackPieces |= 0x2000000000000000ULL;
                    blackPieceArray[63] = EMPTY;
                    blackPieceArray[61] = ROOK;
                }
            }
            // capturing with en passant :
            else if (m.enPassant) {
                whitePieces &= ~(bit << (m.end + 8));
                whites[PAWN] &= ~(bit << (m.end + 8));
                whitePieceArray[m.end + 8] = EMPTY;
            }
            // other capturing  if any :
            else if (m.capture != EMPTY) {
                whitePieces &= ~(bit << m.end);
                whitePieceArray[m.end] = EMPTY;
                whites[m.capture] &= ~(bit << m.end);
                if (m.capture == ROOK) {
                    if (m.end == 0 && wOOO) {
                        wOOO = false;
                        m.wOOOch = true;
                    }
                    else if (m.end == 7 && wOO) {
                        wOO = false;
                        m.wOOch = true;
                    }
                }
            }
            // promotion or "moving" a piece itself :
            // includes castling rights correction
            if (m.promotion == EMPTY) {
                blacks[blackPieceArray[m.start]] &= ~(bit << m.start);
                blacks[blackPieceArray[m.start]] |= (bit << m.end);
                if (blackPieceArray[m.start] == KING) {
                    if (bOO) {
                        bOO = false;
                        m.bOOch = true;
                    }
                    if (bOOO) {
                        bOOO = false;
                        m.bOOOch = true;
                    }
                }
                else if (blackPieceArray[m.start] == ROOK) {
                    if (m.start == 56 && bOOO) {
                        bOOO = false; m.bOOOch = true;
                    }
                    else if (m.start == 63 && bOO) {
                        bOO = false; m.bOOch = true;
                    }
                }
                blackPieceArray[m.end] = blackPieceArray[m.start];
            }
            else {
                blacks[PAWN] &= ~(bit << m.start);
                blacks[m.promotion] |= (bit << m.end);
                blackPieceArray[m.end] = m.promotion;
            }
            blackPieceArray[m.start] = EMPTY;
        }
        occupied = blackPieces | whitePieces;
        turn = !turn;
        moveHistory.push_back(m);
        positionHistory.push_back(computeZobristKey());
    }
    void GameBoard::undoMove(Move& m) {
        plyCount--;
        turn = !turn;
        if (!turn) {  // If the turn is now white (we're undoing a black move)
            blackPieces &= ~(bit << m.end);
            blackPieces |= (bit << m.start);
            blackPieceArray[m.start] = blackPieceArray[m.end];
            blackPieceArray[m.end] = EMPTY;

            if (m.enPassant) {
                whitePieces |= (bit << (m.end + 8));
                whites[PAWN] |= (bit << (m.end + 8));
                whitePieceArray[m.end + 8] = PAWN;
            }
            else if (m.capture != EMPTY) {
                whitePieces |= (bit << m.end);
                whitePieceArray[m.end] = m.capture;
                whites[m.capture] |= (bit << m.end);
            }

            if (m.castle) {
                if (m.end == 58) {
                    blacks[ROOK] &= ~0x0800000000000000ULL;
                    blacks[ROOK] |= 0x0100000000000000ULL;
                    blackPieces &= ~0x0800000000000000ULL;
                    blackPieces |= 0x0100000000000000ULL;
                    blackPieceArray[59] = EMPTY;
                    blackPieceArray[56] = ROOK;
                }
                else {
                    blacks[ROOK] &= ~0x2000000000000000ULL;
                    blacks[ROOK] |= 0x8000000000000000ULL;
                    blackPieces &= ~0x2000000000000000ULL;
                    blackPieces |= 0x8000000000000000ULL;
                    blackPieceArray[61] = EMPTY;
                    blackPieceArray[63] = ROOK;
                }
            }

            if (m.promotion == EMPTY) {
                blacks[blackPieceArray[m.start]] |= (bit << m.start);
                blacks[blackPieceArray[m.start]] &= ~(bit << m.end);
            }
            else {
                blacks[PAWN] |= (bit << m.start);
                blacks[m.promotion] &= ~(bit << m.end);
                blackPieceArray[m.start] = PAWN;
            }

            if (m.wOOch) wOO = true;
            if (m.wOOOch) wOOO = true;
            if (m.bOOch) bOO = true;
            if (m.bOOOch) bOOO = true;
        }
        else {  // If the turn is now black (we're undoing a white move)
            whitePieces &= ~(bit << m.end);
            whitePieces |= (bit << m.start);
            whitePieceArray[m.start] = whitePieceArray[m.end];
            whitePieceArray[m.end] = EMPTY;

            if (m.enPassant) {
                blackPieces |= (bit << (m.end - 8));
                blacks[PAWN] |= (bit << (m.end - 8));
                blackPieceArray[m.end - 8] = PAWN;
            }
            else if (m.capture != EMPTY) {
                blackPieces |= (bit << m.end);
                blackPieceArray[m.end] = m.capture;
                blacks[m.capture] |= (bit << m.end);
            }

            if (m.castle) {
                if (m.end == 2) {
                    whites[ROOK] &= ~0x8ULL;
                    whites[ROOK] |= bit;
                    whitePieces &= ~0x8ULL;
                    whitePieces |= bit;
                    whitePieceArray[3] = EMPTY;
                    whitePieceArray[0] = ROOK;
                }
                else {
                    whites[ROOK] &= ~0x20ULL;
                    whites[ROOK] |= 0x80ULL;
                    whitePieces &= ~0x20ULL;
                    whitePieces |= 0x80ULL;
                    whitePieceArray[5] = EMPTY;
                    whitePieceArray[7] = ROOK;
                }
            }

            if (m.promotion == EMPTY) {
                whites[whitePieceArray[m.start]] |= (bit << m.start);
                whites[whitePieceArray[m.start]] &= ~(bit << m.end);
            }
            else {
                whites[PAWN] |= (bit << m.start);
                whites[m.promotion] &= ~(bit << m.end);
                whitePieceArray[m.start] = PAWN;
            }

            if (m.wOOch) wOO = true;
            if (m.wOOOch) wOOO = true;
            if (m.bOOch) bOO = true;
            if (m.bOOOch) bOOO = true;
        }
        occupied = blackPieces | whitePieces;

        // restore "last" properties
        inDoubleCheck = m.lastInDoubleCheck;
        checksFrom = m.lastCheckedFrom;
        if (ply50MoveRule == 0) irreversibleNumber--;
        ply50MoveRule = m.lastPly50;
        if (m.lastEnPassantTargetIndex == 64) {
            enPassantTarget = 0x0ULL;
        }
        else enPassantTarget = (bit << m.lastEnPassantTargetIndex);

        positionHistory.pop_back();
        moveHistory.pop_back();
    }

    bool GameBoard::isRepetition() const {
        uint64_t key = computeZobristKey();
        /*int count = 0;
        for (int i = 0; i < positionHistory.size(); i++) {
            if (positionHistory[i] == key) count++;
        }
        return count - 1;*/
        for (int i = positionHistory.size() - 3; i >= 0; i-=2) {
            if (positionHistory[i] == key) return true;
        }
        return false;
    }

    long GameBoard::perft(int depth) {
        long num = 0;
        if (depth == 0) {
            //printBoard();
            return 1;
        }
        for (Move m : allMoves()) {
            doMove(m);
            /*TTEntry entry;
            if (TT.retrieve(computeZobristKey(), depth, entry) ) {
                num += entry.nodes_num;
            }
            else {
                long nodes = posToDepth(depth - 1);
                num += nodes;
                TT.insert(computeZobristKey(), depth, nodes);
            }*/
            long nodes = perft(depth - 1);
            num += nodes;
            undoMove(m);
        }
        return num;
    }
    std::string GameBoard::notation_from_index(unsigned long index) const {
        int r = index / 8;
        int c = index % 8;
        char ch1 = 'a' + c;
        char ch2 = '1' + r;
        std::string str{ ch1, ch2 };
        return str;
    }

    std::string GameBoard::rankToFEN(int rank) const {
        std::ostringstream fenRank;
        int emptyCount = 0;

        for (int file = 0; file < 8; ++file) {
            int squareIndex = rank * 8 + file;

            pieceType whitePiece = whitePieceArray[squareIndex];
            pieceType blackPiece = blackPieceArray[squareIndex];
            char pieceChar = ' ';

            if (whitePiece != EMPTY) {
                switch (whitePiece) {
                case PAWN: pieceChar = 'P'; break;
                case KNIGHT: pieceChar = 'N'; break;
                case BISHOP: pieceChar = 'B'; break;
                case ROOK: pieceChar = 'R'; break;
                case QUEEN: pieceChar = 'Q'; break;
                case KING: pieceChar = 'K'; break;
                default: break;
                }
            }
            else if (blackPiece != EMPTY) {
                switch (blackPiece) {
                case PAWN: pieceChar = 'p'; break;
                case KNIGHT: pieceChar = 'n'; break;
                case BISHOP: pieceChar = 'b'; break;
                case ROOK: pieceChar = 'r'; break;
                case QUEEN: pieceChar = 'q'; break;
                case KING: pieceChar = 'k'; break;
                default: break;
                }
            }

            if (pieceChar != ' ') {
                if (emptyCount > 0) {
                    fenRank << emptyCount;
                    emptyCount = 0;
                }
                fenRank << pieceChar;
            }
            else {
                ++emptyCount;
            }
        }

        if (emptyCount > 0) {
            fenRank << emptyCount;
        }

        return fenRank.str();
    }

    std::string GameBoard::computeFEN() const {
        std::ostringstream fen;

        for (int rank = 7; rank >= 0; --rank) {
            fen << rankToFEN(rank);
            if (rank > 0) {
                fen << '/';
            }
        }

        fen << ' ' << (turn ? 'w' : 'b');

        fen << ' ';
        bool castlingAvailable = false;
        if (wOO) { fen << 'K'; castlingAvailable = true; }
        if (wOOO) { fen << 'Q'; castlingAvailable = true; }
        if (bOO) { fen << 'k'; castlingAvailable = true; }
        if (bOOO) { fen << 'q'; castlingAvailable = true; }
        if (!castlingAvailable) {
            fen << '-';
        }

        fen << ' ';
        if (enPassantTarget) {
            unsigned long lsb;
            _BitScanForward64(&lsb, enPassantTarget);
            int rank = lsb / 8 + 1;
            int file = lsb % 8;
            fen << static_cast<char>('a' + file) << rank;
        }
        else {
            fen << '-';
        }
        
        fen << " " << ply50MoveRule << " " << plyCount/2 + 1;

        return fen.str();
    }


