#pragma once
#include "TranspositionTable.h"
#include "ExtTTEntry.h"

struct AgedTT : TranspositionTable
{
	std::array<ExtTTEntry, TTsize> table; // here ext acts like ageOnStarted

	void store(uint64_t key, SearchResult result, EntryType type, int depth, int ageOnStarted, int ageCurrent, uint64_t subtreeSize, int irreversibleNum) override;
	bool retrieve(uint64_t key, TTEntry& entry, bool retrieveOnlyExact) override;

};

