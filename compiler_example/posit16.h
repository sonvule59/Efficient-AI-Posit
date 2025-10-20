#ifndef POSIT16_H
#define POSIT16_H

#include <stdint.h>
#include <stdbool.h>

/* Posit16 type and error codes */
typedef uint16_t posit16;

/* Use the same error codes as posit8 to avoid conflicts */
typedef int posit16_error_t;

/* Posit16 function prototypes */
posit16_error_t posit16_from_double(posit16 *p, double value);
posit16_error_t posit16_to_double(double *result, posit16 p);
posit16_error_t posit16_to_float(float *result, posit16 p);

posit16_error_t posit16_add(posit16 *result, posit16 a, posit16 b);
posit16_error_t posit16_sub(posit16 *result, posit16 a, posit16 b);
posit16_error_t posit16_mul(posit16 *result, posit16 a, posit16 b);
posit16_error_t posit16_div(posit16 *result, posit16 a, posit16 b);
posit16_error_t posit16_fma(posit16 *result, posit16 a, posit16 b, posit16 c);

/* Utility functions */
bool posit16_is_nar(posit16 p);
bool posit16_is_zero(posit16 p);

/* Debug/Visualization functions */
void posit16_print_binary(posit16 p);
void posit16_print_components(posit16 p);
void posit16_decode_components(posit16 p, int *sign, int *regime, int *exponent, int *fraction);

#endif /* POSIT16_H */
