#ifndef SOFTPOSIT_FIXED_H
#define SOFTPOSIT_FIXED_H

#include <stdint.h>
#include <stdbool.h>

/* Fixed SoftPosit header with C99 compatibility
 * Based on official SoftPosit but with compatibility fixes
 */

/* Posit8 type */
typedef uint8_t posit8_t;

/* Posit8 conversion functions */
posit8_t convertDoubleToP8(double a);
double convertP8ToDouble(posit8_t a);

/* Posit8 arithmetic functions */
posit8_t p8_add(posit8_t a, posit8_t b);
posit8_t p8_sub(posit8_t a, posit8_t b);
posit8_t p8_mul(posit8_t a, posit8_t b);
posit8_t p8_div(posit8_t a, posit8_t b);

/* Utility functions */
bool p8_isNaR(posit8_t a);
bool p8_isZero(posit8_t a);

#endif /* SOFTPOSIT_FIXED_H */
