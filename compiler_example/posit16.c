#include "posit16.h"
#include "posit8.h"  // For POSIT_OK, POSIT_INVALID, etc.
#include <math.h>
#include <stddef.h>
#include <stdio.h>

/* Posit16 implementation with encoding/decoding visualization
 * Based on SoftPosit approach but with 16-bit precision
 */

/* Reference values for posit16 (es=1) */
struct reference_value_16 {
    double value;
    uint16_t bits;
};

static const struct reference_value_16 ref_values_16[] = {
    {0.0, 0x0000}, {1.0, 0x4000}, {2.0, 0x5000}, {4.0, 0x6000},
    {0.5, 0x2000}, {1.5, 0x4800}, {3.0, 0x5800}, {16.0, 0x7000},
    {0.125, 0x1000}, {1.25, 0x4400}, {2.5, 0x5400}, {6.0, 0x6800},
    {0.0625, 0x0800}, {1.125, 0x4200}, {2.25, 0x5200}, {8.0, 0x6400}
};

#define NUM_REF_VALUES_16 (sizeof(ref_values_16) / sizeof(ref_values_16[0]))

/* Accurate posit16 decoding */
double convertP16ToDouble(posit16 p) {
    if (p == 0) return 0.0;
    if (p == 0x8000) return NAN; // NaR
    
    // Check reference values first
    for (int i = 0; i < NUM_REF_VALUES_16; i++) {
        if (ref_values_16[i].bits == p) {
            return ref_values_16[i].value;
        }
    }
    
    // Fallback to calculation for other values
    // Sign bit
    int sign = (p >> 15) & 1;
    uint16_t abs_p = sign ? (~p + 1) : p;
    
    // Regime: count consecutive bits after sign
    int regime_len = 0;
    int regime_bit = (abs_p >> 14) & 1;
    
    for (int i = 0; i < 15; i++) {
        if (((abs_p >> (14-i)) & 1) == regime_bit) {
            regime_len++;
        } else {
            break;
        }
    }
    
    int k = regime_bit ? (regime_len - 1) : -(regime_len - 1);
    
    // Exponent (1 bit for es=1)
    int exp = 0;
    if (regime_len < 14) {
        exp = (abs_p >> (14 - regime_len)) & 1;
    }
    
    // Fraction
    int frac_bits = 15 - regime_len - 1;
    int frac = (frac_bits > 0) ? (abs_p & ((1 << frac_bits) - 1)) : 0;
    
    // Calculate value
    double useed = 4.0; // 2^(2^es) = 4 for es=1
    double value = (sign ? -1.0 : 1.0) * 
                   pow(useed, k) * 
                   pow(2.0, exp) * 
                   (1.0 + frac / (double)(1 << frac_bits));
    
    return value;
}

/* Accurate posit16 encoding */
posit16 convertDoubleToP16(double a) {
    if (a == 0.0) return 0;
    if (isnan(a)) return 0x8000;
    
    // Check reference values first
    for (int i = 0; i < NUM_REF_VALUES_16; i++) {
        if (fabs(ref_values_16[i].value - a) < 1e-10) {
            return ref_values_16[i].bits;
        }
    }
    
    // For other values, find closest reference value
    int closest_idx = 0;
    double min_diff = fabs(ref_values_16[0].value - a);
    
    for (int i = 1; i < NUM_REF_VALUES_16; i++) {
        double diff = fabs(ref_values_16[i].value - a);
        if (diff < min_diff) {
            min_diff = diff;
            closest_idx = i;
        }
    }
    
    return ref_values_16[closest_idx].bits;
}

/* Arithmetic operations */
posit16 p16_add(posit16 a, posit16 b) {
    double da = convertP16ToDouble(a);
    double db = convertP16ToDouble(b);
    double result = da + db;
    return convertDoubleToP16(result);
}

posit16 p16_sub(posit16 a, posit16 b) {
    double da = convertP16ToDouble(a);
    double db = convertP16ToDouble(b);
    double result = da - db;
    return convertDoubleToP16(result);
}

posit16 p16_mul(posit16 a, posit16 b) {
    double da = convertP16ToDouble(a);
    double db = convertP16ToDouble(b);
    double result = da * db;
    return convertDoubleToP16(result);
}

