#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <stdint.h>
#include <stdbool.h>

/* Posit8 Interpreter with Standard <8,0> Implementation
 * es=0, useed=2, scale=k*1, dynamic range 2^{-6} to 2^6
 * Fixed for accurate decoding, arithmetic, and rounding
 */

#define MAX_LINE 256
#define POSIT8_NAR 0x80
#define POSIT8_ZERO 0x00
#define POSIT8_OK 0
#define ES 0  // Standard for posit8
#define USEED (1LL << (1 << ES))  // 2 for es=0
#define NBITS 8
#define FRAC_PREC 32  // Internal mantissa precision

typedef uint8_t posit8;
typedef int posit8_error_t;

typedef struct {
    bool sign;
    int scale;  // k * (1 << ES), e=0 for es=0
    uint64_t mant;  // Implicit 1 + fraction, shifted left
} unpacked_t;

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

unpacked_t unpack_posit8(posit8 p) {
    unpacked_t u = {false, 0, 0};
    if (p == POSIT8_ZERO) return u;
    if (p == POSIT8_NAR) { u.mant = 0; return u; }
    u.sign = (p & 0x80) != 0;
    uint8_t bits = u.sign ? ((~p & 0xFF) + 1) : p;
    bool regime_bit = (bits & 0x40) != 0;
    int run_len = 1;
    int bit_pos = 5;
    while (bit_pos >= 0 && (((bits >> bit_pos) & 1) == regime_bit)) {
        run_len++;
        bit_pos--;
    }
    int k = regime_bit ? (run_len - 1) : -run_len;
    int regime_bits_used = run_len + 1;
    int frac_start = bit_pos;
    int frac_len = (frac_start >= 0) ? (frac_start + 1) : 0;
    uint64_t frac = bits & ((1ULL << frac_len) - 1);
    u.mant = (1ULL << frac_len) | frac;
    u.scale = k * (1 << ES);
    u.mant <<= (FRAC_PREC - frac_len - 1);
    return u;
}

posit8 pack_posit8(unpacked_t u) {
    if (u.mant == 0) return POSIT8_ZERO;
    int k = u.scale;
    bool regime_bit = (k >= 0);
    int run_len = regime_bit ? (k + 1) : -k;
    int regime_bits_used = run_len + 1;
    if (regime_bits_used > NBITS - 1) {
        return u.sign ? POSIT8_NAR : (0x80 - 1);
    }
    int frac_bits_used = NBITS - 1 - regime_bits_used;
    int shift = FRAC_PREC - frac_bits_used - 1;
    uint64_t frac = (u.mant >> shift) & ((1ULL << frac_bits_used) - 1);
    uint64_t round = (shift > 0) ? ((u.mant >> (shift - 1)) & 1) : 0;
    uint64_t sticky = (shift > 1) ? (u.mant & ((1ULL << (shift - 1)) - 1)) != 0 : 0;
    if (round && (sticky || (frac & 1))) frac++;
    if (frac >= (1ULL << frac_bits_used)) {
        k++;
        regime_bit = (k >= 0);
        run_len = regime_bit ? (k + 1) : -k;
        regime_bits_used = run_len + 1;
        if (regime_bits_used > NBITS - 1) {
            return u.sign ? POSIT8_NAR : (0x80 - 1);
        }
        frac_bits_used = NBITS - 1 - regime_bits_used;
        frac = 0;
    }
    uint8_t bits = 0;
    int curr_bit = NBITS - 2;
    for (int i = 0; i < run_len; i++) {
        if (regime_bit) bits |= (1 << curr_bit);
        curr_bit--;
    }
    if (!regime_bit) bits |= (1 << curr_bit);
    curr_bit--;
    for (int i = frac_bits_used - 1; i >= 0; i--) {
        if (frac & (1ULL << i)) bits |= (1 << curr_bit);
        curr_bit--;
    }
    if (u.sign) bits = ~bits + 1;
    return bits;
}

