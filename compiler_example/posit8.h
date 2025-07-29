#ifndef POSIT8_H
#define POSIT8_H

#include <stdint.h>

/* 8-bit Posit Type and Operations
 * 
 * This provides a complete interface for 8-bit posit arithmetic,
 * designed to coexist with standard C types while providing
 * a clear, named type for posit operations.
 */

typedef uint8_t posit8;

/* Error codes for posit8 operations */
typedef enum {
    POSIT8_OK = 0,
    POSIT8_OVERFLOW,
    POSIT8_INVALID,
    POSIT8_NOT_A_REAL
} posit8_error_t;

/* Basic arithmetic operations for posit8 */
posit8_error_t posit8_add(posit8 *result, posit8 a, posit8 b);
posit8_error_t posit8_sub(posit8 *result, posit8 a, posit8 b);
posit8_error_t posit8_mul(posit8 *result, posit8 a, posit8 b);
posit8_error_t posit8_div(posit8 *result, posit8 a, posit8 b);

/* Fused multiply-add for posit8 */
posit8_error_t posit8_fma(posit8 *result, posit8 a, posit8 b, posit8 c);

/* Conversions between posit8 and standard types */
posit8_error_t float_to_posit8(posit8 *result, float f);
posit8_error_t posit8_to_float(float *result, posit8 p);
posit8_error_t double_to_posit8(posit8 *result, double d);
posit8_error_t posit8_to_double(double *result, posit8 p);

/* Literal construction from doubles */
posit8_error_t posit8_from_double(posit8 *result, double d);

/* Utility functions */
posit8_error_t posit8_from_bits(posit8 *result, uint8_t bits);
uint8_t posit8_to_bits(posit8 p);
double posit8_to_double_internal(posit8 p);

/* Constants */
#define POSIT8_ZERO    0x00
#define POSIT8_ONE     0x40
#define POSIT8_NAR     0x80  /* Not a Real */

#endif /* POSIT8_H */ 