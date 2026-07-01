#pragma once

#include "term.h"
#include <string>

namespace prolog {

struct Solution {
    Substitution bindings;
    
    std::string toString() const {
        if (bindings.empty()) {
            return "true";
        }
        
        std::string result;
        bool first = true;
        for (const auto& [var_name, term] : bindings) {
            if (!first) result += ", ";
            result += var_name + " = " + term->toString();
            first = false;
        }
        return result;
    }
};

}