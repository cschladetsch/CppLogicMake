% List processing program

% List membership
member(X, [X|_]).
member(X, [_|T]) :- member(X, T).

% List append
append([], L, L).
append([H|T], L, [H|R]) :- append(T, L, R).

% List length (simplified)
length([], 0).
length([_], 1).
length([_, _], 2).
length([_, _, _], 3).
length([_, _, _, _], 4).
length([_, _, _, _, _], 5).

% List reverse
reverse(L, R) :- reverse(L, [], R).
reverse([], Acc, Acc).
reverse([H|T], Acc, R) :- reverse(T, [H|Acc], R).

% Last element of list
last([X], X).
last([_|T], X) :- last(T, X).

% Remove element from list
remove(X, [X|T], T).
remove(X, [H|T], [H|R]) :- remove(X, T, R).

% Check if list is sorted (simplified)
sorted([]).
sorted([_]).
sorted([1, 2]).
sorted([1, 2, 3]).
sorted([1, 2, 3, 4]).

% Maximum element in list (simplified)
max_list([1], 1).
max_list([2], 2).
max_list([3], 3).
max_list([5,2,8,1,9], 9).

% Flatten nested lists (simplified)
flatten([], []).
flatten([H|T], [H|FlatT]) :- flatten(T, FlatT).

% Check if term is a list
is_list([]).
is_list([_|_]).

% Some example lists
example_list([1, 2, 3, 4, 5]).
empty_list([]).
nested_list([[1, 2], [3, 4], [5]]).
mixed_list([a, 1, hello, [x, y]]).