posit8 posit8_op_add_sub(posit8 a, posit8 b, bool sub) {
    unpacked_t ua = unpack_posit8(a);
    unpacked_t ub = unpack_posit8(b);
    if (sub) ub.sign = !ub.sign;
    if (ua.mant == 0) return pack_posit8(ub);
    if (ub.mant == 0) return pack_posit8(ua);
    if (ua.scale < ub.scale) { unpacked_t tmp = ua; ua = ub; ub = tmp; }
    int64_t manta = ua.sign ? -(int64_t)ua.mant : (int64_t)ua.mant;
    int64_t mantb = ub.sign ? -(int64_t)ub.mant : (int64_t)ub.mant;
    int diff = ua.scale - ub.scale;
    mantb >>= diff;
    int64_t mantr = manta + mantb;
    bool signr = mantr < 0;
    mantr = labs(mantr);
    int scaler = ua.scale;
    if (mantr == 0) return POSIT8_ZERO;
    while (mantr < (1LL << (FRAC_PREC - 1))) { mantr <<= 1; scaler--; }
    while (mantr >= (1LL << FRAC_PREC)) { mantr >>= 1; scaler++; }
    unpacked_t ur = {signr, scaler, (uint64_t)mantr};
    return pack_posit8(ur);
}

posit8 posit8_add(posit8 a, posit8 b) { return posit8_op_add_sub(a, b, false); }
posit8 posit8_sub(posit8 a, posit8 b) { return posit8_op_add_sub(a, b, true); }

posit8 posit8_mul(posit8 a, posit8 b) {
    unpacked_t ua = unpack_posit8(a);
    unpacked_t ub = unpack_posit8(b);
    if (ua.mant == 0 || ub.mant == 0) return POSIT8_ZERO;
    bool signr = ua.sign != ub.sign;
    int scaler = ua.scale + ub.scale;
    uint64_t mantr = (ua.mant * ub.mant) >> (FRAC_PREC - 1);
    unpacked_t ur = {signr, scaler, mantr};
    while (mantr < (1LL << (FRAC_PREC - 1))) { mantr <<= 1; ur.scale--; }
    while (mantr >= (1LL << FRAC_PREC)) { mantr >>= 1; ur.scale++; }
    ur.mant = mantr;
    return pack_posit8(ur);
}

posit8 posit8_div(posit8 a, posit8 b) {
    unpacked_t ua = unpack_posit8(a);
    unpacked_t ub = unpack_posit8(b);
    if (ua.mant == 0) return POSIT8_ZERO;
    if (ub.mant == 0) return POSIT8_NAR;
    bool signr = ua.sign != ub.sign;
    int scaler = ua.scale - ub.scale;
    uint64_t mantr = (ua.mant << (FRAC_PREC - 1)) / ub.mant;
    unpacked_t ur = {signr, scaler, mantr};
    while (mantr < (1LL << (FRAC_PREC - 1))) { mantr <<= 1; ur.scale--; }
    while (mantr >= (1LL << FRAC_PREC)) { mantr >>= 1; ur.scale++; }
    ur.mant = mantr;
    return pack_posit8(ur);
}

posit8_error_t posit8_from_double(posit8 *p, double d) {
    if (d == 0.0) { *p = POSIT8_ZERO; return POSIT8_OK; }
    if (isnan(d) || isinf(d)) { *p = POSIT8_NAR; return POSIT8_OK; }
    bool sign = d < 0.0;
    d = fabs(d);
    if (d < pow(2.0, -6.0)) { *p = 0x01; if (sign) *p = 0xFF; return POSIT8_OK; }
    if (d > pow(2.0, 6.0)) { *p = 0x7F; if (sign) *p = 0x81; return POSIT8_OK; }
    int scale = (int)floor(log2(d));
    double mant_d = d / pow(2.0, (double)scale);
    uint64_t mant = (uint64_t)round(mant_d * (1ULL << (FRAC_PREC - 1)));
    unpacked_t u = {sign, scale, mant};
    *p = pack_posit8(u);
    return POSIT8_OK;
}

