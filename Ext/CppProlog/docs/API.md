# CppLProlog API Reference

This document provides a comprehensive reference for the CppLProlog C++ API.

## Table of Contents

1. [Core Classes](#core-classes)
2. [Term System](#term-system)
3. [Unification](#unification)
4. [Parser](#parser)
5. [Database](#database)
6. [Resolver](#resolver)
7. [Interpreter](#interpreter)
8. [Built-in Predicates](#built-in-predicates)
9. [Utilities](#utilities)

## Core Classes

### Namespaces

All CppLProlog classes are in the `prolog` namespace:

```cpp
namespace prolog {
    // All classes here
}

namespace prolog::utils {
    // Utility classes
}
```

### Basic Types

```cpp
using TermPtr = std::shared_ptr<Term>;
using TermList = std::vector<TermPtr>;
using Substitution = std::unordered_map<std::string, TermPtr>;
using ClausePtr = std::unique_ptr<Clause>;
```

## Term System

### Term Base Class

```cpp
class Term {
public:
    virtual ~Term() = default;
    
    TermType type() const;
    virtual std::string toString() const = 0;
    virtual TermPtr clone() const = 0;
    virtual bool equals(const Term& other) const = 0;
    virtual size_t hash() const = 0;
    
    template<typename T>
    const T* as() const;        // Safe downcast
    
    template<typename T>
    bool is() const;            // Type checking
};

enum class TermType {
    ATOM, VARIABLE, COMPOUND, INTEGER, FLOAT, STRING, LIST
};
```

### Concrete Term Types

#### Atom

```cpp
class Atom : public Term {
public:
    explicit Atom(std::string name);
    const std::string& name() const;
    
    // Inherited methods
    std::string toString() const override;
    TermPtr clone() const override;
    bool equals(const Term& other) const override;
    size_t hash() const override;
};
```

#### Variable

```cpp
class Variable : public Term {
public:
    explicit Variable(std::string name);
    const std::string& name() const;
    
    // Inherited methods...
};
```

#### Integer

```cpp
class Integer : public Term {
public:
    explicit Integer(int64_t value);
    int64_t value() const;
    
    // Inherited methods...
};
```

#### Float

```cpp
class Float : public Term {
public:
    explicit Float(double value);
    double value() const;
    
    // Inherited methods...
};
```

#### String

```cpp
class String : public Term {
public:
    explicit String(std::string value);
    const std::string& value() const;
    
    // Inherited methods...
};
```

#### Compound

```cpp
class Compound : public Term {
public:
    Compound(std::string functor, TermList arguments);
    
    const std::string& functor() const;
    const TermList& arguments() const;
    size_t arity() const;
    
    // Inherited methods...
};
```

#### List

```cpp
class List : public Term {
public:
    explicit List(TermList elements, TermPtr tail = nullptr);
    
    const TermList& elements() const;
    const TermPtr& tail() const;
    bool hasProperTail() const;
    
    // Inherited methods...
};
```

### Factory Functions

```cpp
// Term factory functions
TermPtr makeAtom(const std::string& name);
TermPtr makeVariable(const std::string& name);
TermPtr makeInteger(int64_t value);
TermPtr makeFloat(double value);
TermPtr makeString(const std::string& value);
TermPtr makeCompound(const std::string& functor, TermList arguments);
TermPtr makeList(TermList elements, TermPtr tail = nullptr);

// Examples
auto atom = makeAtom("hello");
auto var = makeVariable("X");
auto compound = makeCompound("parent", {makeAtom("tom"), makeAtom("bob")});
auto list = makeList({makeAtom("a"), makeAtom("b"), makeAtom("c")});
```

## Unification

### Unification Class

```cpp
class Unification {
public:
    // Primary unification methods
    static std::optional<Substitution> unify(const TermPtr& term1, const TermPtr& term2);
    static std::optional<Substitution> unify(const TermPtr& term1, const TermPtr& term2, 
                                           Substitution& subst);
    
    // Substitution application
    static TermPtr applySubstitution(const TermPtr& term, const Substitution& subst);
    static void applySubstitutionInPlace(TermList& terms, const Substitution& subst);
    
    // Substitution composition
    static Substitution compose(const Substitution& s1, const Substitution& s2);
    
    // Occurs check
    static bool occursCheck(const std::string& var, const TermPtr& term);
};
```

### Usage Examples

```cpp
// Basic unification
auto term1 = makeCompound("f", {makeVariable("X"), makeAtom("a")});
auto term2 = makeCompound("f", {makeAtom("b"), makeVariable("Y")});

auto result = Unification::unify(term1, term2);
if (result) {
    // Unification succeeded
    const auto& substitution = *result;
    // X -> b, Y -> a
}

// Apply substitution
Substitution subst;
subst["X"] = makeAtom("hello");
auto instantiated = Unification::applySubstitution(makeVariable("X"), subst);
// instantiated is now makeAtom("hello")
```

## Parser

### Lexer

```cpp
class Lexer {
public:
    explicit Lexer(std::string input);
    std::vector<Token> tokenize();
};

enum class TokenType {
    ATOM, VARIABLE, INTEGER, FLOAT, STRING,
    LPAREN, RPAREN, LBRACKET, RBRACKET,
    DOT, COMMA, PIPE, RULE_OP,
    END_OF_INPUT, INVALID
};

class Token {
public:
    TokenType type;
    std::string value;
    size_t position;
};
```

### Parser

```cpp
class Parser {
public:
    explicit Parser(std::vector<Token> tokens);
    
    std::optional<ClausePtr> parseClause();
    std::vector<ClausePtr> parseProgram(const std::string& input);
    TermPtr parseQuery(const std::string& input);
};

class ParseException : public std::exception {
public:
    explicit ParseException(std::string message);
    const char* what() const noexcept override;
};
```

### Usage Examples

```cpp
// Parse a single term
Parser parser({});
auto term = parser.parseQuery("parent(tom, bob)");

// Parse a complete program
std::string program = R"(
    parent(tom, bob).
    parent(bob, ann).
    grandparent(X, Z) :- parent(X, Y), parent(Y, Z).
)";

auto clauses = parser.parseProgram(program);
```

## Database

### Database Class

```cpp
class Database {
public:
    // Clause management
    void addClause(ClausePtr clause);
    void addFact(TermPtr head);
    void addRule(TermPtr head, TermList body);
    void clear();
    
    // Query operations
    std::vector<ClausePtr> findClauses(const std::string& functor, size_t arity) const;
    std::vector<ClausePtr> findMatchingClauses(const TermPtr& goal) const;
    
    // Program loading
    void loadProgram(const std::string& program);
    
    // Inspection
    size_t size() const;
    bool empty() const;
    std::string toString() const;
};
```

### Usage Examples

```cpp
Database db;

// Add facts
db.addFact(makeCompound("parent", {makeAtom("tom"), makeAtom("bob")}));

// Add rules  
auto head = makeCompound("grandparent", {makeVariable("X"), makeVariable("Z")});
TermList body = {
    makeCompound("parent", {makeVariable("X"), makeVariable("Y")}),
    makeCompound("parent", {makeVariable("Y"), makeVariable("Z")})
};
db.addRule(head, body);

// Load complete program
std::string program = "parent(tom, bob). parent(bob, ann).";
db.loadProgram(program);

// Find matching clauses
auto goal = makeCompound("parent", {makeVariable("X"), makeAtom("bob")});
auto clauses = db.findMatchingClauses(goal);
```

## Resolver

### Solution and Choice

```cpp
struct Solution {
    Substitution bindings;
    std::string toString() const;
};

class Choice {
public:
    TermPtr goal;
    TermList remaining_goals;
    std::vector<ClausePtr> clauses;
    size_t clause_index;
    Substitution bindings;
    
    bool hasMoreChoices() const;
    ClausePtr nextClause();
};
```

### Resolver Class

```cpp
class Resolver {
public:
    explicit Resolver(const Database& db, size_t max_depth = 1000);
    
    // Synchronous resolution
    std::vector<Solution> solve(const TermPtr& query);
    std::vector<Solution> solve(const TermList& goals);
    
    // Asynchronous resolution with callbacks
    void solveWithCallback(const TermPtr& query, 
                          std::function<bool(const Solution&)> callback);
    void solveWithCallback(const TermList& goals, 
                          std::function<bool(const Solution&)> callback);
};
```

### Usage Examples

```cpp
Database db;
db.loadProgram("parent(tom, bob). parent(bob, ann).");

Resolver resolver(db);

// Simple query
auto query = makeCompound("parent", {makeAtom("tom"), makeVariable("X")});
auto solutions = resolver.solve(query);

for (const auto& solution : solutions) {
    std::cout << solution.toString() << std::endl;
    // Output: X = bob
}

// Callback-based resolution
resolver.solveWithCallback(query, [](const Solution& solution) {
    std::cout << "Found: " << solution.toString() << std::endl;
    return true; // Continue searching
});
```

## Interpreter

### Interpreter Class

```cpp
class Interpreter {
public:
    explicit Interpreter(bool interactive = true);
    
    // Program loading
    void loadFile(const std::string& filename);
    void loadString(const std::string& program);
    
    // Query execution
    std::vector<Solution> query(const std::string& query_string);
    void queryInteractive(const std::string& query_string);
    
    // Interactive mode
    void run();
    
    // Database access
    Database& database();
    const Database& database() const;
    
    // Utility
    void showHelp() const;
    void showStatistics() const;
};
```

### Usage Examples

```cpp
// Programmatic usage
Interpreter interpreter(false); // Non-interactive

interpreter.loadString(R"(
    parent(tom, bob).
    parent(bob, ann).
    grandparent(X, Z) :- parent(X, Y), parent(Y, Z).
)");

auto solutions = interpreter.query("grandparent(tom, ann)");
// solutions contains one solution with empty bindings (true)

// Interactive mode
Interpreter interactive_interpreter(true);
interactive_interpreter.run(); // Starts REPL
```

## Built-in Predicates

### BuiltinPredicates Class

```cpp
class BuiltinPredicates {
public:
    using BuiltinHandler = std::function<bool(const TermList&, Substitution&, 
                                            std::function<bool(const Solution&)>)>;
    
    static void registerBuiltins();
    static bool isBuiltin(const std::string& functor, size_t arity);
    static bool callBuiltin(const std::string& functor, size_t arity, 
                           const TermList& args, Substitution& bindings,
                           std::function<bool(const Solution&)> callback);
};
```

### Available Built-ins

#### Arithmetic Operations
- `is/2` - Arithmetic evaluation with full expression support
  - Supports: `+`, `-`, `*`, `/`, `//` (integer division), `mod`, unary `-`, `abs`
  - Example: `X is 2 + 3 * 4` evaluates to `X = 14`
- `+/3`, `-/3`, `*/3`, `//3` - Direct arithmetic predicates
  - Example: `+(2, 3, X)` unifies `X = 5`

#### Comparison Operators
- `=/2` - Unification operator: `X = hello`
- `\\=/2` - Negation of unification: `X \\= Y`  
- `</2`, `>/2` - Arithmetic comparison: `5 < 10`
- `=</2`, `>=/2` - Arithmetic comparison: `X >= 0`
- `==/2`, `\\==/2` - Strict term equality/inequality: `X == Y` (structural comparison)
- Standard Prolog term ordering: Variables < Numbers < Atoms < Strings < Compounds < Lists

#### List Operations
- `append/3` - List concatenation with backtracking: `append([1,2], [3,4], L)`
- `member/2` - List membership testing with backtracking: `member(X, [1,2,3])`
- `length/2` - Bidirectional list length: `length([1,2,3], N)` or `length(L, 3)`

#### Type Checking Predicates
- `var/1` - Test if term is an unbound variable: `var(X)`  
- `nonvar/1` - Test if term is bound (not a variable): `nonvar(hello)`
- `atom/1` - Test if term is an atom: `atom(hello)`
- `number/1` - Test if term is a number (integer or float): `number(42)`
- `integer/1` - Test if term is specifically an integer: `integer(42)`
- `float/1` - Test if term is specifically a float: `float(3.14)`
- `compound/1` - Test if term is a compound structure: `compound(f(a,b))`
- `ground/1` - Test if term is fully instantiated (no variables): `ground(hello(world))`

#### Control Structures
- `true/0` - Always succeeds: `true`
- `fail/0` - Always fails (forces backtracking): `fail`
- `\\+/1` - Negation as failure (succeeds if goal fails): `\\+ member(x, [a,b,c])`
- `!/0` - Cut operator for preventing backtracking: `p(X) :- q(X), !, r(X)`

#### Input/Output
- `write/1` - Output a term to stdout: `write('Hello World')`
- `nl/0` - Output a newline character: `nl`

#### Advanced Features
- **First Argument Indexing**: Database optimization for faster clause lookup
- **Comprehensive arithmetic evaluation**: Supports complex expressions with proper precedence
- **Modern C++ implementation**: Uses STL algorithms like `std::lexicographical_compare`
- **Efficient comparison system**: Map-based comparator with lambda functions  
- **Ground term checking**: Recursive validation of fully instantiated terms
- **Efficient term comparison**: Standard Prolog term ordering implemented with modern C++

### Custom Built-ins

```cpp
// Define custom built-in predicate
bool myCustomPredicate(const TermList& args, Substitution& bindings,
                       std::function<bool(const Solution&)> callback) {
    // Implementation
    if (/* success condition */) {
        Solution solution{bindings};
        return callback(solution);
    }
    return false; // Failure
}

// Register during initialization  
BuiltinPredicates::registerBuiltins();
// Add custom registration here
```

## Utilities

### String Utils

```cpp
namespace prolog::utils {

class StringUtils {
public:
    static std::string trim(const std::string& str);
    static std::vector<std::string> split(const std::string& str, char delimiter);
    static std::string join(const std::vector<std::string>& strings, 
                           const std::string& separator);
    static bool startsWith(const std::string& str, const std::string& prefix);
    static bool endsWith(const std::string& str, const std::string& suffix);
    static std::string toLowerCase(const std::string& str);
    static std::string toUpperCase(const std::string& str);
    static std::string escape(const std::string& str);
    static std::string unescape(const std::string& str);
};

}
```

### Memory Pool

```cpp
namespace prolog::utils {

template<typename T>
class MemoryPool {
public:
    explicit MemoryPool(size_t chunk_size = 1024);
    ~MemoryPool();
    
    template<typename... Args>
    T* allocate(Args&&... args);
    
    void deallocate(T* ptr);
    void clear();
    
    size_t totalCapacity() const;
    size_t usedCount() const;
};

}
```

## Error Handling

### Exception Types

```cpp
class ParseException : public std::exception {
public:
    explicit ParseException(std::string message);
    const char* what() const noexcept override;
};

// Usage
try {
    auto term = parser.parseQuery("invalid syntax");
} catch (const ParseException& e) {
    std::cerr << "Parse error: " << e.what() << std::endl;
}
```

## Complete Example

```cpp
#include "prolog/interpreter.h"
#include <iostream>

int main() {
    try {
        // Create interpreter
        prolog::Interpreter interpreter(false);
        
        // Load Prolog program
        std::string program = R"(
            % Family relationships
            parent(tom, bob).
            parent(tom, liz).
            parent(bob, ann).
            parent(bob, pat).
            
            % Rules
            grandparent(X, Z) :- parent(X, Y), parent(Y, Z).
            sibling(X, Y) :- parent(Z, X), parent(Z, Y), X \= Y.
        )";
        
        interpreter.loadString(program);
        
        // Execute queries
        std::vector<std::string> queries = {
            "parent(tom, X)",           // Find tom's children
            "grandparent(tom, X)",      // Find tom's grandchildren  
            "sibling(bob, liz)",        // Are bob and liz siblings?
        };
        
        for (const auto& query_str : queries) {
            std::cout << "Query: " << query_str << std::endl;
            
            auto solutions = interpreter.query(query_str);
            
            if (solutions.empty()) {
                std::cout << "  Result: false" << std::endl;
            } else {
                for (size_t i = 0; i < solutions.size(); ++i) {
                    std::cout << "  Solution " << (i + 1) << ": " 
                              << solutions[i].toString() << std::endl;
                }
            }
            std::cout << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
```

This produces output like:
```
Query: parent(tom, X)
  Solution 1: X = bob
  Solution 2: X = liz

Query: grandparent(tom, X) 
  Solution 1: X = ann
  Solution 2: X = pat

Query: sibling(bob, liz)
  Solution 1: true
```