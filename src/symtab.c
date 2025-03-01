#include "symtab.h"
#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Scope current_scope = NULL;
Scope scope_list    = NULL;

static int hash(char* key) {
	int temp = 0;
	int i    = 0;
	while (key[i] != '\0') {
		temp = ((temp << SHIFT) + key[i]) % HASH_SIZE;
		++i;
	}
	return temp;
}

void st_insert(char* name, StmtKind kind, char* scope, ExpType type, int lineno, int memloc,
               bool isArray) {
	int        h      = hash(name);
	SymbolList symbol = current_scope->hashTable[h];

	while (symbol && strcmp(name, symbol->name) != 0) symbol = symbol->next;

	if (symbol == NULL) {
		symbol                      = (SymbolList) malloc(sizeof(struct SymbolList));
		symbol->name                = strdup(name);
		symbol->lines               = (LineList) malloc(sizeof(struct LineList));
		symbol->lines->lineno       = lineno;
		symbol->lines->next         = NULL;
		symbol->memloc              = memloc;
		symbol->type                = type;
		symbol->kind                = kind;
		symbol->isArray             = isArray;
		symbol->scope               = strdup(scope);
		symbol->next                = current_scope->hashTable[h];
		current_scope->hashTable[h] = symbol;
	} else {
		st_add_lineno(name, lineno);
	}
}

void st_add_lineno(char* name, int lineno) {
	SymbolList symbol = st_lookup(name);

	if (symbol != NULL) {
		LineList line_list = symbol->lines;
		LineList prev      = NULL;

		while (line_list != NULL) {
			if (line_list->lineno == lineno) {
				return;
			}
			prev      = line_list;
			line_list = line_list->next;
		}

		LineList new = malloc(sizeof(struct LineList));
		new->lineno  = lineno;
		new->next    = NULL;
		prev->next   = new;
	}
}

SymbolList st_lookup(char* name) {
	if (name == NULL) {
		return NULL;
	}
	Scope sc = current_scope;
	while (sc != NULL) {
		int        h      = hash(name);
		SymbolList symbol = sc->hashTable[h];

		while (symbol != NULL && strcmp(name, symbol->name) != 0) {
			symbol = symbol->next;
		}

		if (symbol != NULL) {
			return symbol;
		}
		sc = sc->parent;
	}
	return NULL;
}

SymbolList st_lookup_no_parent(char* name) {
	int        h      = hash(name);
	SymbolList symbol = current_scope->hashTable[h];

	while (symbol != NULL && strcmp(name, symbol->name) != 0) {
		symbol = symbol->next;
	}
	return symbol;
}

SymbolList st_lookup_from_given_scope(char* name, Scope scope) {
	if (name == NULL) {
		return NULL;
	}

	while (scope != NULL) {
		int        h      = hash(name);
		SymbolList symbol = scope->hashTable[h];

		while (symbol != NULL && strcmp(name, symbol->name) != 0) {
			symbol = symbol->next;
		}

		if (symbol != NULL) {
			return symbol;
		}
		scope = scope->parent;
	}
	return NULL;
}

static const char* getStmtKindString(StmtKind kind) {
	switch (kind) {
		case FunK:
			return "fun";
		case VarK:
			return "var";
		case VetK:
			return "array";
		case ParamK:
			return "var";
		case ParamVetK:
			return "array";
		case CompoundK:
			return "compound";
		default:
			return "unknown";
	}
}

static const char* getExpTypeString(ExpType type) {
	switch (type) {
		case Void:
			return "void";
		case Integer:
			return "int";
		default:
			return "unknown";
	}
}

// TODO: refactor everything
void printSymTab(void) {
	// pc("Variable Name  Scope     ID Type  Data Type  Line Numbers\n");
	// pc("-------------  --------  -------  ---------  -------------------------\n");

	// Scope list = scope_list;
	// for (int i = 0; i < HASH_SIZE; i++) {
	// 	SymbolList s = list->hashTable[i];
	// 	while (list != NULL) {
	// 		LineList lines = l->lines;
	// 		pc("%-14s ", l->name);

	// 		char scopeName[MAX_SCOPE_LENGTH];
	// 		generateScopeName(scopeName, &l->scope);
	// 		pc("%-9s ", scopeName);

	// 		pc("%-8s ", getStmtKindString(l->idType));
	// 		pc("%-9s  ", getExpTypeString(l->dataType));

	// 		while (lines != NULL) {
	// 			if (lines->lineNo != 0) {
	// 				pc("%2d ", lines->lineNo);
	// 			}
	// 			lines = lines->next;
	// 		}
	// 		pc("\n");
	// 		l = l->next;
	// 	}
	// }
}