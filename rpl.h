#ifndef _RPL_H
#define _RPL_H

enum {
	ATOM = 257,
	VAR,
	LIST,
	ASSIGN,
	FACT,
	QUERY,
};

struct node {
	struct node *next;
	struct node *args;
	union {
		struct node *body;
		struct node *subst;
		struct node *tail;
	};
	char *name;
	int type;
	int arity;
	int is_tail;
	union {
		int _num_args;
		int gen;
	};
};

extern char yytext[];
extern int yyleng;
extern struct node *yynode;

struct node *new_node(int type);

#endif
