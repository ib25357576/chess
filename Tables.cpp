#include "Tables.h"
#include "AgedTT.h"
#include "DepthTT.h"
#include<iostream>
#include<vector>
#include<array>
#include<random>


    const std::array<int, 64> RookShifts = {
       52, 52, 52, 52, 52, 52, 52, 52,
       53, 53, 53, 54, 53, 53, 54, 53,
       53, 54, 54, 54, 53, 53, 54, 53,
       53, 54, 53, 53, 54, 54, 54, 53,
       52, 54, 53, 53, 53, 53, 54, 53,
       52, 53, 54, 54, 53, 53, 54, 53,
       53, 54, 54, 54, 53, 53, 54, 53,
       52, 53, 53, 53, 53, 53, 53, 52
    };
    const std::array<int, 64> BishopShifts = {
       58, 60, 59, 59, 59, 59, 60, 58,
       60, 59, 59, 59, 59, 59, 59, 60,
       59, 59, 57, 57, 57, 57, 59, 59,
       59, 59, 57, 55, 55, 57, 59, 59,
       59, 59, 57, 55, 55, 57, 59, 59,
       59, 59, 57, 57, 57, 57, 59, 59,
       60, 60, 59, 59, 59, 59, 60, 60,
       58, 60, 59, 59, 59, 59, 59, 58
    };

    const std::array<Bitboard, 64> RookMagics = {
       468374916371625120ULL, 18428729537625841661ULL, 2531023729696186408ULL, 6093370314119450896ULL,
       13830552789156493815ULL, 16134110446239088507ULL, 12677615322350354425ULL, 5404321144167858432ULL,
       2111097758984580ULL, 18428720740584907710ULL, 17293734603602787839ULL, 4938760079889530922ULL,
       7699325603589095390ULL, 9078693890218258431ULL, 578149610753690728ULL, 9496543503900033792ULL,
       1155209038552629657ULL, 9224076274589515780ULL, 1835781998207181184ULL, 509120063316431138ULL,
       16634043024132535807ULL, 18446673631917146111ULL, 9623686630121410312ULL, 4648737361302392899ULL,
       738591182849868645ULL, 1732936432546219272ULL, 2400543327507449856ULL, 5188164365601475096ULL,
       10414575345181196316ULL, 1162492212166789136ULL, 9396848738060210946ULL, 622413200109881612ULL,
       7998357718131801918ULL, 7719627227008073923ULL, 16181433497662382080ULL, 18441958655457754079ULL,
       1267153596645440ULL, 18446726464209379263ULL, 1214021438038606600ULL, 4650128814733526084ULL,
       9656144899867951104ULL, 18444421868610287615ULL, 3695311799139303489ULL, 10597006226145476632ULL,
       18436046904206950398ULL, 18446726472933277663ULL, 3458977943764860944ULL, 39125045590687766ULL,
       9227453435446560384ULL, 6476955465732358656ULL, 1270314852531077632ULL, 2882448553461416064ULL,
       11547238928203796481ULL, 1856618300822323264ULL, 2573991788166144ULL, 4936544992551831040ULL,
       13690941749405253631ULL, 15852669863439351807ULL, 18302628748190527413ULL, 12682135449552027479ULL,
       13830554446930287982ULL, 18302628782487371519ULL, 7924083509981736956ULL, 4734295326018586370ULL
    };
    const std::array<Bitboard, 64> BishopMagics = {
       16509839532542417919ULL, 14391803910955204223ULL, 1848771770702627364ULL, 347925068195328958ULL,
       5189277761285652493ULL, 3750937732777063343ULL, 18429848470517967340ULL, 17870072066711748607ULL,
       16715520087474960373ULL, 2459353627279607168ULL, 7061705824611107232ULL, 8089129053103260512ULL,
       7414579821471224013ULL, 9520647030890121554ULL, 17142940634164625405ULL, 9187037984654475102ULL,
       4933695867036173873ULL, 3035992416931960321ULL, 15052160563071165696ULL, 5876081268917084809ULL,
       1153484746652717320ULL, 6365855841584713735ULL, 2463646859659644933ULL, 1453259901463176960ULL,
       9808859429721908488ULL, 2829141021535244552ULL, 576619101540319252ULL, 5804014844877275314ULL,
       4774660099383771136ULL, 328785038479458864ULL, 2360590652863023124ULL, 569550314443282ULL,
       17563974527758635567ULL, 11698101887533589556ULL, 5764964460729992192ULL, 6953579832080335136ULL,
       1318441160687747328ULL, 8090717009753444376ULL, 16751172641200572929ULL, 5558033503209157252ULL,
       17100156536247493656ULL, 7899286223048400564ULL, 4845135427956654145ULL, 2368485888099072ULL,
       2399033289953272320ULL, 6976678428284034058ULL, 3134241565013966284ULL, 8661609558376259840ULL,
       17275805361393991679ULL, 15391050065516657151ULL, 11529206229534274423ULL, 9876416274250600448ULL,
       16432792402597134585ULL, 11975705497012863580ULL, 11457135419348969979ULL, 9763749252098620046ULL,
       16960553411078512574ULL, 15563877356819111679ULL, 14994736884583272463ULL, 9441297368950544394ULL,
       14537646123432199168ULL, 9888547162215157388ULL, 18140215579194907366ULL, 18374682062228545019ULL
    };

    // raw attacks dont include last possible move/capture on the line
    const std::array<Bitboard, 64> RawRookAttacks = {
   0x101010101017eULL, 0x202020202027cULL, 0x404040404047aULL, 0x8080808080876ULL, 0x1010101010106eULL, 0x2020202020205eULL, 0x4040404040403eULL, 0x8080808080807eULL,
   0x1010101017e00ULL, 0x2020202027c00ULL, 0x4040404047a00ULL, 0x8080808087600ULL, 0x10101010106e00ULL, 0x20202020205e00ULL, 0x40404040403e00ULL, 0x80808080807e00ULL,
   0x10101017e0100ULL, 0x20202027c0200ULL, 0x40404047a0400ULL, 0x8080808760800ULL, 0x101010106e1000ULL, 0x202020205e2000ULL, 0x404040403e4000ULL, 0x808080807e8000ULL,
   0x101017e010100ULL, 0x202027c020200ULL, 0x404047a040400ULL, 0x8080876080800ULL, 0x1010106e101000ULL, 0x2020205e202000ULL, 0x4040403e404000ULL, 0x8080807e808000ULL,
   0x1017e01010100ULL, 0x2027c02020200ULL, 0x4047a04040400ULL, 0x8087608080800ULL, 0x10106e10101000ULL, 0x20205e20202000ULL, 0x40403e40404000ULL, 0x80807e80808000ULL,
   0x17e0101010100ULL, 0x27c0202020200ULL, 0x47a0404040400ULL, 0x8760808080800ULL, 0x106e1010101000ULL, 0x205e2020202000ULL, 0x403e4040404000ULL, 0x807e8080808000ULL,
   0x7e010101010100ULL, 0x7c020202020200ULL, 0x7a040404040400ULL, 0x76080808080800ULL, 0x6e101010101000ULL, 0x5e202020202000ULL, 0x3e404040404000ULL, 0x7e808080808000ULL,
   0x7e01010101010100ULL, 0x7c02020202020200ULL, 0x7a04040404040400ULL, 0x7608080808080800ULL, 0x6e10101010101000ULL, 0x5e20202020202000ULL, 0x3e40404040404000ULL, 0x7e80808080808000ULL,
    };
    const std::array<Bitboard, 64> RawBishopAttacks = {
   0x40201008040200ULL, 0x402010080400ULL, 0x4020100a00ULL, 0x40221400ULL, 0x2442800ULL, 0x204085000ULL, 0x20408102000ULL, 0x2040810204000ULL,
   0x20100804020000ULL, 0x40201008040000ULL, 0x4020100a0000ULL, 0x4022140000ULL, 0x244280000ULL, 0x20408500000ULL, 0x2040810200000ULL, 0x4081020400000ULL,
   0x10080402000200ULL, 0x20100804000400ULL, 0x4020100a000a00ULL, 0x402214001400ULL, 0x24428002800ULL, 0x2040850005000ULL, 0x4081020002000ULL, 0x8102040004000ULL,
   0x8040200020400ULL, 0x10080400040800ULL, 0x20100a000a1000ULL, 0x40221400142200ULL, 0x2442800284400ULL, 0x4085000500800ULL, 0x8102000201000ULL, 0x10204000402000ULL,
   0x4020002040800ULL, 0x8040004081000ULL, 0x100a000a102000ULL, 0x22140014224000ULL, 0x44280028440200ULL, 0x8500050080400ULL, 0x10200020100800ULL, 0x20400040201000ULL,
   0x2000204081000ULL, 0x4000408102000ULL, 0xa000a10204000ULL, 0x14001422400000ULL, 0x28002844020000ULL, 0x50005008040200ULL, 0x20002010080400ULL, 0x40004020100800ULL,
   0x20408102000ULL, 0x40810204000ULL, 0xa1020400000ULL, 0x142240000000ULL, 0x284402000000ULL, 0x500804020000ULL, 0x201008040200ULL, 0x402010080400ULL,
   0x2040810204000ULL, 0x4081020400000ULL, 0xa102040000000ULL, 0x14224000000000ULL, 0x28440200000000ULL, 0x50080402000000ULL, 0x20100804020000ULL, 0x40201008040200ULL,
    };

    const std::array<Bitboard, 64> KingBitboards = {
   0x302ULL, 0x705ULL, 0xe0aULL, 0x1c14ULL, 0x3828ULL, 0x7050ULL, 0xe0a0ULL, 0xc040ULL,
   0x30203ULL, 0x70507ULL, 0xe0a0eULL, 0x1c141cULL, 0x382838ULL, 0x705070ULL, 0xe0a0e0ULL, 0xc040c0ULL,
   0x3020300ULL, 0x7050700ULL, 0xe0a0e00ULL, 0x1c141c00ULL, 0x38283800ULL, 0x70507000ULL, 0xe0a0e000ULL, 0xc040c000ULL,
   0x302030000ULL, 0x705070000ULL, 0xe0a0e0000ULL, 0x1c141c0000ULL, 0x3828380000ULL, 0x7050700000ULL, 0xe0a0e00000ULL, 0xc040c00000ULL,
   0x30203000000ULL, 0x70507000000ULL, 0xe0a0e000000ULL, 0x1c141c000000ULL, 0x382838000000ULL, 0x705070000000ULL, 0xe0a0e0000000ULL, 0xc040c0000000ULL,
   0x3020300000000ULL, 0x7050700000000ULL, 0xe0a0e00000000ULL, 0x1c141c00000000ULL, 0x38283800000000ULL, 0x70507000000000ULL, 0xe0a0e000000000ULL, 0xc040c000000000ULL,
   0x302030000000000ULL, 0x705070000000000ULL, 0xe0a0e0000000000ULL, 0x1c141c0000000000ULL, 0x3828380000000000ULL, 0x7050700000000000ULL, 0xe0a0e00000000000ULL, 0xc040c00000000000ULL,
   0x203000000000000ULL, 0x507000000000000ULL, 0xa0e000000000000ULL, 0x141c000000000000ULL, 0x2838000000000000ULL, 0x5070000000000000ULL, 0xa0e0000000000000ULL, 0x40c0000000000000ULL,
    };
    const std::array<Bitboard, 64> KnightBitboards = {
   0x20400ULL, 0x50800ULL, 0xa1100ULL, 0x142200ULL, 0x284400ULL, 0x508800ULL, 0xa01000ULL, 0x402000ULL,
   0x2040004ULL, 0x5080008ULL, 0xa110011ULL, 0x14220022ULL, 0x28440044ULL, 0x50880088ULL, 0xa0100010ULL, 0x40200020ULL,
   0x204000402ULL, 0x508000805ULL, 0xa1100110aULL, 0x1422002214ULL, 0x2844004428ULL, 0x5088008850ULL, 0xa0100010a0ULL, 0x4020002040ULL,
   0x20400040200ULL, 0x50800080500ULL, 0xa1100110a00ULL, 0x142200221400ULL, 0x284400442800ULL, 0x508800885000ULL, 0xa0100010a000ULL, 0x402000204000ULL,
   0x2040004020000ULL, 0x5080008050000ULL, 0xa1100110a0000ULL, 0x14220022140000ULL, 0x28440044280000ULL, 0x50880088500000ULL, 0xa0100010a00000ULL, 0x40200020400000ULL,
   0x204000402000000ULL, 0x508000805000000ULL, 0xa1100110a000000ULL, 0x1422002214000000ULL, 0x2844004428000000ULL, 0x5088008850000000ULL, 0xa0100010a0000000ULL, 0x4020002040000000ULL,
   0x400040200000000ULL, 0x800080500000000ULL, 0x1100110a00000000ULL, 0x2200221400000000ULL, 0x4400442800000000ULL, 0x8800885000000000ULL, 0x100010a000000000ULL, 0x2000204000000000ULL,
   0x4020000000000ULL, 0x8050000000000ULL, 0x110a0000000000ULL, 0x22140000000000ULL, 0x44280000000000ULL, 0x88500000000000ULL, 0x10a00000000000ULL, 0x20400000000000ULL,
    };

    std::array<std::array<Bitboard, 4096>, 64> RookMagicBitboards;
    std::array<std::array<Bitboard, 512>, 64> BishopMagicBitboards;

    std::array<std::array<std::array<uint64_t, 64>, 14>, 2> zobristTable;
    uint64_t zobristTurn;

    std::array<std::array<Bitboard, 64>, 64> BlockCheckPath;


