#include "builtin_predicates.h"
#include "unification.h"
#include <cmath>
#include <iostream>
#include <algorithm>

namespace prolog {

std::unordered_map<std::string, BuiltinPredicates::BuiltinHandler> BuiltinPredicates::builtins_;

void BuiltinPredicates::registerBuiltins() {
    if (!builtins_.empty()) return; // Already registered
    
    // Arithmetic
    builtins_[makeKey("is", 2)] = is;
    builtins_[makeKey("+", 3)] = add;
    builtins_[makeKey("-", 3)] = subtract;
    builtins_[makeKey("*", 3)] = multiply;
    builtins_[makeKey("/", 3)] = divide;
    
    // Comparison
    builtins_[makeKey("=", 2)] = equal;
    builtins_[makeKey("\\=", 2)] = notEqual;
    builtins_[makeKey("==", 2)] = strictEqual;
    builtins_[makeKey("\\==", 2)] = strictNotEqual;
    builtins_[makeKey("<", 2)] = lessThan;
    builtins_[makeKey(">", 2)] = greaterThan;
    builtins_[makeKey("=<", 2)] = lessEqual;
    builtins_[makeKey(">=", 2)] = greaterEqual;
    
    // List operations
    builtins_[makeKey("append", 3)] = append;
    builtins_[makeKey("member", 2)] = member;
    builtins_[makeKey("length", 2)] = length;
    
    // Type checking
    builtins_[makeKey("var", 1)] = var;
    builtins_[makeKey("nonvar", 1)] = nonvar;
    builtins_[makeKey("atom", 1)] = atom;
    builtins_[makeKey("number", 1)] = number;
    builtins_[makeKey("integer", 1)] = integer;
    builtins_[makeKey("float", 1)] = float_check;
    builtins_[makeKey("compound", 1)] = compound;
    builtins_[makeKey("ground", 1)] = ground;
    
    // Control
    builtins_[makeKey("!", 0)] = cut;
    builtins_[makeKey("fail", 0)] = fail;
    builtins_[makeKey("true", 0)] = true_pred;
    builtins_[makeKey("\\+", 1)] = not_provable;
    
    // I/O
    builtins_[makeKey("write", 1)] = write;
    builtins_[makeKey("nl", 0)] = nl;
}

bool BuiltinPredicates::isBuiltin(const std::string& functor, size_t arity) {
    return builtins_.find(makeKey(functor, arity)) != builtins_.end();
}

bool BuiltinPredicates::callBuiltin(const std::string& functor, size_t arity, 
                                   const TermList& args, Substitution& bindings,
                                   std::function<bool(const Solution&)> callback) {
    auto it = builtins_.find(makeKey(functor, arity));
    if (it != builtins_.end()) {
        return it->second(args, bindings, callback);
    }
    return false;
}

std::string BuiltinPredicates::makeKey(const std::string& functor, size_t arity) {
    return functor + "/" + std::to_string(arity);
}

bool BuiltinPredicates::is(const TermList& args, Substitution& bindings, 
                          std::function<bool(const Solution&)> callback) {
    if (args.size() != 2) return false;
    
    TermPtr left = Unification::applySubstitution(args[0], bindings);
    TermPtr right = Unification::applySubstitution(args[1], bindings);
    
    // Evaluate arithmetic expression on the right side
    auto result = evaluateArithmeticExpression(right, bindings);
    if (!result.has_value()) return false;
    
    if (unifyWithNumber(left, result.value(), bindings)) {
        Solution solution{bindings};
        return callback(solution);
    }
    
    return false;
}

bool BuiltinPredicates::add(const TermList& args, Substitution& bindings, 
                           std::function<bool(const Solution&)> callback) {
    if (args.size() != 3) return false;
    
    TermPtr arg1 = Unification::applySubstitution(args[0], bindings);
    TermPtr arg2 = Unification::applySubstitution(args[1], bindings);
    TermPtr result = Unification::applySubstitution(args[2], bindings);
    
    if (!isNumber(arg1) || !isNumber(arg2)) return false;
    
    double val1 = getNumericValue(arg1);
    double val2 = getNumericValue(arg2);
    double sum = val1 + val2;
    
    if (unifyWithNumber(result, sum, bindings)) {
        Solution solution{bindings};
        return callback(solution);
    }
    
    return false;
}

bool BuiltinPredicates::equal(const TermList& args, Substitution& bindings, 
                             std::function<bool(const Solution&)> callback) {
    if (args.size() != 2) return false;
    
    TermPtr left = Unification::applySubstitution(args[0], bindings);
    TermPtr right = Unification::applySubstitution(args[1], bindings);
    
    auto unification = Unification::unify(left, right, bindings);
    if (unification) {
        Solution solution{*unification};
        return callback(solution);
    }
    
    return false;
}

bool BuiltinPredicates::append(const TermList& args, Substitution& bindings, 
                              std::function<bool(const Solution&)> callback) {
    if (args.size() != 3) return false;
    
    TermPtr list1 = Unification::applySubstitution(args[0], bindings);
    TermPtr list2 = Unification::applySubstitution(args[1], bindings);
    TermPtr result = Unification::applySubstitution(args[2], bindings);
    
    // Simple implementation for ground terms
    if (list1->is<List>() && list2->is<List>()) {
        auto l1 = list1->as<List>();
        auto l2 = list2->as<List>();
        
        TermList combined = l1->elements();
        const auto& l2_elements = l2->elements();
        combined.insert(combined.end(), l2_elements.begin(), l2_elements.end());
        
        TermPtr combined_list = makeList(std::move(combined));
        
        auto unification = Unification::unify(result, combined_list, bindings);
        if (unification) {
            Solution solution{*unification};
            return callback(solution);
        }
    }
    
    return false;
}

bool BuiltinPredicates::member(const TermList& args, Substitution& bindings, 
                              std::function<bool(const Solution&)> callback) {
    if (args.size() != 2) return false;
    
    TermPtr element = Unification::applySubstitution(args[0], bindings);
    TermPtr list = Unification::applySubstitution(args[1], bindings);
    
    if (list->is<List>()) {
        auto l = list->as<List>();
        for (const auto& elem : l->elements()) {
            // Create a copy of bindings to work with
            Substitution local_bindings = bindings;
            auto unification = Unification::unify(element, elem, local_bindings);
            if (unification) {
                Solution solution{*unification};
                if (!callback(solution)) {
                    return false; // Callback requested stop
                }
            }
        }
        return true; // All possibilities explored
    }
    
    return false;
}

bool BuiltinPredicates::var(const TermList& args, Substitution& bindings, 
                           std::function<bool(const Solution&)> callback) {
    if (args.size() != 1) return false;
    
    TermPtr term = Unification::applySubstitution(args[0], bindings);
    
    if (term->type() == TermType::VARIABLE) {
        Solution solution{bindings};
        return callback(solution);
    }
    
    return false;
}

bool BuiltinPredicates::true_pred(const TermList& args, Substitution& bindings, 
                                 std::function<bool(const Solution&)> callback) {
    Solution solution{bindings};
    return callback(solution);
}

bool BuiltinPredicates::fail(const TermList& args, Substitution& bindings, 
                            std::function<bool(const Solution&)> callback) {
    return false;
}

// Utility functions
bool BuiltinPredicates::isNumber(const TermPtr& term) {
    return term->type() == TermType::INTEGER || term->type() == TermType::FLOAT;
}

double BuiltinPredicates::getNumericValue(const TermPtr& term) {
    if (term->type() == TermType::INTEGER) {
        return static_cast<double>(term->as<Integer>()->value());
    } else if (term->type() == TermType::FLOAT) {
        return term->as<Float>()->value();
    }
    return 0.0;
}

bool BuiltinPredicates::unifyWithNumber(const TermPtr& term, double value, Substitution& bindings) {
    TermPtr number_term;
    if (std::floor(value) == value && value >= INT64_MIN && value <= INT64_MAX) {
        number_term = makeInteger(static_cast<int64_t>(value));
    } else {
        number_term = makeFloat(value);
    }
    
    auto unification = Unification::unify(term, number_term, bindings);
    return unification.has_value();
}

bool BuiltinPredicates::isGround(const TermPtr& term) {
    switch (term->type()) {
        case TermType::VARIABLE:
            return false;
        
        case TermType::ATOM:
        case TermType::INTEGER:
        case TermType::FLOAT:
        case TermType::STRING:
            return true;
        
        case TermType::COMPOUND:
        {
            auto compound = term->as<Compound>();
            return std::all_of(compound->arguments().begin(), compound->arguments().end(),
                              [](const TermPtr& arg) { return isGround(arg); });
        }
        
        case TermType::LIST:
        {
            auto list = term->as<List>();
            // Check all elements are ground
            bool elements_ground = std::all_of(list->elements().begin(), list->elements().end(),
                                              [](const TermPtr& elem) { return isGround(elem); });
            // Check tail is ground (if it exists)
            bool tail_ground = !list->tail() || isGround(list->tail());
            return elements_ground && tail_ground;
        }
        
        default:
            return false;
    }
}

// Arithmetic operation implementations
bool BuiltinPredicates::subtract(const TermList& args, Substitution& bindings, 
                                std::function<bool(const Solution&)> callback) {
    if (args.size() != 3) return false;
    
    TermPtr arg1 = Unification::applySubstitution(args[0], bindings);
    TermPtr arg2 = Unification::applySubstitution(args[1], bindings);
    TermPtr result = Unification::applySubstitution(args[2], bindings);
    
    if (!isNumber(arg1) || !isNumber(arg2)) return false;
    
    double val1 = getNumericValue(arg1);
    double val2 = getNumericValue(arg2);
    double difference = val1 - val2;
    
    if (unifyWithNumber(result, difference, bindings)) {
        Solution solution{bindings};
        return callback(solution);
    }
    
    return false;
}

bool BuiltinPredicates::multiply(const TermList& args, Substitution& bindings, 
                                std::function<bool(const Solution&)> callback) {
    if (args.size() != 3) return false;
    
    TermPtr arg1 = Unification::applySubstitution(args[0], bindings);
    TermPtr arg2 = Unification::applySubstitution(args[1], bindings);
    TermPtr result = Unification::applySubstitution(args[2], bindings);
    
    if (!isNumber(arg1) || !isNumber(arg2)) return false;
    
    double val1 = getNumericValue(arg1);
    double val2 = getNumericValue(arg2);
    double product = val1 * val2;
    
    if (unifyWithNumber(result, product, bindings)) {
        Solution solution{bindings};
        return callback(solution);
    }
    
    return false;
}

bool BuiltinPredicates::divide(const TermList& args, Substitution& bindings, 
                              std::function<bool(const Solution&)> callback) {
    if (args.size() != 3) return false;
    
    TermPtr arg1 = Unification::applySubstitution(args[0], bindings);
    TermPtr arg2 = Unification::applySubstitution(args[1], bindings);
    TermPtr result = Unification::applySubstitution(args[2], bindings);
    
    if (!isNumber(arg1) || !isNumber(arg2)) return false;
    
    double val1 = getNumericValue(arg1);
    double val2 = getNumericValue(arg2);
    
    if (val2 == 0.0) return false; // Division by zero
    
    double quotient = val1 / val2;
    
    if (unifyWithNumber(result, quotient, bindings)) {
        Solution solution{bindings};
        return callback(solution);
    }
    
    return false;
}
bool BuiltinPredicates::notEqual(const TermList& args, Substitution& bindings, 
                                std::function<bool(const Solution&)> callback) {
    if (args.size() != 2) return false;
    
    TermPtr left = Unification::applySubstitution(args[0], bindings);
    TermPtr right = Unification::applySubstitution(args[1], bindings);
    
    auto unification = Unification::unify(left, right, bindings);
    if (unification) {
        return false; // Terms can be unified, so they are equal
    } else {
        // Terms cannot be unified, so they are not equal
        Solution solution{bindings};
        return callback(solution);
    }
}

bool BuiltinPredicates::strictEqual(const TermList& args, Substitution& bindings, 
                                   std::function<bool(const Solution&)> callback) {
    if (args.size() != 2) return false;
    
    TermPtr left = Unification::applySubstitution(args[0], bindings);
    TermPtr right = Unification::applySubstitution(args[1], bindings);
    
    // Strict equality: terms must be structurally identical (no unification)
    if (left->equals(*right)) {
        Solution solution{bindings};
        return callback(solution);
    }
    
    return false;
}

bool BuiltinPredicates::strictNotEqual(const TermList& args, Substitution& bindings, 
                                      std::function<bool(const Solution&)> callback) {
    if (args.size() != 2) return false;
    
    TermPtr left = Unification::applySubstitution(args[0], bindings);
    TermPtr right = Unification::applySubstitution(args[1], bindings);
    
    // Strict inequality: terms must not be structurally identical
    if (!left->equals(*right)) {
        Solution solution{bindings};
        return callback(solution);
    }
    
    return false;
}

bool BuiltinPredicates::lessThan(const TermList& args, Substitution& bindings, 
                                std::function<bool(const Solution&)> callback) {
    if (args.size() != 2) return false;
    
    TermPtr left = Unification::applySubstitution(args[0], bindings);
    TermPtr right = Unification::applySubstitution(args[1], bindings);
    
    if (compareTerms(left, right) < 0) {
        Solution solution{bindings};
        return callback(solution);
    }
    
    return false;
}

bool BuiltinPredicates::greaterThan(const TermList& args, Substitution& bindings, 
                                   std::function<bool(const Solution&)> callback) {
    if (args.size() != 2) return false;
    
    TermPtr left = Unification::applySubstitution(args[0], bindings);
    TermPtr right = Unification::applySubstitution(args[1], bindings);
    
    if (compareTerms(left, right) > 0) {
        Solution solution{bindings};
        return callback(solution);
    }
    
    return false;
}

bool BuiltinPredicates::lessEqual(const TermList& args, Substitution& bindings, 
                                 std::function<bool(const Solution&)> callback) {
    if (args.size() != 2) return false;
    
    TermPtr left = Unification::applySubstitution(args[0], bindings);
    TermPtr right = Unification::applySubstitution(args[1], bindings);
    
    if (compareTerms(left, right) <= 0) {
        Solution solution{bindings};
        return callback(solution);
    }
    
    return false;
}

bool BuiltinPredicates::greaterEqual(const TermList& args, Substitution& bindings, 
                                    std::function<bool(const Solution&)> callback) {
    if (args.size() != 2) return false;
    
    TermPtr left = Unification::applySubstitution(args[0], bindings);
    TermPtr right = Unification::applySubstitution(args[1], bindings);
    
    if (compareTerms(left, right) >= 0) {
        Solution solution{bindings};
        return callback(solution);
    }
    
    return false;
}

bool BuiltinPredicates::length(const TermList& args, Substitution& bindings, 
                              std::function<bool(const Solution&)> callback) {
    if (args.size() != 2) return false;
    
    TermPtr list_term = Unification::applySubstitution(args[0], bindings);
    TermPtr length_term = Unification::applySubstitution(args[1], bindings);
    
    // Case 1: List is bound, length is variable
    if (list_term->is<List>()) {
        auto list = list_term->as<List>();
        size_t len = list->elements().size();
        
        if (unifyWithNumber(length_term, static_cast<double>(len), bindings)) {
            Solution solution{bindings};
            return callback(solution);
        }
    }
    // Case 2: Length is bound, list is variable (generate list)
    else if (length_term->is<Integer>()) {
        auto integer = length_term->as<Integer>();
        if (integer->value() >= 0 && list_term->is<Variable>()) {
            size_t len = static_cast<size_t>(integer->value());
            TermList elements;
            
            // Create list with unbound variables
            for (size_t i = 0; i < len; ++i) {
                elements.push_back(makeVariable("_G" + std::to_string(i)));
            }
            
            TermPtr generated_list = makeList(std::move(elements));
            auto unification = Unification::unify(list_term, generated_list, bindings);
            if (unification) {
                Solution solution{unification.value()};
                return callback(solution);
            }
        }
    }
    // Case 3: Both are bound, check if they match
    else if (list_term->is<List>() && length_term->is<Integer>()) {
        auto list = list_term->as<List>();
        auto integer = length_term->as<Integer>();
        
        if (static_cast<int64_t>(list->elements().size()) == integer->value()) {
            Solution solution{bindings};
            return callback(solution);
        }
    }
    
    return false;
}
bool BuiltinPredicates::nonvar(const TermList& args, Substitution& bindings, 
                              std::function<bool(const Solution&)> callback) {
    if (args.size() != 1) return false;
    
    TermPtr term = Unification::applySubstitution(args[0], bindings);
    
    if (term->type() != TermType::VARIABLE) {
        Solution solution{bindings};
        return callback(solution);
    }
    
    return false;
}

bool BuiltinPredicates::atom(const TermList& args, Substitution& bindings, 
                            std::function<bool(const Solution&)> callback) {
    if (args.size() != 1) return false;
    
    TermPtr term = Unification::applySubstitution(args[0], bindings);
    
    if (term->type() == TermType::ATOM) {
        Solution solution{bindings};
        return callback(solution);
    }
    
    return false;
}

bool BuiltinPredicates::number(const TermList& args, Substitution& bindings, 
                              std::function<bool(const Solution&)> callback) {
    if (args.size() != 1) return false;
    
    TermPtr term = Unification::applySubstitution(args[0], bindings);
    
    if (isNumber(term)) {
        Solution solution{bindings};
        return callback(solution);
    }
    
    return false;
}
bool BuiltinPredicates::cut(const TermList& args, Substitution& bindings, 
                           std::function<bool(const Solution&)> callback) {
    // Cut always succeeds but prevents backtracking
    // The actual cut logic is handled in the resolver when encountering the cut goal
    Solution solution{bindings};
    callback(solution);
    return true; // Cut succeeds but prevents further choice points
}

bool BuiltinPredicates::not_provable(const TermList& args, Substitution& bindings, 
                                    std::function<bool(const Solution&)> callback) {
    if (args.size() != 1) return false;
    
    TermPtr goal = Unification::applySubstitution(args[0], bindings);
    
    // For basic implementation, we can only handle simple built-in predicates
    // This is a simplified version - full implementation requires resolver access
    
    if (goal->is<Compound>()) {
        auto compound = goal->as<Compound>();
        
        // Test if the built-in predicate succeeds
        if (BuiltinPredicates::isBuiltin(compound->functor(), compound->arguments().size())) {
            Substitution test_bindings = bindings;
            bool goal_succeeded = false;
            
            auto test_callback = [&goal_succeeded](const Solution& solution) -> bool {
                goal_succeeded = true;
                return false; // Stop after first solution
            };
            
            BuiltinPredicates::callBuiltin(
                compound->functor(),
                compound->arguments().size(),
                compound->arguments(),
                test_bindings,
                test_callback
            );
            
            // Negation as failure: succeed if goal failed, fail if goal succeeded
            if (!goal_succeeded) {
                Solution solution{bindings};
                return callback(solution);
            }
        }
    } else if (goal->is<Atom>()) {
        auto atom = goal->as<Atom>();
        
        if (BuiltinPredicates::isBuiltin(atom->name(), 0)) {
            Substitution test_bindings = bindings;
            bool goal_succeeded = false;
            
            auto test_callback = [&goal_succeeded](const Solution& solution) -> bool {
                goal_succeeded = true;
                return false; // Stop after first solution
            };
            
            BuiltinPredicates::callBuiltin(
                atom->name(),
                0,
                {},
                test_bindings,
                test_callback
            );
            
            // Negation as failure: succeed if goal failed, fail if goal succeeded
            if (!goal_succeeded) {
                Solution solution{bindings};
                return callback(solution);
            }
        }
    }
    
    return false; // Goal succeeded or couldn't be evaluated, so \+ fails
}

bool BuiltinPredicates::integer(const TermList& args, Substitution& bindings, 
                               std::function<bool(const Solution&)> callback) {
    if (args.size() != 1) return false;
    
    TermPtr term = Unification::applySubstitution(args[0], bindings);
    
    if (term->type() == TermType::INTEGER) {
        Solution solution{bindings};
        return callback(solution);
    }
    
    return false;
}

bool BuiltinPredicates::float_check(const TermList& args, Substitution& bindings, 
                                   std::function<bool(const Solution&)> callback) {
    if (args.size() != 1) return false;
    
    TermPtr term = Unification::applySubstitution(args[0], bindings);
    
    if (term->type() == TermType::FLOAT) {
        Solution solution{bindings};
        return callback(solution);
    }
    
    return false;
}

bool BuiltinPredicates::compound(const TermList& args, Substitution& bindings, 
                                std::function<bool(const Solution&)> callback) {
    if (args.size() != 1) return false;
    
    TermPtr term = Unification::applySubstitution(args[0], bindings);
    
    if (term->type() == TermType::COMPOUND) {
        Solution solution{bindings};
        return callback(solution);
    }
    
    return false;
}

bool BuiltinPredicates::ground(const TermList& args, Substitution& bindings, 
                              std::function<bool(const Solution&)> callback) {
    if (args.size() != 1) return false;
    
    TermPtr term = Unification::applySubstitution(args[0], bindings);
    
    if (isGround(term)) {
        Solution solution{bindings};
        return callback(solution);
    }
    
    return false;
}

bool BuiltinPredicates::write(const TermList& args, Substitution& bindings, 
                             std::function<bool(const Solution&)> callback) {
    if (args.size() != 1) return false;
    
    TermPtr term = Unification::applySubstitution(args[0], bindings);
    
    // Convert term to string and output it
    if (term->type() == TermType::STRING) {
        std::cout << term->as<String>()->value();
    } else if (term->type() == TermType::ATOM) {
        std::cout << term->as<Atom>()->name();
    } else if (term->type() == TermType::INTEGER) {
        std::cout << term->as<Integer>()->value();
    } else if (term->type() == TermType::FLOAT) {
        std::cout << term->as<Float>()->value();
    } else {
        // For compound terms, variables, lists - use toString if available
        // For now, just output a placeholder
        std::cout << "<term>";
    }
    
    // write/1 always succeeds
    Solution solution{bindings};
    return callback(solution);
}

bool BuiltinPredicates::nl(const TermList& args, Substitution& bindings, 
                          std::function<bool(const Solution&)> callback) {
    if (args.size() != 0) return false;
    
    std::cout << "\n";
    
    // nl/0 always succeeds
    Solution solution{bindings};
    return callback(solution);
}

std::optional<double> BuiltinPredicates::evaluateArithmeticExpression(const TermPtr& expr, const Substitution& bindings) {
    TermPtr term = Unification::applySubstitution(expr, bindings);
    
    // If it's already a number, return its value
    if (isNumber(term)) {
        return getNumericValue(term);
    }
    
    // If it's a compound term, check for arithmetic operations
    if (term->is<Compound>()) {
        auto compound = term->as<Compound>();
        const std::string& functor = compound->functor();
        const auto& args = compound->arguments();
        
        // Binary arithmetic operations
        if (args.size() == 2) {
            auto left_val = evaluateArithmeticExpression(args[0], bindings);
            auto right_val = evaluateArithmeticExpression(args[1], bindings);
            
            if (!left_val.has_value() || !right_val.has_value()) {
                return std::nullopt;
            }
            
            if (functor == "+") {
                return left_val.value() + right_val.value();
            } else if (functor == "-") {
                return left_val.value() - right_val.value();
            } else if (functor == "*") {
                return left_val.value() * right_val.value();
            } else if (functor == "/") {
                if (right_val.value() == 0.0) return std::nullopt; // Division by zero
                return left_val.value() / right_val.value();
            } else if (functor == "//") {
                if (right_val.value() == 0.0) return std::nullopt; // Division by zero
                return std::floor(left_val.value() / right_val.value());
            } else if (functor == "mod") {
                if (right_val.value() == 0.0) return std::nullopt; // Modulo by zero
                return std::fmod(left_val.value(), right_val.value());
            }
        }
        
        // Unary arithmetic operations
        if (args.size() == 1) {
            auto val = evaluateArithmeticExpression(args[0], bindings);
            if (!val.has_value()) return std::nullopt;
            
            if (functor == "-") {
                return -val.value();
            } else if (functor == "abs") {
                return std::abs(val.value());
            }
        }
    }
    
    // Could not evaluate
    return std::nullopt;
}

int BuiltinPredicates::getTermOrder(const TermPtr& term) {
    switch (term->type()) {
        case TermType::VARIABLE: return 1;
        case TermType::INTEGER:
        case TermType::FLOAT: return 2;
        case TermType::ATOM: return 3;
        case TermType::STRING: return 4;
        case TermType::COMPOUND: return 5;
        case TermType::LIST: return 6;
        default: return 7;
    }
}

int BuiltinPredicates::compareTerms(const TermPtr& left, const TermPtr& right) {
    int leftOrder = getTermOrder(left);
    int rightOrder = getTermOrder(right);
    
    if (leftOrder != rightOrder) {
        return leftOrder - rightOrder;
    }
    
    // Same type comparison map
    static const std::unordered_map<TermType, std::function<int(const TermPtr&, const TermPtr&)>> comparators = {
        {TermType::VARIABLE, [](const TermPtr& l, const TermPtr& r) -> int {
            return l->as<Variable>()->name().compare(r->as<Variable>()->name());
        }},
        {TermType::INTEGER, [](const TermPtr& l, const TermPtr& r) -> int {
            auto lVal = l->as<Integer>()->value();
            auto rVal = r->as<Integer>()->value();
            return (lVal < rVal) ? -1 : (lVal > rVal) ? 1 : 0;
        }},
        {TermType::FLOAT, [](const TermPtr& l, const TermPtr& r) -> int {
            auto lVal = l->as<Float>()->value();
            auto rVal = r->as<Float>()->value();
            return (lVal < rVal) ? -1 : (lVal > rVal) ? 1 : 0;
        }},
        {TermType::ATOM, [](const TermPtr& l, const TermPtr& r) -> int {
            return l->as<Atom>()->name().compare(r->as<Atom>()->name());
        }},
        {TermType::STRING, [](const TermPtr& l, const TermPtr& r) -> int {
            return l->as<String>()->value().compare(r->as<String>()->value());
        }},
        {TermType::COMPOUND, [](const TermPtr& l, const TermPtr& r) -> int {
            auto lComp = l->as<Compound>();
            auto rComp = r->as<Compound>();
            
            // Compare by functor first
            int functorCmp = lComp->functor().compare(rComp->functor());
            if (functorCmp != 0) return functorCmp;
            
            // Then by arity
            size_t lArity = lComp->arguments().size();
            size_t rArity = rComp->arguments().size();
            if (lArity != rArity) return (lArity < rArity) ? -1 : 1;
            
            // Finally by arguments
            for (size_t i = 0; i < lArity; ++i) {
                int argCmp = compareTerms(lComp->arguments()[i], rComp->arguments()[i]);
                if (argCmp != 0) return argCmp;
            }
            return 0;
        }},
        {TermType::LIST, [](const TermPtr& l, const TermPtr& r) -> int {
            auto lList = l->as<List>();
            auto rList = r->as<List>();
            
            const auto& lElems = lList->elements();
            const auto& rElems = rList->elements();
            
            // Use lexicographical_compare for element comparison
            auto result = std::lexicographical_compare(
                lElems.begin(), lElems.end(),
                rElems.begin(), rElems.end(),
                [](const TermPtr& a, const TermPtr& b) {
                    return compareTerms(a, b) < 0;
                }
            );
            
            if (result) return -1;
            
            // Check if right is lexicographically smaller than left
            auto reverse_result = std::lexicographical_compare(
                rElems.begin(), rElems.end(),
                lElems.begin(), lElems.end(),
                [](const TermPtr& a, const TermPtr& b) {
                    return compareTerms(a, b) < 0;
                }
            );
            
            if (reverse_result) return 1;
            
            // Elements are equal, compare tails
            if (lList->tail() && rList->tail()) {
                return compareTerms(lList->tail(), rList->tail());
            } else if (lList->tail()) {
                return 1;
            } else if (rList->tail()) {
                return -1;
            }
            
            return 0;
        }}
    };
    
    auto it = comparators.find(left->type());
    return (it != comparators.end()) ? it->second(left, right) : 0;
}

}