#include <stdio.h>
#include "posit8.h"

int main(void) {
    posit8 t0;
    posit8_from_double(&t0, 1.5);
    posit8 t1;
    posit8_from_double(&t1, 2.0);
    posit8 t2;
    posit8_add(&t2, t0, t1);
    posit8 t3;
    posit8_from_double(&t3, 3.0);
    posit8 t4;
    posit8_mul(&t4, t2, t3);
    posit8 x = t4;
    double result0;
    posit8_to_double(&result0, x);
    printf("Result: %.6f\n", result0);
    return 0;
}
