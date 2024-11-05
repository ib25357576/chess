#pragma once
#include "Header.h"
#include "TTEntry.h"
#include "SearchResult.h"
#include <array>

struct TranspositionTable
{
	uint64_t stored = 0;
	uint64_t overriten = 0;
	uint64_t overwritenWithDiff = 0;
	uint64_t refused = 0;
	uint64_t retrieved = 0;
	uint64_t not_retrieved = 0;

    virtual void store(uint64_t key, SearchResult result, EntryType type, int depth, int ageOnStarted, int ageCurrent, uint64_t subtreeSize, int irreversibleNumCurrent) = 0;
	virtual bool retrieve(uint64_t key, TTEntry& entry, bool retrieveOnlyExact) = 0;

	virtual void updateLast(int last) {
		lastIrreversible = last;
	}
	virtual void updateNum(int num) {
		irreversibleNumber = num;
	}
	int getLast() {
		return lastIrreversible;
	}
	int getNum() {
		return irreversibleNumber;
	}

protected:
	int lastIrreversible;
	int irreversibleNumber;
};
