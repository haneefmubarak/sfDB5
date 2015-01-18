#include "query.h"

static uint8_t *__internal_query_subparse (const uint8_t *query, int *pos, int *op, int len);

query_token *QueryParse (const uint8_t *query, int len) {
	len = strnlen (query, xm_min(len, 1024));
	if (!QueryValidate (query, len))
		return NULL;

	query_token *tokens = calloc (128, sizeof (query_token));	// max tokens
	if (!tokens)
		return NULL;	// insufficient memory

	int x, pos = 0, oper = 0;
	for (x = 0; (oper != QUERY_OPER_END) && (x < 128); x++) {	// traverse the string
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

			case ':': {
				oper = QUERY_OPER_CAST;
				break;
			}

			case '[': {
				if ((x + 19) < len)	// prevent SEGFAULT
					if (QueryArrayValidate (&query[x], 20))
						oper = QUERY_OPER_ARRAY, x+= 19;
					else
						oper = QUERY_OPER_INVALID;
			}

			default: {
				break;
			}
		}

		if ((x + 1) > len)
			oper = QUERY_OPER_END, x++;	// 'x++' allows case fallthrough below
	}

	uint8_t *r = NULL;
	switch (oper) {
		case QUERY_OPER_INVALID: {
			return NULL;
		}

		case QUERY_OPER_SUB:	// copying of string is identical for these ops
		case QUERY_OPER_CAST:
		case QUERY_OPER_END:
		case QUERY_OPER_ARRAY: {
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

static int internal__query_validate (const uint8_t *query, int len, pcre *p, pcre_extra *s) {
	len = strnlen (query, xm_min(len, 1024));
	if (len >= 1024)	// too long
		return 0;

	int ovector[REGEX_OVECSIZE];
	int r = pcre_exec (p, s, query, len, 0,
				PCRE_NOTEMPTY | PCRE_NOTEMPTY_ATSTART,
				ovector, REGEX_OVECSIZE);
	r = (r <= 0) ? 0 : r;	// r is count of matches

	return r;
}

int QueryValidate (const uint8_t *query, int len) {
	return internal__query_validate (query, len, regex_query_full,
			regex_study_query_full);
}

int QueryStructureValidate (const uint8_t *query, int len) {
	return internal__query_validate (query, len, regex_query_structure,
			regex_study_query_structure);
}

int QueryArrayValidate (const uint8_t *query, int len) {
	return internal__query_validate (query, len, regex_query_array,
			regex_study_query_array);
}
