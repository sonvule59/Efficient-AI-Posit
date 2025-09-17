#include <stdio.h>
#include "posit8.h"

int main(void) {
    posit8 t0;
    posit8_from_double(&t0, 1.5);
    posit8 t1;
    posit8_from_double(&t1, 2.0);
    posit8 t2;
    posit8_add(&t2, t0, t1);
    posit8 x = t2;
    return 0;
}
