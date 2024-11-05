
#include <iostream>
#include <array>
#include <vector>
#include <random>
#include <fstream>
#include <string>
#include "GameBoard.h"
#include "Tables.h"
#include "Engine.h"
#include "AgedTT.h"

bool equal(GameBoard& b1, GameBoard& b2) {
    for (int i = 0; i < 64; i++) {
        if ((b1.whitePieceArray[i] != b2.whitePieceArray[i]) || (b1.blackPieceArray[i] != b2.blackPieceArray[i])) return false;
    }
    return (
        (b1.whites[PAWN] == b2.whites[PAWN]) && (b1.whites[KNIGHT] == b2.whites[KNIGHT]) && (b1.whites[BISHOP] == b2.whites[BISHOP]) && (b1.whites[ROOK] == b2.whites[ROOK]) && (b1.whites[QUEEN] == b2.whites[QUEEN]) && (b1.whites[KING] == b2.whites[KING]) &&
        (b1.blacks[PAWN] == b2.blacks[PAWN]) && (b1.blacks[KNIGHT] == b2.blacks[KNIGHT]) && (b1.blacks[BISHOP] == b2.blacks[BISHOP]) && (b1.blacks[ROOK] == b2.blacks[ROOK]) && (b1.blacks[QUEEN] == b2.blacks[QUEEN]) && (b1.blacks[KING] == b2.blacks[KING]) &&
        (b1.occupied == b2.occupied) && (b1.blackPieces == b2.blackPieces) && (b1.whitePieces == b2.whitePieces) &&
        (b1.turn == b2.turn) && (b1.wOO == b2.wOO) && (b1.wOOO == b2.wOOO) &&
        (b1.bOO == b2.bOO) && (b1.bOOO == b2.bOOO) && (b1.inDoubleCheck == b2.inDoubleCheck) &&
        (b1.checksFrom == b2.checksFrom) && (b1.enPassantTarget == b2.enPassantTarget) && (b1.plyCount == b2.plyCount) && (b1.ply50MoveRule == b2.ply50MoveRule)
    );
}

void out(Bitboard b) {
    for (int row = 7; row >= 0; row--) {
        std::cout << row + 1 << "   ";
        for (int col = 0; col < 8; col++) {
            if (b & (1ULL << (row * 8 + col))) {
                std::cout << "1 ";
            }
            else {
                std::cout << "0 ";
            }
        }
        std::cout << std::endl;
    }
    std::cout << std::endl << "    a b c d e f g h" << std::endl << std::endl << std::endl;
}
unsigned long index_from_notation(std::string str) {
    int c = str[0] - 'a';
    int r = str[1] - '1';
    return r * 8 + c;
}
std::string notation_from_index(unsigned long index) {
    int r = index / 8;
    int c = index % 8;
    char ch1 = 'a' + c;
    char ch2 = '1' + r;
    std::string str{ ch1, ch2 };
    return str;
}
Move move_from_notation(std::string input) {
    Move m = Move(64, 64, EMPTY, EMPTY, false, false, false, false, 0);
    if (input.length() < 4 || input.length() > 5) {
        return m;
    }
    int startIndex = index_from_notation(input.substr(0, 2));
    int endIndex = index_from_notation(input.substr(2, 2));
    if (startIndex > 64 || endIndex > 64) {
        return m;
    }
    pieceType prom;
    if (input.length() == 5) {
        switch (input[4])
        {
        case 'q':
            prom = QUEEN;
            break;
        case 'r':
            prom = ROOK;
            break;
        case 'b':
            prom = BISHOP;
            break;
        case 'n':
            prom = KNIGHT;
            break;
        default:
            prom = PAWN;
            break;
        }
    }
    else prom = EMPTY;
    if (prom == PAWN) {
        return m;
    }
    return Move(endIndex, startIndex, EMPTY, prom, false, false, false, false, 0);
}
std::string notation_from_move(Move& m) {
    std::string str = notation_from_index(m.start) + notation_from_index(m.end);
    switch (m.promotion)
    {
    case QUEEN:
        str += "q";
        break;
    case ROOK:
        str += "r";
        break;
    case BISHOP:
        str += "b";
        break;
    case KNIGHT:
        str += "n";
        break;
    default:
        break;
    }
    return str;
}

