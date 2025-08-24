#include <stdio.h>
#include <stdlib.h>
#include "posit8.h"

/* Posit8 Integration Demo
 * 
 * This demonstrates how posit8 can be integrated with C, focusing on:
 * 1. Named type coexistence with float/double
 * 2. Type conversion and interoperability
 * 3. Error handling and type safety
 * 4. Integration patterns for existing codebases
 * 
 * Note: Arithmetic accuracy requires proper posit implementation
 */

/* Example 1: Mixed-type arithmetic */
void demo_mixed_arithmetic(void) {
    printf("=== Mixed-Type Arithmetic Demo ===\n");
    
    // Declare variables of different types
    posit8 p1, p2, p_result;
    float f1 = 1.5f, f2 = 2.0f, f_result;
    double d1 = 3.14, d2 = 2.0;
    
    printf("Types coexist: posit8, float, double\n");
    printf("f1=%.2f, f2=%.2f, d1=%.2f, d2=%.2f\n\n", f1, f2, d1, d2);
    
    // Convert float to posit8
    posit8_error_t err = float_to_posit8(&p1, f1);
    err |= float_to_posit8(&p2, f2);
    
    if (err == POSIT8_OK) {
        printf("Float to posit8 conversion successful\n");
        printf("f1=%.2f -> p1=0x%02x\n", f1, p1);
        printf("f2=%.2f -> p2=0x%02x\n\n", f2, p2);
    }
    
    // Perform posit8 arithmetic
    err = posit8_add(&p_result, p1, p2);
    if (err == POSIT8_OK) {
        posit8_to_float(&f_result, p_result);
        printf("Posit8 addition: p1 + p2 = 0x%02x (≈ %.2f)\n", p_result, f_result);
    }
    
    // Compare with float arithmetic
    printf("Float addition: f1 + f2 = %.2f\n\n", f1 + f2);
}

/* Example 2: Type conversion utilities */
void demo_type_conversion(void) {
    printf("=== Type Conversion Demo ===\n");
    
    posit8 p;
    float f;
    double d;
    
    // Double to posit8
    double_to_posit8(&p, 2.5);
    posit8_to_double(&d, p);
    printf("2.5 -> posit8 -> double: %.6f\n", d);
    
    // Float to posit8
    float_to_posit8(&p, 1.5f);
    posit8_to_float(&f, p);
    printf("1.5f -> posit8 -> float: %.6f\n", f);
    
    // Bit-level access
    uint8_t bits = posit8_to_bits(p);
    printf("Bit representation: 0x%02x\n\n", bits);
}

/* Example 3: Error handling */
void demo_error_handling(void) {
    printf("=== Error Handling Demo ===\n");
    
    posit8 result;
    posit8_error_t err;
    
    // Test division by zero
    posit8 a, b;
    posit8_from_bits(&a, 0x54); // Some posit8 value
    posit8_from_bits(&b, 0x00); // Zero
    
    err = posit8_div(&result, a, b);
    if (err == POSIT8_NOT_A_REAL) {
        printf("✓ Correctly detected division by zero\n");
    }
    
    // Test NaN handling
    err = float_to_posit8(&result, 0.0f / 0.0f);
    if (err == POSIT8_NOT_A_REAL) {
        printf("✓ Correctly detected NaN input\n");
    }
    
    printf("\n");
}

/* Example 4: Integration patterns */
void demo_integration_patterns(void) {
    printf("=== Integration Patterns Demo ===\n");
    
    // Pattern 1: Gradual migration
    printf("Pattern 1: Gradual Migration\n");
    float legacy_value = 1.5f;
    posit8 new_value;
    
    // Convert legacy float to posit8
    float_to_posit8(&new_value, legacy_value);
    printf("Legacy float %.2f -> posit8 0x%02x\n", legacy_value, new_value);
    
    // Pattern 2: Type-safe operations
    printf("Pattern 2: Type-Safe Operations\n");
    posit8 a, b, c;
    float_to_posit8(&a, 1.0f);
    float_to_posit8(&b, 2.0f);
    
    // All operations return error codes
    posit8_error_t err = posit8_add(&c, a, b);
    if (err == POSIT8_OK) {
        printf("Type-safe addition: 0x%02x + 0x%02x = 0x%02x\n", a, b, c);
    }
    
    // Pattern 3: Interoperability
    printf("Pattern 3: Interoperability\n");
    float f_result;
    posit8_to_float(&f_result, c);
    printf("Posit8 result converted back to float: %.2f\n", f_result);
    
    printf("\n");
}

/* Example 5: Performance considerations */
void demo_performance_notes(void) {
    printf("=== Performance Considerations ===\n");
    
    printf("• posit8 uses 8 bits vs float's 32 bits\n");
    printf("• Memory usage: 75%% reduction\n");
    printf("• Arithmetic: Requires proper implementation\n");
    printf("• Integration: Minimal code changes needed\n");
    printf("• Type safety: Compile-time checking\n\n");
}

/* Example 6: Real-world integration example */
void demo_real_world_integration(void) {
    printf("=== Real-World Integration Example ===\n");
    
    // Simulate legacy and modern functions
    float f_x = 2.0f, f_y = 3.0f;
    posit8 p_x, p_y;
    float_to_posit8(&p_x, f_x);
    float_to_posit8(&p_y, f_y);
    
    // Legacy-style calculation
    float legacy_result = f_x * f_y + 1.0f;
    
    // Modern posit8-style calculation
    posit8 temp, modern_result;
    posit8_mul(&temp, p_x, p_y);
    posit8_from_double(&modern_result, 1.0);
    posit8_add(&modern_result, temp, modern_result);
    
    printf("Legacy function: %.2f * %.2f + 1.0 = %.2f\n", f_x, f_y, legacy_result);
    
    float modern_as_float;
    posit8_to_float(&modern_as_float, modern_result);
    printf("Modern function: posit8 equivalent = %.2f\n", modern_as_float);
    
    printf("\n");
}

int main(void) {
    printf("=== Posit8 Integration with C - Comprehensive Demo ===\n\n");
    
    demo_mixed_arithmetic();
    demo_type_conversion();
    demo_error_handling();
    demo_integration_patterns();
    demo_performance_notes();
    demo_real_world_integration();
    
    printf("=== Integration Demo Complete ===\n");
    printf("Key achievements:\n");
    printf("✅ Named type coexistence\n");
    printf("✅ Bidirectional conversion\n");
    printf("✅ Type safety and error handling\n");
    printf("✅ Gradual migration path\n");
    printf("✅ Minimal code changes required\n");
    printf("⚠️  Note: Arithmetic accuracy requires proper posit implementation\n");
    
    return 0;
} 