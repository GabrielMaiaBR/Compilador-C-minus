#include "analyze.h"
#include "globals.h"
#include "log.h"
#include "symtab.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

// Flags e variáveis globais para controle de estado
bool mainIsDefined = FALSE;     // Indica se a função main foi definida
bool fromFunction = FALSE;      // Controla escopo de funções
int localoffset = MEMORY_SIZE - 2;  // Alocação de memória local (pilha)
int globaloffset = 0;            // Alocação de memória global
int compound_tracker = 0;        // Rastreia blocos compostos aninhados
static int location = 0;         // Localização para funções built-in

// Cria um novo escopo e atualiza a lista de escopos
void enterNewScope(char* scopeName) {
	Scope newScope   = malloc(sizeof(struct Scope));
	newScope->parent = current_scope;
	newScope->name   = strdup(scopeName);
	newScope->next   = NULL;
	for (int i = 0; i < SIZE; i++) {
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

// Retorna ao escopo pai quando sai de blocos/funções
void exitScope(TreeNode* tree) {
    if (tree->nodekind != StmtK) return;
    if (tree->kind.stmt == CompoundK) {
        current_scope = current_scope->parent;
        return;
    }
    if (tree->kind.stmt == FunK && current_scope->parent != NULL) {
        current_scope = current_scope->parent;
        return;
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

static void nullProc(TreeNode* node) {
    if (node == NULL)
        return;
    else
        return;
}

static void insertNode(TreeNode* node) {
	node->scope = current_scope;
	switch (node->nodekind) {
		case StmtK:
			switch (node->kind.stmt) {
				case FunK: { // Tratamento de declaração de funções
					fromFunction = true;
					if (strcmp(node->attr.name, "main") == 0) {
						mainIsDefined = true;
					}
					localoffset = MEMORY_SIZE - 2;
					if (st_lookup(node->attr.name)) {
						semanticError(node, "Function '%s' already declared", node->attr.name);
					} else {
						st_insert(node->attr.name, FunK, "global", node->type, node->lineno, MEMORY_SIZE - 1, false);
						enterNewScope(node->attr.name); 
					}
					break;
				}
				case VarK: { // Inserção de variáveis simples
                    if (node->type == Void) {
                        semanticError(node, "Variable declared as void");
                        break;
                    }
                    SymbolList existVar = st_lookup_no_parent(node->attr.name);
                    if (existVar == NULL) {
                        if (strcmp(current_scope->name, "global") == 0) {
                            globaloffset += node->child[0]->attr.val;
                            st_insert(node->attr.name, VarK, current_scope->name, node->type, node->lineno, globaloffset, node->isArray);
                            globaloffset++;
                        } else {
                            st_insert(node->attr.name, VarK, current_scope->name, node->type, node->lineno, localoffset, node->isArray);
                            localoffset--;
                        }
                    }
                    break;
				}
				case VetK: { // Inserção de arrays/vetores
                    if (node->type == Void) {
                        semanticError(node, "Variable declared as void");
                        break;
                    }
                    if (st_lookup_no_parent(node->attr.name) == NULL) {
                        if (strcmp(current_scope->name, "global") == 0) {
                            globaloffset += node->child[0]->attr.val;
                            st_insert(node->attr.name, VarK, current_scope->name, node->type, node->lineno, globaloffset++, node->isArray);
                        } else {
                            st_insert(node->attr.name, VarK, current_scope->name, node->type, node->lineno, localoffset, node->isArray);
                            localoffset -= node->child[0]->attr.val;
                        }
                    }
                    break;
				}
				case ParamK: { // Tratamento de parâmetros de função
					if (!st_lookup_no_parent(node->attr.name)) {
						if (strcmp(current_scope->name, "global") == 0) {
							st_insert(node->attr.name, node->kind.stmt, current_scope->name, node->type, node->lineno, globaloffset++, node->isArray);
						} else {
							st_insert(node->attr.name, node->kind.stmt, current_scope->name, node->type, node->lineno, localoffset--, node->isArray);
						}
					} else {
						semanticError(node, "Parameter '%s' already defined", node->attr.name);
					}
					break;
				}
				case CompoundK: { // Criação de escopo para blocos compostos
					if (fromFunction) {
						fromFunction = false;
						break;
					}
					if (strcmp(current_scope->name, "global") == 0) {
						break;
					}
					compound_tracker++;
					char newScope[20];
					sprintf(newScope, "compound%d", compound_tracker);
					enterNewScope(newScope);
					node->attr.name = strdup(newScope);
					break;
				}
				default:
					break;
			}
		break;
		case ExpK: { // Verificação de identificadores e chamadas
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

// Realiza verificações de tipo e uso correto de símbolos
static void checkNode(TreeNode* node) {
	switch (node->nodekind) {
		case ExpK:
			switch (node->kind.exp) {
				case OpK: // Validação de operações
                    if (node->child[0]->type == Integer && node->child[1]->type == Integer) {
                        node->type = Integer;
                    } else {
                        semanticError(node, "operands must be of type integer");
                    }
                    break;
				case ConstK:
					node->type = Integer;
					break;

				case IdK: { // Resolução de tipos de identificadores
					SymbolList symbol = st_lookup(node->attr.name);
					if (symbol == NULL) {
						node->type = Integer;
					} else {
						node->type = symbol->type;
					}
					break;
				}

				case FunCallK: // Verificação de chamadas de função
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
						if (node->child[0]->type != Integer && st_lookup(node->child[0]->attr.name) != NULL)
							semanticError(node, "invalid use of void expression");
					break;
				default:
					break;
			}
			break;

		case StmtK:
			switch (node->kind.stmt) {
				case ReturnK: // Validação de retorno de funções
					if (node->child[0] == NULL) {
						SymbolList funSymbol = st_lookup(current_scope->name);
						if (funSymbol->type != Void) {
							semanticError(node, "Function with non-void return type must return a value");
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

void buildSymtab(TreeNode* syntaxTree) {
	enterNewScope("global");
	st_insert("input", FunK, "global", Integer, -1, location, FALSE);
	location++;
	st_insert("output", FunK, "global", Void, -1, location, FALSE);
	location++;

	traverse(syntaxTree, insertNode, exitScope);

	if (TraceAnalyze) {
		pc("\nSymbol table:\n\n");
		printSymTab();
	}
}

// Verificação final de tipos e existência da main
void typeCheck(TreeNode* syntaxTree) {
	if (!mainIsDefined) {
		semanticError(syntaxTree, "main function not defined");
	}
	traverse(syntaxTree, nullProc, checkNode);
}
