%{
#include "parser.tab.h"
#include <string.h>
%}

%option noyywrap

%%

"posit8"        { return POSIT8; }
"posit16"       { return POSIT16; }
"posit32"       { return POSIT32; }
"quire32"       { return QUIRE32; }
"float"         { return FLOAT; }
"double"        { return DOUBLE; }
[0-9]+(\.[0-9]+)?p(8|16|32) { yylval.str = strdup(yytext); return POSIT_LITERAL; }
[0-9]+(\.[0-9]+)? { yylval.str = strdup(yytext); return POSIT_LITERAL; }
[a-zA-Z_][a-zA-Z0-9_]* { yylval.str = strdup(yytext); return IDENTIFIER; }
"+"             { return PLUS; }
"-"             { return MINUS; }
"*"             { return MULT; }
"/"             { return DIV; }
"="             { return ASSIGN; }
";"             { return SEMICOLON; }
"("             { return LPAREN; }
")"             { return RPAREN; }
"{"             { return LBRACE; }
"}"             { return RBRACE; }
[ \t\n]         { /* Ignore whitespace */ }
.               { printf("Unknown token: %s\n", yytext); }

%%