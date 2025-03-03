#include "analyze.h"
#include "globals.h"
#include "log.h"
#include "symtab.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

bool mainIsDefined = FALSE;

bool fromFunction = FALSE;

int localoffset = MAX_MEM - 2;

int globaloffset = 0;

static int CompoundScopesNum = 0;

static int location = 0;
// ok
void enterNewScope(char* scopeName) {
	Scope newScope   = malloc(sizeof(struct Scope));
	newScope->parent = current_scope;
	newScope->name   = strdup(scopeName);
	newScope->next   = NULL;
	for (int i = 0; i < HASH_SIZE; i++) {
		newScope->hashTable[i] = NULL;
	}
	if (scope_list == NULL) {
		scope_list = newScope;
	} else {
		Scope temp = scope_list;
		while (temp->next != NULL) {
			temp = temp->next;
		}
		temp->next = newScope;
	}
	current_scope = newScope;
}

// ok
void exitScope(TreeNode* tree) {
	if (tree->nodekind == StmtK) {
		if (tree->kind.stmt == CompoundK) {
			current_scope = current_scope->parent;
		} else if (tree->kind.stmt == FunK && current_scope->parent != NULL) {
			current_scope = current_scope->parent;
		}
	}
}

static void semanticError(TreeNode* node, const char* format, ...) {
	va_list args;
	va_start(args, format);
	char errorMessage[256];
	vsnprintf(errorMessage, sizeof(errorMessage), format, args);
	pce("Semantic error at line %d: %s\n", node->lineno, errorMessage);
	va_end(args);
	Error = TRUE;
}

// ok
static void traverse(TreeNode* tree, void (*preProcess)(TreeNode*),
                     void (*postProcess)(TreeNode*)) {
	if (tree != NULL) {
		preProcess(tree);

		for (int i = 0; i < MAXCHILDREN; i++) {
			traverse(tree->child[i], preProcess, postProcess);
		}

		postProcess(tree);
		traverse(tree->sibling, preProcess, postProcess);
	}
}
// ok
static void nullProc(TreeNode* node) {
	return;
}

// conferir
static void insertNode(TreeNode* node) {
	node->scope = current_scope;
	switch (node->nodekind) {
		case StmtK:
			switch (node->kind.stmt) {
				// OK
				case FunK: {
					fromFunction = true;
					if (strcmp(node->attr.name, "main") == 0) {
						mainIsDefined = true;
					}
					localoffset = MAX_MEM - 2;
					if (st_lookup(node->attr.name)) {
						semanticError(node, "Function '%s' already declared", node->attr.name);
					} else {
						st_insert(node->attr.name, FunK, "global", node->type, node->lineno,
								  MAX_MEM - 1, false);
						enterNewScope(node->attr.name); // Entra no escopo da função
					}
					break;
				}
				// ok
				case VarK: {
					if (node->type == Void) {
						semanticError(node, "Variable declared as void");
						break;
					}
					if (st_lookup_no_parent(node->attr.name) == NULL) {
						if (strcmp(current_scope->name, "global") == 0) {
							if(node->isArray) {
								globaloffset += node->child[0]->attr.val;
								st_insert(node->attr.name, VarK, current_scope->name, node->type,
									node->lineno, globaloffset++, node->isArray);
							}
							else {
								st_insert(node->attr.name, VarK, current_scope->name, node->type,
									node->lineno, globaloffset++, node->isArray);
							}
						} else {
							st_insert(node->attr.name, VarK, current_scope->name, node->type,
									  node->lineno, localoffset--, node->isArray);
						}
					}
					break;
				}
				// ok
				case VetK: {
					if (node->type == Void) {
						semanticError(node, "Variable declared as void");
						break;
					}
					if (st_lookup_no_parent(node->attr.name) == NULL) {
						if (strcmp(current_scope->name, "global") == 0) {
							globaloffset += node->child[0]->attr.val;
							st_insert(node->attr.name, VarK, current_scope->name, node->type,
									  node->lineno, globaloffset++, node->isArray);
						} else {
							st_insert(node->attr.name, VarK, current_scope->name, node->type,
									  node->lineno, localoffset--, node->isArray);
							localoffset -= node->child[0]->attr.val;
						}
					}
					break;
				}
				case ParamK: {
					if (!st_lookup_no_parent(node->attr.name)) {
						if (strcmp(current_scope->name, "global") == 0) {
							st_insert(node->attr.name, node->kind.stmt, current_scope->name,
									  node->type, node->lineno, globaloffset++, node->isArray);
						} else {
							st_insert(node->attr.name, node->kind.stmt, current_scope->name,
									  node->type, node->lineno, localoffset--, node->isArray);
						}
					} else {
						semanticError(node, "Parameter '%s' already defined", node->attr.name);
					}
					break;
				}
				case CompoundK: {
					if (fromFunction) {
						fromFunction = false;
						break;
					}
					if (strcmp(current_scope->name, "global") == 0) {
						break;
					}
					CompoundScopesNum++;
					char newScope[20];
					sprintf(newScope, "compound%d", CompoundScopesNum);
					enterNewScope(newScope);
					node->attr.name = strdup(newScope);
					break;
				}
				default:
					break;
			}
		break;
		case ExpK: {
			switch (node->kind.exp) {
				case IdK:
				case FunCallK:
					if (!st_lookup(node->attr.name)) {
						semanticError(node, "'%s' was not declared in this scope", node->attr.name);
					} else {
						st_add_lineno(node->attr.name, node->lineno);
					}
				break;
				default:
					break;
			}
			break;
		}
		default:
			break;
	}
}

