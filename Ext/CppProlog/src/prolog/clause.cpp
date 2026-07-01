#include "clause.h"
#include "unification.h"
#include <unordered_set>
#include <functional>

namespace prolog {

std::unique_ptr<Clause> Clause::rename(const std::string& suffix) const {
    std::unordered_map<std::string, std::string> renaming;
    
    auto renameVariable = [&](const std::string& name) -> std::string {
        if (renaming.find(name) == renaming.end()) {
            renaming[name] = name + suffix;
        }
        return renaming[name];
    };
    
    std::function<TermPtr(const TermPtr&)> renameTerm = [&](const TermPtr& term) -> TermPtr {
        switch (term->type()) {
            case TermType::VARIABLE: {
                auto var = term->as<Variable>();
                return makeVariable(renameVariable(var->name()));
            }
            case TermType::COMPOUND: {
                auto compound = term->as<Compound>();
                TermList new_args;
                new_args.reserve(compound->arguments().size());
                for (const auto& arg : compound->arguments()) {
                    new_args.push_back(renameTerm(arg));
                }
                return makeCompound(compound->functor(), std::move(new_args));
            }
            case TermType::LIST: {
                auto list = term->as<List>();
                TermList new_elements;
                new_elements.reserve(list->elements().size());
                for (const auto& elem : list->elements()) {
                    new_elements.push_back(renameTerm(elem));
                }
                TermPtr new_tail = list->tail() ? renameTerm(list->tail()) : nullptr;
                return makeList(std::move(new_elements), new_tail);
            }
            default:
                return term->clone();
        }
    };
    
    TermPtr new_head = renameTerm(head_);
    TermList new_body;
    new_body.reserve(body_.size());
    for (const auto& term : body_) {
        new_body.push_back(renameTerm(term));
    }
    
    return std::make_unique<Clause>(new_head, std::move(new_body));
}

void Clause::collectVariables(std::vector<std::string>& variables) const {
    std::unordered_set<std::string> seen;
    
    std::function<void(const TermPtr&)> collectFromTerm = [&](const TermPtr& term) {
        switch (term->type()) {
            case TermType::VARIABLE: {
                auto var = term->as<Variable>();
                if (seen.insert(var->name()).second) {
                    variables.push_back(var->name());
                }
                break;
            }
            case TermType::COMPOUND: {
                auto compound = term->as<Compound>();
                for (const auto& arg : compound->arguments()) {
                    collectFromTerm(arg);
                }
                break;
            }
            case TermType::LIST: {
                auto list = term->as<List>();
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
    
    collectFromTerm(head_);
    for (const auto& term : body_) {
        collectFromTerm(term);
    }
}

ClausePtr makeFact(TermPtr head) {
    return std::make_unique<Clause>(std::move(head));
}

ClausePtr makeRule(TermPtr head, TermList body) {
    return std::make_unique<Clause>(std::move(head), std::move(body));
}

}