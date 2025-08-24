#include <stdio.h>
#include "posit8.h"

int main(void) {
    printf("=== Posit8 Math Test ===\n\n");
    
    posit8 a, b, result;
    float f_result;
    
    // Test 1: 2.5 + 2.0
    printf("Test 1: 2.5 + 2.0\n");
    posit8_from_double(&a, 2.5);
    posit8_from_double(&b, 2.0);
    posit8_add(&result, a, b);
    posit8_to_float(&f_result, result);
    printf("Expected: 4.5, Got: %.2f\n", f_result);
    printf("Posit8: 0x%02x\n\n", result);
    
    // Test 2: 0.2 + 0.1
    printf("Test 2: 0.2 + 0.1\n");
    posit8_from_double(&a, 0.2);
    posit8_from_double(&b, 0.1);
    posit8_add(&result, a, b);
    posit8_to_float(&f_result, result);
    printf("Expected: 0.3, Got: %.6f\n", f_result);
    printf("Posit8: 0x%02x\n\n", result);
    
    // Test 3: 1.5 * 2.0
    printf("Test 3: 1.5 * 2.0\n");
    posit8_from_double(&a, 1.5);
    posit8_from_double(&b, 2.0);
    posit8_mul(&result, a, b);
    posit8_to_float(&f_result, result);
    printf("Expected: 3.0, Got: %.2f\n", f_result);
    printf("Posit8: 0x%02x\n\n", result);
    
    // Test 4: 1.0 (should be 0x40)
    printf("Test 4: 1.0\n");
    posit8_from_double(&a, 1.0);
    posit8_to_float(&f_result, a);
    printf("Expected: 1.0, Got: %.2f\n", f_result);
    printf("Posit8: 0x%02x\n\n", a);
    
    return 0;
} 