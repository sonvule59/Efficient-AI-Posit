# Posit Arithmetic Compiler

A simple compiler that translates posit arithmetic expressions into C code.

## What it does

This compiler takes posit arithmetic expressions and generates executable C code that uses the posit8 library.

## Files

- `lexer.l` - Lexical analyzer (tokenizer)
- `parser.y` - Grammar parser with code generation
- `posit8.h` - Posit8 type definitions and function declarations
- `posit8.c` - Posit8 arithmetic implementation
- `main.c` - Simple test program
- `Makefile` - Build system

## Building

```bash
make clean && make
```

## Usage

### Compile posit expressions to C:

```bash
echo "posit8 x = 1.5p8 + 2.0p8;" | ./posit_compiler > output.c
gcc -o output output.c posit8.o -lm
./output
```

### Test the posit8 library:

```bash
./posit_test
```

### Run comprehensive tests:

```bash
./test_compiler.sh
```

This will test various expressions and show both the generated C code and execution results.

## Supported Language Features

- Variable declarations: `posit8 x = 1.5p8;`
- Arithmetic operations: `+`, `-`, `*`, `/`
- Operator precedence and parentheses
- Multiple statements
- Variable references in expressions

## Example

Input:
```
posit8 x = 1.5p8 + 2.0p8;
posit8 y = x * 3.0p8;
```

Generated C:
```c
#include <stdio.h>
#include "posit8.h"

int main(void) {
    posit8 t0;
    posit8_from_double(&t0, 1.5);
    posit8 t1;
    posit8_from_double(&t1, 2.0);
    posit8 t2;
    posit8_add(&t2, t0, t1);
    posit8 x = t2;
    double result0;
    posit8_to_double(&result0, x);
    printf("Result: %.6f\n", result0);
    
    posit8 t3;
    posit8_mul(&t3, x, t1);
    posit8 y = t3;
    double result1;
    posit8_to_double(&result1, y);
    printf("Result: %.6f\n", result1);
    
    return 0;
}
```

## Note

The posit8 arithmetic implementation has accuracy issues and is for demonstration purposes only. For production use, integrate with a proper posit library like SoftPosit.
