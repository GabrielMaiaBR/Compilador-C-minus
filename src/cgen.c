/****************************************************/
/* File: cgen.c                                     */
/* Minimal code generator for the TINY compiler     */
/* (generates code for the TM machine)              */
/****************************************************/

#include "globals.h"
#include "code.h"
#include "cgen.h"
#include <stdio.h>
#include "symtab.h"
#include <stdlib.h>
#include <string.h>

static void cGen(TreeNode *tree);

typedef struct HashNode {
    char* key;
    int value;
    struct HashNode* next;
} HashNode;

static HashNode* hashTable[SIZE];

static int get_hash_value(const char* key) {
    int hash = 0;
    while (*key) {
        hash = (hash << SHIFT) + *key++;
    }
    return hash % SIZE;
}

// Insere um par chave-valor na tabela hash
static void add(const char* key, int value) {
    int index = get_hash_value(key);
    HashNode* newNode = malloc(sizeof(HashNode));
    newNode->key = strdup(key);
    newNode->value = value;
    newNode->next = hashTable[index];
    hashTable[index] = newNode;
}

// Busca o valor associado a uma chave; retorna -1 se não encontrado
static int get(const char* key) {
    int index = get_hash_value(key);
    HashNode* node = hashTable[index];
    while (node) {
        if (strcmp(node->key, key) == 0) {
            return node->value;
        }
        node = node->next;
    }
    return -1;
}

static int tempOffset = -2;          // Offset para armazenamento de temporários
static char *currentScopeName = "global"; // Nome do escopo atual
bool is_param_fun_call = FALSE;      // Flag para controle de parâmetros em chamada de função
static int compound_tracker = 0;     // Contador para escopos compostos
int main_location = 3;               // Endereço da função main
bool first_funk = TRUE;              // Indica se é a primeira função a ser processada
char comment[120];                   // Buffer para mensagens de comentário


/* Gera código para definição de função.
   Trata a função "main" de forma especial e registra o endereço na tabela hash. */
static void generateFunction(TreeNode *tree) {
    bool isMain = (strcmp(tree->attr.name, "main") == 0); // Verifica se é main
    sprintf(comment, "-> Init Function (%s)", tree->attr.name);
    if (TraceCode) emitComment(comment);
    int funcLoc = emitSkip(0); // Marca posição atual
    if (first_funk) {
        main_location = emitSkip(1); // Reserva espaço para main
        first_funk = FALSE;
        add(tree->attr.name, main_location + 1);
    } else {
        add(tree->attr.name, funcLoc);
    }
    currentScopeName = tree->attr.name;
    tempOffset = -2; // Reinicia o offset para o novo escopo
    if (isMain) {
        int savedLoc = emitSkip(0);
        emitBackup(main_location);
        if (savedLoc == 3)
            emitRM_Abs("LDA", PC, savedLoc + 1, "jump to main");
        else
            emitRM_Abs("LDA", PC, savedLoc, "jump to main");
        emitRestore();
        cGen(tree->child[0]); // Gera código para declarações locais
        cGen(tree->child[1]); // Gera código para o corpo da função
        if (TraceCode) emitComment("<- End Function");
        return;
    }
    // Para funções não-main, armazena o endereço de retorno e gera o corpo
    emitRM("ST", ac, -1, fp, "store return address from ac");
    cGen(tree->child[0]);
    cGen(tree->child[1]);
    if (tree->type == Void) {
        emitRM("LDA", ac1, 0, fp, "save current fp into ac1");
        emitRM("LD", fp, 0, fp, "make fp = ofp");
        emitRM("LD", PC, -1, ac1, "return to caller");
    }
    if (TraceCode) emitComment("<- End Function");
}

/* Gera código para declaração de variável (não-array).
   Apenas decrementa o offset para alocar a variável. */
static void generateVarDeclaration(TreeNode *tree) {
    if (TraceCode) emitComment("-> declare var");
    tempOffset--;
    if (TraceCode) emitComment("<- declare var");
}

