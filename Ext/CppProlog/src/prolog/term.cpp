#include "term.h"

namespace prolog {

TermPtr makeAtom(const std::string& name) {
    return std::make_shared<Atom>(name);
}

TermPtr makeVariable(const std::string& name) {
    return std::make_shared<Variable>(name);
}

TermPtr makeInteger(int64_t value) {
    return std::make_shared<Integer>(value);
}

TermPtr makeFloat(double value) {
    return std::make_shared<Float>(value);
}

TermPtr makeString(const std::string& value) {
    return std::make_shared<String>(value);
}

TermPtr makeCompound(const std::string& functor, TermList arguments) {
    return std::make_shared<Compound>(functor, std::move(arguments));
}

TermPtr makeList(TermList elements, TermPtr tail) {
    return std::make_shared<List>(std::move(elements), tail);
}

}