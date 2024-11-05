
#include "Move.h"
#include <iostream>
#include <array>

constexpr std::array<int, 7> promotionOrder = {
    0, 3, 1, 2, 5, 0, 4
};

    Move::Move(uint8_t e, uint8_t s, pieceType cap, pieceType prom, bool cas, bool dp, bool enP, bool db_ch, Bitboard ch_from)
        : end(e), start(s), castle(cas), checks(ch_from), doublePush(dp), enPassant(enP), capture(cap), promotion(prom), doubleCheck(db_ch) {
        wOOch = false; wOOOch = false; bOOch = false; bOOOch = false; // store whether castle rights changed. are modified in doMove
        lastEnPassantTargetIndex = 64;
        lastCheckedFrom = 0x0ULL;
        lastInDoubleCheck = false;
    };
    Move::Move() : Move(64, 64, EMPTY, EMPTY, false, false, false, false, 0) {};

    bool Move::operator==(const Move& other) const {
        return (start == other.start) && (end == other.end) && (checks == other.checks) && (doubleCheck == other.doubleCheck) && (capture == other.capture);
    }

    bool Move::operator>(const Move& other) const {

        if (doubleCheck != other.doubleCheck) {
            return doubleCheck < other.doubleCheck;
        }
        if (promotion != other.promotion) {
            return promotionOrder[promotion] < promotionOrder[other.promotion];
        }
        if ((checks > 0) != (other.checks > 0)) {
            return (checks > 0) < (other.checks > 0);
        }
        if (capture != other.capture) {
            return ((capture + 1) % 7) < ((other.capture + 1) % 7);
        }

        return false;
    }
    
    bool Move::operator<(const Move& other) const {
        return other > *this;
    }

