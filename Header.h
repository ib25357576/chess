#pragma once
#include<iostream>
#include<array>

enum pieceType : uint8_t {
    PAWN = 0, KNIGHT = 1, BISHOP = 2, ROOK = 3, QUEEN = 4, KING = 5, EMPTY = 6
};

enum EntryType : uint8_t {
    EXACT, LOWER, UPPER
};

enum TTType {
    DEPTH, AGE, SIZE, DEPTH_LAST, DEPTH_NUM, SIZE_LAST, SIZE_NUM, COLLECTION
};

typedef uint64_t Bitboard;

const Bitboard bit = 0x1ULL;
const Bitboard aFile = 0x101010101010101ULL;
const Bitboard not_aFile = 0xfefefefefefefefeULL;
const Bitboard hFile = 0x08080808080808080ULL;
const Bitboard not_hFile = 0x7f7f7f7f7f7f7f7fULL;
const Bitboard firstRank = 0xFFULL;
const Bitboard not_firstRank = 0xffffffffffffff00ULL;
const Bitboard secondRank = 0xFF00ULL;
const Bitboard not_secondRank = 0xffffffffffff00ffULL;
const Bitboard sevenRank = 0xFF000000000000ULL;
const Bitboard not_sevenRank = 0xff00ffffffffffffULL;
const Bitboard eightRank = 0xFF00000000000000ULL;
const Bitboard not_eightRank = 0xffffffffffffffULL;

const Bitboard wOOPlaces = 0x0000000000000060ULL;
const Bitboard bOOPlaces = 0x6000000000000000ULL;
const Bitboard wOOOPlaces = 0x000000000000000CULL;
const Bitboard bOOOPlaces = 0x0C00000000000000ULL;
const Bitboard wOOOPlacesExt = 0x000000000000000EULL;
const Bitboard bOOOPlacesExt = 0x0E00000000000000ULL;

constexpr size_t TTsize = (1 << 20);
