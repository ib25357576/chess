#include "TTEntry.h"

TTEntry::TTEntry() {
	key = 0;
	eval = 0;
	depth = 0;
	type = EXACT;
	bestMove = Move();
}