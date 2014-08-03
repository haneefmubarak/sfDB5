#pragma once

//===	Includes

#include <stdint.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "sfDB5.h"

//===	Defines

// the block size for the drive
#define DRIVE_BLKSZ	(VALUE_MAXLEN)
// whatever drive is used must be a multiple of this value
#define DRIVE_MULSZ	(256LL * DRIVE_BLKSZ)
// minimum size for the drive
#define DRIVE_MINSZ	(8LL * DRIVE_MULSZ)
