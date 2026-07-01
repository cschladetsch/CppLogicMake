#include <benchmark/benchmark.h>
#include "prolog/parser.h"

using namespace prolog;

// Benchmark lexing simple atoms
static void BM_LexAtoms(benchmark::State& state) {
    std::string input = "hello world test atom another";
    
    for (auto _ : state) {
        Lexer lexer(input);
        auto tokens = lexer.tokenize();
        benchmark::DoNotOptimize(tokens);
    }
}
BENCHMARK(BM_LexAtoms);

// Benchmark lexing complex terms
static void BM_LexComplexTerms(benchmark::State& state) {
    std::string input = "parent(tom, bob) :- father(tom, bob), male(tom).";
    
    for (auto _ : state) {
        Lexer lexer(input);
        auto tokens = lexer.tokenize();
        benchmark::DoNotOptimize(tokens);
    }
}
BENCHMARK(BM_LexComplexTerms);

// Benchmark lexing lists
static void BM_LexLists(benchmark::State& state) {
    std::string input = "[a, b, c, d, e, [nested, list], f, g]";
    
    for (auto _ : state) {
        Lexer lexer(input);
        auto tokens = lexer.tokenize();
        benchmark::DoNotOptimize(tokens);
    }
}
BENCHMARK(BM_LexLists);

// Benchmark lexing numbers and strings
static void BM_LexNumbersStrings(benchmark::State& state) {
    std::string input = R"(42 3.14159 "hello world" 100 2.71828 "another string")";
    
    for (auto _ : state) {
        Lexer lexer(input);
        auto tokens = lexer.tokenize();
        benchmark::DoNotOptimize(tokens);
    }
}
BENCHMARK(BM_LexNumbersStrings);

// Benchmark parsing simple facts
static void BM_ParseSimpleFact(benchmark::State& state) {
    std::string input = "parent(tom, bob).";
    
    for (auto _ : state) {
        Parser parser({});
        auto clauses = parser.parseProgram(input);
        benchmark::DoNotOptimize(clauses);
    }
}
BENCHMARK(BM_ParseSimpleFact);

// Benchmark parsing simple rules
static void BM_ParseSimpleRule(benchmark::State& state) {
    std::string input = "grandparent(X, Z) :- parent(X, Y), parent(Y, Z).";
    
    for (auto _ : state) {
        Parser parser({});
        auto clauses = parser.parseProgram(input);
        benchmark::DoNotOptimize(clauses);
    }
}
BENCHMARK(BM_ParseSimpleRule);

// Benchmark parsing complex terms
static void BM_ParseComplexTerm(benchmark::State& state) {
    std::string input = "complex_predicate(f(g(X, a), h(b, Y)), [1, 2, 3, Z], \"string\").";
    
    for (auto _ : state) {
        Parser parser({});
        auto clauses = parser.parseProgram(input);
        benchmark::DoNotOptimize(clauses);
    }
}
BENCHMARK(BM_ParseComplexTerm);

// Benchmark parsing lists with tail
static void BM_ParseListsWithTail(benchmark::State& state) {
    std::string input = "list_pred([a, b, c | Tail], [1, 2 | Rest]).";
    
    for (auto _ : state) {
        Parser parser({});
        auto clauses = parser.parseProgram(input);
        benchmark::DoNotOptimize(clauses);
    }
}
BENCHMARK(BM_ParseListsWithTail);

// Benchmark parsing multiple clauses
static void BM_ParseMultipleClauses(benchmark::State& state) {
    std::string input = R"(
        parent(tom, bob).
        parent(tom, liz).
        parent(bob, ann).
        parent(bob, pat).
        parent(pat, jim).
        grandparent(X, Z) :- parent(X, Y), parent(Y, Z).
        ancestor(X, Y) :- parent(X, Y).
        ancestor(X, Z) :- parent(X, Y), ancestor(Y, Z).
    )";
    
    for (auto _ : state) {
        Parser parser({});
        auto clauses = parser.parseProgram(input);
        benchmark::DoNotOptimize(clauses);
    }
}
BENCHMARK(BM_ParseMultipleClauses);

// Benchmark parsing queries
static void BM_ParseQuery(benchmark::State& state) {
    std::string input = "parent(X, Y), grandparent(Y, Z), ancestor(X, Z)";
    
    for (auto _ : state) {
        Parser parser({});
        auto term = parser.parseQuery(input);
        benchmark::DoNotOptimize(term);
    }
}
BENCHMARK(BM_ParseQuery);

// Benchmark parsing arithmetic expressions (simplified without operators)
static void BM_ParseArithmetic(benchmark::State& state) {
    std::string input = "calculate(X, Y, Z) :- result(X, Y, Z).";
    
    for (auto _ : state) {
        Parser parser({});
        auto clauses = parser.parseProgram(input);
        benchmark::DoNotOptimize(clauses);
    }
}
BENCHMARK(BM_ParseArithmetic);

// Benchmark parsing with varying complexity
static void BM_ParseVaryingComplexity(benchmark::State& state) {
    int complexity = state.range(0);
    
    // Generate increasingly complex terms
    std::string input = "pred(";
    for (int i = 0; i < complexity; ++i) {
        if (i > 0) input += ", ";
        input += "f" + std::to_string(i) + "(X" + std::to_string(i) + ")";
    }
    input += ").";
    
    for (auto _ : state) {
        Parser parser({});
        auto clauses = parser.parseProgram(input);
        benchmark::DoNotOptimize(clauses);
    }
}
BENCHMARK(BM_ParseVaryingComplexity)->Range(1, 100);

// Benchmark parsing deeply nested terms
static void BM_ParseDeeplyNested(benchmark::State& state) {
    int depth = state.range(0);
    
    // Generate deeply nested term: f(f(f(...(a)...)))
    std::string input = "pred(";
    for (int i = 0; i < depth; ++i) {
        input += "f(";
    }
    input += "a";
    for (int i = 0; i < depth; ++i) {
        input += ")";
    }
    input += ").";
    
    for (auto _ : state) {
        Parser parser({});
        auto clauses = parser.parseProgram(input);
        benchmark::DoNotOptimize(clauses);
    }
}
BENCHMARK(BM_ParseDeeplyNested)->Range(1, 1000);

// Benchmark parsing long lists
static void BM_ParseLongLists(benchmark::State& state) {
    int length = state.range(0);
    
    std::string input = "pred([";
    for (int i = 0; i < length; ++i) {
        if (i > 0) input += ", ";
        input += "elem" + std::to_string(i);
    }
    input += "]).";
    
    for (auto _ : state) {
        Parser parser({});
        auto clauses = parser.parseProgram(input);
        benchmark::DoNotOptimize(clauses);
    }
}
BENCHMARK(BM_ParseLongLists)->Range(1, 1000);