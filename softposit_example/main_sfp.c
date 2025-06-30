#include <stdio.h>
#include "posit.h"

int main(void) {
    posit32 a, b, c, d;
    float fc;
    posit_error_t err;

    // Test 1: Multiplication (1.5 * 2.0)
    err = posit32_from_double(&a, 1.5);
    err |= posit32_from_double(&b, 2.0);
    err |= posit32_mul(&c, a, b);
    if (err == POSIT_OK) {
        err = posit32_to_float(&fc, c);
        if (err == POSIT_OK) printf("Multiplication Result: %f\n", fc);
        else printf("Error: Not a Real in multiplication\n");
    } else {
        printf("Error in multiplication\n");
    }

    // Test 2: Addition (1.5 + 2.0)
    err = posit32_add(&c, a, b);
    if (err == POSIT_OK) {
        err = posit32_to_float(&fc, c);
        if (err == POSIT_OK) printf("Addition Result: %f\n", fc);
        else printf("Error: Not a Real in addition\n");
    } else {
        printf("Error in addition\n");
    }

    // Test 3: FMA (1.5 * 2.0 + 1.0)
    err = posit32_from_double(&c, 1.0);
    err |= posit32_fma(&d, a, b, c);
    if (err == POSIT_OK) {
        err = posit32_to_float(&fc, d);
        if (err == POSIT_OK) printf("FMA Result: %f\n", fc);
        else printf("Error: Not a Real in FMA\n");
    } else {
        printf("Error in FMA\n");
    }

    // Test 4: Quire (accumulate 1.5 * 2.0)
    quire32 q = quire32_init();
    err = quire32_fma(&q, a, b);
    if (err == POSIT_OK) {
        err = quire32_to_posit(&c, q);
        if (err == POSIT_OK) {
            err = posit32_to_float(&fc, c);
            if (err == POSIT_OK) printf("Quire Result: %f\n", fc);
            else printf("Error: Not a Real in quire\n");
        } else {
            printf("Error in quire extraction\n");
        }
    } else {
        printf("Error in quire accumulation\n");
    }

    // Test 5: Conversion Error (NaN input)
    err = float_to_posit32(&c, 0.0f / 0.0f); // NaN
    if (err == POSIT_NOT_A_REAL) {
        printf("Error: NaN input detected\n");
    }

    return 0;
}