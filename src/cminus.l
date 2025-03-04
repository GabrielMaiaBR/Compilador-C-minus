%{
#include "globals.h"
#include "util.h"
#include "scan.h"
#include "log.h" 

/* lexeme of identifier or reserved word */
char tokenString[MAXTOKENLEN+1];
char prevTokenString[MAXTOKENLEN+1]; 

static void printCurrentToken(TokenType token, const char* tokenString) {
    pc("\t%d: ", lineno);
    printToken(token, tokenString);
}

%}

%option noyywrap

DIGIT           [0-9]
LETTER          [a-zA-Z]
IDENTIFIER      {LETTER}+
NUMBER          {DIGIT}+
NEWLINE         \n|\r\n
WHITESPACE      [ \t]+
COMMENT         "/*"([^*]|[\r\n]|(\*+([^*/]|[\r\n])))*\*+"/"

%%
{COMMENT}       { 
                  char *s;
                  for (s = yytext; *s; s++) {
                      if (*s == '\n') {
                          lineno++;
                          printLine();
                      }
                  }
                }
"if"            { return IF; }
"else"          { return ELSE; }
"int"           { return INT; }
"return"        { return RETURN; }
"void"          { return VOID; }
"while"         { return WHILE; }
"+"             { return PLUS; }
"-"             { return MINUS; }
"*"             { return TIMES; }
"/"             { return OVER; }
"<"             { return LT; }
"<="            { return LTE; }  
">"             { return GT; }
">="            { return GTE; }  
"=="            { return EQ; }   
"!="            { return NEQ; }  
"="             { return ASSIGN; }  
";"             { return SEMI; }
","             { return COMMA; }
"("             { return LPAREN; }
")"             { return RPAREN; }
"["             { return LBRACKET; }  
"]"             { return RBRACKET; }  
"{"             { return LBRACE; }    
"}"             { return RBRACE; }    
{NUMBER}        { yylval.val = atoi(yytext); return NUM; }
{IDENTIFIER}    { yylval.name = strdup(yytext); return ID; }
{NEWLINE}       { 
                  lineno++; 
                  printLine();
                }
{WHITESPACE}    { /* skip whitespace */ }
.               { return ERROR; }

%%

TokenType getToken(void)
{
    static int firstTime = TRUE;
    TokenType currentToken;
    
    if (firstTime)
    {
        firstTime = FALSE;
        yyin = source;
        yyout = listing;
        lineno++;
        printLine();
    }
    else {
        strncpy(prevTokenString, tokenString, MAXTOKENLEN);
    }
    
    currentToken = yylex();
    strncpy(tokenString, yytext, MAXTOKENLEN);
    
    if (TraceScan) {
        printCurrentToken(currentToken, tokenString);
    }
    
    return currentToken;
}