#pragma once

//===	Includes

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "xm.h"
#include "regex.h"

//===	Defines

#define	QUERY_OPER_ERROR	0x00	// BOOM! - you should never see this
#define	QUERY_OPER_INVALID	0x01	// invalid
#define	QUERY_OPER_SUB		0x02	// .	syntax subkey or substructure
#define	QUERY_OPER_DEREFERENCE	0x03	// ->	syntax structure
#define	QUERY_OPER_CAST		0x04	// :	syntax cast
#define QUERY_OPER_ARRAY	0x05	// [0x]	syntax array
#define	QUERY_OPER_END		0x06	// no next token

//===	Structures

typedef struct query_struct_token {
	struct query_struct_token	*next;
	uint8_t	*token;
	int	oper;
} query_token;

//===	Functions

query_token *QueryParse		(const uint8_t *query, const uint8_t **vars, int varcount);
int QueryValidate		(const uint8_t *query, int len);
int QueryStructureValidate	(const uint8_t *query, int len);
int QueryArrayValidate		(const uint8_t *query, int len);
