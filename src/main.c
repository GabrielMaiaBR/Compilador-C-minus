#include "globals.h"
#include "log.h"

/* set NO_PARSE to TRUE to get a scanner-only compiler */
#define NO_PARSE FALSE
/* set NO_ANALYZE to TRUE to get a parser-only compiler */
#define NO_ANALYZE FALSE

/* set NO_CODE to TRUE to get a compiler that does not
 * generate code
 */
#define NO_CODE TRUE

#include "util.h"
#if NO_PARSE
#include "scan.h"
#else
#include "parse.h"
#if !NO_ANALYZE
#include "analyze.h"
#if !NO_CODE
#include "cgen.h"
#endif
#endif
#endif

/* allocate global variables */
int   lineno = 0;
FILE* source;
FILE* listing;
FILE* code;
FILE* redundant_source;

/* allocate and set tracing flags */
int EchoSource   = TRUE;
int TraceScan    = TRUE;
int TraceParse   = TRUE;
int TraceAnalyze = TRUE;
int TraceCode    = FALSE;

int Error = FALSE;

int main(int argc, char* argv[]) {
	TreeNode* syntaxTree;

	char pgm[120]; /* source code file name */
	if ((argc < 2) || (argc > 3)) {
		fprintf(stderr, "usage: %s <filename> [<detailpath>]\n", argv[0]);
		exit(1);
	}
	strcpy(pgm, argv[1]);
	if (strchr(pgm, '.') == NULL) strcat(pgm, ".cm");
	source = fopen(pgm, "r");
	if (source == NULL) {
		fprintf(stderr, "File %s not found\n", pgm);
		exit(1);
	}

	redundant_source = fopen(pgm, "r");
	if (redundant_source == NULL) {
		fprintf(stderr, "File %s not found (redundant)\n", pgm);
		exit(1);
	}

	char detailpath[200];
	if (3 == argc) {
		strcpy(detailpath, argv[2]);
	} else
		strcpy(detailpath, "/tmp/");

	listing = stdout;
	initializePrinter(detailpath, pgm, LOGALL);

	fprintf(listing, "\nTINY COMPILATION: %s\n", pgm);
#if NO_PARSE
	while (getToken() != ENDFILE);
#else
	syntaxTree = parse();
	doneLEXstartSYN();
	if (TraceParse) {
		fprintf(listing, "\nSyntax tree:\n");
		printTree(syntaxTree);
	}
#if !NO_ANALYZE
	doneSYNstartTAB();
	if (!Error) {
		if (TraceAnalyze) fprintf(listing, "\nBuilding Symbol Table...\n");
		buildSymtab(syntaxTree);
		if (TraceAnalyze) fprintf(listing, "\nChecking Types...\n");
		typeCheck(syntaxTree);
		if (TraceAnalyze) fprintf(listing, "\nType Checking Finished\n");
	}
#if !NO_CODE
	doneTABstartGEN();
	if (!Error) {
		codeGen(syntaxTree);
	}
#endif
#endif
#endif
	fclose(source);
	closePrinter();
	return 0;
}