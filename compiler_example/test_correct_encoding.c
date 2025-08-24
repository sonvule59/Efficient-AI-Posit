#include <stdio.h>
#include "posit8.h"

int main(void) {
    printf("=== Correct Posit8 Encoding Test ===\n\n");
    
    posit8 a, b, result;
    float f_result;
    
    // Test 1: 2.5 should be 0x54
    printf("Test 1: 2.5 encoding\n");
    posit8_from_double(&a, 2.5);
    posit8_to_float(&f_result, a);
    printf("Expected: 2.5 -> 0x54, Got: 0x%02x -> %.6f\n", a, f_result);
    
    // Test 2: 2.0 should be 0x50
    printf("Test 2: 2.0 encoding\n");
    posit8_from_double(&b, 2.0);
    posit8_to_float(&f_result, b);
    printf("Expected: 2.0 -> 0x50, Got: 0x%02x -> %.6f\n", b, f_result);
    
    // Test 3: 2.5 + 2.0 should be 0x5c
    printf("Test 3: 2.5 + 2.0\n");
    posit8_add(&result, a, b);
    posit8_to_float(&f_result, result);
    printf("Expected: 4.5 -> 0x5c, Got: 0x%02x -> %.6f\n", result, f_result);
    
    // Test 4: Check specific bit patterns
    printf("\nTest 4: Bit pattern analysis\n");
    printf("2.5 (0x%02x): ", a);
    for (int i = 7; i >= 0; i--) {
        printf("%d", (a >> i) & 1);
        if (i == 7) printf(" "); // Sign
        if (i == 6) printf(" "); // First regime
    }
    printf("\n");
    
    printf("2.0 (0x%02x): ", b);
    for (int i = 7; i >= 0; i--) {
        printf("%d", (b >> i) & 1);
        if (i == 7) printf(" "); // Sign
        if (i == 6) printf(" "); // First regime
    }
    printf("\n");
    
    printf("4.5 (0x%02x): ", result);
    for (int i = 7; i >= 0; i--) {
        printf("%d", (result >> i) & 1);
        if (i == 7) printf(" "); // Sign
        if (i == 6) printf(" "); // First regime
    }
    printf("\n");
    
    return 0;
} 