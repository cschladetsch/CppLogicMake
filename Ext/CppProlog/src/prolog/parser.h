#pragma once

#include "term.h"
#include "clause.h"
#include <string>
#include <vector>
#include <memory>
#include <optional>

namespace prolog {

class Token {
public:
    enum Type {
        ATOM, VARIABLE, INTEGER, FLOAT, STRING,
        LPAREN, RPAREN, LBRACKET, RBRACKET,
        DOT, COMMA, PIPE, RULE_OP, OPERATOR,
        END_OF_INPUT, INVALID
    };
    
    Type type;
    std::string value;
    size_t position;
    
    Token(Type t, std::string v, size_t pos) 
        : type(t), value(std::move(v)), position(pos) {}
};

class Lexer {
private:
    std::string input_;
    size_t position_;
    
public:
    explicit Lexer(std::string input) : input_(std::move(input)), position_(0) {}
    
    std::vector<Token> tokenize();
    
private:
    char peek(size_t offset = 0) const;
    char advance();
    void skipWhitespace();
    void skipComment();
    
    std::string readAtom();
    std::string readVariable();
    std::string readNumber();
    std::string readString();
    std::string readSingleQuotedString();
    
    bool isAtomStart(char c) const;
    bool isAtomChar(char c) const;
    bool isVariableStart(char c) const;
    bool isVariableChar(char c) const;
    bool isDigit(char c) const;
    bool isWhitespace(char c) const;
};

class Parser {
private:
    std::vector<Token> tokens_;
    size_t current_;
    
public:
    explicit Parser(std::vector<Token> tokens) : tokens_(std::move(tokens)), current_(0) {}
    
    std::optional<ClausePtr> parseClause();
    std::vector<ClausePtr> parseProgram(const std::string& input);
    TermPtr parseQuery(const std::string& input);
    
private:
    const Token& current() const;
    bool isAtEnd() const;
    bool check(Token::Type type) const;
    Token advance();
    bool match(Token::Type type);
    
    TermPtr parseTerm();
    TermPtr parseCompoundOrAtom();
    TermPtr parseList();
    TermList parseArguments();
    TermList parseListElements();
    
    void error(const std::string& message);
};

class ParseException : public std::exception {
private:
    std::string message_;
    
public:
    explicit ParseException(std::string message) : message_(std::move(message)) {}
    const char* what() const noexcept override { return message_.c_str(); }
};

}