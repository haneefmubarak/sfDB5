#include <stdio.h>
#include <assert.h>
#include "../src/rendezvous.h"

#define	NODES	20
#define	REPFAC	4
#define	KEYS	16777216
#define	CSKIP	1048576

__uint128_t hashes[NODES]	= {0};
int count[REPFAC][NODES]	= {{0}};

int main (int argc, char **argv) {
	assert (fread (hashes, sizeof (hashes[0]), NODES, stdin) == NODES);

	register uint64_t x, y;
	register int *bin;
	for (x = 0; x < KEYS; x++) {
		bin = rendezvous (hashes, x, NODES);
		for (y = 0; y < REPFAC; y++)
			count[y][bin[y]] += 1;
		free (bin);
		if (!(x % CSKIP)) {fprintf (stdout, "%llu\n", x);}
	}

	for (x = 0; x < REPFAC; x++) {
		fputs ("\n\n\n", stdout);
		fprintf (stdout, "RepSet %llu\n", x);
		for (y = 0; y < NODES; y++)
			fprintf (stdout, "\tBin %llu\t: %i\n", y, count[x][y]);
	}

	return 0;
}
