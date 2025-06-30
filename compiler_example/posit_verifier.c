#include "posit.h"
#include <stdio.h>
#include <math.h>

/* Basic Posit Arithmetic Verifier
 *
 * Verifies posit32 operations by comparing results against expected double-precision
 * values. Designed to validate the correctness of a posit compiler's runtime library.
 * Currently uses simplified double-based arithmetic for comparison.
 *
 * Note: Should be extended with a full posit reference implementation for accuracy.
 */

typedef struct {
    const char *operation;
    posit32 a;
    posit32 b;
    posit32 c; // For FMA
    double expected;
    posit_error_t expected_error;
} posit_test_case;

static posit_test_case tests[] = {
    {"Addition", 0, 0, 0, 1.5 + 2.0, POSIT_OK}, // 1.5 + 2.0
    {"Multiplication", 0, 0, 0, 1.5 * 2.0, POSIT_OK}, // 1.5 * 2.0
    {"FMA", 0, 0, 0, 1.5 * 2.0 + 1.0, POSIT_OK}, // 1.5 * 2.0 + 1.0
    {"Quire", 0, 0, 0, 1.5 * 2.0, POSIT_OK}, // Accumulate 1.5 * 2.0
    {"Division by Zero", 0, 0, 0, 0.0, POSIT_NOT_A_REAL}, // 1.5 / 0
};

static int run_test(posit_test_case *test) {
    posit32 result, a, b, c;
    float fa;
    posit_error_t err;

    err = posit32_from_double(&a, test->operation[0] == 'Q' ? 1.5 : test->a);
    err |= posit32_from_double(&b, test->operation[0] == 'Q' ? 2.0 : test->b);
    err |= posit32_from_double(&c, test->operation[0] == 'F' ? 1.0 : 0.0);
    if (err != POSIT_OK) {
        printf("Test %s: Failed to initialize inputs\n", test->operation);
        return 0;
    }

    if (test->operation[0] == 'A') { // Addition
        err = posit32_add(&result, a, b);
    } else if (test->operation[0] == 'M') { // Multiplication
        err = posit32_mul(&result, a, b);
    } else if (test->operation[0] == 'F') { // FMA
        err = posit32_fma(&result, a, b, c);
    } else if (test->operation[0] == 'Q') { // Quire
        quire32 q = quire32_init();
        err = quire32_fma(&q, a, b);
        if (err == POSIT_OK) {
            err = quire32_to_posit(&result, q);
        }
    } else if (test->operation[0] == 'D') { // Division by Zero
        err = posit32_from_double(&b, 0.0);
        err |= posit32_div(&result, a, b);
    }

    if (err != test->expected_error) {
        printf("Test %s: Unexpected error code %d (expected %d)\n",
               test->operation, err, test->expected_error);
        return 0;
    }

    if (err == POSIT_OK) {
        err = posit32_to_float(&fa, result);
        if (err != POSIT_OK) {
            printf("Test %s: Failed to convert result\n", test->operation);
            return 0;
        }
        if (fabs(fa - test->expected) > 1e-6) {
            printf("Test %s: Incorrect result %f (expected %f)\n",
                   test->operation, fa, test->expected);
            return 0;
        }
    }

    printf("Test %s: Passed\n", test->operation);
    return 1;
}

int main(void) {
    int passed = 0, total = sizeof(tests) / sizeof(tests[0]);
    for (int i = 0; i < total; i++) {
        passed += run_test(&tests[i]);
    }
    printf("%d/%d tests passed\n", passed, total);
    return passed == total ? 0 : 1;
}