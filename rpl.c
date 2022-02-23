#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include <ctype.h>
#include <assert.h>

#include "rpl.h"

char yytext[256];
int yyleng;

struct node *yynode;

static struct node *facts;
static struct node *last_fact;

struct node *
new_node(int type)
{
	struct node *n;

	n = malloc(sizeof(struct node));
	memset(n, 0, sizeof(struct node));
	n->type = type;

	return (n);
}

static struct node *
_dup_node(struct node *o, int one)
{
	struct node *n;

	if (o == NULL)
		return (NULL);
	n = malloc(sizeof(struct node));
	memcpy(n, o, sizeof(struct node));
	n->args = _dup_node(o->args, 0);
	n->body = _dup_node(o->body, 0);
	n->name = strdup(o->name);
	if (!one)
		n->next = _dup_node(o->next, 0);
	else
		n->next = NULL;

	return (n);
}

static struct node *
dup_nodes(struct node *o)
{
	return (_dup_node(o, 0));
}

static struct node *
dup_node(struct node *o)
{
	return (_dup_node(o, 1));
}

static void
free_nodes(struct node *n)
{
	if (n == NULL)
		return;
	free_nodes(n->next);
	free_nodes(n->args);
	free_nodes(n->body);
	free(n->name);
	free(n);
}

static struct node *
find_var(struct node *vars, char *name, int gen)
{
	struct node *v;

	if (vars == NULL)
		return (NULL);
	for (v = vars; v; v = v->next)
		if (!strcmp(v->name, name) && v->gen == gen)
			return (v);
	return (NULL);
}

static void
add_var(struct node **vars, char *name, int gen, struct node *s)
{
	struct node *v;

	v = new_node(VAR);
	v->subst = dup_node(s);
	v->next = *vars;
	v->name = strdup(name);
	v->gen = gen;
	*vars = v;
}

static int
unify(struct node *a, struct node *b, struct node **vars)
{
	struct node *aa, *bb, *v;

	if ((a->type == ATOM || a->type == ASSIGN) && b->type == ATOM &&
	    a->arity == b->arity) {
		if (strcmp(a->name, b->name))
			return (0);
		if (a->arity == 0)
			return (1);
		for (aa = a->args, bb = b->args; aa; aa = aa->next, bb =
		    bb->next)
			if (!unify(aa, bb, vars))
				return (0);
		return (1);
	} else if (a->type == VAR) {
		if ((v = find_var(*vars, a->name, a->gen)) != NULL)
			return (unify(v->subst, b, vars));
		add_var(vars, a->name, a->gen, b);
		return (1);
	} else if (b->type == VAR) {
		if ((v = find_var(*vars, b->name, b->gen)) != NULL)
			return (unify(a, v->subst, vars));
		else {
			add_var(vars, b->name, b->gen, a);
			return (1);
		}
	}

	return (0);
}

static int var_gen = 1;

static void
rename_vars(struct node *rule, int gen) {
	struct node *n;

	if (rule == NULL)
		return;

	if (rule->type == VAR) {
		rule->gen = gen;
		return;
	}
	for (n = rule->args; n; n = n->next)
		if (n->type == VAR)
			n->gen = gen;
		else
			rename_vars(n->args, gen);
	for (n = rule->body; n; n = n->next)
		rename_vars(n, gen);
}

static int
answer(struct node *res, struct node **vars)
{
	struct node *f, *_f, *old_vars;
	int ret;

	if (res == NULL)
		return (1);

	old_vars = *vars;
	for (f = facts; f; f = f->next) {
		_f = f;
		if (f->type == ASSIGN) {
			_f = dup_node(f);
			rename_vars(_f, var_gen++);
		}
		if (!unify(_f, res, vars))
			goto no_match;
		if (_f->type == ASSIGN) /* rule */
			if (!answer(_f->body, vars))
				goto no_match;
		if (answer(res->next, vars)) { /* conjunction */
			ret = 1;
			goto out;
		}
	no_match:
		if (_f != f)
			free_nodes(_f);
		*vars = old_vars; /* XXX leak */
	}

	ret = 0;
out:
	return (ret);
}

static void print_node(struct node *n, struct node *vars);
static void print_one(struct node *n, struct node *vars);

static void
_print_node(struct node *n, struct node *vars, int one)
{
	struct node *v;

	if (n->type == VAR) {
		v = find_var(vars, n->name, n->gen);
		if (v)
			print_one(v->subst, vars);
		else
			printf("%s", n->name);
		return;
	}
	if (n->type == ATOM || n->type == ASSIGN)
		printf("%s", n->name);
	if (n->arity > 0) {
		printf("(");
		print_node(n->args, vars);
		printf(")");
	}
	if (!one && n->next) {
		printf(", ");
		print_node(n->next, vars);
	}
}

static void
print_node(struct node *n, struct node *vars)
{
	_print_node(n, vars, 0);
}

static void
print_one(struct node *n, struct node *vars)
{
	_print_node(n, vars, 1);
}

static int
print_vars(struct node *res, struct node *vars)
{
	struct node *n, *v;
	int found;

	if (res == NULL)
		return (0);

	found = 0;
	for (n = res->args; n; n = n->next) {
		if (n->type == VAR) {
			v = find_var(vars, n->name, 0);
			assert(v);
			printf("%s = ", n->name);
			print_one(v->subst, vars);
			printf("\n");
			found = 1;
		} else
			found |= print_vars(n, vars);
	}
	found |= print_vars(res->next, vars);
	return (found);
}

int yyparse(void);

int
main(int argc, char **argv)
{
	while (1) {
		if (yyparse() == 0) {
			if (yynode->type == FACT || yynode->type == ASSIGN) {
				if (yynode->type == FACT)
					yynode->type = ATOM;
				if (facts)
					last_fact->next = yynode;
				else
					facts = yynode;
				last_fact = yynode;
			} else if (yynode->type == QUERY) {
				struct node *vars;
				int ret;

				yynode->type = ATOM;
				vars = NULL;
				ret = answer(yynode, &vars);
				if (ret) {
					if (!print_vars(yynode, vars))
						printf("yes\n");
				} else
					printf("no\n");
				//free_node(yynode);
			}
		} else {
			printf("syntax error\n");
			break;
		}
	}
	return (0);
}
