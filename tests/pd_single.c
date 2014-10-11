// tests the functions in pd.h
//
// this file should fail when
// run on the 256th block,
// although I haven't a clue
// as to why

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <assert.h>

#include "../pd.h"

#define F_LEN	(1024 * 1024 * 1024)	// 1 GB
#define A_LEN	(F_LEN / 256)

int main (int argc, char **argv) {
	int x, y;

	// setup
	int fd;
	uint8_t *map;
	assert (fd = open ("./pd_single.testout", O_CREAT | O_RDWR | O_EXCL));
	if (ftruncate (fd, F_LEN)) {
		perror ("ftruncate failed: ");
		return 1;
	}
	assert (map = mmap (NULL, F_LEN, PROT_READ | PROT_WRITE, MAP_FILE | MAP_SHARED, fd, 0));

	uint8_t *arena[256];
	for (x = 0; x < 256; x++)
		arena[x] = map + (x * A_LEN);

	// test
	volatile int *var;
	void *list[512];

	pd_init (F_LEN, map);

	// per arena test
	for (x = 0; x < 256; x++) {
		pd_arena = arena[x];
		// allocate and write a few times
		for (y = 0; y < 256; y++) {
			assert ((list[y] = pd_mallocBK ()));
			var = list[y];
			*var = (x + 1) * (y + 1);
		}
		// free some but not all
		for (y = 0; y < 64; y++)
			pd_freeBK (list[y]);

		// now reallocate some and write some
		for (y = 0; y < 16; y++) {
			assert ((list[y] = pd_mallocBK()));
			var = list[y];
			*var = 16;
		}
	}

	// cleanup
	munmap (map, F_LEN);
	close (fd);

	return 0;
}
