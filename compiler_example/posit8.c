#include "posit8.h"
#include "softposit_fixed.h"
#include <math.h>
#include <stddef.h>

/* SoftPosit wrapper implementation
 * This uses the official SoftPosit functions for accurate arithmetic
 */

/* Our interface implementation using SoftPosit */
posit8_error_t posit8_from_double(posit8 *p, double value) {
    if (p == NULL) return POSIT_INVALID;
    
    *p = convertDoubleToP8(value);
    return POSIT_OK;
}

posit8_error_t posit8_to_double(double *result, posit8 p) {
    if (result == NULL) return POSIT_INVALID;
    
    *result = convertP8ToDouble(p);
    return POSIT_OK;
}

posit8_error_t posit8_to_float(float *result, posit8 p) {
    if (result == NULL) return POSIT_INVALID;
    
    double d_result = convertP8ToDouble(p);
    *result = (float)d_result;
    return POSIT_OK;
}

posit8_error_t posit8_add(posit8 *result, posit8 a, posit8 b) {
    if (result == NULL) return POSIT_INVALID;
    
    *result = p8_add(a, b);
    return POSIT_OK;
}

posit8_error_t posit8_sub(posit8 *result, posit8 a, posit8 b) {
    if (result == NULL) return POSIT_INVALID;
    
    *result = p8_sub(a, b);
    return POSIT_OK;
}

posit8_error_t posit8_mul(posit8 *result, posit8 a, posit8 b) {
    if (result == NULL) return POSIT_INVALID;
    
    *result = p8_mul(a, b);
    return POSIT_OK;
}

posit8_error_t posit8_div(posit8 *result, posit8 a, posit8 b) {
    if (result == NULL) return POSIT_INVALID;
    
    *result = p8_div(a, b);
    return POSIT_OK;
}

posit8_error_t posit8_fma(posit8 *result, posit8 a, posit8 b, posit8 c) {
    if (result == NULL) return POSIT_INVALID;
    
    // FMA: a * b + c
    posit8 temp;
    posit8_mul(&temp, a, b);
    posit8_add(result, temp, c);
    return POSIT_OK;
}

bool posit8_is_nar(posit8 p) {
    return p8_isNaR(p);
}

bool posit8_is_zero(posit8 p) {
    return p8_isZero(p);
}
