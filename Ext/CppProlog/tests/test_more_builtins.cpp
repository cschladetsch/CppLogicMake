#include <gtest/gtest.h>
#include "prolog/interpreter.h"
#include "prolog/term.h"
#include "prolog/solution.h"

using namespace prolog;

// Test fixture for interpreter-based tests
class BuiltinTest : public ::testing::Test {
protected:
    Interpreter interpreter;

    void SetUp() override {
        // No setup needed for now, as the discontiguous directive is not supported.
    }

    std::vector<Solution> run_query(const std::string& query) {
        return interpreter.query(query);
    }
};

TEST_F(BuiltinTest, IsOperatorHandlesComplexArithmetic) {
    auto solutions = run_query("X is (10 * 2 + 5) / 5 - 1.");
    ASSERT_EQ(solutions.size(), 1);
    auto binding = solutions[0].bindings.find("X");
    ASSERT_NE(binding, solutions[0].bindings.end());
    ASSERT_TRUE(binding->second->is<Float>());
    EXPECT_DOUBLE_EQ(binding->second->as<Float>()->value(), 4.0);
}

TEST_F(BuiltinTest, IsOperatorHandlesNegativeNumbers) {
    auto solutions = run_query("X is -5 + 3.");
    ASSERT_EQ(solutions.size(), 1);
    auto binding = solutions[0].bindings.find("X");
    ASSERT_NE(binding, solutions[0].bindings.end());
    ASSERT_TRUE(binding->second->is<Integer>());
    EXPECT_EQ(binding->second->as<Integer>()->value(), -2);
}

TEST_F(BuiltinTest, LengthPredicateWithBoundListAndVariableLength) {
    auto solutions = run_query("length([a, b, c, d], L).");
    ASSERT_EQ(solutions.size(), 1);
    auto binding = solutions[0].bindings.find("L");
    ASSERT_NE(binding, solutions[0].bindings.end());
    ASSERT_TRUE(binding->second->is<Integer>());
    EXPECT_EQ(binding->second->as<Integer>()->value(), 4);
}

TEST_F(BuiltinTest, LengthPredicateWithEmptyList) {
    auto solutions = run_query("length([], L).");
    ASSERT_EQ(solutions.size(), 1);
    auto binding = solutions[0].bindings.find("L");
    ASSERT_NE(binding, solutions[0].bindings.end());
    ASSERT_TRUE(binding->second->is<Integer>());
    EXPECT_EQ(binding->second->as<Integer>()->value(), 0);
}

TEST_F(BuiltinTest, LengthPredicateWithVariableListAndBoundLength) {
    auto solutions = run_query("length(L, 3).");
    ASSERT_EQ(solutions.size(), 1);
    auto binding = solutions[0].bindings.find("L");
    ASSERT_NE(binding, solutions[0].bindings.end());
    ASSERT_TRUE(binding->second->is<List>());
    auto list = binding->second->as<List>();
    EXPECT_EQ(list->elements().size(), 3);
    EXPECT_FALSE(list->hasProperTail()); // Should be a proper list of 3 unbound variables
}

TEST_F(BuiltinTest, TypeCheckingAtom) {
    auto solutions_true = run_query("atom(hello).");
    EXPECT_EQ(solutions_true.size(), 1);
    auto solutions_false = run_query("atom('Hello')."); // Single quotes are atoms
    EXPECT_EQ(solutions_false.size(), 1);
    auto solutions_fail = run_query("atom(X).");
    EXPECT_EQ(solutions_fail.size(), 0);
    auto solutions_fail2 = run_query("atom(123).");
    EXPECT_EQ(solutions_fail2.size(), 0);
}

TEST_F(BuiltinTest, TypeCheckingVar) {
    auto solutions_true = run_query("var(X).");
    EXPECT_EQ(solutions_true.size(), 1);
    auto solutions_fail = run_query("X = a, var(X).");
    EXPECT_EQ(solutions_fail.size(), 0);
}
