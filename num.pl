num(a).
num(s(X)) :- num(X).

le(a, X) :- num(X).
le(s(X), s(Y)) :- le(X, Y).
lt(a, s(X)) :- num(X).
lt(s(X), s(Y)) :- lt(X, Y).

le(s(a), s(s(s(a))))? % yes
le(s(s(a)), s(a))? % no
le(s(s(a)), s(s(a)))? % yes
lt(s(a), s(s(s(a))))? % yes
lt(s(s(a)), s(a))? % no
lt(s(s(a)), s(s(a)))? % no

plus(a, X, X) :- num(X).
plus(s(X), Y, s(Z)) :- plus(X, Y, Z).

plus(s(a), s(s(a)), s(s(s(a))))? % yes
plus(s(a), s(s(a)), s(s(a)))? % no
plus(s(s(s(a))), s(s(a)), X)? % X = s(s(s(s(s(a)))))

times(a, X, a).
times(s(X), Y, Z) :- times(X, Y, XY), plus(XY, Y, Z).

times(a, s(s(a)), a)? % yes
times(s(s(a)), s(s(s(a))), s(s(s(s(s(s(a)))))))? % yes
times(s(s(a)), s(s(s(a))), s(s(s(s(s(a))))))? % no
times(s(s(a)), s(s(s(a))), X)? % X = s(s(s(s(s(s(a))))))
