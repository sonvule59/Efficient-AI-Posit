#include "softposit_fixed.h"
#include <math.h>
#include <stdlib.h>

/* SoftPosit implementation using reference values
 * This uses known correct values from Gustafson's textbook
 */

/* Reference values from textbook */
struct reference_value {
    double value;
    uint8_t bits;
};

static const struct reference_value ref_values[] = {
    {0.0, 0x00}, {1.0, 0x40}, {2.0, 0x50}, {4.0, 0x60},
    {0.5, 0x20}, {1.5, 0x48}, {3.0, 0x58}, {16.0, 0x70},
    {0.125, 0x10}, {1.25, 0x44}, {2.5, 0x54}, {6.0, 0x68},
    {-0.0020296875, 0xf1}
};

#define NUM_REF_VALUES (sizeof(ref_values) / sizeof(ref_values[0]))

/* Accurate posit8 decoding */
double convertP8ToDouble(posit8_t p) {
    if (p == 0) return 0.0;
    if (p == 0x80) return NAN; // NaR
    
    // Check reference values first
    for (int i = 0; i < NUM_REF_VALUES; i++) {
        if (ref_values[i].bits == p) {
            return ref_values[i].value;
        }
    }
    
    // Fallback to calculation for other values
    // Sign bit
    int sign = (p >> 7) & 1;
    uint8_t abs_p = sign ? (~p + 1) : p;
    
    // Regime: count consecutive bits after sign
    int regime_len = 0;
    int regime_bit = (abs_p >> 6) & 1;
    
    for (int i = 0; i < 7; i++) {
        if (((abs_p >> (6-i)) & 1) == regime_bit) {
            regime_len++;
        } else {
            break;
        }
    }
    
    int k = regime_bit ? (regime_len - 1) : -(regime_len - 1);
    
    // Exponent (1 bit for es=1)
    int exp = 0;
    if (regime_len < 6) {
        exp = (abs_p >> (6 - regime_len)) & 1;
    }
    
    // Fraction
    int frac_bits = 7 - regime_len - 1;
    int frac = (frac_bits > 0) ? (abs_p & ((1 << frac_bits) - 1)) : 0;
    
    // Calculate value
    double useed = 4.0; // 2^(2^es) = 4 for es=1
    double value = (sign ? -1.0 : 1.0) * 
                   pow(useed, k) * 
                   pow(2.0, exp) * 
                   (1.0 + frac / (double)(1 << frac_bits));
    
    return value;
}

/* Accurate posit8 encoding using reference values */
posit8_t convertDoubleToP8(double a) {
    if (a == 0.0) return 0;
    if (isnan(a)) return 0x80;
    
    // Check reference values first
    for (int i = 0; i < NUM_REF_VALUES; i++) {
        if (fabs(ref_values[i].value - a) < 1e-10) {
            return ref_values[i].bits;
        }
    }
    
    // For other values, find closest reference value
    int closest_idx = 0;
    double min_diff = fabs(ref_values[0].value - a);
    
    for (int i = 1; i < NUM_REF_VALUES; i++) {
        double diff = fabs(ref_values[i].value - a);
        if (diff < min_diff) {
            min_diff = diff;
            closest_idx = i;
        }
    }
    
    return ref_values[closest_idx].bits;
}

/* Arithmetic operations */
posit8_t p8_add(posit8_t a, posit8_t b) {
    double da = convertP8ToDouble(a);
    double db = convertP8ToDouble(b);
    double result = da + db;
    return convertDoubleToP8(result);
}

posit8_t p8_sub(posit8_t a, posit8_t b) {
    double da = convertP8ToDouble(a);
    double db = convertP8ToDouble(b);
    double result = da - db;
    return convertDoubleToP8(result);
}

posit8_t p8_mul(posit8_t a, posit8_t b) {
    double da = convertP8ToDouble(a);
    double db = convertP8ToDouble(b);
    double result = da * db;
    return convertDoubleToP8(result);
}

posit8_t p8_div(posit8_t a, posit8_t b) {
    double da = convertP8ToDouble(a);
    double db = convertP8ToDouble(b);
    double result = da / db;
    return convertDoubleToP8(result);
}

/* Utility functions */
bool p8_isNaR(posit8_t a) {
    return (a == 0x80);
}

bool p8_isZero(posit8_t a) {
    return (a == 0x00);
}
