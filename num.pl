num(z).
num(s(X)) :- num(X).

le(z, X) :- num(X).
le(s(X), s(Y)) :- le(X, Y).
lt(z, s(X)) :- num(X).
lt(s(X), s(Y)) :- lt(X, Y).

le(s(z), s(s(s(z))))? % yes
le(s(s(z)), s(z))? % no
le(s(s(z)), s(s(z)))? % yes
lt(s(z), s(s(s(z))))? % yes
lt(s(s(z)), s(z))? % no
lt(s(s(z)), s(s(z)))? % no

plus(z, X, X) :- num(X).
plus(s(X), Y, s(Z)) :- plus(X, Y, Z).

plus(s(z), s(s(z)), s(s(s(z))))? % yes
plus(s(z), s(s(z)), s(s(z)))? % no
plus(s(s(s(z))), s(s(z)), X)? % X = s(s(s(s(s(z)))))

times(z, X, z).
times(s(X), Y, Z) :- times(X, Y, XY), plus(XY, Y, Z).

times(z, s(s(z)), z)? % yes
times(s(s(z)), s(s(s(z))), s(s(s(s(s(s(z)))))))? % yes
times(s(s(z)), s(s(s(z))), s(s(s(s(s(z))))))? % no
times(s(s(z)), s(s(s(z))), X)? % X = s(s(s(s(s(s(z))))))
