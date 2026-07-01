% Family tree program

% Family facts - parents(Parent, Child)
parent(tom, bob).
parent(tom, liz).
parent(bob, ann).
parent(bob, pat).
parent(pat, jim).
parent(liz, sue).
parent(liz, joe).

% Gender facts
male(tom).
male(bob).
male(jim).
male(joe).
female(liz).
female(ann).
female(pat).
female(sue).

% Family relationship rules
father(X, Y) :- parent(X, Y), male(X).
mother(X, Y) :- parent(X, Y), female(X).

grandparent(X, Z) :- parent(X, Y), parent(Y, Z).
grandfather(X, Z) :- grandparent(X, Z), male(X).
grandmother(X, Z) :- grandparent(X, Z), female(X).

sibling(bob, liz).
sibling(liz, bob).
sibling(ann, pat).
sibling(pat, ann).
sibling(sue, joe).
sibling(joe, sue).
brother(X, Y) :- sibling(X, Y), male(X).
sister(X, Y) :- sibling(X, Y), female(X).

uncle(X, Y) :- brother(X, Z), parent(Z, Y).
aunt(X, Y) :- sister(X, Z), parent(Z, Y).

cousin(X, Y) :- parent(P1, X), parent(P2, Y), sibling(P1, P2).

ancestor(X, Y) :- parent(X, Y).
ancestor(X, Z) :- parent(X, Y), ancestor(Y, Z).

descendant(X, Y) :- ancestor(Y, X).