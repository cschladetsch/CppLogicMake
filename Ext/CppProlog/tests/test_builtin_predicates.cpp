#include <gtest/gtest.h>
#include "prolog/builtin_predicates.h"

using namespace prolog;

class BuiltinPredicatesTest : public ::testing::Test {
protected:
    void SetUp() override {
        BuiltinPredicates::registerBuiltins();
    }
    
    void TearDown() override {}
};

TEST_F(BuiltinPredicatesTest, IsBuiltinCheck) {
    EXPECT_TRUE(BuiltinPredicates::isBuiltin("is", 2));
    EXPECT_TRUE(BuiltinPredicates::isBuiltin("=", 2));
    EXPECT_TRUE(BuiltinPredicates::isBuiltin("var", 1));
    EXPECT_TRUE(BuiltinPredicates::isBuiltin("true", 0));
    
    EXPECT_FALSE(BuiltinPredicates::isBuiltin("unknown", 1));
    EXPECT_FALSE(BuiltinPredicates::isBuiltin("is", 3)); // Wrong arity
}

TEST_F(BuiltinPredicatesTest, ArithmeticIs) {
    TermList args = {makeVariable("X"), makeInteger(42)};
    Substitution bindings;
    
    std::vector<Solution> solutions;
    auto callback = [&solutions](const Solution& solution) {
        solutions.push_back(solution);
        return true;
    };
    
    bool result = BuiltinPredicates::callBuiltin("is", 2, args, bindings, callback);
    
    EXPECT_TRUE(result);
    ASSERT_EQ(solutions.size(), 1);
    EXPECT_EQ(solutions[0].bindings.size(), 1);
    
    auto x_binding = solutions[0].bindings.find("X");
    ASSERT_TRUE(x_binding != solutions[0].bindings.end());
    EXPECT_TRUE(x_binding->second->is<Integer>());
    EXPECT_EQ(x_binding->second->as<Integer>()->value(), 42);
}

TEST_F(BuiltinPredicatesTest, ArithmeticAdd) {
    TermList args = {makeInteger(2), makeInteger(3), makeVariable("Result")};
    Substitution bindings;
    
    std::vector<Solution> solutions;
    auto callback = [&solutions](const Solution& solution) {
        solutions.push_back(solution);
        return true;
    };
    
    bool result = BuiltinPredicates::callBuiltin("+", 3, args, bindings, callback);
    
    EXPECT_TRUE(result);
    ASSERT_EQ(solutions.size(), 1);
    
    auto result_binding = solutions[0].bindings.find("Result");
    ASSERT_TRUE(result_binding != solutions[0].bindings.end());
    EXPECT_TRUE(result_binding->second->is<Integer>());
    EXPECT_EQ(result_binding->second->as<Integer>()->value(), 5);
}

TEST_F(BuiltinPredicatesTest, Unification) {
    TermList args = {makeVariable("X"), makeAtom("hello")};
    Substitution bindings;
    
    std::vector<Solution> solutions;
    auto callback = [&solutions](const Solution& solution) {
        solutions.push_back(solution);
        return true;
    };
    
    bool result = BuiltinPredicates::callBuiltin("=", 2, args, bindings, callback);
    
    EXPECT_TRUE(result);
    ASSERT_EQ(solutions.size(), 1);
    
    auto x_binding = solutions[0].bindings.find("X");
    ASSERT_TRUE(x_binding != solutions[0].bindings.end());
    EXPECT_TRUE(x_binding->second->is<Atom>());
    EXPECT_EQ(x_binding->second->as<Atom>()->name(), "hello");
}

TEST_F(BuiltinPredicatesTest, UnificationFails) {
    TermList args = {makeAtom("hello"), makeAtom("world")};
    Substitution bindings;
    
    std::vector<Solution> solutions;
    auto callback = [&solutions](const Solution& solution) {
        solutions.push_back(solution);
        return true;
    };
    
    bool result = BuiltinPredicates::callBuiltin("=", 2, args, bindings, callback);
    
    EXPECT_FALSE(result);
    EXPECT_EQ(solutions.size(), 0);
}

TEST_F(BuiltinPredicatesTest, VariableCheck) {
    TermList args = {makeVariable("X")};
    Substitution bindings;
    
    std::vector<Solution> solutions;
    auto callback = [&solutions](const Solution& solution) {
        solutions.push_back(solution);
        return true;
    };
    
    bool result = BuiltinPredicates::callBuiltin("var", 1, args, bindings, callback);
    
    EXPECT_TRUE(result);
    ASSERT_EQ(solutions.size(), 1);
}

TEST_F(BuiltinPredicatesTest, VariableCheckFails) {
    TermList args = {makeAtom("hello")};
    Substitution bindings;
    
    std::vector<Solution> solutions;
    auto callback = [&solutions](const Solution& solution) {
        solutions.push_back(solution);
        return true;
    };
    
    bool result = BuiltinPredicates::callBuiltin("var", 1, args, bindings, callback);
    
    EXPECT_FALSE(result);
    EXPECT_EQ(solutions.size(), 0);
}

TEST_F(BuiltinPredicatesTest, TruePredicate) {
    TermList args = {};
    Substitution bindings;
    
    std::vector<Solution> solutions;
    auto callback = [&solutions](const Solution& solution) {
        solutions.push_back(solution);
        return true;
    };
    
    bool result = BuiltinPredicates::callBuiltin("true", 0, args, bindings, callback);
    
    EXPECT_TRUE(result);
    ASSERT_EQ(solutions.size(), 1);
    EXPECT_TRUE(solutions[0].bindings.empty());
}

TEST_F(BuiltinPredicatesTest, FailPredicate) {
    TermList args = {};
    Substitution bindings;
    
    std::vector<Solution> solutions;
    auto callback = [&solutions](const Solution& solution) {
        solutions.push_back(solution);
        return true;
    };
    
    bool result = BuiltinPredicates::callBuiltin("fail", 0, args, bindings, callback);
    
    EXPECT_FALSE(result);
    EXPECT_EQ(solutions.size(), 0);
}

TEST_F(BuiltinPredicatesTest, ListAppend) {
    auto list1 = makeList({makeAtom("a"), makeAtom("b")});
    auto list2 = makeList({makeAtom("c")});
    TermList args = {list1, list2, makeVariable("Result")};
    Substitution bindings;
    
    std::vector<Solution> solutions;
    auto callback = [&solutions](const Solution& solution) {
        solutions.push_back(solution);
        return true;
    };
    
    bool result = BuiltinPredicates::callBuiltin("append", 3, args, bindings, callback);
    
    EXPECT_TRUE(result);
    ASSERT_EQ(solutions.size(), 1);
    
    auto result_binding = solutions[0].bindings.find("Result");
    ASSERT_TRUE(result_binding != solutions[0].bindings.end());
    EXPECT_TRUE(result_binding->second->is<List>());
    
    auto result_list = result_binding->second->as<List>();
    EXPECT_EQ(result_list->elements().size(), 3);
}

TEST_F(BuiltinPredicatesTest, ListMember) {
    auto list = makeList({makeAtom("a"), makeAtom("b"), makeAtom("c")});
    TermList args = {makeVariable("X"), list};
    Substitution bindings;
    
    std::vector<Solution> solutions;
    auto callback = [&solutions](const Solution& solution) {
        solutions.push_back(solution);
        return true;
    };
    
    bool result = BuiltinPredicates::callBuiltin("member", 2, args, bindings, callback);
    
    EXPECT_TRUE(result);
    EXPECT_EQ(solutions.size(), 3); // Should find all three elements
}