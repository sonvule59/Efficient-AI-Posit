%{
#include <stdio.h>
#include <stdlib.h>
#include "posit.h"

void yyerror(const char *s);
extern int yylex(void);
%}

%union {
    char *str;
}

%token POSIT8 POSIT16 POSIT32 QUIRE32 FLOAT DOUBLE
%token <str> POSIT_LITERAL IDENTIFIER
%token PLUS MINUS MULT DIV ASSIGN SEMICOLON LPAREN RPAREN LBRACE RBRACE

%type <str> type expression

%%

program:
    statements
    ;

statements:
    statement
    | statements statement
    ;

statement:
    type IDENTIFIER ASSIGN expression SEMICOLON {
        printf("Declaration: %s %s = %s;\n", $1, $2, $4);
        free($1); free($2); free($4);
    }
    | expression SEMICOLON {
        printf("Expression: %s;\n", $1);
        free($1);
    }
    ;

type:
    POSIT8  { $$ = strdup("posit8"); }
    | POSIT16 { $$ = strdup("posit16"); }
    | POSIT32 { $$ = strdup("posit32"); }
    | QUIRE32 { $$ = strdup("quire32"); }
    | FLOAT   { $$ = strdup("float"); }
    | DOUBLE  { $$ = strdup("double"); }
    ;

expression:
    POSIT_LITERAL { $$ = $1; }
    | IDENTIFIER  { $$ = $1; }
    | expression PLUS expression {
        $$ = malloc(256);
        snprintf($$, 256, "(%s + %s)", $1, $3);
        free($1); free($3);
    }
    | expression MULT expression {
        $$ = malloc(256);
        snprintf($$, 256, "(%s * %s)", $1, $3);
        free($1); free($3);
    }
    | LPAREN expression RPAREN { $$ = $2; }
    ;

%%

void yyerror(const char *s) {
    fprintf(stderr, "Error: %s\n", s);
}

int main(void) {
    return yyparse();
}