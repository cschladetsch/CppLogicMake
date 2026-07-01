#pragma once

#include "term.h"
#include <memory>
#include <vector>
#include <string>

namespace prolog {

class Clause {
private:
    TermPtr head_;
    TermList body_;

public:
    explicit Clause(TermPtr head, TermList body = {})
        : head_(std::move(head)), body_(std::move(body)) {}

    const TermPtr& head() const { return head_; }
    const TermList& body() const { return body_; }
    
    bool isFact() const { return body_.empty(); }
    bool isRule() const { return !body_.empty(); }
    
    std::string toString() const {
        std::string result = head_->toString();
        if (!body_.empty()) {
            result += " :- ";
            for (size_t i = 0; i < body_.size(); ++i) {
                if (i > 0) result += ", ";
                result += body_[i]->toString();
            }
        }
        result += ".";
        return result;
    }
    
    std::unique_ptr<Clause> clone() const {
        TermList cloned_body;
        cloned_body.reserve(body_.size());
        for (const auto& term : body_) {
            cloned_body.push_back(term->clone());
        }
        return std::make_unique<Clause>(head_->clone(), std::move(cloned_body));
    }
    
    std::unique_ptr<Clause> rename(const std::string& suffix) const;
    
    void collectVariables(std::vector<std::string>& variables) const;
};

using ClausePtr = std::unique_ptr<Clause>;

ClausePtr makeFact(TermPtr head);
ClausePtr makeRule(TermPtr head, TermList body);

}