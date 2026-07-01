#include <gtest/gtest.h>
#include "prolog/unification.h"

using namespace prolog;

class UnificationTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(UnificationTest, AtomUnification) {
    auto atom1 = makeAtom("hello");
    auto atom2 = makeAtom("hello");
    auto atom3 = makeAtom("world");
    
    auto result1 = Unification::unify(atom1, atom2);
    EXPECT_TRUE(result1.has_value());
    EXPECT_TRUE(result1->empty());
    
    auto result2 = Unification::unify(atom1, atom3);
    EXPECT_FALSE(result2.has_value());
}

TEST_F(UnificationTest, VariableUnification) {
    auto var_x = makeVariable("X");
    auto var_y = makeVariable("Y");
    auto atom = makeAtom("hello");
    
    auto result1 = Unification::unify(var_x, atom);
    ASSERT_TRUE(result1.has_value());
    EXPECT_EQ(result1->size(), 1);
    EXPECT_TRUE(result1->find("X") != result1->end());
    
    auto result2 = Unification::unify(var_x, var_y);
    ASSERT_TRUE(result2.has_value());
    EXPECT_EQ(result2->size(), 1);
}

TEST_F(UnificationTest, CompoundUnification) {
    auto comp1 = makeCompound("func", {makeAtom("a"), makeVariable("X")});
    auto comp2 = makeCompound("func", {makeAtom("a"), makeAtom("b")});
    auto comp3 = makeCompound("other", {makeAtom("a"), makeAtom("b")});
    
    auto result1 = Unification::unify(comp1, comp2);
    ASSERT_TRUE(result1.has_value());
    EXPECT_EQ(result1->size(), 1);
    EXPECT_TRUE(result1->find("X") != result1->end());
    
    auto result2 = Unification::unify(comp1, comp3);
    EXPECT_FALSE(result2.has_value());
}

TEST_F(UnificationTest, IntegerUnification) {
    auto int1 = makeInteger(42);
    auto int2 = makeInteger(42);
    auto int3 = makeInteger(24);
    auto var = makeVariable("N");
    
    auto result1 = Unification::unify(int1, int2);
    EXPECT_TRUE(result1.has_value());
    EXPECT_TRUE(result1->empty());
    
    auto result2 = Unification::unify(int1, int3);
    EXPECT_FALSE(result2.has_value());
    
    auto result3 = Unification::unify(var, int1);
    ASSERT_TRUE(result3.has_value());
    EXPECT_EQ(result3->size(), 1);
}

TEST_F(UnificationTest, ListUnification) {
    auto list1 = makeList({makeAtom("a"), makeVariable("X")});
    auto list2 = makeList({makeAtom("a"), makeAtom("b")});
    auto list3 = makeList({makeAtom("c"), makeAtom("d")});
    
    auto result1 = Unification::unify(list1, list2);
    ASSERT_TRUE(result1.has_value());
    EXPECT_EQ(result1->size(), 1);
    
    auto result2 = Unification::unify(list1, list3);
    EXPECT_FALSE(result2.has_value());
}

TEST_F(UnificationTest, OccursCheck) {
    auto var_x = makeVariable("X");
    auto comp_with_x = makeCompound("f", {var_x});
    
    auto result = Unification::unify(var_x, comp_with_x);
    EXPECT_FALSE(result.has_value()); // Should fail occurs check
}

TEST_F(UnificationTest, SubstitutionApplication) {
    auto var_x = makeVariable("X");
    auto var_y = makeVariable("Y");
    auto atom_a = makeAtom("a");
    
    Substitution subst;
    subst["X"] = atom_a;
    
    auto result = Unification::applySubstitution(var_x, subst);
    EXPECT_TRUE(result->equals(*atom_a));
    
    auto result2 = Unification::applySubstitution(var_y, subst);
    EXPECT_TRUE(result2->equals(*var_y)); // Unchanged
}

TEST_F(UnificationTest, SubstitutionComposition) {
    Substitution s1;
    s1["X"] = makeAtom("a");
    
    Substitution s2;
    s2["Y"] = makeVariable("X");
    
    auto composed = Unification::compose(s1, s2);
    
    EXPECT_EQ(composed.size(), 2);
    EXPECT_TRUE(composed["X"]->equals(*makeAtom("a")));
    EXPECT_TRUE(composed["Y"]->equals(*makeAtom("a"))); // Y -> X -> a
}