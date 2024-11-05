#pragma once
#include <iostream>
#include "Header.h"
#include "Move.h"

struct TTEntry {
    TTEntry();
    uint64_t key;
    int depth;
    int eval;
    EntryType type;
    Move bestMove;
};

