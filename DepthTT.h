#pragma once
#include "TranspositionTable.h"
#include "TTEntry.h"

struct DepthTT : TranspositionTable
{
	std::array<TTEntry, TTsize> table;

	void store(uint64_t key, SearchResult result, EntryType type, int depth, int ageOnStarted, int ageCurrent, uint64_t subtreeSize, int irreversibleNum) override;
	bool retrieve(uint64_t key, TTEntry& entry, bool retrieveOnlyExact) override;
};

