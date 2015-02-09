#pragma once

//===	Includes

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include "xm.h"
#include "hex.h"

//===	Defines

#define	QUERY_OPER_ERROR	0x00	// BOOM! - you should never see this
#define	QUERY_OPER_INVALID	0x01	// invalid
#define	QUERY_OPER_SUB		0x02	// .	syntax subkey or substructure
#define	QUERY_OPER_DEREFERENCE	0x03	// ->	syntax structure
#define	QUERY_OPER_CAST		0x04	// :	syntax cast
#define QUERY_OPER_ARRAY	0x05	// [0x]	syntax array

//===	Structures

typedef struct query_struct_token {
	struct query_struct_token	*next;
	union {
		uint8_t		*token;
		uint64_t	index;
	};
	int	oper;
} query_token;

//===	Functions

void QueryTokenFree		(query_token *head);	// frees all the way down the chain
query_token *QueryParse		(const uint8_t *query);	// assumes NULL termination

static inline query_token *QuerySafeParse (const uint8_t *query, int len) {
	if (len > 4096)	// lets be reasonable here, no one NEEDS a full PAGE
		return NULL;

	const uint8_t *q = xm_strndupa (query, len);	// thread stack is usually 2M
	return QueryParse (q);
}
