#include "posit.h"
#include <string.h>
#include <math.h>

/* Custom Posit Arithmetic Implementation
 *
 * Implements posit.h functions for posit32 (es=2) with simplified arithmetic,
 * quire operations, and conversions. Designed for a C compiler and verifier,
 * following Dr. John L. Gustafson's requirements. Does not use SoftPosit, instead
 * providing a basic custom implementation for demonstration.
 *
 * Note: Arithmetic is simplified (uses double for intermediate calculations).
 * Quire is rudimentary (single 64-bit accumulation). Full implementation requires
 * bit-level posit decoding and 512-bit quire arithmetic.
 */

/* Helper: Decode posit32 to components (simplified) */
typedef struct {
    int sign; // 0 for positive, 1 for negative
    int regime; // Regime value (shift)
    uint32_t exponent; // Exponent bits (0–3)
    uint32_t fraction; // Fraction bits
} posit32_components;

static posit_error_t decode_posit32(posit32_components *comp, posit32 p) {
    if (!comp) return POSIT_INVALID;
    if (p == 0x80000000) return POSIT_NOT_A_REAL; // Not a Real
    if (p == 0) {
        comp->sign = 0;
        comp->regime = 0;
        comp->exponent = 0;
        comp->fraction = 0;
        return POSIT_OK;
    }

    comp->sign = (p >> 31) & 1;
    uint32_t abs_p = comp->sign ? (~p + 1) : p;

    // Count regime bits (simplified, assumes es=2)
    int regime_len = 0;
    int regime_bit = (abs_p >> 30) & 1;
    while (((abs_p >> (30 - regime_len)) & 1) == regime_bit && regime_len < 30) {
        regime_len++;
    }
    comp->regime = regime_bit ? regime_len : -regime_len;

    // Extract exponent (2 bits)
    int bits_left = 31 - regime_len - 1;
    comp->exponent = bits_left > 0 ? (abs_p >> (bits_left - 2)) & 0x3 : 0;

    // Extract fraction (remaining bits)
    comp->fraction = bits_left > 2 ? abs_p & ((1ULL << (bits_left - 2)) - 1) : 0;

    return POSIT_OK;
}

/* Helper: Encode components to posit32 (simplified) */
static posit_error_t encode_posit32(posit32 *result, posit32_components comp) {
    if (!result) return POSIT_INVALID;
    // Simplified: Convert to double and use double_to_posit32
    double value = (comp.sign ? -1.0 : 1.0) * (1.0 + comp.fraction / (double)(1ULL << 28)) *
                   pow(2.0, comp.regime * 4 + comp.exponent);
    return double_to_posit32(result, value);
}

/* Helper: Convert posit32 to double (simplified) */
static double posit32_to_double_internal(posit32 p) {
    posit32_components comp;
    if (decode_posit32(&comp, p) != POSIT_OK) return 0.0;
    return (comp.sign ? -1.0 : 1.0) * (1.0 + comp.fraction / (double)(1ULL << 28)) *
           pow(2.0, comp.regime * 4 + comp.exponent);
}

posit_error_t posit32_add(posit32 *result, posit32 a, posit32 b) {
    if (!result) return POSIT_INVALID;
    if (a == 0x80000000 || b == 0x80000000) {
        *result = 0x80000000;
        return POSIT_NOT_A_REAL;
    }
    // Simplified: Convert to double, add, convert back
    double fa = posit32_to_double_internal(a);
    double fb = posit32_to_double_internal(b);
    double sum = fa + fb;
    if (isnan(sum) || isinf(sum)) {
        *result = 0x80000000;
        return POSIT_NOT_A_REAL;
    }
    return double_to_posit32(result, sum);
}

posit_error_t posit32_sub(posit32 *result, posit32 a, posit32 b) {
    if (!result) return POSIT_INVALID;
    if (a == 0x80000000 || b == 0x80000000) {
        *result = 0x80000000;
        return POSIT_NOT_A_REAL;
    }
    double fa = posit32_to_double_internal(a);
    double fb = posit32_to_double_internal(b);
    double diff = fa - fb;
    if (isnan(diff) || isinf(diff)) {
        *result = 0x80000000;
        return POSIT_NOT_A_REAL;
    }
    return double_to_posit32(result, diff);
}

posit_error_t posit32_mul(posit32 *result, posit32 a, posit32 b) {
    if (!result) return POSIT_INVALID;
    if (a == 0x80000000 || b == 0x80000000) {
        *result = 0x80000000;
        return POSIT_NOT_A_REAL;
    }
    double fa = posit32_to_double_internal(a);
    double fb = posit32_to_double_internal(b);
    double prod = fa * fb;
    if (isnan(prod) || isinf(prod)) {
        *result = 0x80000000;
        return POSIT_NOT_A_REAL;
    }
    return double_to_posit32(result, prod);
}

