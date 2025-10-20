#include <stdio.h>
#include "posit8.h"

int main(void) {
    posit8 t0;
    posit8_from_double(&t0, 5.0);
    posit8 a = t0;
    double result0;
    posit8_to_double(&result0, a);
    printf("Result: %.6f\n", result0);
    return 0;
}
