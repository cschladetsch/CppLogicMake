#include <gtest/gtest.h>
#include "prolog/interpreter.h"
#include "prolog/database.h"
#include "prolog/builtin_predicates.h"
#include "prolog/parser.h"
#include "prolog/term.h"

using namespace prolog;

class ComprehensiveFeaturesTest : public ::testing::Test {
protected:
    void SetUp() override {
        interpreter = std::make_unique<Interpreter>(false);
        db = std::make_unique<Database>();
        BuiltinPredicates::registerBuiltins();
    }

    std::unique_ptr<Interpreter> interpreter;
    std::unique_ptr<Database> db;
};

// Tests 1-10: Advanced length predicate scenarios
TEST_F(ComprehensiveFeaturesTest, LengthWithVariablesInList) {
    interpreter->loadString("test.");
    auto solutions = interpreter->query("length([X, Y, Z], N)");
    ASSERT_EQ(solutions.size(), 1);
    auto n_binding = solutions[0].bindings.find("N");
    ASSERT_NE(n_binding, solutions[0].bindings.end());
    EXPECT_EQ(n_binding->second->as<Integer>()->value(), 3);
}

TEST_F(ComprehensiveFeaturesTest, LengthWithMixedTerms) {
    interpreter->loadString("test.");
    auto solutions = interpreter->query("length([atom, 42, f(x), [nested]], N)");
    ASSERT_EQ(solutions.size(), 1);
    auto n_binding = solutions[0].bindings.find("N");
    ASSERT_NE(n_binding, solutions[0].bindings.end());
    EXPECT_EQ(n_binding->second->as<Integer>()->value(), 4);
}

TEST_F(ComprehensiveFeaturesTest, LengthGenerateLargeList) {
    interpreter->loadString("test.");
    auto solutions = interpreter->query("length(L, 10)");
    ASSERT_EQ(solutions.size(), 1);
    auto l_binding = solutions[0].bindings.find("L");
    ASSERT_NE(l_binding, solutions[0].bindings.end());
    ASSERT_TRUE(l_binding->second->is<List>());
    EXPECT_EQ(l_binding->second->as<List>()->elements().size(), 10);
}

TEST_F(ComprehensiveFeaturesTest, LengthBoundaryCase) {
    interpreter->loadString("test.");
    auto solutions = interpreter->query("length([], 0)");
    EXPECT_EQ(solutions.size(), 1);
}

TEST_F(ComprehensiveFeaturesTest, LengthWithStringList) {
    interpreter->loadString("test.");
    auto solutions = interpreter->query("length([hello, world, test], N)");
    ASSERT_EQ(solutions.size(), 1);
    auto n_binding = solutions[0].bindings.find("N");
    EXPECT_EQ(n_binding->second->as<Integer>()->value(), 3);
}

// Tests 6-15: Database indexing comprehensive tests  
TEST_F(ComprehensiveFeaturesTest, DatabaseMultiplePredicatesIndexing) {
    db->addFact(makeCompound("likes", {makeAtom("mary"), makeAtom("wine")}));
    db->addFact(makeCompound("likes", {makeAtom("john"), makeAtom("beer")}));
    db->addFact(makeCompound("hates", {makeAtom("mary"), makeAtom("vegetables")}));
    db->addFact(makeCompound("likes", {makeAtom("mary"), makeAtom("chocolate")}));
    
    auto like_clauses = db->findClauses("likes", 2);
    EXPECT_EQ(like_clauses.size(), 3);
    
    auto hate_clauses = db->findClauses("hates", 2);
    EXPECT_EQ(hate_clauses.size(), 1);
}

TEST_F(ComprehensiveFeaturesTest, DatabaseFirstArgIndexingMixed) {
    db->addFact(makeCompound("data", {makeInteger(1), makeAtom("first")}));
    db->addFact(makeCompound("data", {makeInteger(2), makeAtom("second")}));
    db->addFact(makeCompound("data", {makeAtom("symbol"), makeAtom("third")}));
    db->addFact(makeCompound("data", {makeInteger(1), makeAtom("duplicate")}));
    
    auto int_clauses = db->findClausesWithFirstArg("data", 2, makeInteger(1));
    EXPECT_EQ(int_clauses.size(), 2);
    
    auto atom_clauses = db->findClausesWithFirstArg("data", 2, makeAtom("symbol"));
    EXPECT_EQ(atom_clauses.size(), 1);
}