posit_error_t posit32_div(posit32 *result, posit32 a, posit32 b) {
    if (!result) return POSIT_INVALID;
    if (a == 0x80000000 || b == 0x80000000 || b == 0) {
        *result = 0x80000000;
        return POSIT_NOT_A_REAL;
    }
    double fa = posit32_to_double_internal(a);
    double fb = posit32_to_double_internal(b);
    double quot = fa / fb;
    if (isnan(quot) || isinf(quot)) {
        *result = 0x80000000;
        return POSIT_NOT_A_REAL;
    }
    return double_to_posit32(result, quot);
}

posit_error_t posit32_fma(posit32 *result, posit32 a, posit32 b, posit32 c) {
    if (!result) return POSIT_INVALID;
    if (a == 0x80000000 || b == 0x80000000 || c == 0x80000000) {
        *result = 0x80000000;
        return POSIT_NOT_A_REAL;
    }
    double fa = posit32_to_double_internal(a);
    double fb = posit32_to_double_internal(b);
    double fc = posit32_to_double_internal(c);
    double fma = fa * fb + fc;
    if (isnan(fma) || isinf(fma)) {
        *result = 0x80000000;
        return POSIT_NOT_A_REAL;
    }
    return double_to_posit32(result, fma);
}

quire32 quire32_init(void) {
    quire32 q;
    memset(q.data, 0, sizeof(q.data));
    return q;
}

posit_error_t quire32_fma(quire32 *q, posit32 a, posit32 b) {
    if (!q) return POSIT_INVALID;
    if (a == 0x80000000 || b == 0x80000000) return POSIT_NOT_A_REAL;
    // Simplified: Use double for product, store in first 64 bits
    double fa = posit32_to_double_internal(a);
    double fb = posit32_to_double_internal(b);
    double product = fa * fb;
    int64_t scaled = (int64_t)(product * (1ULL << 32)); // Fixed-point scaling
    uint64_t carry = 0;
    uint64_t sum = q->data[0] + scaled;
    if (sum < q->data[0]) carry = 1;
    q->data[0] = sum;
    for (int i = 1; i < 8 && carry; i++) {
        q->data[i] += carry;
        carry = q->data[i] < carry;
    }
    return POSIT_OK;
}

posit_error_t quire32_to_posit(posit32 *result, quire32 q) {
    if (!result) return POSIT_INVALID;
    // Simplified: Extract from first 64 bits
    double value = (double)q.data[0] / (1ULL << 32);
    return double_to_posit32(result, value);
}

posit_error_t float_to_posit32(posit32 *result, float f) {
    if (!result) return POSIT_INVALID;
    if (isnan(f) || isinf(f)) {
        *result = 0x80000000;
        return POSIT_NOT_A_REAL;
    }
    return double_to_posit32(result, (double)f);
}

posit_error_t posit32_to_float(float *result, posit32 p) {
    if (!result) return POSIT_INVALID;
    if (p == 0x80000000) {
        *result = 0.0f;
        return POSIT_NOT_A_REAL;
    }
    *result = (float)posit32_to_double_internal(p);
    return POSIT_OK;
}

posit_error_t double_to_posit32(posit32 *result, double d) {
    if (!result) return POSIT_INVALID;
    if (isnan(d) || isinf(d)) {
        *result = 0x80000000;
        return POSIT_NOT_A_REAL;
    }
    // Simplified: Approximate posit32 encoding
    posit32_components comp;
    comp.sign = d < 0 ? 1 : 0;
    d = fabs(d);
    if (d == 0.0) {
        *result = 0;
        return POSIT_OK;
    }
    int regime = (int)floor(log2(d) / 4); // Approximate regime
    double scale = pow(2.0, regime * 4);
    comp.regime = regime;
    comp.exponent = (int)(log2(d / scale)) & 0x3; // 2-bit exponent
    comp.fraction = (uint32_t)((d / scale / pow(2.0, comp.exponent)) * (1ULL << 28));
    return encode_posit32(result, comp);
}

posit_error_t posit32_to_double(double *result, posit32 p) {
    if (!result) return POSIT_INVALID;
    if (p == 0x80000000) {
        *result = 0.0;
        return POSIT_NOT_A_REAL;
    }
    *result = posit32_to_double_internal(p);
    return POSIT_OK;
}

posit_error_t posit8_from_double(posit8 *result, double d) {
    if (!result) return POSIT_INVALID;
    // Placeholder: Not implemented
    *result = 0;
    return POSIT_INVALID;
}

posit_error_t posit16_from_double(posit16 *result, double d) {
    if (!result) return POSIT_INVALID;
    // Placeholder: Not implemented
    *result = 0;
    return POSIT_INVALID;
}

posit_error_t posit32_from_double(posit32 *result, double d) {
    return double_to_posit32(result, d);
}