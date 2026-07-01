#include <gtest/gtest.h>
#include "prolog/resolver.h"
#include "prolog/database.h"

using namespace prolog;

class ResolverTest : public ::testing::Test {
protected:
    Database db;
    std::unique_ptr<Resolver> resolver;
    
    void SetUp() override {
        db.clear();
        resolver = std::make_unique<Resolver>(db);
        
        // Setup basic family facts
        db.addFact(makeCompound("parent", {makeAtom("tom"), makeAtom("bob")}));
        db.addFact(makeCompound("parent", {makeAtom("tom"), makeAtom("liz")}));
        db.addFact(makeCompound("parent", {makeAtom("bob"), makeAtom("ann")}));
        db.addFact(makeCompound("parent", {makeAtom("bob"), makeAtom("pat")}));
        db.addFact(makeCompound("parent", {makeAtom("pat"), makeAtom("jim")}));
        
        // Add grandparent rule
        auto grandparent_head = makeCompound("grandparent", {makeVariable("X"), makeVariable("Z")});
        TermList grandparent_body = {
            makeCompound("parent", {makeVariable("X"), makeVariable("Y")}),
            makeCompound("parent", {makeVariable("Y"), makeVariable("Z")})
        };
        db.addRule(grandparent_head, grandparent_body);
    }
    
    void TearDown() override {
        db.clear();
    }
};

TEST_F(ResolverTest, SimpleFactQuery) {
    auto query = makeCompound("parent", {makeAtom("tom"), makeAtom("bob")});
    auto solutions = resolver->solve(query);
    
    ASSERT_EQ(solutions.size(), 1);
    EXPECT_TRUE(solutions[0].bindings.empty()); // No variables to bind
}

TEST_F(ResolverTest, QueryWithVariable) {
    auto query = makeCompound("parent", {makeAtom("tom"), makeVariable("X")});
    auto solutions = resolver->solve(query);
    
    EXPECT_EQ(solutions.size(), 2); // tom has two children: bob and liz
    
    bool found_bob = false, found_liz = false;
    for (const auto& solution : solutions) {
        EXPECT_EQ(solution.bindings.size(), 1);
        auto it = solution.bindings.find("X");
        ASSERT_TRUE(it != solution.bindings.end());
        
        if (it->second->is<Atom>()) {
            auto atom = it->second->as<Atom>();
            if (atom->name() == "bob") found_bob = true;
            if (atom->name() == "liz") found_liz = true;
        }
    }
    
    EXPECT_TRUE(found_bob);
    EXPECT_TRUE(found_liz);
}

TEST_F(ResolverTest, QueryWithMultipleVariables) {
    auto query = makeCompound("parent", {makeVariable("X"), makeVariable("Y")});
    auto solutions = resolver->solve(query);
    
    EXPECT_EQ(solutions.size(), 5); // All parent facts
    
    for (const auto& solution : solutions) {
        EXPECT_EQ(solution.bindings.size(), 2);
        EXPECT_TRUE(solution.bindings.find("X") != solution.bindings.end());
        EXPECT_TRUE(solution.bindings.find("Y") != solution.bindings.end());
    }
}

TEST_F(ResolverTest, RuleResolution) {
    auto query = makeCompound("grandparent", {makeAtom("tom"), makeVariable("Z")});
    auto solutions = resolver->solve(query);
    
    EXPECT_EQ(solutions.size(), 2); // tom is grandparent of ann and pat
    
    bool found_ann = false, found_pat = false;
    for (const auto& solution : solutions) {
        EXPECT_EQ(solution.bindings.size(), 1);
        auto it = solution.bindings.find("Z");
        ASSERT_TRUE(it != solution.bindings.end());
        
        if (it->second->is<Atom>()) {
            auto atom = it->second->as<Atom>();
            if (atom->name() == "ann") found_ann = true;
            if (atom->name() == "pat") found_pat = true;
        }
    }
    
    EXPECT_TRUE(found_ann);
    EXPECT_TRUE(found_pat);
}

TEST_F(ResolverTest, ComplexRuleResolution) {
    auto query = makeCompound("grandparent", {makeVariable("X"), makeVariable("Z")});
    auto solutions = resolver->solve(query);
    
    EXPECT_GT(solutions.size(), 0);
    
    for (const auto& solution : solutions) {
        EXPECT_EQ(solution.bindings.size(), 2);
        EXPECT_TRUE(solution.bindings.find("X") != solution.bindings.end());
        EXPECT_TRUE(solution.bindings.find("Z") != solution.bindings.end());
    }
}

TEST_F(ResolverTest, NoSolutionQuery) {
    auto query = makeCompound("parent", {makeAtom("nonexistent"), makeVariable("X")});
    auto solutions = resolver->solve(query);
    
    EXPECT_EQ(solutions.size(), 0);
}

TEST_F(ResolverTest, CallbackInterface) {
    auto query = makeCompound("parent", {makeVariable("X"), makeVariable("Y")});
    
    std::vector<Solution> callback_solutions;
    resolver->solveWithCallback(query, [&callback_solutions](const Solution& solution) {
        callback_solutions.push_back(solution);
        return true; // Continue searching
    });
    
    EXPECT_EQ(callback_solutions.size(), 5);
}

TEST_F(ResolverTest, CallbackTermination) {
    auto query = makeCompound("parent", {makeVariable("X"), makeVariable("Y")});
    
    int solution_count = 0;
    resolver->solveWithCallback(query, [&solution_count](const Solution& solution) {
        solution_count++;
        return solution_count < 2; // Stop after 2 solutions
    });
    
    EXPECT_EQ(solution_count, 2);
}

TEST_F(ResolverTest, ListQuery) {
    // Add some list facts
    db.addFact(makeCompound("list", {makeList({makeAtom("a"), makeAtom("b")})}));
    
    auto query = makeCompound("list", {makeVariable("L")});
    auto solutions = resolver->solve(query);
    
    EXPECT_EQ(solutions.size(), 1);
    EXPECT_EQ(solutions[0].bindings.size(), 1);
    
    auto list_term = solutions[0].bindings["L"];
    EXPECT_TRUE(list_term->is<List>());
}