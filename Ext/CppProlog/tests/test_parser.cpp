#include <gtest/gtest.h>
#include "prolog/parser.h"

using namespace prolog;

class ParserTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(ParserTest, LexerTokenization) {
    Lexer lexer("hello(world, X, 42).");
    auto tokens = lexer.tokenize();
    
    ASSERT_GE(tokens.size(), 9); // At least 9 tokens including END_OF_INPUT
    
    EXPECT_EQ(tokens[0].type, Token::ATOM);
    EXPECT_EQ(tokens[0].value, "hello");
    
    EXPECT_EQ(tokens[1].type, Token::LPAREN);
    EXPECT_EQ(tokens[2].type, Token::ATOM);
    EXPECT_EQ(tokens[2].value, "world");
    
    EXPECT_EQ(tokens[3].type, Token::COMMA);
    EXPECT_EQ(tokens[4].type, Token::VARIABLE);
    EXPECT_EQ(tokens[4].value, "X");
    
    EXPECT_EQ(tokens[5].type, Token::COMMA);
    EXPECT_EQ(tokens[6].type, Token::INTEGER);
    EXPECT_EQ(tokens[6].value, "42");
    
    EXPECT_EQ(tokens[7].type, Token::RPAREN);
    EXPECT_EQ(tokens[8].type, Token::DOT);
}

TEST_F(ParserTest, StringTokenization) {
    Lexer lexer("\"hello world\"");
    auto tokens = lexer.tokenize();
    
    ASSERT_GE(tokens.size(), 2);
    EXPECT_EQ(tokens[0].type, Token::STRING);
    EXPECT_EQ(tokens[0].value, "hello world");
}

TEST_F(ParserTest, FloatTokenization) {
    Lexer lexer("3.14");
    auto tokens = lexer.tokenize();
    
    ASSERT_GE(tokens.size(), 2);
    EXPECT_EQ(tokens[0].type, Token::FLOAT);
    EXPECT_EQ(tokens[0].value, "3.14");
}

TEST_F(ParserTest, ListTokenization) {
    Lexer lexer("[a, b | T]");
    auto tokens = lexer.tokenize();
    
    EXPECT_EQ(tokens[0].type, Token::LBRACKET);
    EXPECT_EQ(tokens[1].type, Token::ATOM);
    EXPECT_EQ(tokens[2].type, Token::COMMA);
    EXPECT_EQ(tokens[3].type, Token::ATOM);
    EXPECT_EQ(tokens[4].type, Token::PIPE);
    EXPECT_EQ(tokens[5].type, Token::VARIABLE);
    EXPECT_EQ(tokens[6].type, Token::RBRACKET);
}

TEST_F(ParserTest, RuleTokenization) {
    Lexer lexer("parent(X, Y) :- father(X, Y).");
    auto tokens = lexer.tokenize();
    
    bool found_rule_op = false;
    for (const auto& token : tokens) {
        if (token.type == Token::RULE_OP) {
            found_rule_op = true;
            EXPECT_EQ(token.value, ":-");
        }
    }
    EXPECT_TRUE(found_rule_op);
}

TEST_F(ParserTest, ParseAtom) {
    Parser parser({});
    auto term = parser.parseQuery("hello");
    
    ASSERT_TRUE(term->is<Atom>());
    auto atom = term->as<Atom>();
    EXPECT_EQ(atom->name(), "hello");
}

TEST_F(ParserTest, ParseVariable) {
    Parser parser({});
    auto term = parser.parseQuery("X");
    
    ASSERT_TRUE(term->is<Variable>());
    auto var = term->as<Variable>();
    EXPECT_EQ(var->name(), "X");
}

TEST_F(ParserTest, ParseInteger) {
    Parser parser({});
    auto term = parser.parseQuery("42");
    
    ASSERT_TRUE(term->is<Integer>());
    auto integer = term->as<Integer>();
    EXPECT_EQ(integer->value(), 42);
}

