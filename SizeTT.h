#pragma once
#include "TranspositionTable.h"
#include "SizedTTEntry.h"

struct SizeTT : TranspositionTable
{
	std::array<SizedTTEntry, TTsize> table; 

	void store(uint64_t key, SearchResult result, EntryType type, int depth, int ageOnStarted, int ageCurrent, uint64_t subtreeSize, int irreversibleNum) override;
	bool retrieve(uint64_t key, TTEntry& entry, bool retrieveOnlyExact) override;
};
