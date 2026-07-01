#include <benchmark/benchmark.h>
#include "prolog/interpreter.h"
#include "prolog/database.h"
#include "prolog/resolver.h"

using namespace prolog;

// Benchmark simple fact resolution
static void BM_ResolveFact(benchmark::State& state) {
    Database db;
    db.loadProgram("parent(tom, bob).");
    
    Resolver resolver(db);
    auto query = makeCompound("parent", {makeAtom("tom"), makeAtom("bob")});
    
    for (auto _ : state) {
        auto solutions = resolver.solve(query);
        benchmark::DoNotOptimize(solutions);
    }
}
BENCHMARK(BM_ResolveFact);

// Benchmark fact resolution with variables
static void BM_ResolveFactWithVariable(benchmark::State& state) {
    Database db;
    db.loadProgram(R"(
        parent(tom, bob).
        parent(tom, liz).
        parent(bob, ann).
    )");
    
    Resolver resolver(db);
    auto query = makeCompound("parent", {makeAtom("tom"), makeVariable("X")});
    
    for (auto _ : state) {
        auto solutions = resolver.solve(query);
        benchmark::DoNotOptimize(solutions);
    }
}
BENCHMARK(BM_ResolveFactWithVariable);

// Benchmark simple rule resolution
static void BM_ResolveSimpleRule(benchmark::State& state) {
    Database db;
    db.loadProgram(R"(
        parent(tom, bob).
        parent(bob, ann).
        grandparent(X, Z) :- parent(X, Y), parent(Y, Z).
    )");
    
    Resolver resolver(db);
    auto query = makeCompound("grandparent", {makeAtom("tom"), makeVariable("Z")});
    
    for (auto _ : state) {
        auto solutions = resolver.solve(query);
        benchmark::DoNotOptimize(solutions);
    }
}
BENCHMARK(BM_ResolveSimpleRule);

// Benchmark recursive rule resolution
static void BM_ResolveRecursiveRule(benchmark::State& state) {
    Database db;
    db.loadProgram(R"(
        parent(a, b).
        parent(b, c).
        parent(c, d).
        parent(d, e).
        ancestor(X, Y) :- parent(X, Y).
        ancestor(X, Z) :- parent(X, Y), ancestor(Y, Z).
    )");
    
    Resolver resolver(db);
    auto query = makeCompound("ancestor", {makeAtom("a"), makeVariable("Z")});
    
    for (auto _ : state) {
        auto solutions = resolver.solve(query);
        benchmark::DoNotOptimize(solutions);
    }
}
BENCHMARK(BM_ResolveRecursiveRule);

// Benchmark list processing
static void BM_ResolveListProcessing(benchmark::State& state) {
    Database db;
    db.loadProgram(R"(
        append([], L, L).
        append([H|T], L, [H|R]) :- append(T, L, R).
    )");
    
    Resolver resolver(db);
    auto query = makeCompound("append", {
        makeList({makeAtom("a"), makeAtom("b")}),
        makeList({makeAtom("c"), makeAtom("d")}),
        makeVariable("Result")
    });
    
    for (auto _ : state) {
        auto solutions = resolver.solve(query);
        benchmark::DoNotOptimize(solutions);
    }
}
BENCHMARK(BM_ResolveListProcessing);

// Benchmark simple rule resolution without arithmetic
static void BM_ResolveFactorial(benchmark::State& state) {
    Database db;
    db.loadProgram(R"(
        factorial(0, 1).
        factorial(1, 1).
        factorial(2, 2).
        factorial(3, 6).
    )");
    
    Resolver resolver(db);
    auto query = makeCompound("factorial", {makeInteger(3), makeVariable("F")});
    
    for (auto _ : state) {
        auto solutions = resolver.solve(query);
        benchmark::DoNotOptimize(solutions);
    }
}
BENCHMARK(BM_ResolveFactorial);

// Benchmark fibonacci computation with simple facts
static void BM_ResolveFibonacci(benchmark::State& state) {
    Database db;
    db.loadProgram(R"(
        fibonacci(0, 0).
        fibonacci(1, 1).
        fibonacci(2, 1).
        fibonacci(3, 2).
        fibonacci(4, 3).
        fibonacci(5, 5).
    )");
    
    Resolver resolver(db);
    int n = state.range(0);
    auto query = makeCompound("fibonacci", {makeInteger(n), makeVariable("F")});
    
    for (auto _ : state) {
        auto solutions = resolver.solve(query);
        benchmark::DoNotOptimize(solutions);
    }
}
BENCHMARK(BM_ResolveFibonacci)->Range(1, 20);

