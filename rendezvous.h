#pragma once
// adapted from
// http://git.io/pgkqvw
// (haneefmubarak/experiments/rendezvous)

#include <stdint.h>
#include <stdlib.h>

#ifndef MAX
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#endif

typedef struct {
	__uint128_t val;
	int bin;
} rdv;

#define SORT_NAME rendezvous
#define SORT_TYPE rdv
#define SORT_CMP(x, y) ((((rdv)x).val) - (((rdv)y).val))
#include "./deps/sort/sort.h"

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
