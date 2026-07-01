#pragma once

#include "term.h"
#include "clause.h"
#include <vector>
#include <unordered_map>
#include <string>
#include <memory>

namespace prolog {

class Database {
private:
    std::vector<ClausePtr> clauses_;
    std::unordered_map<std::string, std::vector<size_t>> index_;
    std::unordered_map<std::string, std::vector<size_t>> first_arg_index_;\
    
public:
    void addClause(ClausePtr clause);
    void addFact(TermPtr head);
    void addRule(TermPtr head, TermList body);
    
    std::vector<ClausePtr> findClauses(const std::string& functor, size_t arity) const;
    std::vector<ClausePtr> findMatchingClauses(const TermPtr& goal) const;
    std::vector<ClausePtr> findClausesWithFirstArg(const std::string& functor, size_t arity, 
                                                   const TermPtr& first_arg) const;
    
    void clear();
    size_t size() const { return clauses_.size(); }
    bool empty() const { return clauses_.empty(); }
    
    void loadProgram(const std::string& program);
    
    std::string toString() const;
    
private:
    std::string makeKey(const std::string& functor, size_t arity) const;
    std::string extractFunctorArity(const TermPtr& term) const;
    std::string makeFirstArgKey(const std::string& functor, size_t arity, const TermPtr& first_arg) const;
    std::string extractFirstArgKey(const TermPtr& head) const;
};

}