// Benchmark family tree queries
static void BM_ResolveFamilyTree(benchmark::State& state) {
    Database db;
    db.loadProgram(R"(
        parent(tom, bob).
        parent(tom, liz).
        parent(bob, ann).
        parent(bob, pat).
        parent(pat, jim).
        parent(liz, sue).
        
        male(tom). male(bob). male(jim).
        female(liz). female(ann). female(pat). female(sue).
        
        father(X, Y) :- parent(X, Y), male(X).
        mother(X, Y) :- parent(X, Y), female(X).
        grandparent(X, Z) :- parent(X, Y), parent(Y, Z).
        sibling(X, Y) :- parent(Z, X), parent(Z, Y).
        uncle(X, Y) :- sibling(X, Z), parent(Z, Y), male(X).
        aunt(X, Y) :- sibling(X, Z), parent(Z, Y), female(X).
    )");
    
    Resolver resolver(db);
    auto query = makeCompound("uncle", {makeVariable("X"), makeVariable("Y")});
    
    for (auto _ : state) {
        auto solutions = resolver.solve(query);
        benchmark::DoNotOptimize(solutions);
    }
}
BENCHMARK(BM_ResolveFamilyTree);

// Benchmark backtracking intensive query
static void BM_ResolveBacktrackingIntensive(benchmark::State& state) {
    Database db;
    
    // Generate many facts to force backtracking
    std::string program;
    int num_facts = state.range(0);
    
    for (int i = 0; i < num_facts; ++i) {
        program += "fact(" + std::to_string(i) + ").\n";
    }
    program += "test(X) :- fact(X).\n";
    
    db.loadProgram(program);
    
    Resolver resolver(db);
    auto query = makeCompound("test", {makeVariable("X")});
    
    for (auto _ : state) {
        auto solutions = resolver.solve(query);
        benchmark::DoNotOptimize(solutions);
    }
}
BENCHMARK(BM_ResolveBacktrackingIntensive)->Range(10, 1000);

// Benchmark deep recursion
static void BM_ResolveDeepRecursion(benchmark::State& state) {
    Database db;
    
    // Create a chain: chain(1, 2). chain(2, 3). ... chain(n-1, n).
    std::string program;
    int chain_length = state.range(0);
    
    for (int i = 1; i < chain_length; ++i) {
        program += "chain(" + std::to_string(i) + ", " + std::to_string(i + 1) + ").\n";
    }
    program += R"(
        path(X, Y) :- chain(X, Y).
        path(X, Z) :- chain(X, Y), path(Y, Z).
    )";
    
    db.loadProgram(program);
    
    Resolver resolver(db);
    auto query = makeCompound("path", {makeInteger(1), makeInteger(chain_length)});
    
    for (auto _ : state) {
        auto solutions = resolver.solve(query);
        benchmark::DoNotOptimize(solutions);
    }
}
BENCHMARK(BM_ResolveDeepRecursion)->Range(10, 100);

// Benchmark interpreter overhead
static void BM_InterpreterQuery(benchmark::State& state) {
    Interpreter interpreter(false);
    interpreter.loadString(R"(
        parent(tom, bob).
        parent(bob, ann).
        grandparent(X, Z) :- parent(X, Y), parent(Y, Z).
    )");
    
    for (auto _ : state) {
        auto solutions = interpreter.query("grandparent(tom, ann)");
        benchmark::DoNotOptimize(solutions);
    }
}
BENCHMARK(BM_InterpreterQuery);

// Benchmark multiple goals
static void BM_ResolveMultipleGoals(benchmark::State& state) {
    Database db;
    db.loadProgram(R"(
        likes(mary, food).
        likes(mary, wine).
        likes(john, wine).
        likes(john, mary).
        
        happy(X) :- likes(X, wine).
        friends(X, Y) :- likes(X, Z), likes(Y, Z).
    )");
    
    Resolver resolver(db);
    
    // Query with multiple goals: happy(X), friends(X, Y)
    TermList goals = {
        makeCompound("happy", {makeVariable("X")}),
        makeCompound("friends", {makeVariable("X"), makeVariable("Y")})
    };
    
    for (auto _ : state) {
        auto solutions = resolver.solve(goals);
        benchmark::DoNotOptimize(solutions);
    }
}
BENCHMARK(BM_ResolveMultipleGoals);