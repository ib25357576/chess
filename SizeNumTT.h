#pragma once
#include "TranspositionTable.h"
#include "SizedExtTTEntry.h"

struct SizeNumTT : TranspositionTable
{
	std::array<SizedExtTTEntry, TTsize> table; // here ext acts like current number of irreversibles

	void store(uint64_t key, SearchResult result, EntryType type, int depth, int ageOnStarted, int ageCurrent, uint64_t subtreeSize, int irreversibleNumCurrent) override;
	bool retrieve(uint64_t key, TTEntry& entry, bool retrieveOnlyExact) override;
};
