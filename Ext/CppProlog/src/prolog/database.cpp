#include "database.h"
#include "parser.h"
#include <sstream>

namespace prolog {

void Database::addClause(ClausePtr clause) {
    std::string key = extractFunctorArity(clause->head());
    index_[key].push_back(clauses_.size());
    
    // Add to first argument index if applicable
    std::string first_arg_key = extractFirstArgKey(clause->head());
    if (!first_arg_key.empty()) {
        first_arg_index_[first_arg_key].push_back(clauses_.size());
    }
    
    clauses_.push_back(std::move(clause));
}

void Database::addFact(TermPtr head) {
    addClause(makeFact(std::move(head)));
}

void Database::addRule(TermPtr head, TermList body) {
    addClause(makeRule(std::move(head), std::move(body)));
}

std::vector<ClausePtr> Database::findClauses(const std::string& functor, size_t arity) const {
    std::string key = makeKey(functor, arity);
    auto it = index_.find(key);
    
    std::vector<ClausePtr> result;
    if (it != index_.end()) {
        result.reserve(it->second.size());
        for (size_t index : it->second) {
            result.push_back(clauses_[index]->clone());
        }
    }
    
    return result;
}

std::vector<ClausePtr> Database::findMatchingClauses(const TermPtr& goal) const {
    std::string key = extractFunctorArity(goal);
    auto it = index_.find(key);
    
    std::vector<ClausePtr> result;
    if (it != index_.end()) {
        result.reserve(it->second.size());
        for (size_t index : it->second) {
            result.push_back(clauses_[index]->clone());
        }
    }
    
    return result;
}

void Database::clear() {
    clauses_.clear();
    index_.clear();
    first_arg_index_.clear();
}

void Database::loadProgram(const std::string& program) {
    try {
        Parser parser({});
        auto clauses = parser.parseProgram(program);
        
        for (auto& clause : clauses) {
            addClause(std::move(clause));
        }
    } catch (const ParseException& e) {
        throw std::runtime_error("Failed to load program: " + std::string(e.what()));
    }
}

std::string Database::toString() const {
    std::ostringstream oss;
    for (const auto& clause : clauses_) {
        oss << clause->toString() << "\n";
    }
    return oss.str();
}

std::string Database::makeKey(const std::string& functor, size_t arity) const {
    return functor + "/" + std::to_string(arity);
}

std::string Database::extractFunctorArity(const TermPtr& term) const {
    switch (term->type()) {
        case TermType::ATOM: {
            auto atom = term->as<Atom>();
            return makeKey(atom->name(), 0);
        }
        case TermType::COMPOUND: {
            auto compound = term->as<Compound>();
            return makeKey(compound->functor(), compound->arity());
        }
        default:
            return "";
    }
}

std::vector<ClausePtr> Database::findClausesWithFirstArg(const std::string& functor, size_t arity, 
                                                         const TermPtr& first_arg) const {
    std::string key = makeFirstArgKey(functor, arity, first_arg);
    auto it = first_arg_index_.find(key);
    
    std::vector<ClausePtr> result;
    if (it != first_arg_index_.end()) {
        result.reserve(it->second.size());
        for (size_t index : it->second) {
            result.push_back(clauses_[index]->clone());
        }
    }
    
    return result;
}

std::string Database::makeFirstArgKey(const std::string& functor, size_t arity, const TermPtr& first_arg) const {
    std::string base_key = makeKey(functor, arity);
    
    // Add first argument information for indexing
    switch (first_arg->type()) {
        case TermType::ATOM: {
            auto atom = first_arg->as<Atom>();
            return base_key + ":" + atom->name();
        }
        case TermType::INTEGER: {
            auto integer = first_arg->as<Integer>();
            return base_key + ":" + std::to_string(integer->value());
        }
        case TermType::FLOAT: {
            auto float_term = first_arg->as<Float>();
            return base_key + ":" + std::to_string(float_term->value());
        }
        case TermType::STRING: {
            auto string_term = first_arg->as<String>();
            return base_key + ":\"" + string_term->value() + "\"";
        }
        case TermType::COMPOUND: {
            auto compound = first_arg->as<Compound>();
            return base_key + ":" + compound->functor() + "/" + std::to_string(compound->arity());
        }
        case TermType::VARIABLE:
            // Variables don't provide useful indexing information
            return "";
        default:
            return "";
    }
}

std::string Database::extractFirstArgKey(const TermPtr& head) const {
    if (head->type() != TermType::COMPOUND) {
        return "";
    }
    
    auto compound = head->as<Compound>();
    if (compound->arity() == 0) {
        return "";
    }
    
    const TermPtr& first_arg = compound->arguments()[0];
    return makeFirstArgKey(compound->functor(), compound->arity(), first_arg);
}

}