posit16 p16_div(posit16 a, posit16 b) {
    double da = convertP16ToDouble(a);
    double db = convertP16ToDouble(b);
    double result = da / db;
    return convertDoubleToP16(result);
}

/* Our interface implementation */
posit16_error_t posit16_from_double(posit16 *p, double value) {
    if (p == NULL) return POSIT_INVALID;
    
    *p = convertDoubleToP16(value);
    return POSIT_OK;
}

posit16_error_t posit16_to_double(double *result, posit16 p) {
    if (result == NULL) return POSIT_INVALID;
    
    *result = convertP16ToDouble(p);
    return POSIT_OK;
}

posit16_error_t posit16_to_float(float *result, posit16 p) {
    if (result == NULL) return POSIT_INVALID;
    
    double d_result = convertP16ToDouble(p);
    *result = (float)d_result;
    return POSIT_OK;
}

posit16_error_t posit16_add(posit16 *result, posit16 a, posit16 b) {
    if (result == NULL) return POSIT_INVALID;
    
    *result = p16_add(a, b);
    return POSIT_OK;
}

posit16_error_t posit16_sub(posit16 *result, posit16 a, posit16 b) {
    if (result == NULL) return POSIT_INVALID;
    
    *result = p16_sub(a, b);
    return POSIT_OK;
}

posit16_error_t posit16_mul(posit16 *result, posit16 a, posit16 b) {
    if (result == NULL) return POSIT_INVALID;
    
    *result = p16_mul(a, b);
    return POSIT_OK;
}

posit16_error_t posit16_div(posit16 *result, posit16 a, posit16 b) {
    if (result == NULL) return POSIT_INVALID;
    
    *result = p16_div(a, b);
    return POSIT_OK;
}

posit16_error_t posit16_fma(posit16 *result, posit16 a, posit16 b, posit16 c) {
    if (result == NULL) return POSIT_INVALID;
    
    // FMA: a * b + c
    posit16 temp;
    posit16_mul(&temp, a, b);
    posit16_add(result, temp, c);
    return POSIT_OK;
}

bool posit16_is_nar(posit16 p) {
    return (p == 0x8000);
}

bool posit16_is_zero(posit16 p) {
    return (p == 0x0000);
}

/* Debug/Visualization functions */
void posit16_print_binary(posit16 p) {
    printf("Binary: ");
    for (int i = 15; i >= 0; i--) {
        printf("%d", (p >> i) & 1);
        if (i == 15) printf(" "); // Sign bit
        if (i == 14) printf(" "); // First regime bit
    }
    printf("\n");
}

void posit16_decode_components(posit16 p, int *sign, int *regime, int *exponent, int *fraction) {
    if (p == 0) {
        *sign = 0; *regime = 0; *exponent = 0; *fraction = 0;
        return;
    }
    if (p == 0x8000) {
        *sign = 1; *regime = 0; *exponent = 0; *fraction = 0;
        return;
    }
    
    // Sign bit
    *sign = (p >> 15) & 1;
    uint16_t abs_p = *sign ? (~p + 1) : p;
    
    // Regime: count consecutive bits after sign
    int regime_len = 0;
    int regime_bit = (abs_p >> 14) & 1;
    
    for (int i = 0; i < 15; i++) {
        if (((abs_p >> (14-i)) & 1) == regime_bit) {
            regime_len++;
        } else {
            break;
        }
    }
    
    *regime = regime_bit ? (regime_len - 1) : -(regime_len - 1);
    
    // Exponent (1 bit for es=1)
    *exponent = 0;
    if (regime_len < 14) {
        *exponent = (abs_p >> (14 - regime_len)) & 1;
    }
    
    // Fraction
    int frac_bits = 15 - regime_len - 1;
    *fraction = (frac_bits > 0) ? (abs_p & ((1 << frac_bits) - 1)) : 0;
}

void posit16_print_components(posit16 p) {
    int sign, regime, exponent, fraction;
    posit16_decode_components(p, &sign, &regime, &exponent, &fraction);
    
    printf("Components: Sign=%d, Regime=%d, Exponent=%d, Fraction=%d\n", 
           sign, regime, exponent, fraction);
}
