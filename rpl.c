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
	if (o->name)
		n->name = strdup(o->name);
	if (!one)
		n->next = _dup_node(o->next, 0);
	else
		n->next = NULL;

	return (n);
}

#if 0
static struct node *
dup_nodes(struct node *o)
{
	return (_dup_node(o, 0));
}
#endif

static struct node *
dup_node(struct node *o)
{
	return (_dup_node(o, 1));
}

static void
_free_node(struct node *n, int one)
{
	if (n == NULL)
		return;
	_free_node(n->body, 0);
	_free_node(n->args, 0);
	if (!one)
		_free_node(n->next, 0);
	free(n->name);
	free(n);
}

static void
free_node(struct node *n)
{
	_free_node(n, 1);
}

static void
free_nodes(struct node *n)
{
	_free_node(n, 0);
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
add_var(struct node **vars, struct node *orig, struct node *subst)
{
	struct node *v;

	v = new_node(VAR);
	v->subst = dup_node(subst);
	v->next = *vars;
	if (orig->name)
		v->name = strdup(orig->name);
	v->gen = orig->gen;
	*vars = v;
}

static int
unify(struct node *a, struct node *b, struct node **vars)
{
	struct node *aa, *bb, *n, *v;
	int ret;

	if (a && b && (a->type == ATOM || a->type == ASSIGN) && b->type ==
	    ATOM && a->arity == b->arity) {
		if (strcmp(a->name, b->name))
			return (0);
		if (a->arity == 0)
			return (1);
		for (aa = a->args, bb = b->args; aa; aa = aa->next, bb =
		    bb->next)
			if (!unify(aa, bb, vars))
				return (0);
		return (1);
	} else if (a && b && a->type == LIST && b->type == LIST) {
		for (aa = a, bb = b; aa && bb; aa = aa->tail, bb = bb->tail) {
			if (!aa->args && !bb->args)
				return (1);
			if (aa->is_tail)
				return (unify(aa->args, bb, vars));
			if (bb->is_tail)
				return (unify(aa, bb->args, vars));
			if (!aa->args || !bb->args)
				return (0);
			if (!unify(aa->args, bb->args, vars))
				return (0);
		}
		if (aa && aa->is_tail) {
			n = new_node(LIST);
			ret = unify(aa->args, n, vars);
			free_node(n);
			return (ret);
		}
		if (bb && bb->is_tail) {
			n = new_node(LIST);
			return (unify(n, bb->args, vars));
			free_node(n);
			return (ret);
		}
		if (aa || bb)
			return (0);
		return (1);
	} else if (a && a->type == VAR) {
		if ((v = find_var(*vars, a->name, a->gen)) != NULL)
			return (unify(v->subst, b, vars));
		add_var(vars, a, b);
		return (1);
	} else if (b && b->type == VAR) {
		if ((v = find_var(*vars, b->name, b->gen)) != NULL)
			return (unify(a, v->subst, vars));
		else {
			add_var(vars, b, a);
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
	} else if (rule->type == LIST)
		for (n = rule; n; n = n->tail)
			rename_vars(n->args, gen);
	for (n = rule->args; n; n = n->next)
		rename_vars(n, gen);
	for (n = rule->body; n; n = n->next)
		rename_vars(n, gen);
}

static int
answer(struct node *res, struct node **vars)
{
	struct node *f, *_f, *old_vars, *v, *vv;
	int ret;

	if (res == NULL)
		return (1);

	old_vars = *vars;
	for (f = facts; f; f = f->next) {
		_f = dup_node(f);
		rename_vars(_f, var_gen++);
		if (!unify(_f, res, vars))
			goto no_match;
		if (_f->type == ASSIGN) /* rule */
			if (!answer(_f->body, vars))
				goto no_match;
		if (answer(res->next, vars)) { /* conjunction */
			ret = 1;
			free_node(_f);
			goto out;
		}
	no_match:
		free_node(_f);
		v = *vars;
		while (v != old_vars) {
			vv = v->next;
			free_node(v);
			v = vv;
		}
		*vars = old_vars;
	}

	ret = 0;
out:
	return (ret);
}

static void _print_node(struct node *n, struct node *vars, int one);

static void
print_list(struct node *n, struct node *vars)
{
	if (n == NULL)
		return;
	_print_node(n->args, vars, 1);
	if (n->tail) {
		printf("%s", n->tail->is_tail ? "|" : ", ");
		print_list(n->tail, vars);
	}
}

static void
_print_node(struct node *n, struct node *vars, int one)
{
	struct node *v;

	if (n == NULL)
		return;

	if (n->type == VAR) {
		v = find_var(vars, n->name, n->gen);
		if (v)
			_print_node(v->subst, vars, 1);
		else
			printf("%s%s", n->is_tail ? "|" : "", n->name);
	}
	if (n->type == LIST) {
		printf("[");
		print_list(n, vars);
		printf("]");
	}
	if (n->type == ATOM || n->type == ASSIGN)
		printf("%s", n->name);
	if (n->arity > 0) {
		printf("(");
		_print_node(n->args, vars, 0);
		printf(")");
	}
	if (!one && n->next) {
		printf(", ");
		_print_node(n->next, vars, 0);
	}
}

#if 0
static void
print_nodes(struct node *n, struct node *vars)
{
	_print_node(n, vars, 0);
}
#endif

static void
print_node(struct node *n, struct node *vars)
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
			print_node(v->subst, vars);
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
				free_node(yynode);
				free_nodes(vars);
			}
		} else {
			printf("syntax error\n");
			break;
		}
	}
	return (0);
}
