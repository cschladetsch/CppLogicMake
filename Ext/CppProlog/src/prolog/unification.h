#pragma once

#include "term.h"
#include <optional>

namespace prolog {

class Unification {
public:
    static std::optional<Substitution> unify(const TermPtr& term1, const TermPtr& term2);
    static std::optional<Substitution> unify(const TermPtr& term1, const TermPtr& term2, Substitution& subst);
    
    static TermPtr applySubstitution(const TermPtr& term, const Substitution& subst);
    static void applySubstitutionInPlace(TermList& terms, const Substitution& subst);
    
    static Substitution compose(const Substitution& s1, const Substitution& s2);
    
    static bool occursCheck(const std::string& var, const TermPtr& term);
    
private:
    static std::optional<Substitution> unifyInternal(const TermPtr& term1, const TermPtr& term2, Substitution& subst);
    static TermPtr dereference(const TermPtr& term, const Substitution& subst);
};

}