// adapted from
// http://git.io/pgkqvw
// (haneefmubarak/experiments/rendezvous)

#include <stdint.h>

#ifndef MAX
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#endif

static inline int rendezvous (__uint128_t *pool, uint64_t id, int nodes) {
	register int x, bin = 0;
	register __uint128_t max = 0, cache;

	for (x = 0; x < nodes; x++) {
		cache = pool[x] * ~id;
		max = MAX (max, cache);
		bin = max == cache ? x : bin;
	}

	return bin;
}
