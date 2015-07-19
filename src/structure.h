#pragma once

//===	Includes

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <pbc.h>

#include "kv/backend.h"
#include "xm.h"
#include "constants.h"

//===	Defines

#define	STRUCTURE_TYPE_NULL	(0x00)	// invalid - we should never see this
#define	STRUCTURE_TYPE_SUB	(0x01)	// substructure
#define	STRUCTURE_TYPE_I64	(0x02)	// 64b signed integer
#define	STRUCTURE_TYPE_U64	(0x03)	// 64b unsigned integer
#define	STRUCTURE_TYPE_H64	(0x04)	// 64b translate to hex (ie: 0x'...')
#define	STRUCTURE_TYPE_F64	(0x05)	// 64b IEEE 754 double float
#define	STRUCTURE_TYPE_BLOB	(0x06)	// variable length binary blob
#define	STRUCTURE_TYPE_STRING	(0x07)	// variable length null terminated string
#define	STRUCTURE_TYPE_TIME	(0x08)	// 64b packed YMDhms time
#define	STRUCTURE_TYPE_UNIXTIME	(0x09)	// 64b signed unix time

//===	Structures

typedef struct {
	int64_t		year	:35;
	uint64_t	month	:4;
	uint64_t	day	:5;
	uint64_t	hour	:5;
	uint64_t	minute	:6;
	uint64_t	second	:6;
} structure_time;

typedef struct structure_struct {
	uint8_t	*key;
	union {
		int64_t			i64;
		uint64_t		u64;
		uint64_t		h64;
		double			f64;
		uint8_t			*blob;
		uint8_t			*string;
		structure_time		time;
		int64_t			unixtime;
		struct structure_struct	*children;
	};
	union {
		uint16_t	len;	// blob + string (includes NULL terminator)
		uint16_t	count;	// children
	};
	uint8_t		type;
} structure;

//===	Special

static inline int cmp_structure (const void *arg1, const void *arg2) {
	const structure *a = arg1;
	const structure *b = arg2;
	return strcmp (a->key, b->key);	// compare keys
}

#define	SORT_NAME structure
#define	SORT_TYPE structure
#define	SORT_CMP(x, y)	cmp_structure (&x, &y)
#include "../deps/sort/sort.h"

//===	Functions

static inline void StructureSortChildren (structure *s) {
	structure_tim_sort (s->children, s->count);
	return;
}

int StructureInitialize	(void);
void StructureTerminate	(void);

int StructureAddChildren	(structure *parent, const structure *children, int count);
void StructureFree		(structure *s);

kv_string StructurePack		(const structure *s);
structure *StructureUnpack	(const kv_string packed);
