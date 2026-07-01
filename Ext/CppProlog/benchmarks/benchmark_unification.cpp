#include <benchmark/benchmark.h>
#include "prolog/term.h"
#include "prolog/unification.h"

using namespace prolog;

// Benchmark unification of atoms
static void BM_UnifyAtoms(benchmark::State& state) {
    auto atom1 = makeAtom("hello");
    auto atom2 = makeAtom("hello");
    
    for (auto _ : state) {
        auto result = Unification::unify(atom1, atom2);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_UnifyAtoms);

// Benchmark unification of variables
static void BM_UnifyVariables(benchmark::State& state) {
    auto var1 = makeVariable("X");
    auto var2 = makeVariable("Y");
    
    for (auto _ : state) {
        auto result = Unification::unify(var1, var2);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_UnifyVariables);

// Benchmark unification of variable with atom
static void BM_UnifyVariableAtom(benchmark::State& state) {
    auto var = makeVariable("X");
    auto atom = makeAtom("hello");
    
    for (auto _ : state) {
        auto result = Unification::unify(var, atom);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_UnifyVariableAtom);

// Benchmark unification of simple compound terms
static void BM_UnifySimpleCompound(benchmark::State& state) {
    auto comp1 = makeCompound("f", {makeAtom("a"), makeVariable("X")});
    auto comp2 = makeCompound("f", {makeAtom("a"), makeAtom("b")});
    
    for (auto _ : state) {
        auto result = Unification::unify(comp1, comp2);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_UnifySimpleCompound);

// Benchmark unification of complex compound terms
static void BM_UnifyComplexCompound(benchmark::State& state) {
    auto comp1 = makeCompound("complex", {
        makeCompound("f", {makeVariable("X"), makeAtom("a")}),
        makeCompound("g", {makeVariable("Y"), makeAtom("b")}),
        makeCompound("h", {makeVariable("Z"), makeAtom("c")})
    });
    
    auto comp2 = makeCompound("complex", {
        makeCompound("f", {makeAtom("1"), makeAtom("a")}),
        makeCompound("g", {makeAtom("2"), makeAtom("b")}),
        makeCompound("h", {makeAtom("3"), makeAtom("c")})
    });
    
    for (auto _ : state) {
        auto result = Unification::unify(comp1, comp2);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_UnifyComplexCompound);

// Benchmark unification of lists
static void BM_UnifyLists(benchmark::State& state) {
    auto list1 = makeList({makeVariable("X"), makeAtom("b"), makeVariable("Y")});
    auto list2 = makeList({makeAtom("a"), makeAtom("b"), makeAtom("c")});
    
    for (auto _ : state) {
        auto result = Unification::unify(list1, list2);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_UnifyLists);

// Benchmark unification failure cases
static void BM_UnifyFailure(benchmark::State& state) {
    auto atom1 = makeAtom("hello");
    auto atom2 = makeAtom("world");
    
    for (auto _ : state) {
        auto result = Unification::unify(atom1, atom2);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_UnifyFailure);

// Benchmark substitution application
static void BM_ApplySubstitution(benchmark::State& state) {
    Substitution subst;
    subst["X"] = makeAtom("hello");
    subst["Y"] = makeAtom("world");
    
    auto term = makeCompound("f", {
        makeVariable("X"), 
        makeCompound("g", {makeVariable("Y"), makeVariable("X")})
    });
    
    for (auto _ : state) {
        auto result = Unification::applySubstitution(term, subst);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_ApplySubstitution);

// Benchmark occurs check
static void BM_OccursCheck(benchmark::State& state) {
    auto complex_term = makeCompound("f", {
        makeCompound("g", {makeVariable("X"), makeAtom("a")}),
        makeCompound("h", {makeAtom("b"), makeVariable("Y")}),
        makeList({makeVariable("X"), makeAtom("c"), makeVariable("Z")})
    });
    
    for (auto _ : state) {
        bool result = Unification::occursCheck("X", complex_term);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_OccursCheck);

// Benchmark deep term unification
static void BM_DeepTermUnification(benchmark::State& state) {
    // Create deeply nested terms
    TermPtr deep1 = makeAtom("base");
    TermPtr deep2 = makeAtom("base");
    
    int depth = state.range(0);
    for (int i = 0; i < depth; ++i) {
        deep1 = makeCompound("f", {deep1});
        deep2 = makeCompound("f", {deep2});
    }
    
    for (auto _ : state) {
        auto result = Unification::unify(deep1, deep2);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_DeepTermUnification)->Range(1, 1000);

// Benchmark wide term unification (many arguments)
static void BM_WideTermUnification(benchmark::State& state) {
    int arity = state.range(0);
    
    TermList args1, args2;
    for (int i = 0; i < arity; ++i) {
        args1.push_back(makeVariable("X" + std::to_string(i)));
        args2.push_back(makeAtom("atom" + std::to_string(i)));
    }
    
    auto comp1 = makeCompound("wide", args1);
    auto comp2 = makeCompound("wide", args2);
    
    for (auto _ : state) {
        auto result = Unification::unify(comp1, comp2);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_WideTermUnification)->Range(1, 1000);