// ok
static void checkNode(TreeNode* node) {
	switch (node->nodekind) {
		case ExpK:
			switch (node->kind.exp) {
				case OpK:
					if (node->child[0]->type != Integer || node->child[1]->type != Integer) {
						semanticError(node, "operands must be of type integer");
						break;
					}
					node->type = Integer;
					break;
				case ConstK:
					node->type = Integer;
					break;

				case IdK: {
					SymbolList symbol = st_lookup(node->attr.name);
					if (symbol == NULL) {
						node->type = Integer;
					} else {
						node->type = symbol->type;
					}
					break;
				}

				case FunCallK:
					if (node->attr.name == NULL) {
						node->type = Void;
						break;
					}
					SymbolList funSymbol = st_lookup(node->attr.name);
					if (funSymbol == NULL) {
						node->type = Void;
						break;
					}
					node->type = funSymbol->type;
					break;

				case AssignK:
					if (node->child[0]->nodekind == ExpK && node->child[0]->kind.exp == FunCallK)
						if (node->child[0]->type != Integer &&
						    st_lookup(node->child[0]->attr.name) != NULL)
							semanticError(node, "invalid use of void expression");
					break;
				default:
					break;
			}
			break;

		case StmtK:
			switch (node->kind.stmt) {
				case ReturnK:
					if (node->child[0] == NULL) { // return vazio
						SymbolList funSymbol = st_lookup(current_scope->name);
						if (funSymbol->type != Void) {
							semanticError(node,
							              "Function with non-void return type must return a value");
						}
					}
					break;

				default:
					break;
			}
			break;
		default:
			break;
	}
}

// ok
void buildSymtab(TreeNode* syntaxTree) {
	enterNewScope("global");
	st_insert("input", FunK, "global", Integer, -1, location++, FALSE);
	st_insert("output", FunK, "global", Void, -1, location++, FALSE);

	traverse(syntaxTree, insertNode, exitScope);

	if (TraceAnalyze) {
		pc("\nSymbol table:\n\n");
		printSymTab();
	}
}

// ok
void typeCheck(TreeNode* syntaxTree) {
	if (!mainIsDefined) {
		semanticError(syntaxTree, "main function not defined");
	}
	traverse(syntaxTree, nullProc, checkNode);
}
