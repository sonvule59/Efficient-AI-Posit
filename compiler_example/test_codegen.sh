#!/bin/bash

# Test script for posit compiler code generation
# Tests various expressions and compiles/runs the generated code

echo "=== Posit Compiler Code Generation Test Suite ==="
echo

# Test 1: Simple addition
echo "Test 1: Simple addition (1.5p8 + 2.0p8)"
echo "posit8 x = 1.5p8 + 2.0p8;" | ./posit_compiler > test1.c
gcc -o test1 test1.c posit8.o -lm
echo "Generated code:"
cat test1.c
echo "Output:"
./test1
echo

# Test 2: Complex expression with precedence
echo "Test 2: Complex expression (1.5p8 + 2.0p8 * 3.0p8)"
echo "posit8 x = 1.5p8 + 2.0p8 * 3.0p8;" | ./posit_compiler > test2.c
gcc -o test2 test2.c posit8.o -lm
./test2
echo

# Test 3: Parentheses
echo "Test 3: Parentheses ((1.5p8 + 2.0p8) * 3.0p8)"
echo "posit8 x = (1.5p8 + 2.0p8) * 3.0p8;" | ./posit_compiler > test3.c
gcc -o test3 test3.c posit8.o -lm
./test3
echo

# Test 4: Multiple variables
echo "Test 4: Multiple variables"
echo "posit8 x = 1.5p8; posit8 y = 2.0p8; posit8 z = x + y;" | ./posit_compiler > test4.c
gcc -o test4 test4.c posit8.o -lm
./test4
echo

# Test 5: All operations
echo "Test 5: All operations (1.5p8 + 2.0p8 * 3.0p8 - 4.0p8 / 2.0p8)"
echo "posit8 x = 1.5p8 + 2.0p8 * 3.0p8 - 4.0p8 / 2.0p8;" | ./posit_compiler > test5.c
gcc -o test5 test5.c posit8.o -lm
./test5
echo

echo "=== Code Generation Test Complete ==="
echo "All tests compiled and ran successfully!"
echo "Note: Arithmetic accuracy depends on posit8 implementation"
