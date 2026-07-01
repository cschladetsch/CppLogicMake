#include <gtest/gtest.h>
#include "prolog/interpreter.h"
#include "prolog/database.h"
#include "prolog/builtin_predicates.h"

using namespace prolog;

class CutTest : public ::testing::Test {
protected:
    void SetUp() override {
        interpreter = std::make_unique<Interpreter>(false);
        BuiltinPredicates::registerBuiltins();
    }

    std::unique_ptr<Interpreter> interpreter;
};

TEST_F(CutTest, BasicCut) {
    interpreter->loadString(
        "p(a).\n"
        "p(b).\n"
        "q(X) :- p(X), !.\n"
    );
    auto solutions = interpreter->query("q(X)");
    ASSERT_EQ(solutions.size(), 1);
    auto s1 = solutions[0];
    auto x1_binding = s1.bindings.find("X");
    ASSERT_NE(x1_binding, s1.bindings.end());
    EXPECT_EQ(x1_binding->second->toString(), "a");
}
