% Test new built-in predicates
test_fact.

test_list([a, b, c]).

% Test rules
test_length(List, Len) :- length(List, Len).
test_strict_eq(X, Y) :- X == Y.
test_strict_neq(X, Y) :- X \== Y.