#!/bin/bash

echo "=== Testing Posit8 Interpreter ==="
echo

# Test 1: Basic posit8 literals
echo "Test 1: Posit8 literals"
echo "1.0p8" | ./posit8_interpreter
echo

# Test 2: Arithmetic operations
echo "Test 2: Addition"
echo "1.0p8 + 2.0p8" | ./posit8_interpreter
echo

# Test 3: Regular number conversion
echo "Test 3: Regular number to posit8"
echo "1.5 + 2.0" | ./posit8_interpreter
echo

# Test 4: Multiplication
echo "Test 4: Multiplication"
echo "1.5p8 * 2.0p8" | ./posit8_interpreter
echo

echo "=== Interpreter Test Complete ===" 