std::vector<Bitboard> generateRookBlockersPermutations(int pos) {
    std::vector<Bitboard> blockerSets;
    Bitboard bb = RawRookAttacks[pos];
    unsigned __int64 bitCount = __popcnt64(bb);
    for (int mask = 0; mask < (1 << bitCount); mask++) {
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
        blockerSets.push_back(blocks);
    }
    return blockerSets;
}
std::vector<Bitboard> generateBishopBlockersPermutations(int pos) {
    std::vector<Bitboard> blockerSets;
    Bitboard bb = RawBishopAttacks[pos];
    unsigned __int64 bitCount = __popcnt64(bb);
    for (int mask = 0; mask < (1 << bitCount); mask++) {
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
        blockerSets.push_back(blocks);
    }
    return blockerSets;
}
Bitboard generateRookAttacksForBlockers(Bitboard blockers, int pos) {
    Bitboard attacks = 0x0ULL;
    Bitboard index = bit << pos;
    while (true) {
        attacks |= index;
        if ((blockers | eightRank) & index) break;
        index <<= 8;
    }
    index = bit << pos;
    while (true) {
        attacks |= index;
        if ((blockers | firstRank) & index) break;
        index >>= 8;
    }
    index = bit << pos;
    while (true) {
        attacks |= index;
        if ((blockers | hFile) & index) break;
        index <<= 1;
    }
    index = bit << pos;
    while (true) {
        attacks |= index;
        if ((blockers | aFile) & index) break;
        index >>= 1;
    }
    return (attacks & ~(bit << pos));
}
Bitboard generateBishopAttacksForBlockers(Bitboard blockers, int pos) {
    Bitboard attacks = 0x0ULL;
    Bitboard index = bit << pos;
    while (true) {
        attacks |= index;
        if ((blockers | eightRank | aFile) & index) break;
        index <<= 7;
    }
    index = bit << pos;
    while (true) {
        attacks |= index;
        if ((blockers | eightRank | hFile) & index) break;
        index <<= 9;
    }
    index = bit << pos;
    while (true) {
        attacks |= index;
        if ((blockers | firstRank | hFile) & index) break;
        index >>= 7;
    }
    index = bit << pos;
    while (true) {
        attacks |= index;
        if ((blockers | firstRank | aFile) & index) break;
        index >>= 9;
    }
    return (attacks & ~(bit << pos));
}

