#include "parser.h"
#include <cctype>
#include <stdexcept>
#include <sstream>

namespace prolog {

// Lexer implementation
std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    
    while (position_ < input_.length()) {
        skipWhitespace();
        
        if (position_ >= input_.length()) break;
        
        char c = peek();
        
        if (c == '%') {
            skipComment();
            continue;
        }
        
        size_t start_pos = position_;
        
        switch (c) {
            case '(':
                advance();
                tokens.emplace_back(Token::LPAREN, "(", start_pos);
                break;
            case ')':
                advance();
                tokens.emplace_back(Token::RPAREN, ")", start_pos);
                break;
            case '[':
                advance();
                tokens.emplace_back(Token::LBRACKET, "[", start_pos);
                break;
            case ']':
                advance();
                tokens.emplace_back(Token::RBRACKET, "]", start_pos);
                break;
            case '.':
                advance();
                tokens.emplace_back(Token::DOT, ".", start_pos);
                break;
            case ',':
                advance();
                tokens.emplace_back(Token::COMMA, ",", start_pos);
                break;
            case '|':
                advance();
                tokens.emplace_back(Token::PIPE, "|", start_pos);
                break;
            case ':':
                advance();
                if (peek() == '-') {
                    advance();
                    tokens.emplace_back(Token::RULE_OP, ":-", start_pos);
                } else {
                    tokens.emplace_back(Token::INVALID, ":", start_pos);
                }
                break;
            case '!':
                advance();
                tokens.emplace_back(Token::ATOM, "!", start_pos);
                break;
            case '"':
                tokens.emplace_back(Token::STRING, readString(), start_pos);
                break;
            case '\'':
                tokens.emplace_back(Token::STRING, readSingleQuotedString(), start_pos);
                break;
            default:
                if (isAtomStart(c)) {
                    std::string value = readAtom();
                    tokens.emplace_back(Token::ATOM, value, start_pos);
                } else if (isVariableStart(c)) {
                    std::string value = readVariable();
                    tokens.emplace_back(Token::VARIABLE, value, start_pos);
                } else if (isDigit(c)) {
                    std::string value = readNumber();
                    if (value.find('.') != std::string::npos) {
                        tokens.emplace_back(Token::FLOAT, value, start_pos);
                    } else {
                        tokens.emplace_back(Token::INTEGER, value, start_pos);
                    }
                } else {
                    advance();
                    tokens.emplace_back(Token::INVALID, std::string(1, c), start_pos);
                }
                break;
        }
    }
    
    tokens.emplace_back(Token::END_OF_INPUT, "", position_);
    return tokens;
}

char Lexer::peek(size_t offset) const {
    size_t pos = position_ + offset;
    return pos < input_.length() ? input_[pos] : '\0';
}

char Lexer::advance() {
    return position_ < input_.length() ? input_[position_++] : '\0';
}

void Lexer::skipWhitespace() {
    while (position_ < input_.length() && isWhitespace(peek())) {
        advance();
    }
}

void Lexer::skipComment() {
    while (position_ < input_.length() && peek() != '\n') {
        advance();
    }
    if (position_ < input_.length()) {
        advance();
    }
}

std::string Lexer::readAtom() {
    std::string result;
    while (position_ < input_.length() && isAtomChar(peek())) {
        result += advance();
    }
    return result;
}

std::string Lexer::readVariable() {
    std::string result;
    while (position_ < input_.length() && isVariableChar(peek())) {
        result += advance();
    }
    return result;
}

std::string Lexer::readNumber() {
    std::string result;
    bool has_dot = false;
    
    while (position_ < input_.length()) {
        char c = peek();
        if (isDigit(c)) {
            result += advance();
        } else if (c == '.' && !has_dot) {
            has_dot = true;
            result += advance();
        } else {
            break;
        }
    }
    
    return result;
}

std::string Lexer::readString() {
    std::string result;
    advance();
    
    while (position_ < input_.length() && peek() != '"') {
        char c = advance();
        if (c == '\\' && position_ < input_.length()) {
            char escaped = advance();
            switch (escaped) {
                case 'n': result += '\n'; break;
                case 't': result += '\t'; break;
                case 'r': result += '\r'; break;
                case '\\': result += '\\'; break;
                case '"': result += '"'; break;
                default: result += escaped; break;
            }
        } else {
            result += c;
        }
    }
    
    if (position_ < input_.length()) {
        advance();
    }
    
    return result;
}

std::string Lexer::readSingleQuotedString() {
    std::string result;
    advance(); // Skip opening single quote
    
    while (position_ < input_.length() && peek() != '\'') {
        char c = advance();
        if (c == '\\' && position_ < input_.length()) {
            char escaped = advance();
            switch (escaped) {
                case 'n': result += '\n'; break;
                case 't': result += '\t'; break;
                case 'r': result += '\r'; break;
                case '\\': result += '\\'; break;
                case '\'': result += '\''; break;
                default: result += escaped; break;
            }
        } else {
            result += c;
        }
    }
    
    if (position_ < input_.length()) {
        advance(); // Skip closing single quote
    }
    
    return result;
}

