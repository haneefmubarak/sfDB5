#include "query.h"

void QueryTokenFree (query_token *head) {
	while (head != NULL) {
		switch (head->oper) {
			case QUERY_OPER_SUB:
			case QUERY_OPER_DEREFERENCE:
			case QUERY_OPER_CAST:
				free (head->token);
			// fallthrough

			case QUERY_OPER_ARRAY:
			default: {
				void *p = head;
				head = head->next;
				free (p);
			}
		}
	}

	return;
}

static inline int internal__query_isw (int c) {
	if (isalnum (c) || c == '_')
		return 1;
	return 0;
}

#define ALNUM()	{ \
	if (isalnum (query[p])) p++, w++; \
	else return NULL; \
}
#define WORDH(wordend)	{ \
	if (internal__query_isw (query[p])) p++, w++; \
	else if (query[p] == '.' && isalnum (query[p - 1])) goto wordend; \
	else goto cleanup; \
}
#define WORDS(wordend)	{ \
	if (internal__query_isw (query[p])) p++, w++; \
	else if ((query[p] == '.' || query[p] == ':' || query[p] == '[' || query[p] == '-' || query[p] == 0)&& isalnum (query[p - 1])) \
		goto wordend; \
	else goto cleanup; \
}
#define WORDC(wordend)	{ \
	if (internal__query_isw (query[p])) p++, w++; \
	else if ((query[p] == '.' || query[p] == '[' || query[p] == '-' || query[p] == 0)&& isalnum (query[p - 1])) \
		goto wordend; \
	else goto cleanup; \
}

query_token *QueryParse (const uint8_t *query) {	// NULL terminator is terminator
	int len = strlen (query);
	int p = 0, w, il, ol;
	query_token *head = NULL, *tail = NULL, *last = NULL;

	w = 0;
	ALNUM ();
	while (w < 255)
		WORDH (w0);
	return NULL;

	w0:
	head = malloc (sizeof (query_token));
	if (!head)
		return NULL;
	head->next = NULL;
	head->oper = QUERY_OPER_SUB;
	head->token = strndup (&query[p - w], w - 1);
	last = head;
	if (!head->token) {
		free (head);
		return NULL;
	}
	p++;

	il = 0;
	while (il < 16) {
		w = 0;
		ALNUM ();
		while (w < 255)
			WORDS (wisx);
		goto cleanup;

		wisx:
		tail = malloc (sizeof (query_token));
		if (!tail)
			goto cleanup;
		tail->next = NULL;
		tail->oper = QUERY_OPER_SUB;
		tail->token = strndup (&query[p - w], w - 1);
		if (!tail->token) {
			free (tail);
			goto cleanup;
		}
		last->next = tail;
		last = tail;
		tail = NULL;

		if (query[p] == ':') {
			p++;
			w = 0;
			ALNUM ();
			while (w < 255)
				WORDC (wicx);
			goto cleanup;

			wicx:
			tail = malloc (sizeof (query_token));
			if (!tail)
				goto cleanup;
			tail->next = NULL;
			tail->oper = QUERY_OPER_CAST;
			tail->token = strndup (&query[p - w], w - 1);
			if (!tail->token) {
				free (tail);
				goto cleanup;
			}
			last->next = tail;
			last = tail;
			tail = NULL;
		}

		if (query[p] == '[') {
			if (p + 18 > len)
				goto cleanup;
			if (query[p + 1] != '0' || query[p + 2] != 'x' || query[p + 18] != ']')
				goto cleanup;
			p += 3;

			tail = malloc (sizeof (query_token));
			if (!tail)
				goto cleanup;
			tail->next = NULL;
			tail->oper = QUERY_OPER_ARRAY;
			tail->index = HexToInt (&query[p]);

			p += 16;
		}

		if (query[p] == '-') {
			if (query[p + 1] != '>')
				goto cleanup;

			p++;
			w = 0;
			ALNUM ();
			while (w < 255)
				WORDH (widx);
			goto cleanup;

			widx:
			tail = malloc (sizeof (query_token));
			if (!tail)
				goto cleanup;
			tail->next = NULL;
			tail->oper = QUERY_OPER_DEREFERENCE;
			tail->token = strndup (&query[p - w], w - 1);
			if (!tail->token) {
				free (tail);
				goto cleanup;
			}
			last->next = tail;
			last = tail;
			tail = NULL;

			goto derefstart;
		}

		if (query[p] == 0)
			return head;

		il++;
		p++;
	}
	QueryTokenFree (head);
	return NULL;

	derefstart:
	ol = 0;
	deref:
	while (ol < 16) {
		il = 0;
		while (il < 16) {
			w = 0;
			ALNUM ();
			while (w < 255)
				WORDS (wosx);
			goto cleanup;

			wosx:
			tail = malloc (sizeof (query_token));
			if (!tail)
				goto cleanup;
			tail->next = NULL;
			tail->oper = QUERY_OPER_SUB;
			tail->token = strndup (&query[p - w], w - 1);
			if (!tail->token) {
				free (tail);
				goto cleanup;
			}
			last->next = tail;
			last = tail;
			tail = NULL;

			if (query[p] == ':') {
				p++;
				w = 0;
				ALNUM ();
				while (w < 255)
					WORDC (wocx);
				goto cleanup;

				wocx:
				tail = malloc (sizeof (query_token));
				if (!tail)
					goto cleanup;
				tail->next = NULL;
				tail->oper = QUERY_OPER_CAST;
				tail->token = strndup (&query[p - w], w - 1);
				if (!tail->token) {
					free (tail);
					goto cleanup;
				}
				last->next = tail;
				last = tail;
				tail = NULL;
			}

			if (query[p] == '[') {
				if (p + 18 > len)
					goto cleanup;
				if (query[p + 1] != '0' || query[p + 2] != 'x' || query[p + 18] != ']')
					goto cleanup;
				p += 3;

				tail = malloc (sizeof (query_token));
				if (!tail)
					goto cleanup;
				tail->next = NULL;
				tail->oper = QUERY_OPER_ARRAY;
				tail->index = HexToInt (&query[p]);

				p += 16;
			}

			if (query[p] == '-') {
				if (query[p + 1] != '>')
					goto cleanup;

				p++;
				w = 0;
				ALNUM ();
				while (w < 255)
					WORDH (wodx);
				goto cleanup;

				wodx:
				tail = malloc (sizeof (query_token));
				if (!tail)
					goto cleanup;
				tail->next = NULL;
				tail->oper = QUERY_OPER_DEREFERENCE;
				tail->token = strndup (&query[p - w], w - 1);
				if (!tail->token) {
					free (tail);
					goto cleanup;
				}
				last->next = tail;
				last = tail;
				tail = NULL;

				goto deref;
			}

			if (query[p] == 0)
				return head;

			il++;
			p++;
		}
		goto cleanup;
	}

	cleanup:
	QueryTokenFree (head);
	return NULL;
}
