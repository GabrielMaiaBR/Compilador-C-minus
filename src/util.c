/****************************************************/
/* File: util.c                                     */
/* Utility function implementation                  */
/* for the C- compiler                             */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/
#include "util.h"
#include "globals.h"
#include "log.h"

static int indentno = 0;

#define INDENT indentno += 4
#define UNINDENT indentno -= 4

static void printSpaces(void) {
	int i;
	for (i = 0; i < indentno; i++) pc(" ");
}

void printLine() {
	char       line[1024];
	static int current = 0;
	static int first   = 1;
	if (first) {
		rewind(redundant_source);
		first = 0;
	}
	const char* ret = fgets(line, sizeof(line), redundant_source);
	if (ret) {
		current++;
		pc("%d: %s", current, line);

		if (feof(redundant_source)) {
			if (line[strlen(line) - 1] != '\n') {
				pc("\n");
			}
		}
	}
}


void printToken(TokenType token, const char* tokenString) {
	switch (token) {
		case IF:
		case WHILE:
		case ELSE:
		case VOID:
		case INT:
		case RETURN:
			pc("reserved word: %s\n", tokenString);
			break;
		case ASSIGN:
			pc("=\n");
			break;
		case LT:
			pc("<\n");
			break;
		case LTE:
			pc("<=\n");
			break;
		case GT:
			pc(">\n");
			break;
		case GTE:
			pc(">=\n");
			break;
		case EQ:
			pc("==\n");
			break;
		case NEQ:
			pc("!=\n");
			break;
		case LPAREN:
			pc("(\n");
			break;
		case RPAREN:
			pc(")\n");
			break;
		case LBRACKET:
			pc("[\n");
			break;
		case RBRACKET:
			pc("]\n");
			break;
		case LBRACE:
			pc("{\n");
			break;
		case RBRACE:
			pc("}\n");
			break;
		case SEMI:
			pc(";\n");
			break;
		case COMMA:
			pc(",\n");
			break;
		case PLUS:
			pc("+\n");
			break;
		case MINUS:
			pc("-\n");
			break;
		case TIMES:
			pc("*\n");
			break;
		case OVER:
			pc("/\n");
			break;
		case ENDFILE:
			pc("EOF\n");
			break;
		case NUM:
			pc("NUM, val= %s\n", tokenString);
			break;
		case ID:
			pc("ID, name= %s\n", tokenString);
			break;
		case ERROR:
			pce("ERROR: %s\n", tokenString);
			break;
		default:
			pc("Unknown token: %d\n", token);
	}
}

TreeNode* newStmtNode(StmtKind kind) {
	TreeNode* t = (TreeNode*) malloc(sizeof(TreeNode));
	int       i;
	if (t == NULL)
		pc("Out of memory error at line %d\n", lineno);
	else {
		for (i = 0; i < MAXCHILDREN; i++) t->child[i] = NULL;
		t->sibling   = NULL;
		t->nodekind  = StmtK;
		t->kind.stmt = kind;
		t->lineno    = lineno;
	}
	return t;
}

TreeNode* newExpNode(ExpKind kind) {
	TreeNode* t = (TreeNode*) malloc(sizeof(TreeNode));
	int       i;
	if (t == NULL)
		pc("Out of memory error at line %d\n", lineno);
	else {
		for (i = 0; i < MAXCHILDREN; i++) t->child[i] = NULL;
		t->sibling  = NULL;
		t->nodekind = ExpK;
		t->kind.exp = kind;
		t->lineno   = lineno;
		t->type     = Void;
	}
	return t;
}

char* copyString(char* s) {
	int   n;
	char* t;
	if (s == NULL) return NULL;
	n = strlen(s) + 1;
	t = malloc(n);
	if (t == NULL)
		pc("Out of memory error at line %d\n", lineno);
	else
		strcpy(t, s);
	return t;
}

