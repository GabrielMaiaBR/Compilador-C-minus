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
		temp = ((temp << SHIFT) + key[i]) % SIZE;
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

void printSymTab(void) {
    pc("Variable Name  Scope     ID Type  Data Type  Line Numbers\n");
    pc("-------------  --------  -------  ---------  -------------------------\n");

    for (Scope sc = scope_list; sc != NULL; sc = sc->next) {
        for (int i = 0; i < SIZE; i++) {
            SymbolList sym = sc->hashTable[i];
            while (sym != NULL) {
                pc("%-14s ", sym->name);
                
                if (strcmp(sym->scope, "global") == 0)
                    pc("%-10s ", "");
                else
                    pc("%-10s ", sym->scope);
                
                char idType[10];
                if (sym->kind == FunK)
                    strcpy(idType, "fun");
                else if (sym->kind == VetK || sym->kind == ParamVetK)
                    strcpy(idType, "array");
                else if (sym->kind == VarK || sym->kind == ParamK)
                    strcpy(idType, (sym->isArray ? "array" : "var"));
                else
                    strcpy(idType, "unknown");
                pc("%-7s ", idType);
                
                pc("%-9s  ", (sym->type == Integer ? "int" : "void"));

                LineList lines = sym->lines;
                while (lines != NULL) {
                    if (lines->lineno > 0)
                        pc("%2d ", lines->lineno);
                    lines = lines->next;
                }
                pc("\n");
                sym = sym->next;
            }
        }
    }
}

