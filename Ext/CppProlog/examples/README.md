# Prolog Examples

This directory contains example Prolog programs and C++ applications demonstrating the CppLProlog interpreter.

For architectural details and data flow diagrams, see:
- [System Architecture](../docs/ARCHITECTURE.md)
- [Data Flow Documentation](../docs/DATA_FLOW.md)

## Structure

- **Prolog Files (*.pl)**: Pure Prolog programs that can be loaded by the interpreter
- **C++ Applications**: Demonstrate how to use the interpreter programmatically

## Prolog Programs

### basic.pl
Simple facts and rules demonstrating basic Prolog concepts:
- Facts about likes relationships
- Rules for happiness and friendship

### family.pl
Comprehensive family tree with relationships:
- Parent/child facts
- Gender classifications
- Derived relationships (father, mother, grandparent, etc.)
- Complex queries about family connections

### lists.pl
List processing predicates:
- Membership testing
- List concatenation (append)
- Length calculation
- Reversal operations
- Element removal
- Sorting checks

## C++ Examples

### basic_example.cpp
Demonstrates basic interpreter usage with simple queries.

### family_tree.cpp
Shows complex relationship queries and family tree visualization.

### list_processing.cpp
Illustrates list manipulation and processing operations.

### arithmetic.cpp
Demonstrates the comprehensive arithmetic system with:
- Expression evaluation using `is/2`
- Comparison operators (`<`, `>`, `=<`, `>=`)
- Type checking predicates
- Mathematical computations

## Running Examples

All example executables are built in the `build/bin/` directory:

```bash
# Build all examples
cmake --build build

# Run individual examples
./build/bin/basic_example
./build/bin/family_tree
./build/bin/list_processing
./build/bin/arithmetic

# Run with the interpreter
./build/bin/prolog_interpreter examples/basic.pl
./build/bin/prolog_interpreter examples/family.pl
./build/bin/prolog_interpreter examples/lists.pl
```

## New Features Demonstrated

The examples now showcase the expanded built-in predicate library:

### Arithmetic Examples
```prolog
% Basic arithmetic evaluation
?- X is 2 + 3 * 4.
X = 14.

% Complex expressions with parentheses  
?- Y is (10 + 5) / 3.
Y = 5.0.

% Integer division and modulo
?- Z is 17 // 5, W is 17 mod 5.
Z = 3, W = 2.

% Comparison operations
?- 5 < 10.
true.

?- 3.14 > 3.
true.
```

### Type Checking Examples
```prolog
% Test variable binding
?- var(X).
true.

?- var(42).
false.

% Check specific types
?- integer(42).
true.

?- float(3.14).
true.

% Test if term is fully ground
?- ground(hello(world)).
true.

?- ground(hello(X)).
false.
```

### Control Structure Examples
```prolog
% Negation as failure
?- \+ member(x, [a, b, c]).
true.

% I/O operations
?- write('Hello '), write('World'), nl.
Hello World
true.
```

## Loading Prolog Files

Examples show two ways to load Prolog programs:

1. **From file**: `interpreter.loadFile("examples/basic.pl")`
2. **From string**: `interpreter.loadString(program_text)`

The file-based approach is recommended for larger programs as it provides better separation of concerns and easier maintenance.