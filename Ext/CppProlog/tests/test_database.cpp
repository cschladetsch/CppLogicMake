#include <gtest/gtest.h>
#include "prolog/database.h"

using namespace prolog;

class DatabaseTest : public ::testing::Test {
protected:
    Database db;
    
    void SetUp() override {
        db.clear();
    }
    
    void TearDown() override {
        db.clear();
    }
};

TEST_F(DatabaseTest, AddFact) {
    auto fact = makeAtom("hello");
    db.addFact(fact);
    
    EXPECT_EQ(db.size(), 1);
    EXPECT_FALSE(db.empty());
}

TEST_F(DatabaseTest, AddRule) {
    auto head = makeCompound("grandparent", {makeVariable("X"), makeVariable("Z")});
    TermList body = {
        makeCompound("parent", {makeVariable("X"), makeVariable("Y")}),
        makeCompound("parent", {makeVariable("Y"), makeVariable("Z")})
    };
    
    db.addRule(head, body);
    
    EXPECT_EQ(db.size(), 1);
}

TEST_F(DatabaseTest, FindClausesByFunctorArity) {
    db.addFact(makeCompound("parent", {makeAtom("tom"), makeAtom("bob")}));
    db.addFact(makeCompound("parent", {makeAtom("bob"), makeAtom("ann")}));
    db.addFact(makeAtom("single"));
    
    auto clauses = db.findClauses("parent", 2);
    EXPECT_EQ(clauses.size(), 2);
    
    auto single_clauses = db.findClauses("single", 0);
    EXPECT_EQ(single_clauses.size(), 1);
    
    auto missing_clauses = db.findClauses("missing", 1);
    EXPECT_EQ(missing_clauses.size(), 0);
}

TEST_F(DatabaseTest, FindMatchingClauses) {
    db.addFact(makeCompound("parent", {makeAtom("tom"), makeAtom("bob")}));
    db.addFact(makeCompound("parent", {makeAtom("bob"), makeAtom("ann")}));
    
    auto query = makeCompound("parent", {makeVariable("X"), makeAtom("bob")});
    auto clauses = db.findMatchingClauses(query);
    
    EXPECT_EQ(clauses.size(), 2); // Should find both parent clauses
}

TEST_F(DatabaseTest, LoadProgram) {
    std::string program = R"(
        parent(tom, bob).
        parent(bob, ann).
        parent(bob, pat).
        grandparent(X, Z) :- parent(X, Y), parent(Y, Z).
    )";
    
    db.loadProgram(program);
    
    EXPECT_EQ(db.size(), 4);
    
    auto parent_clauses = db.findClauses("parent", 2);
    EXPECT_EQ(parent_clauses.size(), 3);
    
    auto grandparent_clauses = db.findClauses("grandparent", 2);
    EXPECT_EQ(grandparent_clauses.size(), 1);
    EXPECT_TRUE(grandparent_clauses[0]->isRule());
}

TEST_F(DatabaseTest, ClearDatabase) {
    db.addFact(makeAtom("test"));
    EXPECT_EQ(db.size(), 1);
    
    db.clear();
    EXPECT_EQ(db.size(), 0);
    EXPECT_TRUE(db.empty());
}

TEST_F(DatabaseTest, DatabaseToString) {
    db.addFact(makeCompound("parent", {makeAtom("tom"), makeAtom("bob")}));
    
    auto head = makeCompound("grandparent", {makeVariable("X"), makeVariable("Z")});
    TermList body = {makeCompound("parent", {makeVariable("X"), makeVariable("Y")})};
    db.addRule(head, body);
    
    std::string db_string = db.toString();
    EXPECT_FALSE(db_string.empty());
    EXPECT_TRUE(db_string.find("parent(tom, bob)") != std::string::npos);
    EXPECT_TRUE(db_string.find("grandparent(X, Z)") != std::string::npos);
    EXPECT_TRUE(db_string.find(":-") != std::string::npos);
}

TEST_F(DatabaseTest, InvalidProgramSyntax) {
    std::string invalid_program = "invalid syntax here";
    
    EXPECT_THROW(db.loadProgram(invalid_program), std::runtime_error);
}