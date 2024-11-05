#pragma once
#include "Header.h"
#include "TranspositionTable.h"
#include<vector>
#include<array>


extern const std::array<int, 64> RookShifts;
extern const std::array<int, 64> BishopShifts;

extern const std::array<Bitboard, 64> RookMagics;
extern const std::array<Bitboard, 64> BishopMagics;

// raw attacks dont include last possible move/capture on the line
extern const std::array<Bitboard, 64> RawRookAttacks;
extern const std::array<Bitboard, 64> RawBishopAttacks;

extern const std::array<Bitboard, 64> KingBitboards;
extern const std::array<Bitboard, 64> KnightBitboards;

extern std::array<std::array<Bitboard, 4096>, 64> RookMagicBitboards;
extern std::array<std::array<Bitboard, 512>, 64> BishopMagicBitboards;

extern std::array<std::array<std::array<uint64_t, 64>, 14>, 2> zobristTable;
extern uint64_t zobristTurn;

extern std::array<std::array<Bitboard, 64>, 64> BlockCheckPath;

std::vector<Bitboard> generateRookBlockersPermutations(int pos);
std::vector<Bitboard> generateBishopBlockersPermutations(int pos);
Bitboard generateRookAttacksForBlockers(Bitboard blockers, int pos);
Bitboard generateBishopAttacksForBlockers(Bitboard blockers, int pos);

void MagicSetup();
void BlockCheckSetup();
void ZobristSetup();