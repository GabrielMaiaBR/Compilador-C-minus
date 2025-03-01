#include "cgen.h"
#include "code.h"
#include "globals.h"
#include "symtab.h"
#include <search.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int        ofpFO  = 0;
int        retFO  = -1;
int        initFO = -2;
int        paramFO;
int        localVarFO;
char       actual_fun[100];
static int tmpOffset       = -2;
int        mainline        = 0;
int        initial_cur     = 0;
int        global_position = 0;
int        first_funk      = 0;

bool isParamFuncall = FALSE;

struct qoffset {
	struct qoffset* forward;
	struct qoffset* backward;
	int             offset;
};
static struct qoffset* offset_top = NULL;

extern Scope globalScope;
extern Scope curren_scope;

static void cGen(TreeNode* tree);
static void cGenNoSibling(TreeNode* tree);
static void genStmt(TreeNode* tree);
static void genExp(TreeNode* tree);

static void push_offset(int _tmpOffset) {
	struct qoffset* offset_wrapper = malloc(sizeof(struct qoffset));
	offset_wrapper->offset         = _tmpOffset;
	insque(offset_wrapper, offset_top);
	offset_top = offset_wrapper;
}

static int pop_offset() {
	int             _tmpOffset = offset_top->offset;
	struct qoffset* aux        = offset_top;
	offset_top                 = offset_top->backward;
	remque(aux);
	free(aux);
	return _tmpOffset;
}

static int isLocalVar(char* name) {
	return (st_lookup_no_parent(name, &curren_scope) == 1);
}

static int isGlobalVar(char* name) {
	return (st_lookup(name, &globalScope) == 1);
}

/* load var in ac */
/* load var in ac */
/* load var in ac */
static void loadVarToAC(char* name) {
	int    isG      = isGlobalVar(name);
	Scope* varScope = isG ? &globalScope : &curren_scope;

	// 🔍 Procuramos a variável no escopo correto
	while (varScope != NULL && st_lookup(name, varScope) == -1) {
		varScope = varScope->parent;
	}

	if (varScope == NULL) {
		printf("ERRO CRÍTICO: Tentativa de acessar '%s' sem escopo válido.\n", name);
		return;
	}

	int off = return_paramOF(name, varScope);
	if (off == -999) {
		printf("ERRO CRÍTICO: Offset inválido ao carregar '%s'.\n", name);
		return;
	}

	if (isG) {
		emitRM("LDC", r5, 0, 0, "load 0 for global");
		emitRM("LD", ac, off, r5, "load global var");
	} else {
		printf("DEBUG: Loading %s from offset %d (Scope: %s)\n", name, off, varScope->name);
		emitRM("LD", ac, off, fp, "load id value");
	}
}

/* store ac in var */
static void storeACtoVar(char* name) {
	int    isG      = isGlobalVar(name);
	Scope* varScope = isG ? &globalScope : &curren_scope;
	int    off      = return_paramOF(name, varScope);

	if (off == -999) return; // ✅ Previne armazenamento em posição inválida.

	if (isG) {
		emitRM("LDC", r5, 0, 0, "0");
		emitRM("ST", ac, off, r5, "store global var");
	} else {
		printf("DEBUG: Storing AC into %s at offset %d (Scope: %s)\n", name, off, varScope->name);
		emitRM("ST", ac, off, fp, "assign: store value");
	}
}

/* load vector base in ac */
static void loadVectorBase(char* name) {
	int    isG      = isGlobalVar(name);
	Scope* varScope = isG ? &globalScope : &curren_scope;
	int    off      = return_paramOF(name, varScope);
	if (isG) {
		emitRM("LDC", r5, 0, 0, "0");
		emitRM("LD", ac, off, r5, "get the address of the vector");
	} else {
		emitRM("LD", ac, off, fp, "get the address of the vector");
	}
}

