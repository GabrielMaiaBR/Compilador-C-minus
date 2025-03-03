/****************************************************/
/* File: symtab.h                                   */
/* Symbol table interface for CMINUS compiler       */
/* Symbol table implemented as a chained hash table */
/****************************************************/

#ifndef _SYMTAB_H_
#define _SYMTAB_H_

#include "globals.h"
#include "log.h"
#include <stdbool.h>

extern Scope current_scope;
extern Scope scope_list;

void st_insert(char* name, StmtKind kind, char* scope, ExpType type, int lineno, int memloc, bool isArray);
SymbolList st_lookup(char* name);
SymbolList st_lookup_no_parent(char* name);
SymbolList st_lookup_from_given_scope(char* name, Scope scope);
void st_add_lineno(char* name, int lineno);

void printSymTab(void);

#endif