bool Lexer::isAtomStart(char c) const {
    return std::islower(c) || c == '_';
}

bool Lexer::isAtomChar(char c) const {
    return std::isalnum(c) || c == '_';
}

bool Lexer::isVariableStart(char c) const {
    return std::isupper(c) || c == '_';
}

bool Lexer::isVariableChar(char c) const {
    return std::isalnum(c) || c == '_';
}

bool Lexer::isDigit(char c) const {
    return std::isdigit(c);
}

bool Lexer::isWhitespace(char c) const {
    return std::isspace(c);
}

// Parser implementation
std::optional<ClausePtr> Parser::parseClause() {
    if (isAtEnd() || current().type == Token::END_OF_INPUT) {
        return std::nullopt;
    }
    
    TermPtr head = parseTerm();
    
    if (match(Token::RULE_OP)) {
        TermList body;
        
        do {
            body.push_back(parseTerm());
        } while (match(Token::COMMA));
        
        if (!match(Token::DOT)) {
            error("Expected '.' after rule body");
        }
        
        return makeRule(head, std::move(body));
    } else if (match(Token::DOT)) {
        return makeFact(head);
    } else {
        error("Expected ':-' or '.' after term");
    }
    
    return std::nullopt;
}

std::vector<ClausePtr> Parser::parseProgram(const std::string& input) {
    Lexer lexer(input);
    tokens_ = lexer.tokenize();
    current_ = 0;
    
    std::vector<ClausePtr> clauses;
    
    while (!isAtEnd()) {
        auto clause = parseClause();
        if (clause) {
            clauses.push_back(std::move(*clause));
        } else {
            break;
        }
    }
    
    return clauses;
}

TermPtr Parser::parseQuery(const std::string& input) {
    Lexer lexer(input);
    tokens_ = lexer.tokenize();
    current_ = 0;
    
    return parseTerm();
}

const Token& Parser::current() const {
    return tokens_[current_];
}

bool Parser::isAtEnd() const {
    return current_ >= tokens_.size() || current().type == Token::END_OF_INPUT;
}

bool Parser::check(Token::Type type) const {
    if (isAtEnd()) return false;
    return current().type == type;
}

Token Parser::advance() {
    if (!isAtEnd()) current_++;
    return tokens_[current_ - 1];
}

bool Parser::match(Token::Type type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

TermPtr Parser::parseTerm() {
    if (check(Token::LBRACKET)) {
        return parseList();
    }
    
    if (check(Token::ATOM)) {
        return parseCompoundOrAtom();
    }
    
    if (check(Token::VARIABLE)) {
        std::string name = advance().value;
        return makeVariable(name);
    }
    
    if (check(Token::INTEGER)) {
        std::string value = advance().value;
        return makeInteger(std::stoll(value));
    }
    
    if (check(Token::FLOAT)) {
        std::string value = advance().value;
        return makeFloat(std::stod(value));
    }
    
    if (check(Token::STRING)) {
        std::string value = advance().value;
        return makeString(value);
    }
    
    error("Expected term");
    return nullptr;
}

TermPtr Parser::parseCompoundOrAtom() {
    std::string functor = advance().value;
    
    if (match(Token::LPAREN)) {
        TermList args = parseArguments();
        if (!match(Token::RPAREN)) {
            error("Expected ')' after arguments");
        }
        return makeCompound(functor, std::move(args));
    } else {
        return makeAtom(functor);
    }
}

TermPtr Parser::parseList() {
    if (!match(Token::LBRACKET)) {
        error("Expected '['");
    }
    
    if (match(Token::RBRACKET)) {
        return makeList({});
    }
    
    TermList elements = parseListElements();
    TermPtr tail = nullptr;
    
    if (match(Token::PIPE)) {
        tail = parseTerm();
    }
    
    if (!match(Token::RBRACKET)) {
        error("Expected ']'");
    }
    
    return makeList(std::move(elements), tail);
}

TermList Parser::parseArguments() {
    TermList args;
    
    if (!check(Token::RPAREN)) {
        do {
            args.push_back(parseTerm());
        } while (match(Token::COMMA));
    }
    
    return args;
}

TermList Parser::parseListElements() {
    TermList elements;
    
    do {
        elements.push_back(parseTerm());
    } while (match(Token::COMMA) && !check(Token::PIPE));
    
    return elements;
}

void Parser::error(const std::string& message) {
    std::ostringstream oss;
    oss << "Parse error at position " << (isAtEnd() ? tokens_.size() : current().position) 
        << ": " << message;
    throw ParseException(oss.str());
}

}