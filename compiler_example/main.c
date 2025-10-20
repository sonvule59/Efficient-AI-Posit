#include <stdio.h>
#include "posit8.h"

int main(void) {
    posit8 a, b, c;
    float fc;
    posit8_error_t err;

    printf("=== Posit8 Arithmetic Test ===\n\n");

    // Test 1: Multiplication (1.5 * 2.0)
    err = posit8_from_double(&a, 1.5);
    err |= posit8_from_double(&b, 2.0);
    err |= posit8_mul(&c, a, b);
    if (err == POSIT8_OK) {
        err = posit8_to_float(&fc, c);
        if (err == POSIT8_OK) printf("Multiplication Result: %f\n", fc);
        else printf("Error: Not a Real in multiplication\n");
    } else {
        printf("Error in multiplication\n");
    }

    // Test 2: Addition (1.5 + 2.0)
    err = posit8_add(&c, a, b);
    if (err == POSIT8_OK) {
        err = posit8_to_float(&fc, c);
        if (err == POSIT8_OK) printf("Addition Result: %f\n", fc);
        else printf("Error: Not a Real in addition\n");
    } else {
        printf("Error in addition\n");
    }

    // Test 3: Subtraction (2.0 - 1.5)
    err = posit8_sub(&c, b, a);
    if (err == POSIT8_OK) {
        err = posit8_to_float(&fc, c);
        if (err == POSIT8_OK) printf("Subtraction Result: %f\n", fc);
        else printf("Error: Not a Real in subtraction\n");
    } else {
        printf("Error in subtraction\n");
    }

    // Test 4: Division (2.0 / 1.5)
    err = posit8_div(&c, b, a);
    if (err == POSIT8_OK) {
        err = posit8_to_float(&fc, c);
        if (err == POSIT8_OK) printf("Division Result: %f\n", fc);
        else printf("Error: Not a Real in division\n");
    } else {
        printf("Error in division\n");
    }

    // Test 5: FMA (1.5 * 2.0 + 1.0)
    posit8 d;
    err = posit8_from_double(&d, 1.0);
    err |= posit8_fma(&c, a, b, d);
    if (err == POSIT8_OK) {
        err = posit8_to_float(&fc, c);
        if (err == POSIT8_OK) printf("FMA Result: %f\n", fc);
        else printf("Error: Not a Real in FMA\n");
    } else {
        printf("Error in FMA\n");
    }

    printf("\n=== Test Complete ===\n");
    printf("Note: Results may not be accurate due to simplified posit8 implementation.\n");

    return 0;
}