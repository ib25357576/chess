#pragma once
#include "Header.h"

struct Move {
    uint8_t end, start;
    Bitboard checks;
    bool castle, doublePush, enPassant, doubleCheck;
    pieceType capture, promotion;
    bool wOOch; bool wOOOch; bool bOOch; bool bOOOch; 
    uint8_t lastEnPassantTargetIndex = 64;
    Bitboard lastCheckedFrom = 0x0ULL;
    bool lastInDoubleCheck = false;
    uint8_t lastPly50 = 0;
    Move(uint8_t e, uint8_t s, pieceType cap, pieceType prom, bool cas, bool dp, bool enP, bool db_ch, Bitboard ch_from);
    Move();
    bool operator==(const Move& other) const;
    bool operator>(const Move& other) const;
    bool operator<(const Move& other) const;
};