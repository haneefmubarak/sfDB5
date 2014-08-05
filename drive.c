#include "drive.h"

xmattr_malloc uint8_t *load (size_t len, char *path, int *fd, int *err) {
	err[0] = 0;	// stat errors
	err[1] = 0;	// open / lock / mmap errors

	struct stat statbuf;
	stat (path, &statbuf);

	// checks
	if (xm_unlikely (statbuf.st_uid != getuid ()))
		err[0] |= 0x01;	// bad user
	if (xm_unlikely (!(S_ISREG (statbuf.st_mode) || S_ISBLK (statbuf.st_mode))))
		err[0] |= 0x02;	// not a regular or block file
	if (xm_unlikely (access (path, R_OK | W_OK) || !access (path, X_OK)))
		err[0] |= 0x04;	// bad permissions
	if (xm_unlikely (statbuf.st_size < (typeof (statbuf.st_size)) len))
		err[0] |= 0x08;	// file size is greater than given size
	if (xm_unlikely (len % DRIVE_MULSZ))
		err[0] |= 0x16;	// given size is not a multiple of constant
	if (xm_unlikely (len < DRIVE_MINSZ))
		err[0] |= 0x32;	// given size is less than minimum size

	if (xm_unlikely (err[0]))
		return NULL;

	// open and lock
	*fd = open (path, O_RDWR);
	if (xm_unlikely (*fd <= 0))
		err[1] |= 0x01;	// could not open the file
	if (xm_unlikely (flock (*fd, LOCK_EX | LOCK_NB)))
		err[1] |= 0x02;	// could not obtain an exclusive lock

	if (xm_unlikely (err[1]))
		return NULL;

	// mmap
	uint8_t *map = mmap (NULL, len, PROT_READ | PROT_WRITE, MAP_FILE | MAP_SHARED, *fd, 0);
	if (xm_unlikely ((map == MAP_FAILED) || (map == NULL)))
		err[1] |= 0x04;	// could not map the file correctly

	if (xm_unlikely (err[1]))
		return NULL;

	return map;
}

void unload (size_t len, uint8_t *map, int fd) {
	// unmap
	munmap (map, len);

	// unlock
	flock (fd, LOCK_UN);

	// close
	close (fd);

	return;
}

//---

typedef union {
	size_t	len;
	uint8_t	*map;
} initArgs;

static void * tzero (void *ptr) {
	initArgs *args = ptr;

	memset (args->map, 0x00, args->len);	// zero memory

	return NULL;
}

int initialize (size_t len, uint8_t *map) {
	int err = 0;

	// checks
	if (xm_unlikely (len % DRIVE_MULSZ))
		err |= 0x16;	// given size is not a multiple of constant
	if (xm_unlikely (len < DRIVE_MINSZ))
		err |= 0x32;	// given size is less than minimum size

	if (xm_unlikely (err))
		return err;

	// zero out the image
	int cpus = sysconf (_SC_NPROCESSORS_ONLN);	// get number of cpus
	cpus /= 2;	// don't use all of the cpus
	while (len % cpus)
		cpus--;	// get a number we can divide the drive into

	int chunklen = len / cpus;	// get the length of each thread's chunk

	pthread_t thread[cpus];
	initArgs args[cpus];
	int status = 0, x;
	for (x = 0; x < cpus; x++) {
		args[x].len = chunklen;
		args[x].map = &map[x * chunklen];	// assign a range for the thread to zero
		status |= pthread_create (&thread[x], NULL, tzero, &args[x]);
	}

	if (xm_unlikely (status))	// thread creation failed somewhere
		memset (map, 0x00, len);	// do it ourselves
	else		// expected operation
		for (x = 0; x < cpus; x++)
			pthread_join (thread[x], NULL);	// wait for threads to complete execution

	// add magic to files
	const int arenalen = len / 256;
	for (x = 0; x < 256; x++)
		strcpy ((char *) &map[x * arenalen], "sfDB5v0.dev");

	return 0;
}
