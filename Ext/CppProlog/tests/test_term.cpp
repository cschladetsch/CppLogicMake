#include <gtest/gtest.h>
#include "prolog/term.h"

using namespace prolog;

class TermTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(TermTest, AtomCreationAndAccess) {
    auto atom = makeAtom("hello");
    
    ASSERT_TRUE(atom->is<Atom>());
    EXPECT_EQ(atom->type(), TermType::ATOM);
    
    auto atom_ptr = atom->as<Atom>();
    EXPECT_EQ(atom_ptr->name(), "hello");
    EXPECT_EQ(atom->toString(), "hello");
}

TEST_F(TermTest, VariableCreationAndAccess) {
    auto var = makeVariable("X");
    
    ASSERT_TRUE(var->is<Variable>());
    EXPECT_EQ(var->type(), TermType::VARIABLE);
    
    auto var_ptr = var->as<Variable>();
    EXPECT_EQ(var_ptr->name(), "X");
    EXPECT_EQ(var->toString(), "X");
}

TEST_F(TermTest, IntegerCreationAndAccess) {
    auto integer = makeInteger(42);
    
    ASSERT_TRUE(integer->is<Integer>());
    EXPECT_EQ(integer->type(), TermType::INTEGER);
    
    auto int_ptr = integer->as<Integer>();
    EXPECT_EQ(int_ptr->value(), 42);
    EXPECT_EQ(integer->toString(), "42");
}

TEST_F(TermTest, FloatCreationAndAccess) {
    auto float_term = makeFloat(3.14);
    
    ASSERT_TRUE(float_term->is<Float>());
    EXPECT_EQ(float_term->type(), TermType::FLOAT);
    
    auto float_ptr = float_term->as<Float>();
    EXPECT_DOUBLE_EQ(float_ptr->value(), 3.14);
}

TEST_F(TermTest, StringCreationAndAccess) {
    auto string = makeString("world");
    
    ASSERT_TRUE(string->is<String>());
    EXPECT_EQ(string->type(), TermType::STRING);
    
    auto str_ptr = string->as<String>();
    EXPECT_EQ(str_ptr->value(), "world");
    EXPECT_EQ(string->toString(), "\"world\"");
}

TEST_F(TermTest, CompoundTermCreation) {
    auto arg1 = makeAtom("a");
    auto arg2 = makeVariable("X");
    auto compound = makeCompound("func", {arg1, arg2});
    
    ASSERT_TRUE(compound->is<Compound>());
    EXPECT_EQ(compound->type(), TermType::COMPOUND);
    
    auto comp_ptr = compound->as<Compound>();
    EXPECT_EQ(comp_ptr->functor(), "func");
    EXPECT_EQ(comp_ptr->arity(), 2);
    EXPECT_EQ(compound->toString(), "func(a, X)");
}

TEST_F(TermTest, ListCreation) {
    auto elem1 = makeAtom("a");
    auto elem2 = makeAtom("b");
    auto list = makeList({elem1, elem2});
    
    ASSERT_TRUE(list->is<List>());
    EXPECT_EQ(list->type(), TermType::LIST);
    
    auto list_ptr = list->as<List>();
    EXPECT_EQ(list_ptr->elements().size(), 2);
    EXPECT_EQ(list->toString(), "[a, b]");
}

TEST_F(TermTest, ListWithTail) {
    auto elem1 = makeAtom("a");
    auto tail = makeVariable("T");
    auto list = makeList({elem1}, tail);
    
    auto list_ptr = list->as<List>();
    EXPECT_TRUE(list_ptr->hasProperTail());
    EXPECT_EQ(list->toString(), "[a | T]");
}

TEST_F(TermTest, TermEquality) {
    auto atom1 = makeAtom("test");
    auto atom2 = makeAtom("test");
    auto atom3 = makeAtom("different");
    
    EXPECT_TRUE(atom1->equals(*atom2));
    EXPECT_FALSE(atom1->equals(*atom3));
    
    auto var1 = makeVariable("X");
    auto var2 = makeVariable("X");
    auto var3 = makeVariable("Y");
    
    EXPECT_TRUE(var1->equals(*var2));
    EXPECT_FALSE(var1->equals(*var3));
}

TEST_F(TermTest, TermCloning) {
    auto original = makeCompound("test", {makeAtom("a"), makeVariable("X")});
    auto cloned = original->clone();
    
    EXPECT_TRUE(original->equals(*cloned));
    EXPECT_NE(original.get(), cloned.get()); // Different objects
}

TEST_F(TermTest, TermHashing) {
    auto atom1 = makeAtom("test");
    auto atom2 = makeAtom("test");
    auto atom3 = makeAtom("different");
    
    EXPECT_EQ(atom1->hash(), atom2->hash());
    EXPECT_NE(atom1->hash(), atom3->hash());
}