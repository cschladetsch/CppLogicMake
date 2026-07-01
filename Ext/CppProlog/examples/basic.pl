% Basic Prolog example program
likes(mary, food).
likes(mary, wine).
likes(john, wine).
likes(john, mary).

happy(X) :- likes(X, wine).
friends(mary, john) :- likes(mary, wine), likes(john, wine).