TEST_F(ParserTest, ParseFloat) {
    Parser parser({});
    auto term = parser.parseQuery("3.14");
    
    ASSERT_TRUE(term->is<Float>());
    auto float_term = term->as<Float>();
    EXPECT_DOUBLE_EQ(float_term->value(), 3.14);
}

TEST_F(ParserTest, ParseString) {
    Parser parser({});
    auto term = parser.parseQuery("\"hello world\"");
    
    ASSERT_TRUE(term->is<String>());
    auto string = term->as<String>();
    EXPECT_EQ(string->value(), "hello world");
}

TEST_F(ParserTest, ParseCompound) {
    Parser parser({});
    auto term = parser.parseQuery("func(a, X, 42)");
    
    ASSERT_TRUE(term->is<Compound>());
    auto compound = term->as<Compound>();
    EXPECT_EQ(compound->functor(), "func");
    EXPECT_EQ(compound->arity(), 3);
    
    const auto& args = compound->arguments();
    EXPECT_TRUE(args[0]->is<Atom>());
    EXPECT_TRUE(args[1]->is<Variable>());
    EXPECT_TRUE(args[2]->is<Integer>());
}

TEST_F(ParserTest, ParseList) {
    Parser parser({});
    auto term = parser.parseQuery("[a, b, c]");
    
    ASSERT_TRUE(term->is<List>());
    auto list = term->as<List>();
    EXPECT_EQ(list->elements().size(), 3);
    EXPECT_FALSE(list->hasProperTail());
}

TEST_F(ParserTest, ParseListWithTail) {
    Parser parser({});
    auto term = parser.parseQuery("[a, b | T]");
    
    ASSERT_TRUE(term->is<List>());
    auto list = term->as<List>();
    EXPECT_EQ(list->elements().size(), 2);
    EXPECT_TRUE(list->hasProperTail());
    EXPECT_TRUE(list->tail()->is<Variable>());
}

TEST_F(ParserTest, ParseFact) {
    Parser parser({});
    auto clauses = parser.parseProgram("parent(tom, bob).");
    
    ASSERT_EQ(clauses.size(), 1);
    EXPECT_TRUE(clauses[0]->isFact());
    EXPECT_FALSE(clauses[0]->isRule());
    
    auto head = clauses[0]->head();
    ASSERT_TRUE(head->is<Compound>());
    auto compound = head->as<Compound>();
    EXPECT_EQ(compound->functor(), "parent");
    EXPECT_EQ(compound->arity(), 2);
}

TEST_F(ParserTest, ParseRule) {
    Parser parser({});
    auto clauses = parser.parseProgram("grandparent(X, Z) :- parent(X, Y), parent(Y, Z).");
    
    ASSERT_EQ(clauses.size(), 1);
    EXPECT_FALSE(clauses[0]->isFact());
    EXPECT_TRUE(clauses[0]->isRule());
    
    auto head = clauses[0]->head();
    ASSERT_TRUE(head->is<Compound>());
    
    const auto& body = clauses[0]->body();
    EXPECT_EQ(body.size(), 2);
    EXPECT_TRUE(body[0]->is<Compound>());
    EXPECT_TRUE(body[1]->is<Compound>());
}

TEST_F(ParserTest, ParseMultipleClauses) {
    std::string program = R"(
        parent(tom, bob).
        parent(bob, ann).
        parent(bob, pat).
        grandparent(X, Z) :- parent(X, Y), parent(Y, Z).
    )";
    
    Parser parser({});
    auto clauses = parser.parseProgram(program);
    
    EXPECT_EQ(clauses.size(), 4);
    EXPECT_TRUE(clauses[0]->isFact());
    EXPECT_TRUE(clauses[1]->isFact());
    EXPECT_TRUE(clauses[2]->isFact());
    EXPECT_TRUE(clauses[3]->isRule());
}

TEST_F(ParserTest, ParseInvalidSyntax) {
    Parser parser({});
    
    EXPECT_THROW(parser.parseQuery("func("), ParseException);
    EXPECT_THROW(parser.parseQuery("[a, b"), ParseException);
    EXPECT_THROW(parser.parseProgram("parent(X, Y) :-"), ParseException);
}