void printTree(TreeNode* tree) {
    int i;
    while (tree != NULL) {
        if (tree->kind.stmt == CompoundK && tree->nodekind == StmtK) {
            // Para CompoundK, apenas percorre os filhos sem imprimir mensagem ou indentação extra
            for (i = 0; i < MAXCHILDREN; i++) {
                if (tree->child[i] != NULL) {
                    printTree(tree->child[i]);
                }
            }
        } else {
            // Imprime a indentação sempre (não depende de indentno > 0)
            printSpaces();

            if (tree->nodekind == StmtK) {
                switch (tree->kind.stmt) {
                    case FunK:
                        pc("Declare function (return type \"%s\"): %s\n",
                           tree->type == Integer ? "int" :
                           tree->type == Void  ? "void" : "ERROR",
                           tree->attr.name);
                        break;
                    case VarK:
                        // Se a variável for um array (isArray==TRUE), imprime "array"; caso contrário, "var"
                        pc("Declare %s %s: %s\n",
                           tree->type == Integer ? "int" :
                           tree->type == Void  ? "void" : "ERROR",
                           (tree->isArray ? "array" : "var"),
                           tree->attr.name);
                        break;
                    case VetK:
                        // Se houver uso de VetK, pode ser tratado aqui
                        pc("Declare %s array: %s\n",
                           tree->type == Integer ? "int" :
                           tree->type == Void  ? "void" : "ERROR",
                           tree->attr.name);
                        break;
					case ParamK:
						// Se o parâmetro for array (isArray == TRUE), imprima "array", caso contrário "var".
						pc("Function param (%s %s): %s\n",
						   tree->type == Integer ? "int"
						   : tree->type == Void  ? "void" : "ERROR",
						   tree->isArray ? "array" : "var",
						   tree->attr.name);
						break;
		
                    case ParamVetK:
                        pc("Function param (%s array): %s\n",
                           tree->type == Integer ? "int" :
                           tree->type == Void  ? "void" : "ERROR",
                           tree->attr.name);
                        break;
                    case IfK:
                        pc("Conditional selection\n");
                        break;
                    case IteraK:
                        pc("Iteration (loop)\n");
                        break;
                    case ReturnK:
                        pc("Return\n");
                        break;
                    default:
                        pc("Unknown Stmt kind\n");
                        break;
                }
                INDENT;
                for (i = 0; i < MAXCHILDREN; i++) {
                    if (tree->child[i] != NULL) {
                        printTree(tree->child[i]);
                    }
                }
                UNINDENT;
            } else if (tree->nodekind == ExpK) {
                switch (tree->kind.exp) {
                    case OpK:
                        pc("Op: ");
                        printToken(tree->attr.op, "\0");
                        break;
                    case ConstK:
                        pc("Const: %d\n", tree->attr.val);
                        break;
                    case IdK:
                        pc("Id: %s\n", tree->attr.name);
                        break;
                    case FunCallK:
                        pc("Function call: %s\n", tree->attr.name);
                        break;
                    case AssignK: {
                        if (tree->child[0] != NULL && tree->child[0]->kind.exp == IdK && tree->child[0]->nodekind == ExpK) {
                            if (tree->child[0]->child[0] != NULL) {
                                pc("Assign to array: %s\n", tree->child[0]->attr.name);
                                INDENT;
                                if (tree->child[0]->child[0]->attr.val != 0){
									printTree(tree->child[0]->child[0]);
								}
                                UNINDENT;
                            } else {
                                pc("Assign to var: %s\n", tree->child[0]->attr.name);
                            }
                        } else {
                            pc("ERROR\n");
                        }
                        break;
                    }
                    default:
                        pc("ERROR\n");
                        break;
                }
                INDENT;
                // Ao imprimir os filhos do nó de atribuição, pula o índice 0 para não duplicar o nó já processado.
                for (i = 0; i < MAXCHILDREN; i++) {
                    if (tree->nodekind == ExpK && tree->kind.exp == AssignK && i == 0)
                        continue;
                    if (tree->child[i] != NULL) {
                        printTree(tree->child[i]);
                    }
                }
                UNINDENT;
            } else {
                pc("Unknown node kind\n");
            }
        }
        tree = tree->sibling;
    }
}
