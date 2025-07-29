#include <stdio.h>
#include <stdlib.h>
#include "posit8.h"

/* Simple example showing posit8 integration with C
 * 
 * This demonstrates:
 * 1. How posit8 can coexist with float/double
 * 2. Named type usage (posit8 vs float)
 * 3. Conversion between types
 * 4. Basic arithmetic operations
 */

int main(void) {
    printf("=== Posit8 Integration Example ===\n\n");
    
    // 1. Declare variables with different types
    posit8 p1, p2, p_result;
    float f1 = 1.5f, f2 = 2.0f, f_result;
    double d1 = 3.14, d2 = 2.0, d_result;
    
    printf("1. Type coexistence:\n");
    printf("   posit8: p1, p2, p_result\n");
    printf("   float:  f1=%.2f, f2=%.2f\n", f1, f2);
    printf("   double: d1=%.2f, d2=%.2f\n\n", d1, d2);
    
    // 2. Convert float to posit8
    posit8_error_t err = float_to_posit8(&p1, f1);
    err |= float_to_posit8(&p2, f2);
    
    if (err != POSIT8_OK) {
        printf("Error converting float to posit8\n");
        return 1;
    }
    
    printf("2. Float to posit8 conversion:\n");
    printf("   f1=%.2f -> p1=0x%02x\n", f1, p1);
    printf("   f2=%.2f -> p2=0x%02x\n\n", f2, p2);
    
    // 3. Perform arithmetic with posit8
    printf("3. Posit8 arithmetic:\n");
    
    // Addition
    err = posit8_add(&p_result, p1, p2);
    if (err == POSIT8_OK) {
        posit8_to_float(&f_result, p_result);
        printf("   p1 + p2 = 0x%02x (≈ %.2f)\n", p_result, f_result);
    }
    
    // Multiplication
    err = posit8_mul(&p_result, p1, p2);
    if (err == POSIT8_OK) {
        posit8_to_float(&f_result, p_result);
        printf("   p1 * p2 = 0x%02x (≈ %.2f)\n", p_result, f_result);
    }
    
    // Fused multiply-add
    posit8 p3;
    float_to_posit8(&p3, 1.0f);
    err = posit8_fma(&p_result, p1, p2, p3);
    if (err == POSIT8_OK) {
        posit8_to_float(&f_result, p_result);
        printf("   p1 * p2 + 1.0 = 0x%02x (≈ %.2f)\n", p_result, f_result);
    }
    
    printf("\n");
    
    // 4. Compare with float arithmetic
    printf("4. Comparison with float arithmetic:\n");
    printf("   Float: %.2f + %.2f = %.2f\n", f1, f2, f1 + f2);
    printf("   Float: %.2f * %.2f = %.2f\n", f1, f2, f1 * f2);
    printf("   Float: %.2f * %.2f + 1.0 = %.2f\n\n", f1, f2, f1 * f2 + 1.0f);
    
    // 5. Demonstrate conversion from double
    printf("5. Double to posit8 conversion:\n");
    err = double_to_posit8(&p1, d1);
    err |= double_to_posit8(&p2, d2);
    
    if (err == POSIT8_OK) {
        posit8_to_double(&d_result, p1);
        printf("   d1=%.2f -> p1=0x%02x -> %.2f\n", d1, p1, d_result);
        
        posit8_to_double(&d_result, p2);
        printf("   d2=%.2f -> p2=0x%02x -> %.2f\n", d2, p2, d_result);
    }
    
    // 6. Show bit-level operations
    printf("\n6. Bit-level operations:\n");
    uint8_t bits = 0x40; // posit8 representation of 1.0
    posit8_from_bits(&p1, bits);
    posit8_to_float(&f_result, p1);
    printf("   Bits 0x%02x -> posit8 -> float %.2f\n", bits, f_result);
    
    // 7. Demonstrate error handling
    printf("\n7. Error handling:\n");
    posit8 nar = POSIT8_NAR;
    err = posit8_add(&p_result, p1, nar);
    if (err == POSIT8_NOT_A_REAL) {
        printf("   Correctly detected NaR (Not a Real)\n");
    }
    
    printf("\n=== Example completed successfully ===\n");
    return 0;
} 