#!/bin/bash

# Simple test script for the posit compiler
# Tests basic functionality and generates example outputs

echo "=== Posit Compiler Test Suite ==="
echo

# Build the compiler first
echo "Building compiler..."
make clean && make
if [ $? -ne 0 ]; then
    echo "Build failed!"
    exit 1
fi
echo "Build successful!"
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
echo "Generated code:"
cat test2.c
echo "Output:"
./test2
echo

# Test 3: Parentheses
echo "Test 3: Parentheses ((1.5p8 + 2.0p8) * 3.0p8)"
echo "posit8 x = (1.5p8 + 2.0p8) * 3.0p8;" | ./posit_compiler > test3.c
gcc -o test3 test3.c posit8.o -lm
echo "Generated code:"
cat test3.c
echo "Output:"
./test3
echo

# Test 4: Multiple variables
echo "Test 4: Multiple variables"
echo "posit8 x = 1.5p8; posit8 y = 2.0p8; posit8 z = x + y;" | ./posit_compiler > test4.c
gcc -o test4 test4.c posit8.o -lm
echo "Generated code:"
cat test4.c
echo "Output:"
./test4
echo

# Test 5: All operations
echo "Test 5: All operations (1.5p8 + 2.0p8 * 3.0p8 - 4.0p8 / 2.0p8)"
echo "posit8 x = 1.5p8 + 2.0p8 * 3.0p8 - 4.0p8 / 2.0p8;" | ./posit_compiler > test5.c
gcc -o test5 test5.c posit8.o -lm
echo "Generated code:"
cat test5.c
echo "Output:"
./test5
echo

# Test the posit8 library directly
echo "Test 6: Direct posit8 library test"
./posit_test
echo

echo "=== All tests completed ==="
echo "Note: Arithmetic accuracy depends on posit8 implementation"
echo "Generated test files: test1.c, test2.c, test3.c, test4.c, test5.c"
echo "Generated executables: test1, test2, test3, test4, test5"
