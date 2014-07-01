#include <stdint.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

uint8_t *load (size_t len, char *path, int *fd, int *err) {
	err[0] = 0;	// stat errors
	err[1] = 0;	// open / lock / mmap errors

	const int permmask	= S_IRUSR | S_IWUSR | S_IXUSR;	// -rwx------
	const int perms		= S_IRUSR | S_IWUSR;		// -rw-------

	struct stat statbuf;
	stat (path, &statbuf);

	// checks
	if (statbuf.st_uid != getuid ())
		err[0] |= 0x01;	// bad user
	if (!(S_ISREG (statbuf.st_mode) || S_ISBLK (statbuf.st_mode))))
		err[0] |= 0x02;	// not a regular or block file
	if ((statbuf.st_mode & permmask) != perms)
		err[0] |= 0x04;	// bad permissions
	if (statbuf.st_size != len)
		err[0] |= 0x08;	// file size does not match given size
	if (len % 256)
		err[0] |= 0x16;	// file size is not a multiple of 256

	if (err[0])
		return NULL;

	// open and lock the file
	*fd = open (path, O_RDWR);
	if (fd <= 0)
		err[1] |= 0x01;	// could not open the file
	if (flock (*fd, LOCK_EX))
		err[1] |= 0x02;

	if (err[1])
		return NULL;
}
