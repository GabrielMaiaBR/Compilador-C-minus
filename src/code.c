#include "code.h"
#include "globals.h"
#include "log.h"

/* TM location number for current instruction emission */
static int emitLoc     = 0;
static int highEmitLoc = 0;

void emitComment(char* c) {
	if (TraceCode) pc("* %s\n", c);
}

void emitRO(char* op, int r, int s, int t, char* c) {
	pc("%3d:  %5s  %d,%d,%d ", emitLoc++, op, r, s, t);
	if (TraceCode) pc("\t%s", c);
	pc("\n");
	if (highEmitLoc < emitLoc) highEmitLoc = emitLoc;
}

void emitRM(char* op, int r, int d, int s, char* c) {
	pc("%3d:  %5s  %d,%d(%d) ", emitLoc++, op, r, d, s);
	if (TraceCode) pc("\t%s", c);
	pc("\n");
	if (highEmitLoc < emitLoc) highEmitLoc = emitLoc;
}

int emitSkip(int howMany) {
	int i = emitLoc;
	emitLoc += howMany;
	if (highEmitLoc < emitLoc) highEmitLoc = emitLoc;
	return i;
}

void emitBackup(int loc) {
	if (loc > highEmitLoc) emitComment("BUG in emitBackup");
	emitLoc = loc;
}

void emitRestore(void) {
	emitLoc = highEmitLoc;
}

void emitRM_Abs(char* op, int r, int a, char* c) {
	pc("%3d:  %5s  %d,%d(%d) ", emitLoc, op, r, a - (emitLoc + 1), PC);
	++emitLoc;
	if (TraceCode) pc("\t%s", c);
	pc("\n");
	if (highEmitLoc < emitLoc) highEmitLoc = emitLoc;
}
