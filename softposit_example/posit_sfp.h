#ifndef POSIT_H
#define POSIT_H

#include <stdint.h>

/* Posit Arithmetic Interface for C
 *
 * Defines posit types (posit8, posit16, posit32), the quire type (quire32), and
 * functions for arithmetic, fused operations, quire manipulations, and IEEE 754
 * conversions. Supports Dr. John L. Gustafson's requirements for posit integration
 * in C, including coexistence with floats, fused multiply-add, and quire-based
 * exact accumulation. Uses SoftPosit for true posit arithmetic.
 *
 * Note: Requires SoftPosit library for implementation. Quire operations are
 * software-emulated pending hardware support.
 */

/* Posit Types
 * - posit8: 8-bit posit, 0 exponent bits (es=0)
 * - posit16: 16-bit posit, 1 exponent bit (es=1)
 * - posit32: 32-bit posit, 2 exponent bits (es=2)
 * Compatible with SoftPosit types (posit8_t, posit16_t, posit32_t).
 */
typedef uint8_t posit8;
typedef uint16_t posit16;
typedef uint32_t posit32;

/* Quire Type
 * - quire32: 512-bit accumulator for posit32 operations, storing exact sums
 *   (e.g., dot products) without intermediate rounding.
 * - Implemented as 8 uint64_t (512 bits) for software emulation.
 */
typedef struct {
    uint64_t data[8];
} quire32;

/* Error Codes
 * Returned by functions to indicate success or failure.
 */
typedef enum {
    POSIT_OK = 0,
    POSIT_OVERFLOW,
    POSIT_INVALID,
    POSIT_NOT_A_REAL
} posit_error_t;

/* Arithmetic Operations for posit32
 * Standard operations using SoftPosit. Store result in *result.
 */
posit_error_t posit32_add(posit32 *result, posit32 a, posit32 b);
posit_error_t posit32_sub(posit32 *result, posit32 a, posit32 b);
posit_error_t posit32_mul(posit32 *result, posit32 a, posit32 b);
posit_error_t posit32_div(posit32 *result, posit32 a, posit32 b);

/* Fused Operations for posit32
 * Fused multiply-add (FMA): a * b + c without intermediate rounding.
 */
posit_error_t posit32_fma(posit32 *result, posit32 a, posit32 b, posit32 c);

/* Quire Operations for posit32
 * - quire32_init: Initialize quire to zero.
 * - quire32_fma: Accumulate a * b into quire.
 * - quire32_to_posit: Extract posit32 result from quire.
 */
quire32 quire32_init(void);
posit_error_t quire32_fma(quire32 *q, posit32 a, posit32 b);
posit_error_t quire32_to_posit(posit32 *result, quire32 q);

/* Conversions
 * Convert between posit32 and IEEE 754 float/double using SoftPosit.
 */
posit_error_t float_to_posit32(posit32 *result, float f);
posit_error_t posit32_to_float(float *result, posit32 p);
posit_error_t double_to_posit32(posit32 *result, double d);
posit_error_t posit32_to_double(double *result, posit32 p);

/* Literal Construction
 * Create posit values from doubles.
 */
posit_error_t posit8_from_double(posit8 *result, double d);
posit_error_t posit16_from_double(posit16 *result, double d);
posit_error_t posit32_from_double(posit32 *result, double d);

#endif /* POSIT_H */