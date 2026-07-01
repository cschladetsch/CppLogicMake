#pragma once

#include <string>
#include <vector>
#include <memory>
#include <variant>
#include <optional>
#include <unordered_map>
#include <sstream>

namespace prolog {

class Term;
using TermPtr = std::shared_ptr<Term>;
using TermList = std::vector<TermPtr>;
using Substitution = std::unordered_map<std::string, TermPtr>;

enum class TermType {
    ATOM,
    VARIABLE,
    COMPOUND,
    INTEGER,
    FLOAT,
    STRING,
    LIST
};

class Term {
public:
    explicit Term(TermType type) : type_(type) {}
    virtual ~Term() = default;

    TermType type() const { return type_; }
    
    virtual std::string toString() const = 0;
    virtual TermPtr clone() const = 0;
    virtual bool equals(const Term& other) const = 0;
    virtual size_t hash() const = 0;

    template<typename T>
    const T* as() const {
        return dynamic_cast<const T*>(this);
    }

    template<typename T>
    T* as() {
        return dynamic_cast<T*>(this);
    }

    template<typename T>
    bool is() const {
        return dynamic_cast<const T*>(this) != nullptr;
    }

protected:
    TermType type_;
};

class Atom : public Term {
private:
    std::string name_;

public:
    explicit Atom(std::string name) 
        : Term(TermType::ATOM), name_(std::move(name)) {}

    const std::string& name() const { return name_; }
    
    std::string toString() const override { return name_; }
    TermPtr clone() const override { 
        return std::make_shared<Atom>(name_); 
    }
    
    bool equals(const Term& other) const override {
        if (auto atom = other.as<Atom>()) {
            return name_ == atom->name_;
        }
        return false;
    }
    
    size_t hash() const override {
        return std::hash<std::string>{}(name_);
    }
};

class Variable : public Term {
private:
    std::string name_;

public:
    explicit Variable(std::string name) 
        : Term(TermType::VARIABLE), name_(std::move(name)) {}

    const std::string& name() const { return name_; }
    
    std::string toString() const override { 
        return name_; 
    }
    
    TermPtr clone() const override { 
        return std::make_shared<Variable>(name_); 
    }
    
    bool equals(const Term& other) const override {
        if (auto var = other.as<Variable>()) {
            return name_ == var->name_;
        }
        return false;
    }
    
    size_t hash() const override {
        return std::hash<std::string>{}(name_);
    }
};

class Integer : public Term {
private:
    int64_t value_;

public:
    explicit Integer(int64_t value) 
        : Term(TermType::INTEGER), value_(value) {}

    int64_t value() const { return value_; }
    
    std::string toString() const override { 
        return std::to_string(value_); 
    }
    
    TermPtr clone() const override { 
        return std::make_shared<Integer>(value_); 
    }
    
    bool equals(const Term& other) const override {
        if (auto integer = other.as<Integer>()) {
            return value_ == integer->value_;
        }
        return false;
    }
    
    size_t hash() const override {
        return std::hash<int64_t>{}(value_);
    }
};

class Float : public Term {
private:
    double value_;

public:
    explicit Float(double value) 
        : Term(TermType::FLOAT), value_(value) {}

    double value() const { return value_; }
    
    std::string toString() const override { 
        return std::to_string(value_); 
    }
    
    TermPtr clone() const override { 
        return std::make_shared<Float>(value_); 
    }
    
    bool equals(const Term& other) const override {
        if (auto float_term = other.as<Float>()) {
            return value_ == float_term->value_;
        }
        return false;
    }
    
    size_t hash() const override {
        return std::hash<double>{}(value_);
    }
};

class String : public Term {
private:
    std::string value_;

public:
    explicit String(std::string value) 
        : Term(TermType::STRING), value_(std::move(value)) {}

    const std::string& value() const { return value_; }
    
    std::string toString() const override { 
        return "\"" + value_ + "\""; 
    }
    
    TermPtr clone() const override { 
        return std::make_shared<String>(value_); 
    }
    
    bool equals(const Term& other) const override {
        if (auto string_term = other.as<String>()) {
            return value_ == string_term->value_;
        }
        return false;
    }
    
