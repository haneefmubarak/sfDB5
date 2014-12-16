#include "query.h"

static uint8_t *__internal_query_subparse (const uint8_t *query, int *pos, int *op, int len);

query_token *query_parse (const uint8_t *query, const uint8_t **vars, int varcount) {
	int len = strlen (query);
	if (len < varcount)
		return NULL;	// bad string

	query_token *tokens = calloc (varcount, sizeof (query_token));
	if (!tokens)
		return NULL;	// insufficient memory

	int x, pos = 0, oper;
	for (x = 0; oper != QUERY_OPER_END; x++) {	// traverse the string
		oper = 0;
		tokens[x].token = __internal_query_subparse (query, &pos, &oper, len);

		if	(!tokens[x].token	|| \
			oper == QUERY_OPER_ERROR	|| \
			oper == QUERY_OPER_INVALID) {
			// error in subparse; cleanup and return

			int y;
			for (y = 0; y <= x; y++)
				free (tokens[x].token);
			free (tokens);
			return NULL;
		}

		tokens[x].oper = oper;
	}

	return tokens;
}

static uint8_t *__internal_query_subparse (const uint8_t *query, int *pos, int *op, int len) {
	int x;
	int oper = 0;
	for (x = *pos; !oper; x++) {	// string traversal
		switch (query[x]) {
			case 128 ... 255: {	// invalid ASCII chars
				oper = QUERY_OPER_INVALID;
				break;
			}
			case '.': {
				oper = QUERY_OPER_SUB;
				break;
			}
			case '-': {
				if ((x + 1) < len)	// ensure we don't SEGFAULT
					if (query[x + 1] == '>')
						oper = QUERY_OPER_DEREFERENCE, x++;
				break;
			}
			case '@': {
				oper = QUERY_OPER_CAST;
				break;
			}
			default: {
				break;
			}
		}
		if ((x + 1) > len)
			oper = QUERY_OPER_END, x++;	// 'x++' allows case fallthrough below
	}

	uint8_t *r;
	switch (oper) {
		case QUERY_OPER_INVALID: {
			return NULL;
		}
		case QUERY_OPER_SUB:	// copying of string is identical for these ops
		case QUERY_OPER_CAST:
		case QUERY_OPER_END: {
			r = strndup (&query[*pos], x - *pos);	// handle cleanup on return
			break;
		}
		case QUERY_OPER_DEREFERENCE: {
			r = strndup (&query[*pos], x - (*pos + 1));	// handle dual char oper
			break;
		}
		default: {
			// there shouldn't be a way to get here
			assert (!"__internal_query_subparse (): case default should be unreachable");
		}
	}

	// set vars and return
	*pos	= x;
	*op	= oper;
	return r;
}