/* vectorAccessLoad: var[i], i in child[0], result in ac */
static void vectorAccessLoad(char* name, TreeNode* indexNode) {
	int    isG      = isGlobalVar(name);
	Scope* varScope = isG ? &globalScope : &curren_scope;
	int    off      = return_paramOF(name, varScope);

	// 1️⃣ Carrega o endereço correto do vetor
	if (isG)
		emitRM("LD", ac1, off, mp, "get the address of the global vector");
	else
		emitRM("LD", ac1, off, fp, "get the address of the local vector");

	// 2️⃣ Calcula o índice corretamente
	cGenNoSibling(indexNode);
	// emitRM("ST", ac, --tmpOffset, fp, "store index value temporarily");
	// emitRM("LD", r3, tmpOffset++, fp, "get the value of the index");

	// 3️⃣ Ajusta índice para indexação correta (Tiny Machine começa de 1)
	emitRM("LDC", r4, 1, 0, "load 1");
	emitRO("ADD", r3, r3, r4, "sub 3 by 1");

	// 4️⃣ Calcula o endereço final do vetor
	emitRO("SUB", ac1, ac1, r3, "get the address");

	// 5️⃣ Carrega o valor do vetor na posição correta
	emitRM("LD", ac, 0, ac1, "load value from vector");
}

/* vectorAccessStore: var[i] = ac */
static void vectorAccessStore(char* name, TreeNode* indexNode) {
	int    isG      = isGlobalVar(name);
	Scope* varScope = isG ? &globalScope : &curren_scope;
	int    off      = return_paramOF(name, varScope);

	// 1️⃣ Carrega o endereço correto do vetor
	if (isG)
		emitRM("LD", ac1, off, mp, "get the address of the global vector");
	else
		emitRM("LD", ac1, off, fp, "get the address of the local vector");

	// 2️⃣ Calcula o índice corretamente
	cGenNoSibling(indexNode);
	// emitRM("ST", ac, --tmpOffset, fp, "store index value temporarily");
	// emitRM("LD", r3, tmpOffset++, fp, "get the value of the index");

	// 3️⃣ Ajusta índice para indexação correta (Tiny Machine começa de 1)
	emitRM("LDC", r4, 1, 0, "load 1");
	emitRO("ADD", r3, r3, r4, "sub 3 by 1");

	// 4️⃣ Calcula o endereço final do vetor
	emitRO("SUB", ac1, ac1, r3, "get the address");

	// 5️⃣ Armazena o valor na posição correta do vetor
	emitRM("ST", ac, 0, ac1, "get the value of the vector");
}

/* Gera Param e Var antes do corpo */
static void generateParamAndVar(TreeNode* tree) {
	while (tree != NULL) {
		if (tree->nodekind == StmtK) {
			Scope* varScope = &curren_scope; // Usa o escopo correto da função atual

			switch (tree->kind.stmt) {
				case ParamK:
					if (TraceCode) emitComment("-> Param");
					add_paramOF(paramFO, tree->attr.name, varScope);
					set_active(tree->attr.name, varScope);
					paramFO--;
					tmpOffset--;
					if (TraceCode) emitComment("<- Param");
					break;

				case ParamVetK:
					if (TraceCode) emitComment("-> Param vet");
					paramFO--;
					tmpOffset--;
					add_paramOF(paramFO, tree->attr.name, varScope);
					set_active(tree->attr.name, varScope);
					if (TraceCode) emitComment("<- Param vet");
					break;

				case VarK: // Declaração de variável local
					if (TraceCode) emitComment("-> declare var");

					localVarFO--;
					add_paramOF(localVarFO, tree->attr.name, &curren_scope);
					set_active(tree->attr.name, &curren_scope);

					// ✅ Decremento único para garantir que não sobrescrevemos variáveis

					tmpOffset = localVarFO; // ✅ Atualiza tmpOffset corretamente

					if (TraceCode) emitComment("<- declare var");
					break;

				case VetK:
					if (TraceCode) emitComment("-> declare vector");
					int tam = tree->child[0]->attr.val;

					if (strcmp(actual_fun, "\0") == 0) {
						global_position += tam;
						add_paramOF(global_position, tree->attr.name, &globalScope);
						set_active(tree->attr.name, &globalScope);
						emitRM("LDC", ac, global_position, 0, "load global position to ac");
						emitRM("LDC", r5, 0, 0, "load 0");
						emitRM("ST", ac, global_position, r5, "store ac in global_position_aux");
						global_position++;
					} else {
						emitRM("LDA", ac, --localVarFO, fp, "guard addr of vector");
						emitRM("ST", ac, localVarFO, fp, "store addr of vector");

						localVarFO -= tam;
						add_paramOF(localVarFO, tree->attr.name, &curren_scope);
						set_active(tree->attr.name, &curren_scope);
						tmpOffset = localVarFO;
					}

					if (TraceCode) emitComment("<- declare vector");
					break;

				default:
					break;
			}
		}
		tree = tree->sibling;
	}
}