    size_t hash() const override {
        return std::hash<std::string>{}(value_);
    }
};

class Compound : public Term {
private:
    std::string functor_;
    TermList arguments_;

public:
    Compound(std::string functor, TermList arguments) 
        : Term(TermType::COMPOUND), functor_(std::move(functor)), 
          arguments_(std::move(arguments)) {}

    const std::string& functor() const { return functor_; }
    const TermList& arguments() const { return arguments_; }
    size_t arity() const { return arguments_.size(); }
    
    std::string toString() const override {
        if (arguments_.empty()) {
            return functor_;
        }
        
        std::string result = functor_ + "(";
        for (size_t i = 0; i < arguments_.size(); ++i) {
            if (i > 0) result += ", ";
            result += arguments_[i]->toString();
        }
        result += ")";
        return result;
    }
    
    TermPtr clone() const override {
        TermList cloned_args;
        cloned_args.reserve(arguments_.size());
        for (const auto& arg : arguments_) {
            cloned_args.push_back(arg->clone());
        }
        return std::make_shared<Compound>(functor_, std::move(cloned_args));
    }
    
    bool equals(const Term& other) const override {
        if (auto compound = other.as<Compound>()) {
            if (functor_ != compound->functor_ || 
                arguments_.size() != compound->arguments_.size()) {
                return false;
            }
            for (size_t i = 0; i < arguments_.size(); ++i) {
                if (!arguments_[i]->equals(*compound->arguments_[i])) {
                    return false;
                }
            }
            return true;
        }
        return false;
    }
    
    size_t hash() const override {
        size_t h = std::hash<std::string>{}(functor_);
        for (const auto& arg : arguments_) {
            h ^= arg->hash() + 0x9e3779b9 + (h << 6) + (h >> 2);
        }
        return h;
    }
};

class List : public Term {
private:
    TermList elements_;
    TermPtr tail_;

public:
    explicit List(TermList elements, TermPtr tail = nullptr) 
        : Term(TermType::LIST), elements_(std::move(elements)), tail_(tail) {}

    const TermList& elements() const { return elements_; }
    const TermPtr& tail() const { return tail_; }
    bool hasProperTail() const { return tail_ != nullptr; }
    
    std::string toString() const override {
        std::string result = "[";
        for (size_t i = 0; i < elements_.size(); ++i) {
            if (i > 0) result += ", ";
            result += elements_[i]->toString();
        }
        if (tail_) {
            result += " | " + tail_->toString();
        }
        result += "]";
        return result;
    }
    
    TermPtr clone() const override {
        TermList cloned_elements;
        cloned_elements.reserve(elements_.size());
        for (const auto& elem : elements_) {
            cloned_elements.push_back(elem->clone());
        }
        TermPtr cloned_tail = tail_ ? tail_->clone() : nullptr;
        return std::make_shared<List>(std::move(cloned_elements), cloned_tail);
    }
    
    bool equals(const Term& other) const override {
        if (auto list = other.as<List>()) {
            if (elements_.size() != list->elements_.size()) {
                return false;
            }
            for (size_t i = 0; i < elements_.size(); ++i) {
                if (!elements_[i]->equals(*list->elements_[i])) {
                    return false;
                }
            }
            if (tail_ && list->tail_) {
                return tail_->equals(*list->tail_);
            }
            return tail_ == list->tail_;
        }
        return false;
    }
    
    size_t hash() const override {
        size_t h = 0;
        for (const auto& elem : elements_) {
            h ^= elem->hash() + 0x9e3779b9 + (h << 6) + (h >> 2);
        }
        if (tail_) {
            h ^= tail_->hash() + 0x9e3779b9 + (h << 6) + (h >> 2);
        }
        return h;
    }
};

TermPtr makeAtom(const std::string& name);
TermPtr makeVariable(const std::string& name);
TermPtr makeInteger(int64_t value);
TermPtr makeFloat(double value);
TermPtr makeString(const std::string& value);
TermPtr makeCompound(const std::string& functor, TermList arguments);
TermPtr makeList(TermList elements, TermPtr tail = nullptr);

}

template<>
struct std::hash<prolog::Term> {
    size_t operator()(const prolog::Term& term) const {
        return term.hash();
    }
};