void MagicSetup() {
    for (int pos = 0; pos < 64; pos++) {
        std::vector<Bitboard> RookBlockerSets = generateRookBlockersPermutations(pos);
        std::vector<Bitboard> BishopBlockerSets = generateBishopBlockersPermutations(pos);
        int index;
        for (Bitboard set : RookBlockerSets) {
            index = set * RookMagics[pos] >> RookShifts[pos];
            RookMagicBitboards[pos][index] = (generateRookAttacksForBlockers(set, pos));
        }
        for (Bitboard set : BishopBlockerSets) {
            index = set * BishopMagics[pos] >> BishopShifts[pos];
            BishopMagicBitboards[pos][index] = (generateBishopAttacksForBlockers(set, pos));
        }
    }
}
void BlockCheckSetup() {
    Bitboard bb;
    Bitboard kingMoves, checkMoves;
    for (int king = 0; king < 64; king++) {
        for (int check = 0; check < 64; check++) {
            checkMoves = RookMagicBitboards[check][0];
            if (checkMoves & (bit << king)) {
                kingMoves = RookMagicBitboards[king][((bit << check) & RawRookAttacks[king]) * RookMagics[king] >> RookShifts[king]];
                checkMoves = RookMagicBitboards[check][((bit << king) & RawRookAttacks[check]) * RookMagics[check] >> RookShifts[check]];
                bb = kingMoves & checkMoves | (bit << check);
            }
            else {
                checkMoves = BishopMagicBitboards[check][0];
                if (checkMoves & (bit << king)) {
                    kingMoves = BishopMagicBitboards[king][((bit << check) & RawBishopAttacks[king]) * BishopMagics[king] >> BishopShifts[king]];
                    checkMoves = BishopMagicBitboards[check][((bit << king) & RawBishopAttacks[check]) * BishopMagics[check] >> BishopShifts[check]];
                    bb = kingMoves & checkMoves | (bit << check);
                }
                else bb = (bit << check);
            }
            BlockCheckPath[king][check] = bb;
        }
    }
}
void ZobristSetup() {
    std::random_device rd;
    std::mt19937_64 rng(rd());
    std::uniform_int_distribution<uint64_t> dist;

    for (int i = 0; i < 14; ++i) {
        for (int j = 0; j < 64; ++j) {
            zobristTable[0][i][j] = dist(rng);
            zobristTable[1][i][j] = dist(rng);
        }
    }
    zobristTurn = dist(rng);
}
