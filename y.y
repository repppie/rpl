%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "rpl.h"

#define YYSTYPE struct node *
%}
%token ATOM VAR NUM LIST ASSIGN IS LE GE

%left IS
%left '+' '-'
%left '*' '/'

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
conjuction : term | conjuction ',' term {
		struct node *n;
		$$ = $1;
		for (n = $$; n->next != NULL; n = n->next);
		n->next = $3;
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
term : functor | list | atom | var | num | term IS expr {
		$$ = new_node(ATOM);
		$$->name = "(is)";
		$$->arity = 2;
		$$->args = $1;
		$1->next = $3;
	} | term '<' expr {
		$$ = new_node(ATOM);
		$$->name = "<";
		$$->arity = 2;
		$$->args = $1;
		$1->next = $3;
	} | term '>' expr {
		$$ = new_node(ATOM);
		$$->name = ">";
		$$->arity = 2;
		$$->args = $1;
		$1->next = $3;
	} | term LE expr {
		$$ = new_node(ATOM);
		$$->name = "<=";
		$$->arity = 2;
		$$->args = $1;
		$1->next = $3;
	} | term GE expr {
		$$ = new_node(ATOM);
		$$->name = ">=";
		$$->arity = 2;
		$$->args = $1;
		$1->next = $3;
	}
expr : num | var | expr '+' expr {
		$$ = new_node(ATOM);
		$$->name = "+";
		$$->arity = 2;
		$$->args = $1;
		$1->next = $3;
	} | expr '-' expr {
		$$ = new_node(ATOM);
		$$->name = "-";
		$$->arity = 2;
		$$->args = $1;
		$1->next = $3;
	} | expr '*' expr {
		$$ = new_node(ATOM);
		$$->name = "*";
		$$->arity = 2;
		$$->args = $1;
		$1->next = $3;
	} | expr '/' expr {
		$$ = new_node(ATOM);
		$$->name = "-";
		$$->arity = 2;
		$$->args = $1;
		$1->next = $3;
	} 
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
num : NUM {
		$$ = new_node(NUM);
		$$->val = (long)yylval;
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
		if (!strcmp(yytext, "is"))
			return (IS);
		return (ATOM);
	} else if (c >= 'A' && c <= 'Z') {
		do {
			yytext[yyleng++] = c;
			c = getchar();
		} while (isalpha(c) || (c >= '0' && c <= '9'));
		yytext[yyleng] = '\0';
		ungetc(c, stdin);
		return (VAR);
	} else if (c >= '0' && c <= '9') {
		long num;

		num = 0;
		do {
			num = 10 * num + c - '0';
			c = getchar();
		} while (c >= '0' && c <= '9');
		yylval = (struct node *)num;
		ungetc(c, stdin);
		return (NUM);
	} else if (c == ':') {
		int c2;

		c2 = getchar();
		if (c2 == '-')
			return (ASSIGN);
		else
			ungetc(c2, stdin);
	} else if (c == '<') {
		int c2;

		c2 = getchar();
		if (c2 == '=')
			return (LE);
		else
			ungetc(c2, stdin);
	} else if (c == '>') {
		int c2;

		c2 = getchar();
		if (c2 == '=')
			return (GE);
		else
			ungetc(c2, stdin);
	}
	return (c);
}
