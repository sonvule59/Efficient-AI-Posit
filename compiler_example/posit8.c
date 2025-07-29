#include "posit8.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>

/* 8-bit Posit Implementation (es=1)
 * 
 * This implements proper 8-bit posit arithmetic with:
 * - 1 sign bit
 * - Variable regime bits
 * - 1 exponent bit (es=1)
 * - Remaining fraction bits
 */

/* Decode posit8 to components */
typedef struct {
    int sign;
    int regime;
    uint8_t exponent;
    uint8_t fraction;
    int frac_bits;
} posit8_components;

static posit8_error_t decode_posit8(posit8_components *comp, posit8 p) {
    if (!comp) return POSIT8_INVALID;
    
    if (p == POSIT8_NAR) return POSIT8_NOT_A_REAL;
    if (p == POSIT8_ZERO) {
        comp->sign = 0;
        comp->regime = 0;
        comp->exponent = 0;
        comp->fraction = 0;
        comp->frac_bits = 0;
        return POSIT8_OK;
    }

    // Sign bit
    comp->sign = (p >> 7) & 1;
    uint8_t abs_p = comp->sign ? (~p + 1) : p;

    // Regime (variable length)
    int regime_len = 0;
    int regime_bit = (abs_p >> 6) & 1;
    int i = 1;
    while (((abs_p >> (7-i)) & 1) == regime_bit && i < 8) {
        regime_len++;
        i++;
    }
    comp->regime = regime_bit ? regime_len : -regime_len;

    // Exponent (1 bit, es=1)
    int exp_offset = 1 + regime_len;
    if (exp_offset + 1 > 8) {
        comp->exponent = 0;
    } else {
        comp->exponent = (abs_p >> (8 - exp_offset - 1)) & 1;
    }

    // Fraction (remaining bits)
    comp->frac_bits = 8 - 1 - regime_len - 1; // 1 sign + regime + 1 exp
    if (comp->frac_bits > 0) {
        comp->fraction = abs_p & ((1 << comp->frac_bits) - 1);
    } else {
        comp->fraction = 0;
    }

    return POSIT8_OK;
}

/* Encode components to posit8 */
static posit8_error_t encode_posit8(posit8 *result, posit8_components comp) {
    if (!result) return POSIT8_INVALID;
    
    uint8_t bits = 0;
    
    // Sign bit
    if (comp.sign) {
        bits |= 0x80;
    }
    
    // Regime bits
    if (comp.regime >= 0) {
        // Positive regime: 1 followed by regime 1s
        int regime_ones = comp.regime + 1;
        if (regime_ones > 7) regime_ones = 7;
        bits |= ((1 << regime_ones) - 1) << (7 - regime_ones);
    } else {
        // Negative regime: 0 followed by |regime| 0s
        int regime_zeros = -comp.regime;
        if (regime_zeros > 6) regime_zeros = 6;
        // No bits set for negative regime
    }
    
    // Exponent bit
    int exp_pos = 7 - (abs(comp.regime) + 1);
    if (exp_pos >= 0) {
        bits |= (comp.exponent << exp_pos);
    }
    
    // Fraction bits
    int frac_pos = exp_pos - 1;
    if (frac_pos >= 0 && comp.frac_bits > 0) {
        bits |= (comp.fraction & ((1 << comp.frac_bits) - 1));
    }
    
    *result = bits;
    return POSIT8_OK;
}

/* Convert posit8 to double */
double posit8_to_double_internal(posit8 p) {
    posit8_components comp;
    if (decode_posit8(&comp, p) != POSIT8_OK) return 0.0;
    
    if (comp.sign && p == POSIT8_NAR) return NAN;
    
    double useed = 4.0; // 2^(2^es) = 2^2 = 4
    double value = (comp.sign ? -1.0 : 1.0) * 
                   pow(useed, comp.regime) * 
                   pow(2.0, comp.exponent) * 
                   (1.0 + comp.fraction / (double)(1 << comp.frac_bits));
    
    return value;
}

/* Convert double to posit8 */
posit8_error_t posit8_from_double(posit8 *result, double d) {
    if (!result) return POSIT8_INVALID;
    
    if (isnan(d)) {
        *result = POSIT8_NAR;
        return POSIT8_NOT_A_REAL;
    }
    
    if (d == 0.0) {
        *result = POSIT8_ZERO;
        return POSIT8_OK;
    }
    
    posit8_components comp;
    comp.sign = (d < 0) ? 1 : 0;
    double abs_d = fabs(d);
    
    // Calculate regime
    double useed = 4.0;
    int k = 0;
    double tmp = abs_d;
    
    while (tmp >= useed) {
        tmp /= useed;
        k++;
    }
    while (tmp < 1.0) {
        tmp *= useed;
        k--;
    }
    comp.regime = k;
    
    // Calculate exponent
    int exp = 0;
    while (tmp >= 2.0) {
        tmp /= 2.0;
        exp++;
    }
    comp.exponent = (exp > 0) ? 1 : 0;
    
    // Calculate fraction
    double frac = tmp - 1.0;
    comp.frac_bits = 8 - 1 - (abs(k) + 1) - 1; // 1 sign + regime + 1 exp
    if (comp.frac_bits > 0) {
        comp.fraction = (uint8_t)(frac * (1 << comp.frac_bits));
    } else {
        comp.fraction = 0;
    }
    
    return encode_posit8(result, comp);
}

