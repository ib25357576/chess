#pragma once
#include "TranspositionTable.h"
#include "Header.h"
#include<vector>

struct TTCollection : TranspositionTable
{
	TTCollection(TTType better, TTType worse);
	TranspositionTable* better;
	TranspositionTable* worse;
	long betterRetrieve =0;
	int worseRetrieve =0;
	long equal =0;

	void updateLast(int irr) override;
	void updateNum(int irr) override;
	void store(uint64_t key, SearchResult result, EntryType type, int depth, int ageOnStarted, int ageCurrent, uint64_t subtreeSize, int irreversibleNumCurrent) override;
	bool retrieve(uint64_t key, TTEntry& entry, bool retrieveOnlyExact) override;
};