void generateRawRookAttacks() {
    std::cout << "static constexpr std::array<Bitboard, 64> RawRookAttacks = {" << std::endl;
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            Bitboard bb = aFile << c | firstRank << r * 8;
            bb &= ~(bit << (r * 8 + c));
            if (r != 0) bb &= ~firstRank;
            if (r != 7) bb &= ~eightRank;
            if (c != 0) bb &= ~aFile;
            if (c != 7) bb &= ~hFile;
            std::cout << "0x" << std::hex << bb << "ULL, ";
        }
        std::cout << std::endl;
    }
    std::cout << "};" << std::endl;
}
void generateRawBishopAttacks() {
    Bitboard a1h8 = 0x8040201008040201ULL;
    Bitboard a8h1 = 0x102040810204080ULL;
    std::cout << "static constexpr std::array<Bitboard, 64> RawBishopAttacks = {" << std::endl;
    for (int R = 0; R < 8; R++) {
        for (int C = 0; C < 8; C++) {
            Bitboard bb = 0x0ULL;
            if (R > C) bb |= a1h8 << 8 * (R - C);
            else bb |= a1h8 >> 8 * (C - R);
            if (8 - R > C) bb |= a8h1 >> 8 * (8 - R - C - 1);
            else bb |= a8h1 << 8 * (C - 8 + R + 1);
            bb &= ~(bit << (R*8+C));
            bb &= ~firstRank;
            bb &= ~eightRank;
            bb &= ~aFile;
            bb &= ~hFile;
            std::cout << "0x" << std::hex << bb << "ULL, ";
        }
        std::cout << std::endl;
    }
    std::cout << "};" << std::endl;
}
void generateKingMoves() {
    std::cout << "static constexpr std::array<Bitboard, 64> KingBitboards = {" << std::endl;
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            Bitboard bb = bit << (r * 8 + c);
            bb |= bb << 1;
            bb |= bb >> 8;
            bb |= bb >> 1;
            bb |= bb << 8;
            if (c == 0) bb &= ~hFile;
            if (c == 7) bb &= ~aFile;
            bb &= ~(bit << (r * 8 + c));
            std::cout << "0x" << std::hex << bb << "ULL, ";
        }
        std::cout << std::endl;
    }
    std::cout << "};" << std::endl;
}
void generateKnightMoves() {
    std::cout << "static constexpr std::array<Bitboard, 64> KnightBitboards = {" << std::endl;
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            Bitboard bb = 0x0ULL;
            if (r < 6 && c != 7) bb |= bit << (r * 8 + c + 17);
            if (r != 7 && c < 6) bb |= bit << (r * 8 + c + 10);
            if (r != 0 && c < 6) bb |= bit << (r * 8 + c - 6);
            if (r > 1 && c != 7) bb |= bit << (r * 8 + c - 15);
            if (r > 1 && c != 0) bb |= bit << (r * 8 + c - 17);
            if (r != 0 && c > 1) bb |= bit << (r * 8 + c - 10);
            if (r != 7 && c > 1) bb |= bit << (r * 8 + c + 6);
            if (r < 6 && c != 0) bb |= bit << (r * 8 + c + 15);
            std::cout << "0x" << std::hex << bb << "ULL, ";
        }
        std::cout << std::endl;
    }
    std::cout << "};" << std::endl;
}

Bitboard randomRookBlockerSet(int r, int c) {
    std::random_device rd;
    std::mt19937 gen(rd());
    Bitboard bb = RawRookAttacks[r * 8 + c];
    bb &= ~(bit << (r * 8 + c));
    if (r != 0) bb &= ~firstRank;
    if (r != 7) bb &= ~eightRank;
    if (c != 0) bb &= ~aFile;
    if (c != 7) bb &= ~hFile;
    unsigned __int64 bitCount = __popcnt64(bb);
    std::uniform_int_distribution<> distr(0, 1 << bitCount);
    int mask = distr(gen);
    Bitboard blocks = 0x0ULL;
    int maskIndex = 0;
    for (int i = 0; i < 64; ++i) {
        if (bb & (bit << i)) {
            if (mask & (1 << maskIndex)) {
                blocks |= (bit << i);
            }
            maskIndex++;
        }
    }
    return blocks;
}
Bitboard randomBishopBlockerSet(int r, int c) {
    std::random_device rd;
    std::mt19937 gen(rd());
    Bitboard bb = RawBishopAttacks[r * 8 + c];
    bb &= ~(bit << (r * 8 + c));
    bb &= ~firstRank;
    bb &= ~eightRank;
    bb &= ~aFile;
    bb &= ~hFile;
    unsigned __int64 bitCount = __popcnt64(bb);
    std::uniform_int_distribution<> distr(0, 1 << bitCount);
    int mask = distr(gen);
    Bitboard blocks = 0x0ULL;
    int maskIndex = 0;
    for (int i = 0; i < 64; ++i) {
        if (bb & (bit << i)) {
            if (mask & (1 << maskIndex)) {
                blocks |= (bit << i);
            }
            maskIndex++;
        }
    }
    return blocks;
}

