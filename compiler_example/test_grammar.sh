#!/bin/bash

# Test suite for the posit compiler grammar
# Tests various expressions and statements

echo "=== Posit Compiler Grammar Test Suite ==="
echo

# Test 1: Basic arithmetic operations
echo "Test 1: Basic arithmetic operations"
echo "posit8 x = 1.5p8 + 2.0p8;" | ./posit_compiler
echo "posit8 y = 3.0p8 - 1.0p8;" | ./posit_compiler
echo "posit8 z = 2.0p8 * 3.0p8;" | ./posit_compiler
echo "posit8 w = 6.0p8 / 2.0p8;" | ./posit_compiler
echo

# Test 2: Operator precedence
echo "Test 2: Operator precedence"
echo "posit8 x = 1.5p8 + 2.0p8 * 3.0p8;" | ./posit_compiler
echo "posit8 y = (1.5p8 + 2.0p8) * 3.0p8;" | ./posit_compiler
echo

# Test 3: Complex expressions
echo "Test 3: Complex expressions"
echo "posit8 x = 1.5p8 + 2.0p8 * 3.0p8 - 4.0p8 / 2.0p8;" | ./posit_compiler
echo "posit8 y = (1.5p8 + 2.0p8) * (3.0p8 - 1.0p8);" | ./posit_compiler
echo

# Test 4: Multiple statements
echo "Test 4: Multiple statements"
echo "posit8 x = 1.5p8; posit8 y = 2.0p8; posit8 z = x + y;" | ./posit_compiler
echo

# Test 5: Different types
echo "Test 5: Different types"
echo "posit16 a = 1.5p16;" | ./posit_compiler
echo "posit32 b = 2.0p32;" | ./posit_compiler
echo "float c = 3.5;" | ./posit_compiler
echo "double d = 4.5;" | ./posit_compiler
echo

# Test 6: Variable references
echo "Test 6: Variable references"
echo "posit8 x = 1.5p8; posit8 y = x;" | ./posit_compiler
echo "posit8 z = x + y * 2.0p8;" | ./posit_compiler
echo

echo "=== All tests completed ==="
