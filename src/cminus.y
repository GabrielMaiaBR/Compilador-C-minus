/****************************************************/
/* File: cminus.y                                   */
/* The CMINUS Yacc/Bison specification file         */
/* Compiler Construction: Principles and Practice   */
/* originally Kenneth C. Louden, adapted            */
/****************************************************/
%{
#define YYPARSER /* distinguishes Yacc output from other code files */

#include "globals.h"
#include "util.h"
#include "scan.h"
#include "parse.h"
#include "log.h"

#ifndef _PARSE_H_
#define _PARSE_H_

TreeNode* parse(void);

#endif

static char* savedName; /* for use in assignments */
static int savedLineNo;  /* ditto */
static TreeNode* savedTree; /* stores syntax tree for later return */
static int yylex(void);
int yyerror(char *);
%}

%union {
  int val;
  char *name;
  TokenType token;
  TreeNode* node;
  ExpType type;
}

/* Token declarations */
%token <name> ID
%token <val>  NUM
%token <type> INT VOID
%token IF ELSE WHILE RETURN
%token EQ NEQ ASSIGN LTE GTE LT GT PLUS MINUS TIMES OVER 
%token LPAREN RPAREN SEMI COMMA LBRACKET RBRACKET LBRACE RBRACE 
%token ERROR 

/* Type declarations */
%type <node>  program decl_list decl
%type <node>  var_decl fun_decl
%type <node>  params param_list param compound_decl
%type <node>  local_decl list_stmt stmt
%type <node>  exp_decl selection_decl iteration_decl return_decl
%type <node>  exp simple_exp sum_exp var
%type <node>  term factor func_activation args args_list 
%type <token> sum mult relational 
%type <type>  type_sp 

%% /* Grammar for CMINUS */

program     : decl_list
                 { savedTree = $1; } 
            ;

decl_list   : decl_list decl {
                TreeNode *current = $1;
                if(current != NULL) {
                    while(current->sibling != NULL)
                        current = current->sibling;
                    current->sibling = $2;
                    $$ = $1;
                }
                else $$ = $2;
            }
            | decl { $$ = $1; }
            ;

decl        : var_decl { $$ = $1; }
            | fun_decl { $$ = $1; }
            ;

var_decl    : type_sp ID SEMI
            {
              $$ = newStmtNode(VarK);
              $$->attr.name = $2;
              $$->type = $1;
              $$->isArray = FALSE;
              $$->lineno = lineno;
            }
             | type_sp ID LBRACKET NUM RBRACKET SEMI
             {
              $$ = newStmtNode(VarK);
              $$->attr.name = $2;
              $$->type = $1;
              $$->isArray = TRUE;
              $$->child[0] = newExpNode(ConstK);
              $$->child[0]->attr.val = $4;
              $$->lineno = lineno;
             }
            ;
type_sp:
  INT
    { $$ = Integer; }
  | VOID
    { $$ = Void; }
  ;

fun_decl    : type_sp ID {savedLineNo = lineno;} LPAREN params RPAREN compound_decl
              {
                $$ = newStmtNode(FunK);
                $$->attr.name = $2;
                $$->type = $1;
                $$->child[0] = $5;
                $$->child[1] = $7;
                $$->lineno = savedLineNo;
              }
            ;

params      : param_list { $$ = $1; }
            | VOID { $$ = NULL; }
            ;

param_list  : param_list COMMA param {
                TreeNode *current = $1;
                if(current != NULL) {
                    while(current->sibling != NULL)
                        current = current->sibling;
                    current->sibling = $3;
                    $$ = $1;
                }
                else $$ = $3;
            }
            | param { $$ = $1; }
            ;

param       : type_sp ID 
              {
                $$ = newStmtNode(ParamK);
                $$->attr.name = $2;
                $$->type = $1;
                $$->isArray = FALSE;
                $$->lineno = lineno;
              }
              | type_sp ID LBRACKET RBRACKET
              {
                $$ = newStmtNode(ParamK);
                $$->attr.name = $2;
                $$->type = $1;
                $$->isArray = TRUE;
                $$->lineno = lineno;
              }
            ;

compound_decl : LBRACE local_decl list_stmt RBRACE
                {
                  $$ = newStmtNode(CompoundK);
                  $$->child[0] = $2;
                  $$->child[1] = $3;
                  $$->lineno = lineno;
                }
              ;

local_decl : local_decl var_decl {
                TreeNode *current = $1;
                if(current != NULL) {
                    while(current->sibling != NULL)
                        current = current->sibling;
                    current->sibling = $2;
                    $$ = $1;
                }
                else $$ = $2;
             }
           | { $$ = NULL; }
           ;

list_stmt : list_stmt stmt {
                TreeNode *current = $1;
                if(current != NULL) {
                    while(current->sibling != NULL)
                        current = current->sibling;
                    current->sibling = $2;
                    $$ = $1;
                }
                else $$ = $2;
            }
          | { $$ = NULL; }
          ;