bool isThreefoldRepetition(std::vector<uint64_t>& history, uint64_t key) {
    int count = 0;
    for (int i = history.size() - 3; i >= 0 && count < 3; i -= 2) {
        if (history[i] == key) count++;
    }
    return (count >= 3);
}

void runMatchup(std::string& fen, bool depthIsWhite, int testNum, int depth, int timelimit) {
    GameBoard board1 = GameBoard();
    GameBoard board2 = GameBoard();
    Engine engine1(board1);
    Engine engine2(board2);
    // change TT
    if (depth != 0) {
        engine1.useTimer = false;
        engine2.useTimer = false;
    }
    else {
        depth = 100;
        engine1.useTimer = true;
        engine2.useTimer = true;
        engine1.timer.timeLimit = timelimit;
        engine2.timer.timeLimit = timelimit;
    }
    engine1.board.setFromFen(fen);
    engine2.board.setFromFen(fen);
    uint64_t curNodes, curTT;
    double curTime;
    uint64_t nodes1, nodes2, ttHit1, ttHit2;
    nodes1 = nodes2 = ttHit1 = ttHit2 = 0;
    double time1, time2;
    time1 = time2 = 0;
    SearchResult result;
    bool isFirstEngineTurn = engine1.board.turn ^ !depthIsWhite;
    std::ofstream fout;
    std::string fileName = "data\\match";
    fileName += std::to_string(testNum);
    fileName += ".txt";
    fout.open(fileName);
    fout << "depth " << depth << " timelimit " << timelimit << "\n";
    std::string info = depthIsWhite ? "depthTT vs ageTT\n" : "ageTT vs depthTT\n";
    fout << info << fen << "\n";
    std::string gameResult;
    std::vector<uint64_t> history;
    int depthSearched = 0;
    while (true) {
        if (isThreefoldRepetition(history, engine1.board.computeZobristKey())) {
            gameResult = "draw by repetition\n";
            break;
        }
        if (engine1.isDrawByIM()) {
            gameResult = "draw by insufficient material\n";
            break;
        }
        if (engine1.board.ply50MoveRule >= 100) {
            gameResult = "draw by 50 move rule\n";
            break;
        }
        Engine& active = isFirstEngineTurn ? engine1 : engine2;
        history.push_back(active.board.computeZobristKey());
        curNodes = curTT = curTime = 0;
        result = active.iterativeDeepening(depth, curNodes, curTT, curTime, depthSearched);
        if (isFirstEngineTurn) {
            nodes1 += curNodes;
            ttHit1 += curTT;
            time1 += curTime;
        }
        else {
            nodes2 += curNodes;
            ttHit2 += curTT;
            time2 += curTime;
        }
        if (result.bestLine.empty()) {
            if (engine1.board.checksFrom) {
                std::string won = engine1.board.turn ? "black" : "white";
                gameResult = won + " won by checkmate\n";
            }
            else gameResult = "draw by stalemate\n";
            break;
        }
        Move move = result.bestLine[result.bestLine.size() - 1];
        //std::cout << curNodes << std::endl << curTT << std::endl << result.eval << std::endl << curTime << std::endl << notation_from_move(move) << std::endl;
        fout << notation_from_move(move) << " " << depthSearched << std::endl;
        std::cout << notation_from_move(move) << " " << depthSearched << std::endl;
        engine1.board.doMove(move);
        engine1.board.doMove(move);
        engine2.board.doMove(move);
        if (engine1.updateTTmode() || engine2.updateTTmode()) {
            fout << "TT mode changed : " << engine1.ttMode << " " << engine2.ttMode << "\n";
        }
        isFirstEngineTurn = !isFirstEngineTurn;
        //std::cout << "stored : " << active.TT->stored << "\noverwriten : " << active.TT->overriten << "(with diff : " << active.TT->overwritenWithDiff << ")\nrefused : " << active.TT->refused << "\nretrieved : " << active.TT->retrieved << "\nnot : " << active.TT->not_retrieved << "\n\n";
    }
    fout << "end\n";
    fout << "depth TT : \n" << nodes1 << "\n" << ttHit1 << "\n" << time1 << "\n" << (double)ttHit1 / nodes1 << "\n" << engine1.TT->stored << "\n" << engine1.TT->retrieved << "\n" << engine1.TT->not_retrieved << "\n" << engine1.TT->overriten << "\n" << engine1.TT->overwritenWithDiff << "\n" << engine1.TT->refused << "\n" << (double)engine1.TT->retrieved / (engine1.TT->retrieved + engine1.TT->not_retrieved) << "\n";
    fout << "\nage TT : \n" << nodes2 << "\n" << ttHit2 << "\n" << time2 << "\n" << (double)ttHit2 / nodes2 << "\n" << engine2.TT->stored << "\n" << engine2.TT->retrieved << "\n" << engine2.TT->not_retrieved << "\n" << engine2.TT->overriten << "\n" << engine2.TT->overwritenWithDiff << "\n" << engine2.TT->refused << "\n" << (double)engine2.TT->retrieved / (engine2.TT->retrieved + engine2.TT->not_retrieved) << "\n";
    std::cout << "depth TT : \n" << nodes1 << "\n" << ttHit1 << "\n" << time1 << "\n" << (double)ttHit1 / nodes1;
    std::cout << "\nage TT : \n" << nodes2 << "\n" << ttHit2 << "\n" << time2 << "\n" << (double)ttHit2 / nodes2 << "\n";
    std::cout << gameResult;
    fout << gameResult;
    fout.close();
    //engine2.start();
    delete engine1.TT;
    delete engine2.TT;
}

