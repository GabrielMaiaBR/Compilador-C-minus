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

const char* ExpTypeToString(const ExpType type) {
	switch (type) {
		case Void:
			return "void";
		case Integer:
			return "int";
		default:
			return "unknown";
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
	while (tree != NULL) {
		if (tree->nodekind == StmtK && tree->kind.stmt == CompoundK) {
			for (int i = 0; i < MAXCHILDREN; i++) {
				printTree(tree->child[i]);
			}
		} else {
			printSpaces();
			if (tree->nodekind == StmtK) {
				switch (tree->kind.stmt) {
					case IfK:
						pc("Conditional selection\n");
						break;
					case IteraK:
						pc("Iteration (loop)\n");
						break;
					case ReturnK:
						pc("Return\n");
						break;
					case ParamK:
						pc("Function param (%s %s): %s\n", ExpTypeToString(tree->type),
						   tree->isArray ? "array" : "var", tree->attr.name);
						break;
					case VarK:
						pc("Declare %s %s: %s\n", ExpTypeToString(tree->type),
						   (tree->child[0] ? "array" : "var"), tree->attr.name);
						break;
					case FunK:
						pc("Declare function (return type \"%s\"): %s\n",
						   ExpTypeToString(tree->type), tree->attr.name);
						break;
					default:
						pce("Unknown Stmt kind\n");
						break;
				}
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
					case UnaryK:
						pc("Unary: ");
						printToken(tree->attr.op, "\0");
						break;
					case AssignK: {
						const TreeNode* varNode = tree->child[0];
						if (varNode->nodekind == ExpK && varNode->kind.exp == IdK) {
							if (varNode->child[0] != NULL) {
								pc("Assign to array: %s\n", varNode->attr.name);
								INDENT;
								if (varNode->child[0]->attr.val != 0) printTree(varNode->child[0]);
								UNINDENT;
							} else {
								pc("Assign to var: %s\n", varNode->attr.name);
							}
						} else {
							pc("Assign to: (unknown)\n");
						}
						break;
					}
					default:
						pce("Unknown ExpNode kind\n");
						break;
				}
			} else {
				pce("Unknown node kind\n");
			}

			for (int i = 0; i < MAXCHILDREN; i++) {
				if (tree->nodekind == ExpK && tree->kind.exp == AssignK && i == 0) continue;
				INDENT;
				printTree(tree->child[i]);
				UNINDENT;
			}
		}
		tree = tree->sibling;
	}
}
