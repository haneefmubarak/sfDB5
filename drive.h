#pragma once

//===	Includes

#include <stdint.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/file.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "sfDB5.h"
#include "pd.h"

//===	Variables

extern		int8_t drive_count;

//===	Defines

// the block size for the drive
#define DRIVE_BLKSZ	(VALUE_MAXLEN)
// whatever drive is used must be a multiple of this value
#define DRIVE_MULSZ	(256LL * DRIVE_BLKSZ)
// minimum size for the drive
#define DRIVE_MINSZ	(8LL * DRIVE_MULSZ)

//=== Functions

// load a drive into memory
xmattr_malloc uint8_t *drive_load (size_t len, char *path, int *fd, int *err);
// unload a drive from memory
void drive_unload (size_t len, uint8_t *map, int fd);
// initialize a new drive
int drive_initialize (size_t len, uint8_t *map);
