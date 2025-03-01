/****************************************************/
/* File: analyze.h                                  */
/* Semantic analyzer interface for TINY compiler    */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#ifndef _ANALYZE_H_
#define _ANALYZE_H_

#include "globals.h"
#include "log.h"

#define MAX_LEN 100

void enterNewScope(char* scopeName);

void exitScope(TreeNode *tree);

void buildSymtab(TreeNode*);

void typeCheck(TreeNode*);

static char* getCurrentScope(void);

static void generateScopeName(char* fullScope);

static void semanticError(TreeNode* node, const char* format, ...);

static void traverse(TreeNode* tree, void (*preProcess)(TreeNode*), void (*postProcess)(TreeNode*));

static void insertNode(TreeNode* node);

static void checkNode(TreeNode* node);

static void checkForMainFunction(TreeNode* node);

static void nullProc(TreeNode* node);

#endif