#include "unification.h"

namespace prolog {

std::optional<Substitution> Unification::unify(const TermPtr& term1, const TermPtr& term2) {
    Substitution subst;
    return unify(term1, term2, subst);
}

std::optional<Substitution> Unification::unify(const TermPtr& term1, const TermPtr& term2, Substitution& subst) {
    auto result = unifyInternal(term1, term2, subst);
    if (result) {
        return subst;
    }
    return std::nullopt;
}

std::optional<Substitution> Unification::unifyInternal(const TermPtr& term1, const TermPtr& term2, Substitution& subst) {
    TermPtr t1 = dereference(term1, subst);
    TermPtr t2 = dereference(term2, subst);
    
    if (t1->type() == TermType::VARIABLE && t2->type() == TermType::VARIABLE) {
        auto var1 = t1->as<Variable>();
        auto var2 = t2->as<Variable>();
        if (var1->name() == var2->name()) {
            return subst;
        }
        subst[var1->name()] = t2;
        return subst;
    }
    
    if (t1->type() == TermType::VARIABLE) {
        auto var = t1->as<Variable>();
        if (occursCheck(var->name(), t2)) {
            return std::nullopt;
        }
        subst[var->name()] = t2;
        return subst;
    }
    
    if (t2->type() == TermType::VARIABLE) {
        auto var = t2->as<Variable>();
        if (occursCheck(var->name(), t1)) {
            return std::nullopt;
        }
        subst[var->name()] = t1;
        return subst;
    }
    
    if (t1->type() != t2->type()) {
        return std::nullopt;
    }
    
    switch (t1->type()) {
        case TermType::ATOM: {
            auto atom1 = t1->as<Atom>();
            auto atom2 = t2->as<Atom>();
            return atom1->name() == atom2->name() ? std::make_optional(subst) : std::nullopt;
        }
        
        case TermType::INTEGER: {
            auto int1 = t1->as<Integer>();
            auto int2 = t2->as<Integer>();
            return int1->value() == int2->value() ? std::make_optional(subst) : std::nullopt;
        }
        
        case TermType::FLOAT: {
            auto float1 = t1->as<Float>();
            auto float2 = t2->as<Float>();
            return float1->value() == float2->value() ? std::make_optional(subst) : std::nullopt;
        }
        
        case TermType::STRING: {
            auto str1 = t1->as<String>();
            auto str2 = t2->as<String>();
            return str1->value() == str2->value() ? std::make_optional(subst) : std::nullopt;
        }
        
        case TermType::COMPOUND: {
            auto comp1 = t1->as<Compound>();
            auto comp2 = t2->as<Compound>();
            
            if (comp1->functor() != comp2->functor() || comp1->arity() != comp2->arity()) {
                return std::nullopt;
            }
            
            for (size_t i = 0; i < comp1->arguments().size(); ++i) {
                if (!unifyInternal(comp1->arguments()[i], comp2->arguments()[i], subst)) {
                    return std::nullopt;
                }
            }
            return subst;
        }
        
        case TermType::LIST: {
            auto list1 = t1->as<List>();
            auto list2 = t2->as<List>();
            
            if (list1->elements().size() != list2->elements().size()) {
                return std::nullopt;
            }
            
            for (size_t i = 0; i < list1->elements().size(); ++i) {
                if (!unifyInternal(list1->elements()[i], list2->elements()[i], subst)) {
                    return std::nullopt;
                }
            }
            
            if (list1->tail() && list2->tail()) {
                return unifyInternal(list1->tail(), list2->tail(), subst);
            } else if (!list1->tail() && !list2->tail()) {
                return subst;
            } else {
                return std::nullopt;
            }
        }
        
        default:
            return std::nullopt;
    }
}

TermPtr Unification::dereference(const TermPtr& term, const Substitution& subst) {
    if (term->type() == TermType::VARIABLE) {
        auto var = term->as<Variable>();
        auto it = subst.find(var->name());
        if (it != subst.end()) {
            return dereference(it->second, subst);
        }
    }
    return term;
}

TermPtr Unification::applySubstitution(const TermPtr& term, const Substitution& subst) {
    switch (term->type()) {
        case TermType::VARIABLE: {
            auto var = term->as<Variable>();
            auto it = subst.find(var->name());
            if (it != subst.end()) {
                return applySubstitution(it->second, subst);
            }
            return term;
        }
        
        case TermType::COMPOUND: {
            auto compound = term->as<Compound>();
            TermList new_args;
            new_args.reserve(compound->arguments().size());
            for (const auto& arg : compound->arguments()) {
                new_args.push_back(applySubstitution(arg, subst));
            }
            return makeCompound(compound->functor(), std::move(new_args));
        }
        
        case TermType::LIST: {
            auto list = term->as<List>();
            TermList new_elements;
            new_elements.reserve(list->elements().size());
            for (const auto& elem : list->elements()) {
                new_elements.push_back(applySubstitution(elem, subst));
            }
            TermPtr new_tail = list->tail() ? applySubstitution(list->tail(), subst) : nullptr;
            return makeList(std::move(new_elements), new_tail);
        }
        
        default:
            return term;
    }
}

void Unification::applySubstitutionInPlace(TermList& terms, const Substitution& subst) {
    for (auto& term : terms) {
        term = applySubstitution(term, subst);
    }
}

Substitution Unification::compose(const Substitution& s1, const Substitution& s2) {
    Substitution result = s1;
    
    for (const auto& [var, term] : s2) {
        result[var] = applySubstitution(term, s1);
    }
    
    for (auto& [var, term] : result) {
        term = applySubstitution(term, s2);
    }
    
    return result;
}

bool Unification::occursCheck(const std::string& var, const TermPtr& term) {
    switch (term->type()) {
        case TermType::VARIABLE: {
            auto variable = term->as<Variable>();
            return variable->name() == var;
        }
        
        case TermType::COMPOUND: {
            auto compound = term->as<Compound>();
            for (const auto& arg : compound->arguments()) {
                if (occursCheck(var, arg)) {
                    return true;
                }
            }
            return false;
        }
        
        case TermType::LIST: {
            auto list = term->as<List>();
            for (const auto& elem : list->elements()) {
                if (occursCheck(var, elem)) {
                    return true;
                }
            }
            if (list->tail() && occursCheck(var, list->tail())) {
                return true;
            }
            return false;
        }
        
        default:
            return false;
    }
}

}