/* Gera código para declaração de vetor (array).
   Trata a alocação de espaço conforme o escopo (global ou local). */
static void generateVectorDeclaration(TreeNode *tree) {
    if (TraceCode) emitComment("-> declare vector");
    if (strcmp(tree->scope->name, "global") == 0) {
        SymbolList vecSymbol = st_lookup_from_given_scope(tree->attr.name, tree->scope);
        int memloc = vecSymbol->memloc;
        int vecSize = tree->child[0]->attr.val;
        emitRM("LDC", ac, vecSize, 0, "load global position to ac");
        emitRM("LDC", r5, 0, 0, "load 0");
        emitRM("ST", ac, vecSize, r5, "store ac in global_position_aux");
    } else {
        emitRM("LDA", ac, tempOffset, fp, "guard vector address");
        emitRM("ST", ac, tempOffset, fp, "store vector address");
        tempOffset -= tree->child[0]->attr.val + 1;
    }
    if (TraceCode) emitComment("<- declare vector");
}

/* Gera código para instrução if.
   Reserva espaços para saltos e gera os blocos then e else. */
static void generateIfStatement(TreeNode *tree) {
    if (TraceCode) emitComment("-> if");
    cGen(tree->child[0]); // Gera condição
    int jumpElse = emitSkip(1); // Reserva espaço para salto para o else
    if (TraceCode) emitComment("if: jump to else belongs here");
    cGen(tree->child[1]);  // Gera bloco then
    int jumpEnd = emitSkip(1); // Reserva espaço para salto para o fim do if
    if (TraceCode) emitComment("if: jump to end belongs here");
    int elseAddr = jumpEnd + 1; // Calcula endereço para salto para o else
    emitBackup(jumpElse);
    emitRM_Abs("JEQ", ac, elseAddr, "if: jmp to else");
    emitRestore();
    if (tree->child[2])
        cGen(tree->child[2]);  // Gera bloco else, se existir
    int endLoc = emitSkip(0); // Marca fim do if
    emitBackup(jumpEnd);
    emitRM_Abs("LDA", PC, endLoc, "jmp to end");
    emitRestore();
    if (TraceCode) emitComment("<- if");
}

/* Gera código para laço while.
   Marca início da condição, gera corpo do loop e ajusta os saltos de saída. */
static void generateWhileStatement(TreeNode *tree) {
    if (TraceCode) emitComment("-> repeat");
    int whileCondLoc = emitSkip(0);  // Marca início da condição
    cGen(tree->child[0]);            // Gera condição
    int whileExitLoc = emitSkip(1);    // Reserva espaço para o salto de saída
    if (TraceCode) emitComment("repeat: jump after body comes back here");
    cGen(tree->child[1]);            // Gera corpo do loop
    emitRM_Abs("LDA", PC, whileCondLoc, "jump to savedLoc1"); // Salta de volta para a condição
    int whileEndLoc = emitSkip(0);     // Marca fim do loop
    emitBackup(whileExitLoc);
    emitRM_Abs("JEQ", ac, whileEndLoc, "repeat: jmp to end");
    emitRestore();
    if (TraceCode) emitComment("<- repeat");
}

/* Gera código para expressão de operação (OpK).
   Empilha o operando esquerdo, avalia o direito e aplica o operador. */
static void generateOpExpression(TreeNode *tree) {

    if (TraceCode) emitComment("-> Op");
    // Caso binário: avalia operando esquerdo e direito
    TreeNode *left = tree->child[0];
    TreeNode *right = tree->child[1];
    cGen(left); // Gera operando esquerdo
    emitRM("ST", ac, tempOffset--, fp, "op: push left");
    cGen(right); // Gera operando direito
    emitRM("LD", ac1, ++tempOffset, fp, "op: load left");
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
            if (TraceCode) emitComment("BUG: Unknown op");
            break;
    }
    if (TraceCode) emitComment("<- Op");
}