TEST_F(ComprehensiveFeaturesTest, DatabaseLargeDatasetIndexing) {
    // Add many facts to test indexing performance
    for (int i = 0; i < 100; ++i) {
        db->addFact(makeCompound("number", {makeInteger(i), 
                                           makeAtom("value_" + std::to_string(i))}));
    }
    
    auto clauses_50 = db->findClausesWithFirstArg("number", 2, makeInteger(50));
    EXPECT_EQ(clauses_50.size(), 1);
    
    auto clauses_99 = db->findClausesWithFirstArg("number", 2, makeInteger(99));
    EXPECT_EQ(clauses_99.size(), 1);
    
    auto clauses_200 = db->findClausesWithFirstArg("number", 2, makeInteger(200));
    EXPECT_EQ(clauses_200.size(), 0);
}

TEST_F(ComprehensiveFeaturesTest, DatabaseIndexingPerformanceComparison) {
    // Add facts with various first arguments
    for (int i = 0; i < 50; ++i) {
        db->addFact(makeCompound("test", {makeAtom("a"), makeInteger(i)}));
        db->addFact(makeCompound("test", {makeAtom("b"), makeInteger(i)}));
        db->addFact(makeCompound("test", {makeAtom("c"), makeInteger(i)}));
    }
    
    // Indexed search should find exact matches
    auto a_clauses = db->findClausesWithFirstArg("test", 2, makeAtom("a"));
    EXPECT_EQ(a_clauses.size(), 50);
    
    auto b_clauses = db->findClausesWithFirstArg("test", 2, makeAtom("b"));  
    EXPECT_EQ(b_clauses.size(), 50);
    
    // Non-indexed search (all clauses of that functor/arity)
    auto all_clauses = db->findClauses("test", 2);
    EXPECT_EQ(all_clauses.size(), 150);
}

TEST_F(ComprehensiveFeaturesTest, DatabaseIndexingAfterClear) {
    db->addFact(makeCompound("temp", {makeAtom("data")}));
    auto before_clear = db->findClauses("temp", 1);
    EXPECT_EQ(before_clear.size(), 1);
    
    db->clear();
    auto after_clear = db->findClauses("temp", 1);
    EXPECT_EQ(after_clear.size(), 0);
    
    auto indexed_after_clear = db->findClausesWithFirstArg("temp", 1, makeAtom("data"));
    EXPECT_EQ(indexed_after_clear.size(), 0);
}

// Tests 16-25: Parser and term handling
TEST_F(ComprehensiveFeaturesTest, ParserComplexTerms) {
    std::string program = "complex_term(f(g(h(x)), [a, b, c], nested(term(here)))).";
    interpreter->loadString(program);
    
    auto solutions = interpreter->query("complex_term(X)");
    EXPECT_EQ(solutions.size(), 1);
}

TEST_F(ComprehensiveFeaturesTest, ParserNestedLists) {
    std::string program = "nested_list([[1, 2], [3, [4, 5]], []]).";
    interpreter->loadString(program);
    
    auto solutions = interpreter->query("nested_list(X)");
    EXPECT_EQ(solutions.size(), 1);
}

TEST_F(ComprehensiveFeaturesTest, ParserFloatingPointNumbers) {
    std::string program = "pi(3.14159). e(2.71828).";
    interpreter->loadString(program);
    
    auto solutions = interpreter->query("pi(X)");
    EXPECT_EQ(solutions.size(), 1);
    
    solutions = interpreter->query("e(Y)");
    EXPECT_EQ(solutions.size(), 1);
}

TEST_F(ComprehensiveFeaturesTest, ParserStringLiterals) {
    std::string program = R"(message("Hello, World!"). greeting("Hi there!").)";;
    interpreter->loadString(program);
    
    auto solutions = interpreter->query("message(X)");
    EXPECT_EQ(solutions.size(), 1);
}

TEST_F(ComprehensiveFeaturesTest, ParserVariableNaming) {
    std::string program = "test_vars(X, Y, Z).";
    interpreter->loadString(program);
    
    auto solutions = interpreter->query("test_vars(a, b, c)");
    EXPECT_EQ(solutions.size(), 1);
}

// Tests 26-35: Built-in predicate integration
TEST_F(ComprehensiveFeaturesTest, ArithmeticIsWithLength) {
    interpreter->loadString("list_data([a, b, c, d, e]).");
    auto solutions = interpreter->query("list_data(L), length(L, N), N is 5");
    EXPECT_EQ(solutions.size(), 1);
}

