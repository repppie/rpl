father(abraham, isaac). male(isaac).
father(haran, lot). male(lot).
father(haran, milcah). female(milcah).
father(haran, yiscah). female(yiscah).
son(X, Y) :- father(Y, X), male(X).
daughter(X, Y) :- father(Y, X), female(X).

father(haran, lot)? % yes
father(haran, isaac)? % no
father(haran, yiscah), female(yiscah)? % yes
father(haran, yiscah), male(yiscah)? % no

son(lot, haran), male(lot)? % yes
son(lot, haran), male(yiscah)? % no
son(lot, abraham)? % no
