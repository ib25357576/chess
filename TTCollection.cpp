#include "TTCollection.h"
#include "AgedTT.h"
#include "DepthTT.h"
#include "SizeTT.h"
#include "DepthLastTT.h"
#include "SizeLastTT.h"
#include "DepthNumTT.h"
#include "SizeNumTT.h"

TTCollection::TTCollection(TTType better, TTType worse) {
	switch (better)
	{
	case AGE:
		this->better = new AgedTT();
		break;
	case DEPTH:
		this->better = new DepthTT();
		break;
	case DEPTH_LAST:
		this->better = new DepthLastTT();
		break;
	case DEPTH_NUM:
		this->better = new DepthNumTT();
		break;
	case SIZE:
		this->better = new SizeTT();
		break;
	case SIZE_LAST:
		this->better = new SizeLastTT();
		break;
	case SIZE_NUM:
		this->better = new SizeNumTT();
		break;
	default:
		this->better = new SizeLastTT();
		break;
	}
	switch (worse)
	{
	case AGE:
		this->worse = new AgedTT();
		break;
	case DEPTH:
		this->worse = new DepthTT();
		break;
	case DEPTH_LAST:
		this->worse = new DepthLastTT();
		break;
	case DEPTH_NUM:
		this->worse = new DepthNumTT();
		break;
	case SIZE:
		this->worse = new SizeTT();
		break;
	case SIZE_LAST:
		this->worse = new SizeLastTT();
		break;
	case SIZE_NUM:
		this->worse = new SizeNumTT();
		break;
	default:
		this->worse = new SizeLastTT();
		break;
	}
}

void TTCollection::store(uint64_t key, SearchResult result, EntryType type, int depth, int ageOnStarted, int ageCurrent, uint64_t subtreeSize, int irreversibleNumCurrent) {
	uint64_t worseStart = worse->overriten + worse->stored;
	uint64_t betterStart = better->overriten + worse->stored;
	uint64_t worseEnd = 0;
	uint64_t betterEnd = 0;
	better->store(key, result, type, depth, ageOnStarted, ageCurrent, subtreeSize, irreversibleNumCurrent);
	betterEnd = better->overriten + better->stored;
 	worse->store(key, result, type, depth, ageOnStarted, ageCurrent, subtreeSize, irreversibleNumCurrent);
	worseEnd = worse->overriten + worse->stored;
	if ((betterEnd - betterStart) < (worseEnd - worseStart)) {
		std::cout << "unexpected store tt result\t better " << (betterEnd - betterStart) << ", worse " << (worseEnd - worseStart) << "\n";
	}
}

bool TTCollection::retrieve(uint64_t key, TTEntry& entry, bool retrieveOnlyExact) {
	TTEntry worseEntry, betterEntry;
	bool worseSucc = worse->retrieve(key, worseEntry, retrieveOnlyExact);
	bool betterSucc = better->retrieve(key, betterEntry, retrieveOnlyExact);
	if (worseSucc && !betterSucc) {
		std::cout << "unexpected tt retrieve result" << "\n";
		worseRetrieve++;
		entry = worseEntry;
		return true;
	}
	if (betterSucc && !worseSucc) {
		entry = betterEntry;
		//std::cout << "better\n";
		betterRetrieve++;
		return true;
	}
	if (betterSucc && worseSucc) {
		equal++;
		entry = betterEntry;
		return true;
	}
    return false;
}

void TTCollection::updateLast(int irr) {
	lastIrreversible = irr;
	better->updateLast(irr);
	worse->updateLast(irr);
}

void TTCollection::updateNum(int irr) {
	irreversibleNumber = irr;
	better->updateNum(irr);
	worse->updateNum(irr);
}