int main()
{
    MagicSetup();
    BlockCheckSetup();
    ZobristSetup();

    // magic bitboards testing :
    /*int index;
    while (true) {
        std::string str;
        std::cin >> str;
        char piece = str[0];
        int c = str[1] - 'a';
        int r = str[2] - '1';
        if (piece == 'b') {
            Bitboard blockerSet = randomBishopBlockerSet(r, c);
            out(blockerSet);
            index = blockerSet * BishopMagics[r * 8 + c] >> BishopShifts[r * 8 + c];
            out(BishopMagicBitboards[r * 8 + c][index]);
        }
        else if (piece == 'r') {
            Bitboard blockerSet = randomRookBlockerSet(r, c);
            out(blockerSet);
            index = blockerSet * RookMagics[r * 8 + c] >> RookShifts[r * 8 + c];
            out(RookMagicBitboards[r * 8 + c][index]);
        }
        else {
            Bitboard bishopBlockers = randomBishopBlockerSet(r, c);
            Bitboard rookBlockers = randomRookBlockerSet(r, c);
            out(bishopBlockers | rookBlockers);
            index = bishopBlockers * BishopMagics[r * 8 + c] >> BishopShifts[r * 8 + c];
            Bitboard attacks = BishopMagicBitboards[r * 8 + c][index];
            index = rookBlockers * RookMagics[r * 8 + c] >> RookShifts[r * 8 + c];
            attacks |= RookMagicBitboards[r * 8 + c][index];
            out(attacks);
        }
    }*/

    // block check testing :
    /*MagicSetup();
    BlockCkeckSetup();
    std::string str;
    while (true) {
        std::cin >> str;
        int c = str[0] - 'a';
        int r = str[1] - '1';
        int king = r * 8 + c;
        std::cin >> str;
        c = str[0] - 'a';
        r = str[1] - '1';
        int check = r * 8 + c;
        out(BlockCkeckPath[king][check]);
    }*/

    /*int testNum = 0;
    for(int depth = 10; depth > 2; depth--){
        std::ifstream fin;
        fin.open("silver_op_suite.txt");
        std::string fen;
        while (true) {
            std::getline(fin, fen);
            if (fen == "end") break;
            std::cout << "<----------------------------------->\n" << testNum << " match started\n";
            runMatchup(fen, true, testNum, depth, 0);
            testNum++;
            std::cout << "<----------------------------------->\n" << testNum << " match started\n";
            runMatchup(fen, false, testNum, depth, 0);
            testNum++;
        }
        fin.close();
    }
    std::vector<int> limits = { 30, 20,15, 10, 8,7,6,5,4,3,2, 1 };
    for (int lim : limits) {
        std::ifstream fin;
        fin.open("silver_op_suite.txt");
        std::string fen;
        while (true) {
            std::getline(fin, fen);
            if (fen == "end") break;
            std::cout << "<----------------------------------->\n" << testNum << " match started\n";
            runMatchup(fen, true, testNum, 0, lim);
            testNum++;
            std::cout << "<----------------------------------->\n" << testNum << " match started\n";
            runMatchup(fen, false, testNum, 0, lim);
            testNum++;
        }
        fin.close();
    }*/

    /*std::ifstream fin;
    fin.open("silver_op_suite.txt");
    std::string fen;
    std::getline(fin, fen);
    std::getline(fin, fen);
    runMatchup(fen, false, 0, 0, 5);
    fin.close();*/

    GameBoard board = GameBoard();
    Engine engine = Engine(board);
    engine.start();

    return 0;
}