stmt      : exp_decl { $$ = $1; }
          | compound_decl { 
              $$ = $1; 
          }
          | selection_decl { $$ = $1; }
          | iteration_decl { $$ = $1; }
          | return_decl { $$ = $1; }
          ;

exp_decl  : exp SEMI { $$ = $1; }
          | SEMI { $$ = NULL; }
          ;

selection_decl : IF LPAREN exp RPAREN stmt
                {
                    $$ = newStmtNode(IfK);
                    $$->child[0] = $3;
                    $$->child[1] = $5;
                    $$->lineno = lineno;
                }
            | IF LPAREN exp RPAREN stmt ELSE stmt
                {
                    $$ = newStmtNode(IfK);
                    $$->child[0] = $3;
                    $$->child[1] = $5;
                    $$->child[2] = $7;
                    $$->lineno = lineno;
                }
              ;


iteration_decl : WHILE LPAREN exp RPAREN stmt {
                  $$ = newStmtNode(IteraK);
                  $$->child[0] = $3;
                  $$->child[1] = $5;
                  $$->lineno = lineno;
                }
              ;

return_decl : RETURN SEMI {
                $$ = newStmtNode(ReturnK);
                $$->lineno = lineno;
             }
            | RETURN exp SEMI {
                $$ = newStmtNode(ReturnK);
                $$->child[0] = $2;
                $$->lineno = lineno;
             }
            ;

exp       : var ASSIGN exp {
              $$ = newExpNode(AssignK);
              $$->child[0] = $1;
              $$->child[1] = $3;
              $$->lineno = lineno;
            }
          | simple_exp { $$ = $1; }
          ;

var:
    ID
        {
            $$ = newExpNode(IdK);
            $$->attr.name = $1;
            $$->isArray = FALSE;
            $$->lineno = lineno;
        }
    | ID LBRACKET exp RBRACKET
        {
            $$ = newExpNode(IdK);
            $$->attr.name = $1;
            $$->child[0] = $3;
            $$->isArray = TRUE;
            $$->lineno = lineno;
        }
    ;


simple_exp : sum_exp relational sum_exp
              {
                $$ = newExpNode(OpK);
                $$->child[0] = $1;
                $$->child[1] = $3;
                $$->attr.op = $2;
                $$->lineno = lineno;
              }
           | sum_exp { $$ = $1; }
           ;

relational: 
    LTE
        { $$ = LTE; }
    | LT
        { $$ = LT; }
    | GT
        { $$ = GT; }
    | GTE
        { $$ = GTE; }
    | EQ
        { $$ = EQ; }
    | NEQ
        { $$ = NEQ; }
    ;

sum_exp   : sum_exp sum term
            {
              $$ = newExpNode(OpK);
              $$->child[0] = $1;
              $$->child[1] = $3;
              $$->attr.op = $2;
              $$->lineno = lineno;
            }
          | term { $$ = $1; }
          ;

sum:
    PLUS
        { $$ = PLUS; }
    | MINUS
        { $$ = MINUS; }
    ;

term      : term mult factor
            {
              $$ = newExpNode(OpK);
              $$->child[0] = $1;
              $$->child[1] = $3;
              $$->attr.op = $2;
              $$->lineno = lineno;
            }
          | factor { $$ = $1; }
          ;

mult:
    TIMES
        { $$ = TIMES; }
    | OVER
        { $$ = OVER; }
    ;

factor    :     LPAREN exp RPAREN
                 { $$ = $2; }
                | var
                    { $$ = $1; }
                | func_activation
                    { $$ = $1; }
                | NUM
                    {
                        $$ = newExpNode(ConstK);
                        $$->attr.val = $1;
                        $$->lineno = lineno;
                    }
          ;

func_activation : ID LPAREN args RPAREN {
                    $$ = newExpNode(FunCallK);
                    $$->child[0] = $3;
                    $$->attr.name = $1;
                    $$->lineno = lineno;
                 }
               ;

args      : args_list { $$ = $1; }
          | { $$ = NULL; }
          ;

args_list : args_list COMMA exp {
                TreeNode *current = $1;
                if(current != NULL) {
                    while(current->sibling != NULL)
                        current = current->sibling;
                    current->sibling = $3;
                    $$ = $1;
                }
                else $$ = $3;
            }
          | exp { $$ = $1; }
          ;

%%

int yyerror(char *message) {
    pce("Syntax error at line %d: %s\n", lineno, message);
    pce("Current token: ");
    printToken(yychar, tokenString);
    Error = TRUE;
    return 0;
}

static int yylex(void) { 
    return getToken(); 
}

TreeNode *parse(void) {
    yyparse();
    return savedTree;
}