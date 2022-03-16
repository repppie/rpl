append([], X, X).
append([X|Xs], Y, [X|Z]) :- append(Xs, Y, Z).

qsort([X|Xs], Ys) :- partition(Xs, X, L, R),
    qsort(L, Ls),
    qsort(R, Rs),
    append(Ls, [X|Rs], Ys).
qsort([], []).
 
partition([X|Xs], Y, [X|Ls], Rs) :- X <= Y, partition(Xs, Y, Ls, Rs).
partition([X|Xs], Y, Ls, [X|Rs]) :- X > Y, partition(Xs, Y, Ls, Rs).
partition([], Y, [], []).
 
qsort([2,6,4,5,1,3], X)? % X = [1,2,3,4,5,6]