static void genStmt(TreeNode *tree) {
    switch (tree->kind.stmt) {
        case FunK:
            generateFunction(tree);
            break;
        case VarK:
            if (tree->isArray)
                generateVectorDeclaration(tree);
            else
                generateVarDeclaration(tree);
            break;
        case VetK:
            generateVectorDeclaration(tree);
            break;
        case ParamK:
            if (TraceCode) emitComment("-> Param");
            tempOffset--;
            if (TraceCode) emitComment("<- Param");
            break;
        case CompoundK:
        {
            if (TraceCode) emitComment("-> Compound Statement");
            char compoundName[40];
            sprintf(compoundName, "compound%d", compound_tracker);
            strcpy(currentScopeName, compoundName);
            cGen(tree->child[0]); // Gera declarações locais
            cGen(tree->child[1]); // Gera comandos do bloco
            if (TraceCode) emitComment("<- Compound Statement");
            break;
        }
        case IfK:
            generateIfStatement(tree);
            break;
        case IteraK:
            generateWhileStatement(tree);
            break;
        case ReturnK:
        {
            if (TraceCode) emitComment("-> return");
            cGen(tree->child[0]);
            emitRM("LDA", ac1, 0, fp, "save current fp into ac1");
            emitRM("LD", fp, 0, fp, "make fp = ofp");
            emitRM("LD", PC, -1, ac1, "return to caller");
            if (TraceCode) emitComment("<- return");
            break;
        }
        default:
            break;
    }
}

