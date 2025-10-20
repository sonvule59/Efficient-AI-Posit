#ifndef POSIT8_SOFTPOSIT_WRAPPER_H
#define POSIT8_SOFTPOSIT_WRAPPER_H

#include <stdint.h>
#include <stdbool.h>

/* SoftPosit wrapper for our compiler
 * This provides the interface our compiler expects
 */

typedef uint8_t posit8;

/* Error codes */
typedef enum {
    POSIT_OK = 0,
    POSIT_OVERFLOW,
    POSIT_INVALID,
    POSIT_NOT_A_REAL,
    POSIT8_OK = 0  // Alias for compatibility
} posit8_error_t;

/* Function prototypes that match our existing posit8.h interface */
posit8_error_t posit8_from_double(posit8 *p, double value);
posit8_error_t posit8_to_double(double *result, posit8 p);
posit8_error_t posit8_to_float(float *result, posit8 p);

posit8_error_t posit8_add(posit8 *result, posit8 a, posit8 b);
posit8_error_t posit8_sub(posit8 *result, posit8 a, posit8 b);
posit8_error_t posit8_mul(posit8 *result, posit8 a, posit8 b);
posit8_error_t posit8_div(posit8 *result, posit8 a, posit8 b);
posit8_error_t posit8_fma(posit8 *result, posit8 a, posit8 b, posit8 c);

/* Utility functions */
bool posit8_is_nar(posit8 p);
bool posit8_is_zero(posit8 p);

#endif /* POSIT8_SOFTPOSIT_WRAPPER_H */
