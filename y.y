%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "rpl.h"

#define YYSTYPE struct node *
%}
%token ATOM VAR TAIL ASSIGN

%%

goal : clause {
		yynode = $1;
		YYACCEPT;
	}
clause : fact | query | rule
rule : functor ASSIGN conjuction '.' {
		$$ = $1;
		$$->type = ASSIGN;
		$$->body = $3;
	}
fact : functor '.' {
		$$ = $1;
		$$->type = FACT;
	}
query : conjuction '?' {
		$$ = $1;
		$$->type = QUERY;
	}
conjuction : functor | conjuction ',' functor {
		$$ = $1;
		$$->next = $3;
	}
functor : atom '(' args ')' {
		$$ = $1;
		$$->args = $3;
		$$->arity = $3->_num_args;
		$3->_num_args = 0;
	}
args : term {
		$$ = $1;
		$$->_num_args = 1;
	} | args ',' term {
		struct node *n;
		$$ = $1;
		$$->_num_args++;
		for (n = $$; n->next != NULL; n = n->next);
		n->next = $3;
	} | ;
term : functor | list | atom | var
list : '[' elems ']' {
		$$ = $2;
	} | '[' elems '|' tail ']' {
		struct node *n;
		$$ = $2;
		for (n = $$; n->tail != NULL; n = n->tail);
		n->tail = $4;
	} | '[' ']' {
		$$ = new_node(LIST);
	}
elems : term {
		$$ = new_node(LIST);
		$$->args = $1;
	} | elems ',' term {
		struct node *n;
		$$ = $1;
		for (n = $$; n->tail != NULL; n = n->tail);
		n->tail = new_node(LIST);
		n->tail->args = $3;
	}
tail : list {
		$$ = $1;
	} | var {
		$$ = new_node(LIST);
		$$->args = $1;
		$$->is_tail = 1;
	}
atom : ATOM {
		$$ = new_node(ATOM);
		$$->name = strdup(yytext);
	}
var : VAR {
		$$ = new_node(VAR);
		$$->name = strdup(yytext);
	}

%%
 
static void
yyerror(char *s)
{
}

int
yylex(void)
{
	int c;

	yyleng = 0;
	do {
		c = getchar();
		if (c == '%')
			while ((c = getchar()) != '\n');
	} while (c == ' ' || c == '\t' || c == '\n');

	if (c >= 'a' && c <= 'z') {
		do {
			yytext[yyleng++] = c;
			c = getchar();
		} while (isalpha(c) || (c >= '0' && c <= '9'));
		yytext[yyleng] = '\0';
		ungetc(c, stdin);
		return (ATOM);
	} else if (c >= 'A' && c <= 'Z') {
		do {
			yytext[yyleng++] = c;
			c = getchar();
		} while (isalpha(c) || (c >= '0' && c <= '9'));
		yytext[yyleng] = '\0';
		ungetc(c, stdin);
		return (VAR);
	} else if (c == ':') {
		int c2;

		c2 = getchar();
		if (c2 == '-')
			return (ASSIGN);
		else
			ungetc(c2, stdin);
	}
	return (c);
}
