#pragma once

//===	Includes

#include <stdint.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "sfDB5.h"
#include "mapped.h"

//===	Variables

extern xm_tlv	void *arena;

//===	Defines

// arena allocation block size
#define	ABLKLEN	4096	// 4K pagesize

//===	Functions

// initialize an arena
void pd_init (size_t len, uint8_t *map);

// require arena specification
xmattr_malloc void *pd_mallocBK	(void);
xmattr_malloc void *pd_callocBK	(void);
void pd_free			(void *ptr);