TEST_F(ComprehensiveFeaturesTest, MemberAndLengthCombination) {
    interpreter->loadString("data([1, 2, 3, 4]).");
    auto solutions = interpreter->query("data(L), length(L, 4)");
    EXPECT_EQ(solutions.size(), 1); // should find the list with length 4
}

TEST_F(ComprehensiveFeaturesTest, AppendAndLength) {
    interpreter->loadString("test.");
    auto solutions = interpreter->query("append([a, b], [c, d], L), length(L, N)");
    ASSERT_EQ(solutions.size(), 1);
    auto n_binding = solutions[0].bindings.find("N");
    EXPECT_EQ(n_binding->second->as<Integer>()->value(), 4);
}

TEST_F(ComprehensiveFeaturesTest, TypeCheckingPredicates) {
    interpreter->loadString("test_data(atom, 42, 3.14, [list], f(compound)).");
    
    auto solutions = interpreter->query("test_data(A, I, F, L, C), atom(A)");
    EXPECT_EQ(solutions.size(), 1);
    
    solutions = interpreter->query("test_data(A, I, F, L, C), integer(I)");
    EXPECT_EQ(solutions.size(), 1);
    
    solutions = interpreter->query("test_data(A, I, F, L, C), compound(C)");
    EXPECT_EQ(solutions.size(), 1);
}

TEST_F(ComprehensiveFeaturesTest, VariableInstantiationChecks) {
    interpreter->loadString("check_var(X) :- var(X). check_nonvar(X) :- nonvar(X).");
    
    auto solutions = interpreter->query("check_nonvar(hello)");
    EXPECT_EQ(solutions.size(), 1);
    
    solutions = interpreter->query("X = hello, check_nonvar(X)");
    EXPECT_EQ(solutions.size(), 1);
}

// Tests 36-40: Complex query scenarios
TEST_F(ComprehensiveFeaturesTest, MultiLevelUnification) {
    interpreter->loadString(R"(
        person(john, 25, engineer).
        person(mary, 30, doctor).
        person(bob, 22, student).
        older(X, Y) :- person(X, AgeX, _), person(Y, AgeY, _), AgeX > AgeY.
    )");
    
    auto solutions = interpreter->query("older(mary, john)");
    EXPECT_EQ(solutions.size(), 1);
}

TEST_F(ComprehensiveFeaturesTest, RecursiveDataStructures) {
    interpreter->loadString(R"(
        tree(leaf(Value), Value).
        tree(node(Left, Right), Value) :- 
            tree(Left, Value).
        tree(node(Left, Right), Value) :- 
            tree(Right, Value).
    )");
    
    auto solutions = interpreter->query("tree(leaf(42), X)");
    EXPECT_EQ(solutions.size(), 1);
}

TEST_F(ComprehensiveFeaturesTest, ListProcessingChain) {
    interpreter->loadString("test.");
    auto solutions = interpreter->query("append([1, 2], [3, 4], L1), append(L1, [5], L2), length(L2, N)");
    ASSERT_EQ(solutions.size(), 1);
    auto n_binding = solutions[0].bindings.find("N");
    EXPECT_EQ(n_binding->second->as<Integer>()->value(), 5);
}

TEST_F(ComprehensiveFeaturesTest, ComplexTermMatching) {
    interpreter->loadString(R"(
        data_structure(record(name(john), age(25), skills([prolog, cpp, python]))).
        extract_name(record(name(N), _, _), N).
    )");
    
    auto solutions = interpreter->query("data_structure(D), extract_name(D, Name)");
    EXPECT_EQ(solutions.size(), 1);
}

TEST_F(ComprehensiveFeaturesTest, DatabaseIntegrityAfterOperations) {
    // Test that database remains consistent after various operations
    interpreter->loadString(R"(
        fact1(a).
        fact2(b).
        rule1(X) :- fact1(X).
    )");
    
    auto initial_solutions = interpreter->query("fact1(X)");
    EXPECT_EQ(initial_solutions.size(), 1);
    
    auto rule_solutions = interpreter->query("rule1(Y)"); 
    EXPECT_EQ(rule_solutions.size(), 1);
    
    // Verify queries still work after multiple operations
    auto combined_solutions = interpreter->query("fact1(X), fact2(Y)");
    EXPECT_EQ(combined_solutions.size(), 1);
}