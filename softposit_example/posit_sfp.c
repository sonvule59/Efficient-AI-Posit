#include "posit.h"
#include <string.h>
// #include <softposit.h> // SoftPosit library - TODO: Install or provide correct path

/* Runtime Library for Posit Arithmetic
 *
 * Implements posit.h functions using SoftPosit for true posit arithmetic and
 * a basic quire emulation for posit32. Replaces IEEE 754 float-based placeholder
 * logic with proper posit operations, supporting Dr. John L. Gustafson's
 * requirements for posit integration in C.
 *
 * Note: Quire implementation is simplified and requires optimization (e.g., GMP
 * or custom 512-bit arithmetic) for full accuracy.
 */

posit_error_t posit32_add(posit32 *result, posit32 a, posit32 b) {
    if (!result) return POSIT_INVALID;
    *result = addP32(a, b); // SoftPosit addition
    if (*result == 0x80000000) return POSIT_NOT_A_REAL; // Check for Not a Real
    return POSIT_OK;
}

posit_error_t posit32_sub(posit32 *result, posit32 a, posit32 b) {
    if (!result) return POSIT_INVALID;
    *result = subP32(a, b);
    if (*result == 0x80000000) return POSIT_NOT_A_REAL;
    return POSIT_OK;
}

posit_error_t posit32_mul(posit32 *result, posit32 a, posit32 b) {
    if (!result) return POSIT_INVALID;
    *result = mulP32(a, b);
    if (*result == 0x80000000) return POSIT_NOT_A_REAL;
    return POSIT_OK;
}

posit_error_t posit32_div(posit32 *result, posit32 a, posit32 b) {
    if (!result) return POSIT_INVALID;
    if (b == 0) return POSIT_NOT_A_REAL; // Division by zero
    *result = divP32(a, b);
    if (*result == 0x80000000) return POSIT_NOT_A_REAL;
    return POSIT_OK;
}

posit_error_t posit32_fma(posit32 *result, posit32 a, posit32 b, posit32 c) {
    if (!result) return POSIT_INVALID;
    *result = fmaP32(a, b, c); // SoftPosit FMA
    if (*result == 0x80000000) return POSIT_NOT_A_REAL;
    return POSIT_OK;
}

quire32 quire32_init(void) {
    quire32 q;
    memset(q.data, 0, sizeof(q.data));
    return q;
}

posit_error_t quire32_fma(quire32 *q, posit32 a, posit32 b) {
    if (!q) return POSIT_INVALID;
    // Simplified: Convert to double for high-precision product
    // TODO: Replace with 512-bit fixed-point arithmetic or GMP
    double fa = convertP32ToDouble(a);
    double fb = convertP32ToDouble(b);
    if (fa == 0.0 || fb == 0.0) return POSIT_OK; // No accumulation needed
    double product = fa * fb;
    // Scale and add to quire (first 64 bits, incomplete)
    int64_t scaled = (int64_t)(product * (1ULL << 32)); // Fixed-point scaling
    uint64_t carry = 0;
    uint64_t sum = q->data[0] + scaled;
    if (sum < q->data[0]) carry = 1; // Detect carry
    q->data[0] = sum;
    // Propagate carry (simplified, only one step)
    for (int i = 1; i < 8 && carry; i++) {
        q->data[i] += carry;
        carry = q->data[i] < carry;
    }
    return POSIT_OK;
}

posit_error_t quire32_to_posit(posit32 *result, quire32 q) {
    if (!result) return POSIT_INVALID;
    // Simplified: Extract from first 64 bits
    // TODO: Use full 512-bit value with proper rounding
    double value = (double)q.data[0] / (1ULL << 32); // Undo scaling
    *result = convertDoubleToP32(value);
    if (*result == 0x80000000) return POSIT_NOT_A_REAL;
    return POSIT_OK;
}

posit_error_t float_to_posit32(posit32 *result, float f) {
    if (!result) return POSIT_INVALID;
    if (isnan(f) || isinf(f)) {
        *result = 0x80000000; // Not a Real
        return POSIT_NOT_A_REAL;
    }
    *result = convertDoubleToP32((double)f);
    return POSIT_OK;
}

posit_error_t posit32_to_float(float *result, posit32 p) {
    if (!result) return POSIT_INVALID;
    if (p == 0x80000000) {
        *result = 0.0f; // Map Not a Real to 0.0 (or NaN, depending on policy)
        return POSIT_NOT_A_REAL;
    }
    *result = (float)convertP32ToDouble(p);
    return POSIT_OK;
}

posit_error_t double_to_posit32(posit32 *result, double d) {
    if (!result) return POSIT_INVALID;
    if (isnan(d) || isinf(d)) {
        *result = 0x80000000;
        return POSIT_NOT_A_REAL;
    }
    *result = convertDoubleToP32(d);
    return POSIT_OK;
}

posit_error_t posit32_to_double(double *result, posit32 p) {
    if (!result) return POSIT_INVALID;
    if (p == 0x80000000) {
        *result = 0.0;
        return POSIT_NOT_A_REAL;
    }
    *result = convertP32ToDouble(p);
    return POSIT_OK;
}

posit_error_t posit8_from_double(posit8 *result, double d) {
    if (!result) return POSIT_INVALID;
    if (isnan(d) || isinf(d)) {
        *result = 0x80; // Not a Real for posit8
        return POSIT_NOT_A_REAL;
    }
    *result = convertDoubleToP8(d);
    return POSIT_OK;
}

posit_error_t posit16_from_double(posit16 *result, double d) {
    if (!result) return POSIT_INVALID;
    if (isnan(d) || isinf(d)) {
        *result = 0x8000; // Not a Real for posit16
        return POSIT_NOT_A_REAL;
    }
    *result = convertDoubleToP16(d);
    return POSIT_OK;
}

posit_error_t posit32_from_double(posit32 *result, double d) {
    if (!result) return POSIT_INVALID;
    if (isnan(d) || isinf(d)) {
        *result = 0x80000000;
        return POSIT_NOT_A_REAL;
    }
    *result = convertDoubleToP32(d);
    return POSIT_OK;
}