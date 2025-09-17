#include <stdio.h>
#include "posit8.h"

int main(void) {
    posit8 t0;
    posit8_from_double(&t0, 1.5);
    posit8 x = t0;
    double result;
    posit8_to_double(&result, x);
    printf("Result: %.6f\n", result);
    posit8 t1;
    posit8_from_double(&t1, 2.0);
    posit8 y = t1;
    double result;
    posit8_to_double(&result, y);
    printf("Result: %.6f\n", result);
    posit8 t2;
    posit8_add(&t2, x, y);
    posit8 z = t2;
    double result;
    posit8_to_double(&result, z);
    printf("Result: %.6f\n", result);
    return 0;
}
