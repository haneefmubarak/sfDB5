#include "test.h"
#include "../query.h"

const char *query = "head->child.cast:member[0x0123456789ABCDEF]";
const char *opers[7] = { "error", "invalid", "sub", "dereference", "cast", "array", "end" };

void printtoken (const query_token *t) {
	printf ("Token:\t%s\nType:\t%s\n\n", t->token, opers[t->oper]);
}

void test (void) {
	query_token *token = QueryParse (query, strlen (query));

	while (token != NULL) {
		printtoken (token);
		query_token *oldtoken = token;
		token = token->next;
		free (oldtoken);
	}

	return;
}

#include "test.c"
