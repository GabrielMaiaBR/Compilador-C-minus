#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#define MAXRESERVED 8
#define MAX_MEM 1023
#define HASH_SIZE 211
#define SHIFT 4

#define MAXCHILDREN 3

#define ac 0
#define ac1 1
#define fp 2
#define r3 3
#define r4 4
#define r5 5
#define mp 6
#define PC 7

typedef int TokenType;

extern FILE* source;
extern FILE* listing;
extern FILE* code;
extern FILE* redundant_source;

extern int lineno;

typedef enum { StmtK, ExpK } NodeKind;
typedef enum { FunK, VarK, VetK, ParamK, ParamVetK, IfK, IteraK, ReturnK, CompoundK } StmtKind;
typedef enum { OpK, ConstK, IdK, AssignK, FunCallK, UnaryK } ExpKind;

typedef enum { Void, Integer } ExpType;

typedef struct LineList {
	int              lineno;
	struct LineList* next;
}* LineList;

typedef struct SymbolList {
	char*              name;
	LineList           lines;
	int                memloc;
	ExpType            type;
	StmtKind           kind;
	bool               isArray;
	char*              scope;
	struct SymbolList* next;
}* SymbolList;

typedef struct Scope {
	char*         name;
	struct Scope* parent;
	struct Scope* next;
	SymbolList    hashTable[HASH_SIZE];
}* Scope;

typedef struct treeNode {
	struct treeNode* child[MAXCHILDREN];
	struct treeNode* sibling;
	struct treeNode* parent;
	int              lineno;
	NodeKind         nodekind;
	union {
		StmtKind stmt;
		ExpKind  exp;
	} kind;
	union {
		TokenType op;
		int       val;
		char*     name;
	} attr;
	ExpType type;
	Scope   scope;
	bool    isArray;
} TreeNode;

#ifndef YYPARSER
#include "parser.h"
#define ENDFILE 0
#endif

extern int EchoSource;
extern int TraceScan;
extern int TraceParse;
extern int TraceAnalyze;
extern int TraceCode;
extern int Error;

#endif
