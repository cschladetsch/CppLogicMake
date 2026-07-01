#include "resolver.h"
#include <algorithm>
#include <sstream>
#include <cstdlib>
#include <unordered_set>

namespace prolog {

std::vector<Solution> Resolver::solve(const TermPtr& query) {
    // Collect variables from the query
    std::vector<std::string> queryVariables;
    collectVariablesFromTerm(query, queryVariables);
    
    std::vector<Solution> solutions;
    
    solveWithCallback({query}, [&solutions, this, &queryVariables](const Solution& solution) {
        // Filter bindings to only include query variables
        Solution filteredSolution{filterBindings(solution.bindings, queryVariables)};
        solutions.push_back(filteredSolution);
        return true; // Continue to find more solutions
    });
    
    return solutions;
}

std::vector<Solution> Resolver::solve(const TermList& goals) {
    // Collect variables from all goals
    std::vector<std::string> queryVariables;
    for (const auto& goal : goals) {
        collectVariablesFromTerm(goal, queryVariables);
    }
    
    std::vector<Solution> solutions;
    
    solveWithCallback(goals, [&solutions, this, &queryVariables](const Solution& solution) {
        // Filter bindings to only include query variables
        Solution filteredSolution{filterBindings(solution.bindings, queryVariables)};
        solutions.push_back(filteredSolution);
        return true; // Continue to find more solutions
    });
    
    return solutions;
}

void Resolver::solveWithCallback(const TermPtr& query,
                                std::function<bool(const Solution&)> callback) {
    choice_stack_.clear();
    current_depth_ = 0;
    termination_requested_ = false;
    cut_encountered_ = false;
    current_cut_level_ = 0;

    Substitution empty_bindings;
    solveGoals({query}, empty_bindings, callback, 0);
}

void Resolver::solveWithCallback(const TermList& goals,
                                std::function<bool(const Solution&)> callback) {
    choice_stack_.clear();
    current_depth_ = 0;
    termination_requested_ = false;
    cut_encountered_ = false;
    current_cut_level_ = 0;

    Substitution empty_bindings;
    solveGoals(goals, empty_bindings, callback, 0);
}

int Resolver::solveGoals(const TermList& goals, const Substitution& bindings,
                         std::function<bool(const Solution&)> callback, size_t parent_cut_level) {
    if (current_depth_ > max_depth_) {
        return -1; // Failed
    }

    if (goals.empty()) {
        Solution solution{bindings};
        bool continue_search = callback(solution);
        if (!continue_search) {
            termination_requested_ = true;
        }
        return continue_search ? 0 : -1; // 0 = success without cut
    }

    TermPtr current_goal = Unification::applySubstitution(goals[0], bindings);
    TermList remaining_goals(goals.begin() + 1, goals.end());
    Unification::applySubstitutionInPlace(remaining_goals, bindings);

    // Check for cut (!) first
    if (current_goal->is<Atom>()) {
        auto atom = current_goal->as<Atom>();
        if (atom->name() == "!") {
            // Cut: succeed and continue with remaining goals, but signal cut
            int result = solveGoals(remaining_goals, bindings, callback, parent_cut_level);
            if (result >= 0) {
                return 1; // Signal that cut was encountered
            }
            return result;
        }
    }

    // Check for built-in predicates
    if (current_goal->is<Compound>()) {
        auto compound = current_goal->as<Compound>();
        if (BuiltinPredicates::isBuiltin(compound->functor(), compound->arguments().size())) {
            // Handle built-in predicate
            Substitution new_bindings = bindings;
            int result = -1;
            bool builtin_success = BuiltinPredicates::callBuiltin(
                compound->functor(),
                compound->arguments().size(),
                compound->arguments(),
                new_bindings,
                [&](const Solution& builtin_solution) {
                    // Continue with remaining goals
                    int sub_result = solveGoals(remaining_goals, builtin_solution.bindings, callback, parent_cut_level);
                    if (sub_result >= 0) {
                        result = sub_result;
                        return sub_result == 0; // Continue searching if no cut
                    }
                    return false;
                }
            );
            return builtin_success ? result : -1;
        }
    } else if (current_goal->is<Atom>()) {
        auto atom = current_goal->as<Atom>();
        if (BuiltinPredicates::isBuiltin(atom->name(), 0)) {
            // Handle built-in predicate with arity 0
            Substitution new_bindings = bindings;
            int result = -1;
            bool builtin_success = BuiltinPredicates::callBuiltin(
                atom->name(),
                0,
                {}, // No arguments for atoms
                new_bindings,
                [&](const Solution& builtin_solution) {
                    // Continue with remaining goals
                    int sub_result = solveGoals(remaining_goals, builtin_solution.bindings, callback, parent_cut_level);
                    if (sub_result >= 0) {
                        result = sub_result;
                        return sub_result == 0; // Continue searching if no cut
                    }
                    return false;
                }
            );
            return builtin_success ? result : -1;
        }
    }

    auto matching_clauses = database_.findMatchingClauses(current_goal);

    if (matching_clauses.empty()) {
        return -1; // Failed
    }

    int result = -1;
    size_t clause_cut_level = current_depth_;

    // Try each matching clause
    for (auto& clause : matching_clauses) {
        // Check if termination was requested
        if (termination_requested_) {
            break;
        }

        current_depth_++;

        if (current_depth_ > max_depth_) {
            current_depth_--;
            break;
        }

        // Rename clause variables to avoid conflicts
        std::string suffix = "_" + std::to_string(current_depth_) + "_" + std::to_string(rand());
        ClausePtr renamed_clause = clause->rename(suffix);

        // Try to unify goal with clause head
        auto unification_result = Unification::unify(current_goal, renamed_clause->head());

        if (unification_result) {
            // Unification succeeded
            Substitution new_bindings = Unification::compose(bindings, *unification_result);

            // Create new goals: clause body + remaining goals
            TermList new_goals;
            for (const auto& body_goal : renamed_clause->body()) {
                new_goals.push_back(Unification::applySubstitution(body_goal, *unification_result));
            }
            new_goals.insert(new_goals.end(), remaining_goals.begin(), remaining_goals.end());

            // Recursively solve new goals
            int sub_result = solveGoals(new_goals, new_bindings, callback, clause_cut_level);

            if (sub_result >= 0) {
                result = 0; // Found at least one solution

                if (sub_result == 1) {
                    // Cut was encountered in the clause body
                    // Don't try any more clauses for this goal
                    current_depth_--;
                    return 0; // Success but don't propagate cut upward
                }
                // Continue to try more clauses (for backtracking)
            }
        }

        current_depth_--;
    }

    return result;
}

void Resolver::pushChoice(TermPtr goal, TermList remaining_goals, 
                         std::vector<ClausePtr> clauses, Substitution bindings) {
    choice_stack_.emplace_back(std::move(goal), std::move(remaining_goals), 
                              std::move(clauses), std::move(bindings));
}

bool Resolver::backtrack() {
    // This method is no longer used in the simplified implementation
    return false;
}

std::string Resolver::renameVariables(size_t clause_id) const {
    return "_" + std::to_string(clause_id) + "_" + std::to_string(current_depth_);
}

void Resolver::collectVariablesFromTerm(const TermPtr& term, std::vector<std::string>& variables) const {
    std::unordered_set<std::string> seen;
    
    std::function<void(const TermPtr&)> collectFromTerm = [&](const TermPtr& t) {
        switch (t->type()) {
            case TermType::VARIABLE: {
                auto var = t->as<Variable>();
                if (seen.insert(var->name()).second) {
                    variables.push_back(var->name());
                }
                break;
            }
            case TermType::COMPOUND: {
                auto compound = t->as<Compound>();
                for (const auto& arg : compound->arguments()) {
                    collectFromTerm(arg);
                }
                break;
            }
            case TermType::LIST: {
                auto list = t->as<List>();
                for (const auto& elem : list->elements()) {
                    collectFromTerm(elem);
                }
                if (list->tail()) {
                    collectFromTerm(list->tail());
                }
                break;
            }
            default:
                break;
        }
    };
    
    collectFromTerm(term);
}

Substitution Resolver::filterBindings(const Substitution& bindings, 
                                     const std::vector<std::string>& queryVariables) const {
    Substitution filtered;
    for (const auto& varName : queryVariables) {
        auto it = bindings.find(varName);
        if (it != bindings.end()) {
            filtered[varName] = it->second;
        }
    }
    return filtered;
}

void Resolver::performCut() {
    cut_encountered_ = true;
    
    // Remove all choice points at or above the current cut level
    while (!choice_stack_.empty() && choice_stack_.back().cut_level >= current_cut_level_) {
        choice_stack_.pop_back();
    }
}

}