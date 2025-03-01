/****************************************************/
/* File: cgen.c                                     */
/* Minimal code generator for the TINY compiler     */
/* (generates code for the TM machine)              */
/* Adapted for current project                      */
/****************************************************/

#include "globals.h"
#include "symtab.h"
#include "code.h"
#include "cgen.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Define gp (global pointer) como o registrador 3 */
#define gp r3

/* tmpOffset é usado para armazenamento temporário */
static int tmpOffset = 0;

/* Protótipo da função interna recursiva */
static void cGen(TreeNode * tree);

/* Gera código para nós de declaração ou comando */
static void genStmt(TreeNode * tree)
{
    TreeNode *p1, *p2, *p3;
    int savedLoc1, savedLoc2, currentLoc;
    int loc;
    switch (tree->kind.stmt) {
        case IfK:
            if (TraceCode) emitComment("-> if");
            p1 = tree->child[0];  /* condição */
            p2 = tree->child[1];  /* ramo then */
            p3 = tree->child[2];  /* ramo else */
            cGen(p1);
            savedLoc1 = emitSkip(1);
            emitComment("if: jump to else");
            cGen(p2);
            savedLoc2 = emitSkip(1);
            emitComment("if: jump to end");
            currentLoc = emitSkip(0);
            emitBackup(savedLoc1);
            emitRM_Abs("JEQ", ac, currentLoc, "if: jmp to else");
            emitRestore();
            cGen(p3);
            currentLoc = emitSkip(0);
            emitBackup(savedLoc2);
            emitRM_Abs("LDA", PC, currentLoc, "jmp to end");
            emitRestore();
            if (TraceCode) emitComment("<- if");
            break;
        case IteraK:
            if (TraceCode) emitComment("-> iter");
            p1 = tree->child[0];  /* condição */
            p2 = tree->child[1];  /* corpo */
            savedLoc1 = emitSkip(0);
            emitComment("iter: jump after body");
            cGen(p1);
            savedLoc2 = emitSkip(1);
            emitComment("iter: jump to end");
            cGen(p2);
            emitRM_Abs("LDA", PC, savedLoc1, "jump to condition");
            savedLoc1 = emitSkip(0);
            emitBackup(savedLoc2);
            emitRM_Abs("JEQ", ac, savedLoc1, "iter: jmp to end");
            emitRestore();
            if (TraceCode) emitComment("<- iter");
            break;
        case AssignK:
            if (TraceCode) emitComment("-> assign");
            cGen(tree->child[0]);  /* gera código para o lado direito */
            loc = 0;  /* Neste esqueleto, assumimos offset fixo 0 */
            emitRM("ST", ac, loc, gp, "assign: store value");
            if (TraceCode) emitComment("<- assign");
            break;
        case ReturnK:
            if (TraceCode) emitComment("-> return");
            if (tree->child[0] != NULL)
                cGen(tree->child[0]);
            emitRO("HALT", 0, 0, 0, "return");
            if (TraceCode) emitComment("<- return");
            break;
        default:
            /* Para CompoundK e demais, processa recursivamente os filhos */
            for (int i = 0; i < MAXCHILDREN; i++) {
                if (tree->child[i] != NULL)
                    cGen(tree->child[i]);
            }
            break;
    }
}

/* Gera código para nós de expressão */
static void genExp(TreeNode * tree)
{
    int loc;
    TreeNode *p1, *p2;
    switch (tree->kind.exp) {
        case ConstK:
            if (TraceCode) emitComment("-> Const");
            emitRM("LDC", ac, tree->attr.val, 0, "load constant");
            if (TraceCode) emitComment("<- Const");
            break;
        case IdK:
            if (TraceCode) emitComment("-> Id");
            loc = 0;  /* Neste esqueleto, assume offset 0 */
            emitRM("LD", ac, loc, gp, "load id");
            if (TraceCode) emitComment("<- Id");
            break;
        case OpK:
            if (TraceCode) emitComment("-> Op");
            p1 = tree->child[0];
            p2 = tree->child[1];
            cGen(p1);
            emitRM("ST", ac, tmpOffset--, mp, "op: push left");
            cGen(p2);
            emitRM("LD", ac1, ++tmpOffset, mp, "op: load left");
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
                    emitRM("JLT", ac, 2, PC, "branch if true");
                    emitRM("LDC", ac, 0, ac, "false case");
                    emitRM("LDA", PC, 1, PC, "jmp");
                    emitRM("LDC", ac, 1, ac, "true case");
                    break;
                case EQ:
                    emitRO("SUB", ac, ac1, ac, "op ==");
                    emitRM("JEQ", ac, 2, PC, "branch if true");
                    emitRM("LDC", ac, 0, ac, "false case");
                    emitRM("LDA", PC, 1, PC, "jmp");
                    emitRM("LDC", ac, 1, ac, "true case");
                    break;
                default:
                    emitComment("BUG: Unknown operator");
                    break;
            }
            if (TraceCode) emitComment("<- Op");
            break;
        default:
            break;
    }
}

/* Função recursiva que percorre a árvore e gera código */
static void cGen(TreeNode * tree)
{
    if (tree != NULL) {
        if (tree->nodekind == StmtK)
            genStmt(tree);
        else if (tree->nodekind == ExpK)
            genExp(tree);
        cGen(tree->sibling);
    }
}

/* Função principal de geração de código */
void codeGen(TreeNode * syntaxTree)
{
    emitComment("TINY Compilation to TM Code");
    emitComment("Standard prelude:");
    emitRM("LD", mp, 0, ac, "load maxaddress");
    emitRM("ST", ac, 0, ac, "clear location 0");
    emitComment("End of prelude.");
    cGen(syntaxTree);
    emitComment("End of execution.");
    emitRO("HALT", 0, 0, 0, "");
}
