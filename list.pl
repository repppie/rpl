append([], X, X).
append([X|Xs], Y, [X|Z]) :- append(Xs, Y, Z).

append([a], [b], [a])? % no
append([a], [b], [a,b])? % yes
append([a], [b], X)? % X = [a, b]
append([a,b,c], [d,e,f], X)? % X = [a, b, c, d, e, f]

member(X, [X|Xs]).
member(X, [Y|Ys]) :- member(X, Ys).

member(a, [b])? % no
member(a, [c,b,a])? % yes
member(a, [])? % no
