parent(john, mary).
parent(john, tom).
parent(mary, ann).
parent(mary, pat).

grandparent(X, Z) :- parent(X, Y), parent(Y, Z).
sibling(X, Y) :- parent(Z, X), parent(Z, Y), X \= Y.