/* Gera o corpo após param/var */
static void generateBody(TreeNode* tree) {
	cGen(tree);
}

static void genStmt(TreeNode* tree) {
	TreeNode *p1, *p2, *p3;
	int       savedLoc1, savedLoc2, currentLoc;
	switch (tree->kind.stmt) {
		case FunK: {
			char comment[64];
			sprintf(comment, "-> Init Function (%s)", tree->attr.name);
			if (!first_funk) {
				first_funk  = 1;
				initial_cur = emitSkip(1);
			}
			if (TraceCode) emitComment(comment);

			// Atualiza `currentScope` para o escopo da função
			Scope oldScope = curren_scope; // Salva o escopo anterior
			strcpy(curren_scope.name, tree->attr.name);
			curren_scope.parent = &globalScope;

			if (strcmp(tree->attr.name, "main") == 0) {
				mainline = emitSkip(0);
				emitBackup(initial_cur);
				emitRM_Abs("LDA", PC, mainline, "jump to main");
				emitRestore();
			} else {
				int funLoc = emitSkip(0);
				add_funloc(funLoc, tree->attr.name, &globalScope);
				emitRM("ST", ac, retFO, fp, "store return address from ac");
			}

			paramFO    = initFO;
			localVarFO = paramFO - 3;
			tmpOffset  = initFO;
			strcpy(actual_fun, tree->attr.name);

			p1 = tree->child[0]; // Params
			p2 = tree->child[1]; // Corpo da função

			if (p1) generateParamAndVar(p1);
			if (p2) {
				if (p2->nodekind == StmtK && p2->kind.stmt == CompoundK) {
					generateParamAndVar(p2->child[0]);
					generateBody(p2->child[1]);
				} else {
					generateBody(p2);
				}
			}

			if (strcmp(tree->attr.name, "main") != 0 && tree->type == Void) {
				emitRM("LDA", ac1, 0, fp, "save current fp into ac1");
				emitRM("LD", fp, ofpFO, fp, "make fp = ofp");
				emitRM("LD", PC, retFO, ac1, "return to caller");
			}

			strcpy(actual_fun, "\0");

			// Restaura o escopo antigo ao sair da função
			curren_scope = oldScope;

			if (TraceCode) emitComment("<- End Function");
		} break;

		case ParamK:
			break;
		case ParamVetK:
			break;
		case VarK:
			break;
		case CompoundK: {
			printf("DEBUG: Entrando no bloco aninhado dentro do escopo '%s'\n", curren_scope.name);

			// Salva o escopo atual
			Scope oldScope = curren_scope;

			// Cria um novo escopo para o bloco composto – semelhante ao que foi feito em analyze.c
			Scope* newScope = malloc(sizeof(Scope));
			*newScope       = curren_scope; // copia o escopo atual
			curren_scope.blockLevel++;      // incrementa o nível do bloco
			// Atualiza o blockId; você pode usar um contador global ou, se não houver, definir 1 se
			// ainda não houver
			curren_scope.blockId = (oldScope.blockId == 0) ? 1 : oldScope.blockId + 1;
			// Define o parent da nova versão de currentScope para ser o escopo antigo
			curren_scope.parent = newScope;

			int oldLocalVarFO = localVarFO;
			localVarFO        = localVarFO - 2; // aloca espaço para variáveis locais do bloco

			if (tree->child[0]) generateParamAndVar(tree->child[0]); // declarações locais
			if (tree->child[1]) generateBody(tree->child[1]);        // comandos do bloco

			localVarFO = oldLocalVarFO;

			// Restaura o escopo antigo ao sair do bloco
			curren_scope = oldScope;
			printf("DEBUG: Saindo do bloco aninhado, restaurando escopo para '%s'\n",
			       curren_scope.name);

			// free(newScope); // Libera a memória alocada para o novo escopo (se não for necessário
			// manter depois)
		} break;

		case VetK:
			/* Já tratados em generateParamAndVar */
			break;

		case IfK:
			if (TraceCode) emitComment("-> if");
			p1 = tree->child[0]; /* condição */
			p2 = tree->child[1]; /* then */
			p3 = tree->child[2]; /* else */
			cGen(p1);
			savedLoc1 = emitSkip(1);
			emitComment("if: jump to else belongs here");
			cGen(p2);
			savedLoc2 = emitSkip(1);
			emitComment("if: jump to end belongs here");
			currentLoc = emitSkip(0);
			printf("DEBUG: Avaliando IF - Pos atual: %d, Salto para ELSE em: %d\n", emitSkip(0),
			       savedLoc1);
			emitBackup(savedLoc1);
			printf("DEBUG: Emitindo JEQ (se falso, pula para %d)\n", currentLoc);
			emitRM_Abs("JEQ", ac, currentLoc, "if: jmp to else");
			emitRestore();
			printf("DEBUG: Depois do JEQ, posição: %d\n", emitSkip(0));
			cGen(p3);
			currentLoc = emitSkip(0);
			emitBackup(savedLoc2);
			emitRM_Abs("LDA", PC, currentLoc, "jmp to end");
			emitRestore();
			if (TraceCode) emitComment("<- if");
			break;

		case ReturnK:
			if (TraceCode) emitComment("-> return");
			p1 = tree->child[0];
			if (p1) cGen(p1);
			if (strcmp(actual_fun, "main") != 0) {
				emitRM("LDA", ac1, 0, fp, "save current fp into ac1");
				emitRM("LD", fp, ofpFO, fp, "make fp = ofp");
				emitRM("LD", PC, retFO, ac1, "return to caller");
			} else {
				emitRO("HALT", 0, 0, 0, "return from main");
			}
			if (TraceCode) emitComment("<- return");
			break;

		case WhileK:
			if (TraceCode) emitComment("-> while");
			emitComment("repeat: jump after body comes back here");

			savedLoc1 = emitSkip(0);
			p1        = tree->child[0]; // cond
			cGen(p1);

			savedLoc2 = emitSkip(1);
			p2        = tree->child[1]; // body
			cGen(p2);

			// salto de volta à condição
			emitRM_Abs("LDA", PC, savedLoc1, "jump to savedLoc1");

			// Label de saída
			savedLoc1 = emitSkip(0);
			emitBackup(savedLoc2);
			emitRM_Abs("JEQ", ac, savedLoc1, "repeat: jmp to end");
			emitRestore();

			if (TraceCode) emitComment("<- repeat");
			break;

		case AssignK:
			if (TraceCode) emitComment("-> assign");
			cGen(tree->child[0]);
			storeACtoVar(tree->attr.name);
			if (TraceCode) emitComment("<- assign");
			break;

		case AssignVetK:
			if (TraceCode) emitComment("-> assign vector");
			emitComment("-> Vector");
			p1 = tree->child[0];
			p2 = tree->child[1];
			cGen(p2); // val in ac
			vectorAccessStore(tree->attr.name, p1);
			break;

		default:
			break;
	}
}

