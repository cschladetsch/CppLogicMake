#include <gtest/gtest.h>
#include "prolog/interpreter.h"
#include <sstream>

using namespace prolog;

class InterpreterTest : public ::testing::Test {
protected:
    std::unique_ptr<Interpreter> interpreter;
    
    void SetUp() override {
        interpreter = std::make_unique<Interpreter>(false); // Non-interactive
    }
    
    void TearDown() override {
        interpreter.reset();
    }
};

TEST_F(InterpreterTest, LoadStringProgram) {
    std::string program = R"(
        parent(tom, bob).
        parent(bob, ann).
        parent(X, Y) :- father(X, Y).
        parent(X, Y) :- mother(X, Y).
    )";
    
    EXPECT_NO_THROW(interpreter->loadString(program));
    EXPECT_EQ(interpreter->database().size(), 4);
}

TEST_F(InterpreterTest, SimpleQuery) {
    interpreter->loadString("parent(tom, bob).");
    
    auto solutions = interpreter->query("parent(tom, bob)");
    ASSERT_EQ(solutions.size(), 1);
    EXPECT_TRUE(solutions[0].bindings.empty());
}

TEST_F(InterpreterTest, QueryWithVariable) {
    interpreter->loadString(R"(
        parent(tom, bob).
        parent(tom, liz).
    )");
    
    auto solutions = interpreter->query("parent(tom, X)");
    EXPECT_EQ(solutions.size(), 2);
    
    for (const auto& solution : solutions) {
        EXPECT_EQ(solution.bindings.size(), 1);
        EXPECT_TRUE(solution.bindings.find("X") != solution.bindings.end());
    }
}

TEST_F(InterpreterTest, QueryWithRule) {
    interpreter->loadString(R"(
        parent(tom, bob).
        parent(bob, ann).
        grandparent(X, Z) :- parent(X, Y), parent(Y, Z).
    )");
    
    auto solutions = interpreter->query("grandparent(tom, ann)");
    ASSERT_EQ(solutions.size(), 1);
    EXPECT_TRUE(solutions[0].bindings.empty());
}

TEST_F(InterpreterTest, QueryNoSolution) {
    interpreter->loadString("parent(tom, bob).");
    
    auto solutions = interpreter->query("parent(bob, tom)");
    EXPECT_EQ(solutions.size(), 0);
}

TEST_F(InterpreterTest, ComplexQuery) {
    interpreter->loadString(R"(
        parent(tom, bob).
        parent(tom, liz).
        parent(bob, ann).
        parent(bob, pat).
        parent(pat, jim).
        
        grandparent(X, Z) :- parent(X, Y), parent(Y, Z).
        ancestor(X, Y) :- parent(X, Y).
        ancestor(X, Z) :- parent(X, Y), ancestor(Y, Z).
    )");
    
    auto solutions = interpreter->query("ancestor(tom, jim)");
    EXPECT_GT(solutions.size(), 0);
}

TEST_F(InterpreterTest, InvalidQuery) {
    interpreter->loadString("parent(tom, bob).");
    
    EXPECT_THROW(interpreter->query("parent(tom"), std::exception);
}

TEST_F(InterpreterTest, MultipleQueries) {
    interpreter->loadString(R"(
        likes(mary, food).
        likes(mary, wine).
        likes(john, wine).
        likes(john, mary).
    )");
    
    auto solutions1 = interpreter->query("likes(mary, X)");
    EXPECT_EQ(solutions1.size(), 2);
    
    auto solutions2 = interpreter->query("likes(X, wine)");
    EXPECT_EQ(solutions2.size(), 2);
    
    auto solutions3 = interpreter->query("likes(X, Y)");
    EXPECT_EQ(solutions3.size(), 4);
}

TEST_F(InterpreterTest, ListQuery) {
    interpreter->loadString(R"(
        member(X, [X|_]).
        member(X, [_|T]) :- member(X, T).
        
        list_example([a, b, c]).
    )");
    
    auto solutions = interpreter->query("list_example(L)");
    ASSERT_EQ(solutions.size(), 1);
    
    auto list_term = solutions[0].bindings["L"];
    EXPECT_TRUE(list_term->is<List>());
}

TEST_F(InterpreterTest, DatabaseStatistics) {
    interpreter->loadString(R"(
        fact1.
        fact2.
        rule1 :- fact1.
    )");
    
    EXPECT_EQ(interpreter->database().size(), 3);
    EXPECT_FALSE(interpreter->database().empty());
}