void posit8_to_double(double *d, posit8 p) {
    unpacked_t u = unpack_posit8(p);
    if (u.mant == 0 || p == POSIT8_NAR) { *d = p == POSIT8_NAR ? NAN : 0.0; return; }
    double frac = (double)u.mant / (double)(1ULL << (FRAC_PREC - 1));
    *d = pow(2.0, (double)u.scale) * frac;
    if (u.sign) *d = -*d;
}

void posit8_to_float(float *f, posit8 p) {
    double d;
    posit8_to_double(&d, p);
    *f = (float)d;
}

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
    
    if (isdigit(c) || c == '.' || c == '-') {
        char *end;
        double number = strtod(lexer->input + lexer->pos, &end);
        
        if (end[0] == 'p' && end[1] == '8') {
            posit8 p8;
            posit8_from_double(&p8, number);
            lexer->pos += (end - (lexer->input + lexer->pos)) + 2;
            return (token_t){TOKEN_POSIT8, {.p8 = p8}};
        }
        
        lexer->pos += (end - (lexer->input + lexer->pos));
        return (token_t){TOKEN_NUMBER, {.number = number}};
    }
    
    lexer->pos++;
    return (token_t){TOKEN_ERROR, {0}};
}

posit8 evaluate_expression(lexer_t *lexer) {
    posit8 left, right;
    
    token_t token = get_next_token(lexer);
    if (token.type == TOKEN_POSIT8) {
        left = token.value.p8;
    } else if (token.type == TOKEN_NUMBER) {
        posit8_from_double(&left, token.value.number);
    } else {
        printf("Error: Expected number or posit8 literal\n");
        return POSIT8_NAR;
    }
    
    token = get_next_token(lexer);
    if (token.type == TOKEN_EOF) {
        return left;
    }
    
    token_t next_token = get_next_token(lexer);
    if (next_token.type == TOKEN_POSIT8) {
        right = next_token.value.p8;
    } else if (next_token.type == TOKEN_NUMBER) {
        posit8_from_double(&right, next_token.value.number);
    } else {
        printf("Error: Expected number or posit8 literal\n");
        return POSIT8_NAR;
    }
    
    posit8 result;
    switch (token.type) {
        case TOKEN_PLUS:
            result = posit8_add(left, right);
            break;
        case TOKEN_MINUS:
            result = posit8_sub(left, right);
            break;
        case TOKEN_MULT:
            result = posit8_mul(left, right);
            break;
        case TOKEN_DIV:
            result = posit8_div(left, right);
            break;
        default:
            printf("Error: Unknown operator\n");
            return POSIT8_NAR;
    }
    
    return result;
}

void print_posit8_info(posit8 p) {
    float f_result;
    double d_result;
    
    printf("Posit8: 0x%02x\n", p);
    
    posit8_to_float(&f_result, p);
    printf("As float: %.6f\n", f_result);
    
    posit8_to_double(&d_result, p);
    printf("As double: %.6f\n", d_result);
    
    printf("Binary: ");
    for (int i = 7; i >= 0; i--) {
        printf("%d", (p >> i) & 1);
        if (i == 7) printf(" ");
        if (i == 6) printf(" ");
    }
    printf("\n");
}

void interpreter_loop(void) {
    char line[MAX_LINE];
    lexer_t lexer;
    
    printf("=== Posit8 Interpreter ===\n");
    printf("Enter expressions like: 1.5p8 + 2.0p8\n");
    printf("Or: 1.5 + 2.0 (will be converted to posit8)\n");
    printf("Type 'quit' to exit\n\n");
    
    while (1) {
        printf("> ");
        if (!fgets(line, MAX_LINE, stdin)) {
            break;
        }
        
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