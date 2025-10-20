#include <stdio.h>
#include "posit8.h"
#include "posit16.h"

/* Posit Visualizer - Shows encoding/decoding steps */

void visualize_posit8(double value) {
    printf("=== Posit8 Visualization ===\n");
    printf("Input value: %.6f\n", value);
    
    posit8 p;
    posit8_from_double(&p, value);
    
    printf("Encoded as: 0x%02x\n", p);
    
    // Show binary representation
    printf("Binary: ");
    for (int i = 7; i >= 0; i--) {
        printf("%d", (p >> i) & 1);
        if (i == 7) printf(" "); // Sign bit
        if (i == 6) printf(" "); // First regime bit
    }
    printf("\n");
    
    // Decode components
    int sign = (p >> 7) & 1;
    uint8_t abs_p = sign ? (~p + 1) : p;
    
    // Regime
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
    
    // Exponent
    int exp = 0;
    if (regime_len < 6) {
        exp = (abs_p >> (6 - regime_len)) & 1;
    }
    
    // Fraction
    int frac_bits = 7 - regime_len - 1;
    int frac = (frac_bits > 0) ? (abs_p & ((1 << frac_bits) - 1)) : 0;
    
    printf("Components:\n");
    printf("  Sign: %d\n", sign);
    printf("  Regime: %d (length: %d)\n", k, regime_len);
    printf("  Exponent: %d\n", exp);
    printf("  Fraction: %d (bits: %d)\n", frac, frac_bits);
    
    // Decode back
    double decoded;
    posit8_to_double(&decoded, p);
    printf("Decoded value: %.6f\n", decoded);
    printf("Error: %.10f\n", decoded - value);
    printf("\n");
}

void visualize_posit16(double value) {
    printf("=== Posit16 Visualization ===\n");
    printf("Input value: %.6f\n", value);
    
    posit16 p;
    posit16_from_double(&p, value);
    
    printf("Encoded as: 0x%04x\n", p);
    
    // Show binary representation
    printf("Binary: ");
    for (int i = 15; i >= 0; i--) {
        printf("%d", (p >> i) & 1);
        if (i == 15) printf(" "); // Sign bit
        if (i == 14) printf(" "); // First regime bit
    }
    printf("\n");
    
    // Decode components
    int sign = (p >> 15) & 1;
    uint16_t abs_p = sign ? (~p + 1) : p;
    
    // Regime
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
    
    // Exponent
    int exp = 0;
    if (regime_len < 14) {
        exp = (abs_p >> (14 - regime_len)) & 1;
    }
    
    // Fraction
    int frac_bits = 15 - regime_len - 1;
    int frac = (frac_bits > 0) ? (abs_p & ((1 << frac_bits) - 1)) : 0;
    
    printf("Components:\n");
    printf("  Sign: %d\n", sign);
    printf("  Regime: %d (length: %d)\n", k, regime_len);
    printf("  Exponent: %d\n", exp);
    printf("  Fraction: %d (bits: %d)\n", frac, frac_bits);
    
    // Decode back
    double decoded;
    posit16_to_double(&decoded, p);
    printf("Decoded value: %.6f\n", decoded);
    printf("Error: %.10f\n", decoded - value);
    printf("\n");
}

int main(void) {
    printf("=== Posit Encoding/Decoding Visualizer ===\n\n");
    
    // Test values
    double test_values[] = {1.0, 2.0, 3.0, 0.5, 1.5, 2.5, 4.0, 8.0};
    
    for (int i = 0; i < 8; i++) {
        visualize_posit8(test_values[i]);
        visualize_posit16(test_values[i]);
        printf("---\n");
    }
    
    return 0;
}
