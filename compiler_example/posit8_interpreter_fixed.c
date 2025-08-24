#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "posit8.h"

/* Fixed Posit8 Interpreter
 * 
 * This interpreter uses the corrected posit8 arithmetic
 */

#define MAX_LINE 256

typedef enum {
    TOKEN_NUMBER,
    TOKEN_POSIT8,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_MULT,
    TOKEN_DIV,
    TOKEN_EOF,
    TOKEN_ERROR
} token_type_t;

typedef struct {
    token_type_t type;
    union {
        double number;
        posit8 p8;
    } value;
} token_t;

typedef struct {
    char *input;
    int pos;
    token_t current;
} lexer_t;

/* Lexer functions */
void lexer_init(lexer_t *lexer, char *input) {
    lexer->input = input;
    lexer->pos = 0;
}

void skip_whitespace(lexer_t *lexer) {
    while (lexer->input[lexer->pos] && isspace(lexer->input[lexer->pos])) {
        lexer->pos++;
    }
}

token_t get_next_token(lexer_t *lexer) {
    skip_whitespace(lexer);
    
    if (!lexer->input[lexer->pos]) {
        return (token_t){TOKEN_EOF, {0}};
    }
    
    char c = lexer->input[lexer->pos];
    
    // Handle operators
    if (c == '+') {
        lexer->pos++;
        return (token_t){TOKEN_PLUS, {0}};
    }
    if (c == '-') {
        lexer->pos++;
        return (token_t){TOKEN_MINUS, {0}};
    }
    if (c == '*') {
        lexer->pos++;
        return (token_t){TOKEN_MULT, {0}};
    }
    if (c == '/') {
        lexer->pos++;
        return (token_t){TOKEN_DIV, {0}};
    }
    
    // Handle numbers and posit8 literals
    if (isdigit(c) || c == '.') {
        char *end;
        double number = strtod(lexer->input + lexer->pos, &end);
        
        // Check if it's a posit8 literal (ends with 'p8')
        if (end[0] == 'p' && end[1] == '8') {
            posit8 p8;
            posit8_error_t err = posit8_from_double(&p8, number);
            if (err == POSIT8_OK) {
                lexer->pos += (end - (lexer->input + lexer->pos)) + 2;
                return (token_t){TOKEN_POSIT8, {.p8 = p8}};
            }
        }
        
        // Regular number
        lexer->pos += (end - (lexer->input + lexer->pos));
        return (token_t){TOKEN_NUMBER, {.number = number}};
    }
    
    // Unknown character
    lexer->pos++;
    return (token_t){TOKEN_ERROR, {0}};
}

/* Parser and evaluator */
posit8 evaluate_expression(lexer_t *lexer) {
    posit8 left, right;
    posit8_error_t err;
    
    // Get first operand
    token_t token = get_next_token(lexer);
    if (token.type == TOKEN_POSIT8) {
        left = token.value.p8;
    } else if (token.type == TOKEN_NUMBER) {
        err = posit8_from_double(&left, token.value.number);
        if (err != POSIT8_OK) {
            printf("Error: Invalid number\n");
            return POSIT8_NAR;
        }
    } else {
        printf("Error: Expected number or posit8 literal\n");
        return POSIT8_NAR;
    }
    
    // Get operator
    token = get_next_token(lexer);
    if (token.type == TOKEN_EOF) {
        return left; // Single value
    }
    
    // Get second operand
    token_t next_token = get_next_token(lexer);
    if (next_token.type == TOKEN_POSIT8) {
        right = next_token.value.p8;
    } else if (next_token.type == TOKEN_NUMBER) {
        err = posit8_from_double(&right, next_token.value.number);
        if (err != POSIT8_OK) {
            printf("Error: Invalid number\n");
            return POSIT8_NAR;
        }
    } else {
        printf("Error: Expected number or posit8 literal\n");
        return POSIT8_NAR;
    }
    
    // Perform operation
    posit8 result;
    switch (token.type) {
        case TOKEN_PLUS:
            err = posit8_add(&result, left, right);
            break;
        case TOKEN_MINUS:
            err = posit8_sub(&result, left, right);
            break;
        case TOKEN_MULT:
            err = posit8_mul(&result, left, right);
            break;
        case TOKEN_DIV:
            err = posit8_div(&result, left, right);
            break;
        default:
            printf("Error: Unknown operator\n");
            return POSIT8_NAR;
    }
    
    if (err != POSIT8_OK) {
        printf("Error: Arithmetic operation failed\n");
        return POSIT8_NAR;
    }
    
    return result;
}

/* Display functions */
void print_posit8_info(posit8 p) {
    float f_result;
    double d_result;
    
    printf("Posit8: 0x%02x\n", p);
    
    posit8_to_float(&f_result, p);
    printf("As float: %.6f\n", f_result);
    
    posit8_to_double(&d_result, p);
    printf("As double: %.6f\n", d_result);
    
    // Show binary representation
    printf("Binary: ");
    for (int i = 7; i >= 0; i--) {
        printf("%d", (p >> i) & 1);
        if (i == 7) printf(" "); // Sign bit
        if (i == 6) printf(" "); // First regime bit
    }
    printf("\n");
}

/* Main interpreter loop */
void interpreter_loop(void) {
    char line[MAX_LINE];
    lexer_t lexer;
    
    printf("=== Fixed Posit8 Interpreter ===\n");
    printf("Enter expressions like: 1.5p8 + 2.0p8\n");
    printf("Or: 1.5 + 2.0 (will be converted to posit8)\n");
    printf("Type 'quit' to exit\n\n");
    
    while (1) {
        printf("> ");
        if (!fgets(line, MAX_LINE, stdin)) {
            break;
        }
        
        // Remove newline
        line[strcspn(line, "\n")] = 0;
        
        if (strcmp(line, "quit") == 0) {
            break;
        }
        
        if (strlen(line) == 0) {
            continue;
        }
        
        lexer_init(&lexer, line);
        posit8 result = evaluate_expression(&lexer);
        
        if (result != POSIT8_NAR) {
            print_posit8_info(result);
        }
        
        printf("\n");
    }
    
    printf("Goodbye!\n");
}

int main(void) {
    interpreter_loop();
    return 0;
} 