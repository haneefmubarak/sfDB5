#pragma once
// adapted from
// http://git.io/pgkqvw
// (haneefmubarak/experiments/rendezvous)

//===	Includes

#include <stdint.h>
#include <stdlib.h>

//===	Structures

typedef struct {
	__uint128_t val;
	int bin;
} rdv;

//===	Special

static inline int cmp_rdv (const void *arg1, const void* arg2) {
	const rdv* lhs = arg1;
	const rdv* rhs = arg2;
	return (lhs->val < rhs->val) ? -1 : (rhs->val < lhs->val);
}

#define SORT_NAME rendezvous
#define SORT_TYPE rdv
#define SORT_CMP(x, y) cmp_rdv (&x, &y)
#include "sort.h"

//===	Functions

static inline int *rendezvous (__uint128_t *pool, uint64_t id, int nodes) {
	register int x;

	// store temporary results
	rdv *cache	= malloc (nodes * sizeof (rdv));
	if (!cache)
		return NULL;
	int *bin	= malloc (nodes * sizeof (int));
	if (!bin) {
		free (cache);
		return NULL;
	}

	// calculate for each server
	for (x = 0; x < nodes; x++) {
		cache[x].val	= pool[x] * ~id;
		cache[x].bin	= x;
	}

	// sort the results
	rendezvous_tim_sort (cache, nodes);

	// extract the results
	for (x = 0; x < nodes; x++) {
		bin[x] = cache[x].bin;
	}

	// cleanup
	free (cache);

	return bin;
}
