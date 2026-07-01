#pragma once

#include "term.h"
#include "solution.h"
#include <functional>
#include <unordered_map>
#include <string>
#include <optional>

namespace prolog {

class BuiltinPredicates {
public:
    using BuiltinHandler = std::function<bool(const TermList&, Substitution&, 
                                            std::function<bool(const Solution&)>)>;
    
    static void registerBuiltins();
    static bool isBuiltin(const std::string& functor, size_t arity);
    static bool callBuiltin(const std::string& functor, size_t arity, 
                           const TermList& args, Substitution& bindings,
                           std::function<bool(const Solution&)> callback);
    
private:
    static std::unordered_map<std::string, BuiltinHandler> builtins_;
    
    static std::string makeKey(const std::string& functor, size_t arity);
    
    // Arithmetic predicates
    static bool is(const TermList& args, Substitution& bindings, 
                   std::function<bool(const Solution&)> callback);
    static bool add(const TermList& args, Substitution& bindings, 
                    std::function<bool(const Solution&)> callback);
    static bool subtract(const TermList& args, Substitution& bindings, 
                         std::function<bool(const Solution&)> callback);
    static bool multiply(const TermList& args, Substitution& bindings, 
                         std::function<bool(const Solution&)> callback);
    static bool divide(const TermList& args, Substitution& bindings, 
                       std::function<bool(const Solution&)> callback);
    
    // Comparison predicates
    static bool equal(const TermList& args, Substitution& bindings, 
                      std::function<bool(const Solution&)> callback);
    static bool notEqual(const TermList& args, Substitution& bindings, 
                         std::function<bool(const Solution&)> callback);
    static bool strictEqual(const TermList& args, Substitution& bindings, 
                           std::function<bool(const Solution&)> callback);
    static bool strictNotEqual(const TermList& args, Substitution& bindings, 
                               std::function<bool(const Solution&)> callback);
    static bool lessThan(const TermList& args, Substitution& bindings, 
                         std::function<bool(const Solution&)> callback);
    static bool greaterThan(const TermList& args, Substitution& bindings, 
                            std::function<bool(const Solution&)> callback);
    static bool lessEqual(const TermList& args, Substitution& bindings, 
                          std::function<bool(const Solution&)> callback);
    static bool greaterEqual(const TermList& args, Substitution& bindings, 
                             std::function<bool(const Solution&)> callback);
    
    // List predicates
    static bool append(const TermList& args, Substitution& bindings, 
                       std::function<bool(const Solution&)> callback);
    static bool member(const TermList& args, Substitution& bindings, 
                       std::function<bool(const Solution&)> callback);
    static bool length(const TermList& args, Substitution& bindings, 
                       std::function<bool(const Solution&)> callback);
    
    // Type checking predicates
    static bool var(const TermList& args, Substitution& bindings, 
                    std::function<bool(const Solution&)> callback);
    static bool nonvar(const TermList& args, Substitution& bindings, 
                       std::function<bool(const Solution&)> callback);
    static bool atom(const TermList& args, Substitution& bindings, 
                     std::function<bool(const Solution&)> callback);
    static bool number(const TermList& args, Substitution& bindings, 
                       std::function<bool(const Solution&)> callback);
    static bool integer(const TermList& args, Substitution& bindings, 
                        std::function<bool(const Solution&)> callback);
    static bool float_check(const TermList& args, Substitution& bindings, 
                            std::function<bool(const Solution&)> callback);
    static bool compound(const TermList& args, Substitution& bindings, 
                         std::function<bool(const Solution&)> callback);
    static bool ground(const TermList& args, Substitution& bindings, 
                       std::function<bool(const Solution&)> callback);
    
    // Control predicates
    static bool cut(const TermList& args, Substitution& bindings, 
                    std::function<bool(const Solution&)> callback);
    static bool fail(const TermList& args, Substitution& bindings, 
                     std::function<bool(const Solution&)> callback);
    static bool true_pred(const TermList& args, Substitution& bindings, 
                          std::function<bool(const Solution&)> callback);
    static bool not_provable(const TermList& args, Substitution& bindings, 
                             std::function<bool(const Solution&)> callback);
    
    // I/O predicates
    static bool write(const TermList& args, Substitution& bindings, 
                      std::function<bool(const Solution&)> callback);
    static bool nl(const TermList& args, Substitution& bindings, 
                   std::function<bool(const Solution&)> callback);
    
    // Utility functions
    static bool isNumber(const TermPtr& term);
    static double getNumericValue(const TermPtr& term);
    static bool unifyWithNumber(const TermPtr& term, double value, Substitution& bindings);
    static bool isGround(const TermPtr& term);
    
public:
    // Arithmetic evaluation (made public for testing)
    static std::optional<double> evaluateArithmeticExpression(const TermPtr& expr, const Substitution& bindings);
    
    // Term comparison utilities
    static int compareTerms(const TermPtr& left, const TermPtr& right);
    static int getTermOrder(const TermPtr& term);
};

}