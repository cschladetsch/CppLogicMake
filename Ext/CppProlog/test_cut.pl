
p(X) :- q(X), !.
p(c).

q(a).
q(b).

r(X) :- s(X), !.
r(c).

s(a).
s(b).

t(x).
t(y) :- !.
t(z).

u(a,b).
u(c,d) :- !.
u(e,f).

v(X,Y) :- w(X), !, x(Y).
w(a).
w(b).
x(c).
x(d).

a(1).
a(2).
b(1).
b(2).
c(X,Y) :- a(X), !, b(Y).