static void genExp(TreeNode* tree) {
	TreeNode *p1, *p2;
	switch (tree->kind.exp) {
		case ConstK:
			if (TraceCode) emitComment("-> Const");
			emitRM("LDC", ac, tree->attr.val, 0, "load const");
			if (TraceCode) emitComment("<- Const");
			break;

		case IdK:
			if (TraceCode) emitComment("-> Id");
			{
				int    isG      = isGlobalVar(tree->attr.name);
				Scope* varScope = isG ? &globalScope : &curren_scope;
				int    off      = return_paramOF(tree->attr.name, varScope);
				int    idType   = st_getIdType(tree->attr.name, varScope);
				int    isVec    = (idType == VetK || idType == ParamVetK) ? 1 : 0;
				if (isVec && tree->child[0] != NULL) {
					vectorAccessLoad(tree->attr.name, tree->child[0]);
				} else if (isVec && tree->child[0] == NULL) {
					loadVectorBase(tree->attr.name);
				} else {
					loadVarToAC(tree->attr.name);
				}
			}
			if (TraceCode) emitComment("<- Id");
			break;

		case FunCallK: {
			if (TraceCode) {
				char cmm[100];
				sprintf(cmm, "-> Function call (%s)", tree->attr.name);
				emitComment(cmm);
			}
			if (strcmp(tree->attr.name, "input") == 0) {
				emitRO("IN", ac, 0, 0, "read input");
				break;
			}
			if (strcmp(tree->attr.name, "output") == 0) {
				p1 = tree->child[0];
				cGen(p1);
				emitRO("OUT", ac, 0, 0, "print value");
				break;
			}

			/*
			   Neste modelo antigo, a análise já determinou que os parâmetros são
			   passados em posições fixas. Por exemplo, para a função "binario",
			   o parâmetro "x" foi atribuído ao offset -2 (isto é, em fp-2).

			   Assim, para a chamada, basta avaliar o argumento e armazená-lo
			   nesse slot fixo.
			*/
			int paramSlot;
			/* Para generalizar, deveríamos obter os offsets de cada parâmetro a partir
			   da tabela de símbolos. No exemplo, "binario" tem 1 parâmetro, "x". */
			paramSlot     = return_paramOF("x", &globalScope);
			TreeNode* arg = tree->child[0];
			if (arg != NULL) {
				cGenNoSibling(arg); // avalia o argumento e o resultado fica em ac
				emitRM("ST", ac, paramSlot, fp, "Store argument in parameter slot");
			}

			/* Armazena o endereço de retorno em um slot fixo (retFO, definido globalmente como -1)
			   para que o callee, em seu ReturnK, possa recuperá-lo e saltar de volta ao chamador.
			*/
			int retAddr = emitSkip(0) + 1;
			emitRM("LDC", ac, retAddr, 0, "Load return address");
			emitRM("ST", ac, retFO, fp, "Store return address in fixed slot");

			/* Agora, saltamos para a função chamada. Como os parâmetros já estão
			   em seus slots fixos e o FP não é modificado, a função (FunK)
			   vai buscar seus parâmetros (por exemplo, via loadVarToAC) corretamente.
			*/
			int funLoc = return_funloc(tree->attr.name, &globalScope);
			if (funLoc < 0) {
				emitRO("HALT", 0, 0, 0, "function not found");
			} else {
				emitRM_Abs("LDA", PC, funLoc, "Jump to function");
			}

			if (TraceCode) emitComment("<- Function Call");
		} break;

		case OpK:
			if (TraceCode) emitComment("-> Op");
			p1 = tree->child[0];
			p2 = tree->child[1];
			cGen(p1);
			printf("DEBUG: Operação '%d' -> Primeiro operando (AC) = %d\n", tree->attr.op, ac);
			emitRM("ST", ac, --tmpOffset, fp, "op: push left");
			cGen(p2);
			printf("DEBUG: Operação '%d' -> Segundo operando (AC) = %d\n", tree->attr.op, ac);
			emitRM("LD", ac1, tmpOffset++, fp, "op: load left");
			switch (tree->attr.op) {
				case PLUS:
					emitRO("ADD", ac, ac1, ac, "op +");
					break;
				case MINUS:
					emitRO("SUB", ac, ac1, ac, "op -");
					break;
				case TIMES:
					emitRO("MUL", ac, ac1, ac, "op *");
					break;
				case OVER:
					emitRO("DIV", ac, ac1, ac, "op /");
					break;
				case LT:
					emitRO("SUB", ac, ac1, ac, "op <");
					emitRM("JLT", ac, 2, PC, "br if true");
					emitRM("LDC", ac, 0, ac, "false case");
					emitRM("LDA", PC, 1, PC, "unconditional jmp");
					emitRM("LDC", ac, 1, ac, "true case");
					break;
				case EQ:
					emitRO("SUB", ac, ac1, ac, "op ==");
					emitRM("JEQ", ac, 2, PC, "br if true");
					emitRM("LDC", ac, 0, ac, "false case");
					emitRM("LDA", PC, 1, PC, "unconditional jmp");
					emitRM("LDC", ac, 1, ac, "true case");
					break;
				case GT:
					emitRO("SUB", ac, ac1, ac, "op >");
					emitRM("JGT", ac, 2, PC, "br if true");
					emitRM("LDC", ac, 0, ac, "false case");
					emitRM("LDA", PC, 1, PC, "unconditional jmp");
					emitRM("LDC", ac, 1, ac, "true case");
					break;
				case LTE:
					emitRO("SUB", ac, ac1, ac, "op <=");
					emitRM("JLE", ac, 2, PC, "br if true");
					emitRM("LDC", ac, 0, ac, "false case");
					emitRM("LDA", PC, 1, PC, "unconditional jmp");
					emitRM("LDC", ac, 1, ac, "true case");
					break;
				case NEQ:
					emitRO("SUB", ac, ac1, ac, "op !=");
					emitRM("JNE", ac, 2, PC, "br if true");
					emitRM("LDC", ac, 0, ac, "false case");
					emitRM("LDA", PC, 1, PC, "unconditional jmp");
					emitRM("LDC", ac, 1, ac, "true case");
					break;
				case GTE:
					emitRO("SUB", ac, ac1, ac, "op >=");
					emitRM("JGE", ac, 2, PC, "br if true");
					emitRM("LDC", ac, 0, ac, "false case");
					emitRM("LDA", PC, 1, PC, "unconditional jmp");
					emitRM("LDC", ac, 1, ac, "true case");
					break;
				default:
					emitComment("BUG: Unknown op");
					break;
			}
			if (TraceCode) emitComment("<- Op");
			break;

		default:
			break;
	}
}

static void cGen(TreeNode* tree) {
	if (tree != NULL) {
		switch (tree->nodekind) {
			case StmtK:
				genStmt(tree);
				break;
			case ExpK:
				genExp(tree);
				break;
			default:
				break;
		}
		if (!isParamFuncall) cGen(tree->sibling);
	}
}

static void cGenNoSibling(TreeNode* tree) {
	if (tree != NULL) {
		switch (tree->nodekind) {
			case StmtK:
				genStmt(tree);
				break;
			case ExpK:
				genExp(tree);
				break;
			default:
				break;
		}
	}
}

void codeGen(TreeNode* syntaxTree) {
	emitComment("TINY Compilation to TM Code");
	emitComment("Standard prelude:");
	emitRM("LD", mp, 0, 0, "load maxaddress from location 0");
	emitRM("LD", fp, 0, 0, "load maxaddress from location 0");
	emitRM("ST", ac, 0, 0, "clear location 0");
	emitComment("End of standard prelude.");

	strcpy(curren_scope.name, "main");
	cGen(syntaxTree);

	emitComment("End of execution.");
	emitRO("HALT", 0, 0, 0, "");
}