/* Arithmetic operations */
posit8_error_t posit8_add(posit8 *result, posit8 a, posit8 b) {
    if (!result) return POSIT8_INVALID;
    
    if (a == POSIT8_NAR || b == POSIT8_NAR) {
        *result = POSIT8_NAR;
        return POSIT8_NOT_A_REAL;
    }
    
    double fa = posit8_to_double_internal(a);
    double fb = posit8_to_double_internal(b);
    double sum = fa + fb;
    
    if (isnan(sum) || isinf(sum)) {
        *result = POSIT8_NAR;
        return POSIT8_NOT_A_REAL;
    }
    
    return posit8_from_double(result, sum);
}

posit8_error_t posit8_sub(posit8 *result, posit8 a, posit8 b) {
    if (!result) return POSIT8_INVALID;
    
    if (a == POSIT8_NAR || b == POSIT8_NAR) {
        *result = POSIT8_NAR;
        return POSIT8_NOT_A_REAL;
    }
    
    double fa = posit8_to_double_internal(a);
    double fb = posit8_to_double_internal(b);
    double diff = fa - fb;
    
    if (isnan(diff) || isinf(diff)) {
        *result = POSIT8_NAR;
        return POSIT8_NOT_A_REAL;
    }
    
    return posit8_from_double(result, diff);
}

posit8_error_t posit8_mul(posit8 *result, posit8 a, posit8 b) {
    if (!result) return POSIT8_INVALID;
    
    if (a == POSIT8_NAR || b == POSIT8_NAR) {
        *result = POSIT8_NAR;
        return POSIT8_NOT_A_REAL;
    }
    
    double fa = posit8_to_double_internal(a);
    double fb = posit8_to_double_internal(b);
    double product = fa * fb;
    
    if (isnan(product) || isinf(product)) {
        *result = POSIT8_NAR;
        return POSIT8_NOT_A_REAL;
    }
    
    return posit8_from_double(result, product);
}

posit8_error_t posit8_div(posit8 *result, posit8 a, posit8 b) {
    if (!result) return POSIT8_INVALID;
    
    if (a == POSIT8_NAR || b == POSIT8_NAR) {
        *result = POSIT8_NAR;
        return POSIT8_NOT_A_REAL;
    }
    
    double fa = posit8_to_double_internal(a);
    double fb = posit8_to_double_internal(b);
    
    if (fb == 0.0) {
        *result = POSIT8_NAR;
        return POSIT8_NOT_A_REAL;
    }
    
    double quotient = fa / fb;
    
    if (isnan(quotient) || isinf(quotient)) {
        *result = POSIT8_NAR;
        return POSIT8_NOT_A_REAL;
    }
    
    return posit8_from_double(result, quotient);
}

posit8_error_t posit8_fma(posit8 *result, posit8 a, posit8 b, posit8 c) {
    if (!result) return POSIT8_INVALID;
    
    posit8 temp;
    posit8_error_t err = posit8_mul(&temp, a, b);
    if (err != POSIT8_OK) {
        *result = POSIT8_NAR;
        return err;
    }
    
    return posit8_add(result, temp, c);
}

/* Conversion functions */
posit8_error_t float_to_posit8(posit8 *result, float f) {
    return posit8_from_double(result, (double)f);
}

posit8_error_t posit8_to_float(float *result, posit8 p) {
    if (!result) return POSIT8_INVALID;
    *result = (float)posit8_to_double_internal(p);
    return POSIT8_OK;
}

posit8_error_t double_to_posit8(posit8 *result, double d) {
    return posit8_from_double(result, d);
}

posit8_error_t posit8_to_double(double *result, posit8 p) {
    if (!result) return POSIT8_INVALID;
    *result = posit8_to_double_internal(p);
    return POSIT8_OK;
}

/* Utility functions */
posit8_error_t posit8_from_bits(posit8 *result, uint8_t bits) {
    if (!result) return POSIT8_INVALID;
    *result = bits;
    return POSIT8_OK;
}

uint8_t posit8_to_bits(posit8 p) {
    return p;
} 