/* Gera código para expressões */
static void genExp(TreeNode *tree) {
    switch (tree->kind.exp) {
        case OpK:
            generateOpExpression(tree);
            break;
        case ConstK:
        {
            if (TraceCode) emitComment(" -> Const");
            emitRM("LDC", ac, tree->attr.val, 0, "load const");
            if (TraceCode) emitComment(" <- Const");
            break;
        }
        case IdK:
        {
            if (TraceCode) emitComment("-> Id");
            SymbolList varSymbol = st_lookup_from_given_scope(tree->attr.name, tree->scope);
            if (tree->isArray) {
                if (strcmp(varSymbol->scope, "global") == 0) {
                    emitRM("LDC", r5, 0, 0, "load 0");
                    emitRM("LD", ac, varSymbol->memloc, r5, "get the address of the vector");
                } else {
                    emitRM("LD", ac, varSymbol->memloc - MEMORY_SIZE, fp, "get the address of the vector");
                }
                TreeNode *p1 = tree->child[0];
                if (p1->nodekind == ExpK && p1->kind.exp == ConstK) {
                    emitRM("LDC", r3, p1->attr.val, 0, "load array index");
                } else {
                    SymbolList indexSymbol = st_lookup_from_given_scope(p1->attr.name, p1->scope);
                    emitRM("LD", r3, indexSymbol->memloc - MEMORY_SIZE, fp, "get the value of the index");
                }
                emitRM("LDC", r4, 1, 0, "load 1");
                emitRO("ADD", r3, r3, r4, "sub 3 by 1");
                emitRO("SUB", ac, ac, r3, "get the address");
                emitRM("LD", ac, 0, ac, "get the value of the vector");
            } else {
                if (strcmp(varSymbol->scope, "global") == 0) {
                    emitRM("LDC", r5, 0, 0, "load 0");
                    emitRM("LD", ac, varSymbol->memloc, r5, "load global variable");
                } else {
                    emitRM("LD", ac, varSymbol->memloc - MEMORY_SIZE, fp, "load id value");
                }
            }
            if (TraceCode) emitComment("<- Id");
            break;
        }
        case AssignK:
        {
            if (TraceCode) emitComment("-> assign");
            if (tree->child[0]->isArray) {
                if (TraceCode) emitComment("-> Vector");
                cGen(tree->child[1]);
                SymbolList assignSymbol = st_lookup_from_given_scope(tree->child[0]->attr.name, tree->child[0]->scope);
                if (strcmp(assignSymbol->scope, "global") == 0) {
                    emitRM("LDC", r5, 0, 0, "load 0");
                    emitRM("LD", ac1, assignSymbol->memloc, r5, "get the address of the vector");
                } else {
                    emitRM("LD", ac1, assignSymbol->memloc - MEMORY_SIZE, fp, "get the address of the vector");
                }
                TreeNode *p1 = tree->child[0]->child[0];
                if (p1->nodekind == ExpK && p1->kind.exp == ConstK) {
                    emitRM("LDC", r3, p1->attr.val, 0, "load array index");
                } else {
                    SymbolList indexSymbol = st_lookup_from_given_scope(p1->attr.name, p1->scope);
                    emitRM("LD", r3, indexSymbol->memloc - MEMORY_SIZE, fp, "get the value of the index");
                }
                emitRM("LDC", r4, 1, 0, "load 1");
                emitRO("ADD", r3, r3, r4, "sub 3 by 1");
                emitRO("SUB", ac1, ac1, r3, "get the address");
                emitRM("ST", ac, 0, ac1, "store vector value");
                if (TraceCode) emitComment("<- assign vector");
            } else {
                cGen(tree->child[1]);
                SymbolList assignSymbol = st_lookup_from_given_scope(tree->child[0]->attr.name, tree->child[0]->scope);
                emitRM("ST", ac, assignSymbol->memloc - MEMORY_SIZE, fp, "assign: store value");
            }
            if (TraceCode) emitComment("<- assign");
            break;
        }
        case FunCallK:
        {
            sprintf(comment, "-> Function Call (%s)", tree->attr.name);
            if (TraceCode) emitComment(comment);
            if (strcmp(tree->attr.name, "input") == 0) {
                emitRO("IN", ac, 0, 0, "read input");
                break;
            }
            if (strcmp(tree->attr.name, "output") == 0) {
                cGen(tree->child[0]);
                emitRO("OUT", ac, 0, 0, "print value");
                break;
            }
            int oldOffset = tempOffset;
            emitRM("ST", fp, tempOffset, fp, "Guard fp");
            tempOffset -= 2;
            TreeNode *p1 = tree->child[0];
            is_param_fun_call = TRUE;
            while (p1 != NULL) {
                cGen(p1);
                emitRM("ST", ac, tempOffset--, fp, "Store value of func argument");
                p1 = p1->sibling;
            }
            is_param_fun_call = FALSE;
            tempOffset = oldOffset;
            emitRM("LDA", fp, tempOffset, fp, "change fp");
            int returnAddress = emitSkip(0);
            emitRM("LDC", ac, returnAddress + 2, 0, "Guard return adress");
            emitRM_Abs("LDA", PC, get(tree->attr.name), "jump to function");
            if (TraceCode) emitComment("<- Function Call");
            break;
        }
        default:
            break;
    }
}

/* Função principal de geração de código: percorre recursivamente a árvore sintática */
static void cGen(TreeNode *tree) {
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
        if (!is_param_fun_call)
            cGen(tree->sibling);
    }
}

/* Função pública que inicia a geração de código.
   Configura o preâmbulo e chama cGen com a árvore sintática. */
void codeGen(TreeNode *syntaxTree) {
    emitComment("TINY Compilation to TM Code");
    emitComment("Standard prelude:");
    emitRM("LD", mp, 0, 0, "load maxaddress from location 0");
    emitRM("LD", fp, 0, 0, "load maxaddress from location 0");
    emitRM("ST", ac, 0, 0, "clear location 0");
    emitComment("End of standard prelude.");
    currentScopeName = "global";
    cGen(syntaxTree);
    emitComment("End of execution.");
    emitRO("HALT", 0, 0, 0, "");
}
