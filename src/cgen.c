/****************************************************/
/* File: cgen.c                                     */
/* Minimal code generator for the TINY compiler     */
/* (generates code for the TM machine)              */
/* Adapted for current project                      */
/****************************************************/

#include "globals.h"
#include "code.h"
#include "cgen.h"
#include <stdio.h>
#include "symtab.h"

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

static void add(const char* key, int value) {
    int index = get_hash_value(key);
    HashNode* newNode = malloc(sizeof(HashNode));
    newNode->key = strdup(key);
    newNode->value = value;
    newNode->next = hashTable[index];
    hashTable[index] = newNode;
}

static int get(const char* key) {
    int index = get_hash_value(key);
    HashNode* node = hashTable[index];
    while (node) {
        if (strcmp(node->key, key) == 0) {
            return node->value;
        }
        node = node->next;
    }
    return -1; // Key not found
}

static int tempOffset = -2;

static void cGen(TreeNode *tree);

static char *currentScopeName = "global";

bool isParamFuncall = FALSE;

static int compoundTracker = 0;

int main_loc = 3;

bool first_funk = TRUE;

static void genStmt(TreeNode *tree)
{
    TreeNode *p1, *p2, *p3;
    int savedLoc1, savedLoc2;

    switch (tree->kind.stmt) {
        case FunK:
        {
            char comment[40];
            sprintf(comment, "-> Init Function (%s)", tree->attr.name);
            emitComment(comment);

            const int funcLoc = emitSkip(0); 

            if (first_funk) {
                main_loc = emitSkip(1);
                first_funk = FALSE;
                add(tree->attr.name, main_loc + 1);
            } else {
                add(tree->attr.name, funcLoc);
            }

            currentScopeName = tree->attr.name;
            tempOffset = -2;

            if (strcmp(tree->attr.name, "main") == 0)
            {
                savedLoc1 = emitSkip(0);
                emitBackup(main_loc);
                if (savedLoc1 == 3)
                    emitRM_Abs("LDA", PC, savedLoc1 + 1, "jump to main");
                else
                    emitRM_Abs("LDA", PC, savedLoc1, "jump to main");
                emitRestore();

                cGen(tree->child[0]); 
                cGen(tree->child[1]); 

                emitComment("<- End Function");
                break;
            }

            emitRM("ST", ac, -1, fp, "store return address from ac");

            cGen(tree->child[0]); 
            cGen(tree->child[1]); 

            if (tree->type == Void)
            {
                emitRM("LDA", ac1, 0, fp, "save current fp into ac1");
                emitRM("LD", fp, 0, fp, "make fp = ofp");
                emitRM("LD", PC, -1, ac1, "return to caller");
            }

            emitComment("<- End Function");
            break;
        }

        case VarK:
        {
            if (tree->isArray)
            {
                emitComment("-> declare vector");
                if(strcmp(tree->scope->name, "global") == 0) {
                SymbolList vecSymbol = st_lookup_from_given_scope(tree->attr.name, tree->scope);
                int memloc = vecSymbol->memloc;
                int vecSize = tree->child[0]->attr.val;
                emitRM("LDC", ac, vecSize, 0, "load global position to ac");
                emitRM("LDC", r5, 0, 0, "load 0");
                emitRM("ST", ac, vecSize, r5, "store ac in global_position_aux");
                emitComment("<- declare vector");
                break;
            }
            emitRM("LDA", ac, tempOffset, fp, "guard vector address");
            emitRM("ST", ac, tempOffset, ac, "store vector address");
            tempOffset -= tree->child[0]->attr.val + 1;
            emitComment("<- declare vector");
            break;
            }
            
            emitComment("-> declare var");
            tempOffset--;
            emitComment("<- declare var");
            break;
        }

        case VetK: {
            emitComment("-> declare vector");
            if(strcmp(tree->scope->name, "global") == 0) {
                SymbolList vecSymbol = st_lookup_from_given_scope(tree->attr.name, tree->scope);
                int memloc = vecSymbol->memloc;
                emitRM("LDC", ac, memloc, 0, "load global position to ac");
                emitRM("LDC", r5, 0, 0, "load 0");
                emitRM("ST", ac, memloc, r5, "store ac in global_position_aux");
                emitComment("<- declare vector");
                break;
            }
            emitRM("LDA", ac, tempOffset, fp, "guard vector address");
            emitRM("ST", ac, tempOffset, ac, "store vector address");
            tempOffset -= tree->child[0]->attr.val + 1;
            emitComment("<- declare vector");
            break;
        }

        case ParamK: {
            emitComment("-> Param");
            tempOffset--;
            emitComment("<- Param");
            break;
        }

        case CompoundK:
        {
            emitComment("-> Compound Statement");

            sprintf(currentScopeName, "compound%d", compoundTracker);

            cGen(tree->child[0]);
            cGen(tree->child[1]);
            

            emitComment("<- Compound Statement");
            break;
        }

        case IfK: {
            if (TraceCode)
                emitComment("-> if");

            /* Gera código para a condição */
            cGen(tree->child[0]);

            /* Reserva espaço para o salto condicional (if false -> else) */
            int jumpElse = emitSkip(1);
            emitComment("if: jump to else belongs here");

            /* Gera código para o bloco "then" */
            cGen(tree->child[1]);

            /* Reserva espaço para pular o bloco "else" (se existir) */
            int jumpEnd = emitSkip(1);
            emitComment("if: jump to end belongs here");

            /* Ajusta o salto para o bloco "else" */
            emitBackup(jumpElse);
            emitRM_Abs("JEQ", ac,jumpEnd + 1 , "if: jmp to else");
            emitRestore();

            /* Gera código para o bloco "else" (se existir) */
            if(tree->child[2])
                cGen(tree->child[2]);

            /* Ajusta o salto para o fim do `if` */
            int endLoc = emitSkip(0); // Posição atual será o fim do if
            emitBackup(jumpEnd);
            emitRM_Abs("LDA", PC, endLoc, "jmp to end");
            emitRestore();

            if (TraceCode)
                emitComment("<- if");
            break;
        }

        case IteraK: {
            if (TraceCode)
                emitComment("-> while");

            /* 🔹 1. Marca posição do início da condição */
            int whileCondLoc = emitSkip(0);
            // emitComment("while: evaluate condition");

            /* 🔹 2. Gera código para a condição */
            cGen(tree->child[0]);

            /* 🔹 3. Salta para o fim do loop se a condição for falsa */
            int whileExitLoc = emitSkip(1);
            emitComment("repeat: jump after body comes back here");

            /* 🔹 4. Gera código para o corpo do loop */
            cGen(tree->child[1]);

            /* 🔹 5. Volta para a condição do while */
            emitRM_Abs("LDA", PC, whileCondLoc, "jump to savedLoc1");

            /* 🔹 6. Ajusta o salto de saída do loop */
            int whileEndLoc = emitSkip(0);
            emitBackup(whileExitLoc);
            emitRM_Abs("JEQ", ac, whileEndLoc, "repeat: jmp to end");
            emitRestore();

            if (TraceCode)
                emitComment("<- repeat");
            break;
        }

        case ReturnK: {
            emitComment("-> return");

            cGen(tree->child[0]);

            emitRM("LDA", ac1, 0, fp, "save current fp into ac1");
            emitRM("LD", fp, 0, fp, "make fp = ofp");
            emitRM("LD", PC, -1, ac1, "return to caller");

            emitComment("<- return");
            break;
        }

        default:
            break;
    }
}

