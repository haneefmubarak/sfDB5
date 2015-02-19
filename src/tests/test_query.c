#include "test.h"
#include "../query.h"

const char *query	= "head.child:cast.member[0x0123456789ABCDEF]->deref.sub.final";
const char *types[6]	= {
	[QUERY_OPER_ERROR]		= "error",
	[QUERY_OPER_INVALID]		= "invalid",
	[QUERY_OPER_SUB]		= "sub",
	[QUERY_OPER_DEREFERENCE]	= "dereference",
	[QUERY_OPER_CAST]		= "cast",
	[QUERY_OPER_ARRAY]		= "array"
};

static void printtoken (const query_token *t) {
	printf ("Type:\t%s\n", types[t->oper]);

	switch (t->oper) {
		case QUERY_OPER_SUB:
		case QUERY_OPER_DEREFERENCE:
		case QUERY_OPER_CAST: {
			printf ("Token:\t%s\n\n", t->token);
			break;
		}

		case QUERY_OPER_ARRAY: {
			printf ("Token:\t0x%" PRIX64 "\t(%" PRIi64 ")\n\n", t->index, t->index);
			break;
		}
	}
}

void test (void) {
	const query_token *head = QuerySafeParse (query, strlen (query));
	query_token *token = head;

	while (token != NULL) {
		printtoken (token);
		token = token->next;
	}

	QueryTokenFree (head);

	return;
}

#include "test.c"
