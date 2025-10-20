%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "posit8.h"
#include "posit16.h"

void yyerror(const char *s);
extern int yylex(void);

/* Simple temp generator for codegen */
static int temp_counter = 0;
static int result_counter = 0;
static char* new_temp(void) {
    char buf[32];
    snprintf(buf, sizeof(buf), "t%d", temp_counter++);
    return strdup(buf);
}
static char* new_result_var(void) {
    char buf[32];
    snprintf(buf, sizeof(buf), "result%d", result_counter++);
    return strdup(buf);
}

/* Strip posit suffix like p8/p16/p32 from literal text; return malloc'd */
static char* strip_posit_suffix(const char *text) {
    const char *p = strchr(text, 'p');
    if (!p) return strdup(text);
    size_t n = (size_t)(p - text);
    char *out = (char*)malloc(n + 1);
    memcpy(out, text, n);
    out[n] = '\0';
    return out;
}
%}

%union {
    char *str;
}

%token POSIT8 POSIT16 POSIT32 QUIRE32 FLOAT DOUBLE
%token <str> POSIT_LITERAL IDENTIFIER
%token PLUS MINUS MULT DIV ASSIGN SEMICOLON LPAREN RPAREN LBRACE RBRACE

%left PLUS MINUS
%left MULT DIV
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
        if (strcmp($1, "posit8") == 0) {
            /* Declare and assign from expression temp/value */
            printf("    posit8 %s = %s;\n", $2, $4);
            char *result_var = new_result_var();
            printf("    double %s;\n", result_var);
            printf("    posit8_to_double(&%s, %s);\n", result_var, $2);
            printf("    printf(\"Result: %%.6f\\n\", %s);\n", result_var);
            free(result_var);
        } else if (strcmp($1, "posit16") == 0) {
            /* Declare and assign from expression temp/value */
            printf("    posit16 %s = %s;\n", $2, $4);
            char *result_var = new_result_var();
            printf("    double %s;\n", result_var);
            printf("    posit16_to_double(&%s, %s);\n", result_var, $2);
            printf("    printf(\"Result: %%.6f\\n\", %s);\n", result_var);
            free(result_var);
        } else {
            /* For now: only posit8 and posit16 codegen are supported */
            printf("    /* TODO: codegen for type %s */\n", $1);
        }
        free($1); free($2); free($4);
    }
    | expression SEMICOLON {
        /* Expression statement result already materialized via temps */
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
    POSIT_LITERAL {
        /* Materialize literal into appropriate posit temp via from_double */
        char *num = strip_posit_suffix($1);
        char *t = new_temp();
        
        /* Determine posit type from literal suffix */
        if (strstr($1, "p16") != NULL) {
            printf("    posit16 %s;\n", t);
            printf("    posit16_from_double(&%s, %s);\n", t, num);
        } else {
            /* Default to posit8 for p8 or no suffix */
            printf("    posit8 %s;\n", t);
            printf("    posit8_from_double(&%s, %s);\n", t, num);
        }
        
        free(num);
        free($1);
        $$ = t;
    }
    | IDENTIFIER  { $$ = $1; }
    | expression PLUS expression {
        char *t = new_temp();
        /* Determine result type based on operands - for now, default to posit8 */
        printf("    posit8 %s;\n", t);
        printf("    posit8_add(&%s, %s, %s);\n", t, $1, $3);
        free($1); free($3);
        $$ = t;
    }
    | expression MINUS expression {
        char *t = new_temp();
        printf("    posit8 %s;\n", t);
        printf("    posit8_sub(&%s, %s, %s);\n", t, $1, $3);
        free($1); free($3);
        $$ = t;
    }
    | expression MULT expression {
        char *t = new_temp();
        printf("    posit8 %s;\n", t);
        printf("    posit8_mul(&%s, %s, %s);\n", t, $1, $3);
        free($1); free($3);
        $$ = t;
    }
    | expression DIV expression {
        char *t = new_temp();
        printf("    posit8 %s;\n", t);
        printf("    posit8_div(&%s, %s, %s);\n", t, $1, $3);
        free($1); free($3);
        $$ = t;
    }
    | LPAREN expression RPAREN { $$ = $2; }
    ;

%%

void yyerror(const char *s) {
    fprintf(stderr, "Error: %s\n", s);
}

int main(void) {
    /* Prolog */
    printf("#include <stdio.h>\n");
    printf("#include \"posit8.h\"\n");
    printf("#include \"posit16.h\"\n\n");
    printf("int main(void) {\n");

    int rc = yyparse();

    /* Epilog */
    printf("    return 0;\n");
    printf("}\n");
    return rc;
}