static void genExp(TreeNode *tree)
{
    TreeNode *p1, *p2;
    switch (tree->kind.exp)
    {
    case OpK: {
        if (TraceCode)
            emitComment("-> Op");
        p1 = tree->child[0];
        p2 = tree->child[1];
        cGen(p1);
        // printf("DEBUG: Operação '%d' -> Primeiro operando (AC) = %d\n", tree->attr.op, ac);
        emitRM("ST", ac, tempOffset--, fp, "op: push left");
        cGen(p2);
        // printf("DEBUG: Operação '%d' -> Segundo operando (AC) = %d\n", tree->attr.op, ac);
        emitRM("LD", ac1, ++tempOffset, fp, "op: load left");
        switch (tree->attr.op)
        {
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
        if (TraceCode)
            emitComment("<- Op");
        break;
    }

    case ConstK: {
        emitComment(" -> Const");
        emitRM("LDC", ac, tree->attr.val, 0, "load const");
        emitComment(" <- Const");
        break;
    }

    case IdK: {
        emitComment("-> Id");

        SymbolList varSymbol = st_lookup_from_given_scope(tree->attr.name, tree->scope);

        if (tree->isArray)
        {
            if (strcmp(varSymbol->scope, "global") == 0)
            {
                emitRM("LDC", r5, 0, 0, "load 0");
                emitRM("LD", ac, varSymbol->memloc, r5, "get the address of the vector");
            }
            else
            {
                emitRM("LD", ac, varSymbol->memloc - MEMORY_SIZE, fp, "get the address of the vector");
            }

            p1 = tree->child[0];
            if (p1->nodekind == ExpK && p1->kind.exp == ConstK)
            {
                emitRM("LDC", r3, p1->attr.val, 0, "load array index");
            }
            else
            {
                SymbolList indexSymbol = st_lookup_from_given_scope(p1->attr.name, p1->scope);
                emitRM("LD", r3, indexSymbol->memloc - MEMORY_SIZE, fp, "get the value of the index");
            }

            emitRM("LDC", r4, 1, 0, "load 1");
            emitRO("ADD", r3, r3, r4, "sub 3 by 1");
            emitRO("SUB", ac, ac, r3, "get the address");
            emitRM("LD", ac, 0, ac, "get the value of the vector");
        }
        else
        {
            if (strcmp(varSymbol->scope, "global") == 0)
            {
                emitRM("LDC", r5, 0, 0, "load 0");
                emitRM("LD", ac, varSymbol->memloc, r5, "load global variable");
            }
            else
            {
                emitRM("LD", ac, varSymbol->memloc - MEMORY_SIZE, fp, "load id value");
            }
        }
        emitComment("<- Id");
        break;
    }

    case AssignK: {
        emitComment("-> assign"); 

        
        if (tree->child[0]->isArray)
        {
            emitComment("-> Vector");
            cGen(tree->child[1]);
            SymbolList assignSymbol = st_lookup_from_given_scope(tree->child[0]->attr.name, tree->child[0]->scope);
            if (strcmp(assignSymbol->scope, "global") == 0)
            {
                emitRM("LDC", r5, 0, 0, "load 0");
                emitRM("LD", ac1, assignSymbol->memloc, r5, "get the address of the vector");
            }
            else
            {
                emitRM("LD", ac1, assignSymbol->memloc - MEMORY_SIZE, fp, "get the address of the vector");
            }

            p1 = tree->child[0]->child[0];
            if (p1->nodekind == ExpK && p1->kind.exp == ConstK)
            {
                emitRM("LDC", r3, p1->attr.val, 0, "load array index");
            }
            else
            {
                SymbolList indexSymbol = st_lookup_from_given_scope(p1->attr.name, p1->scope);
                emitRM("LD", r3, indexSymbol->memloc - MEMORY_SIZE, fp, "get the value of the index");
            }

            emitRM("LDC", r4, 1, 0, "load 1");
            emitRO("ADD", r3, r3, r4, "sub 3 by 1");
            emitRO("SUB", ac1, ac1, r3, "get the address");
            emitRM("ST", ac, 0, ac1, "get the value of the vector");

            emitComment("<- Vector");
            emitComment("<- assign vector");
        }
        else
        {
            cGen(tree->child[1]);
            SymbolList assignSymbol = st_lookup_from_given_scope(tree->child[0]->attr.name, tree->child[0]->scope);
            emitRM("ST", ac, assignSymbol->memloc - MEMORY_SIZE, fp, "assign: store value");
        }
        emitComment("<- assign");
        break;
    }

    case FunCallK:
    {
        char comment[100];
        sprintf(comment, "-> Function Call (%s)", tree->attr.name);
        emitComment(comment);

        if (strcmp(tree->attr.name, "input") == 0)
        {
            emitRO("IN", ac, 0, 0, "read input");
            break;
        }

        if (strcmp(tree->attr.name, "output") == 0)
        {
            cGen(tree->child[0]);
            emitRO("OUT", ac, 0, 0, "print value");
            break;
        }

        int oldOffset = tempOffset;                   // Salva o deslocamento atual
        emitRM("ST", fp, tempOffset, fp, "Guard fp"); // Salva o fp do chamador
        tempOffset -= 2;                              // Reserva espaço para o antigo fp e para o endereço de retorno

        p1 = tree->child[0]; 
        isParamFuncall = TRUE;
        while (p1 != NULL)
        {
            cGen(p1);
            emitRM("ST", ac, tempOffset--, fp, "Store value of func argument");
            p1 = p1->sibling;
        }
        isParamFuncall = FALSE;
        tempOffset = oldOffset;

        emitRM("LDA", fp, tempOffset, fp, "change fp");
        int returnAddress = emitSkip(0);
        emitRM("LDC", ac, returnAddress + 2, 0, "Guard return adress");
        emitRM_Abs("LDA", PC, get(tree->attr.name), "jump to function");

        emitComment("<- Function Call");
        break;
    }

    default:
        break;
    }
}

static void cGen(TreeNode *tree)
{
    if (tree != NULL)
    {
        switch (tree->nodekind) {
            case StmtK: {
                genStmt(tree);
                break;
            }
            case ExpK: {
                genExp(tree);
                break;
            }
            default:
                break;
        }
        if (!isParamFuncall)
            cGen(tree->sibling);
    }
}


void codeGen(TreeNode *syntaxTree)
{
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