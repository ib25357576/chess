#include "DepthLastTT.h"

void DepthLastTT::store(uint64_t key, SearchResult result, EntryType type, int depth, int ageOnStarted, int ageCurrent, uint64_t subtreeSize, int irreversibleNumCurrent) {
    if (result.eval > 30000 || result.eval < -30000) return;

    //size_t index = key % TTsize;
    size_t index = key & (TTsize - 1);
    ExtTTEntry& entry = table[index];

    if ((entry.key == 0 || entry.ext < lastIrreversible || entry.depth <= depth) && !result.bestLine.empty()) {
        if (entry.key == 0) stored++;
        else {
            overriten++;
            if (entry.key == key) overwritenWithDiff++;
        }
        entry.key = key;
        entry.depth = depth;
        entry.eval = result.eval;
        entry.type = type;
        entry.ext = ageCurrent;
        entry.bestMove = result.bestLine[result.bestLine.size() - 1];
    }
    else {
        refused++;
    }
}

bool DepthLastTT::retrieve(uint64_t key, TTEntry& entry, bool retrieveOnlyExact) {
    //size_t index = key % TTsize;
    size_t index = key & (TTsize - 1);
    const TTEntry& storedEntry = table[index];

    if (storedEntry.key == key && (!retrieveOnlyExact || storedEntry.type == EXACT)) {
        retrieved++;
        entry = storedEntry;
        return true;
    }
    not_retrieved++;
    return false;
}