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

//===	Variables

xm_tlv	int32_t pd_arena;

//===	Functions

// require arena specification
xmattr_malloc void *pd_emalloc	(size_t size, int32_t pd_arena);
xmattr_malloc void *pd_ecalloc	(size_t count, size_t size, int32_t pd_arena);
void *pd_erealloc		(void *ptr, size_t size, int32_t pd_arena);
void pd_efree			(void *ptr, int32_t pd_arena);

// just like normal malloc() family functions
xmattr_malloc static inline void *pd_malloc (size_t size) {
	return pd_emalloc (size, pd_arena);
}

xmattr_malloc static inline void *pd_calloc (size_t count, size_t size) {
	return pd_ecalloc (count, size, pd_arena);
}

void *pd_realloc (void *ptr, size_t size) {
	return pd_erealloc (ptr, size, pd_arena);
}

void pd_free (void *ptr) {
	return pd_efree (ptr, pd_arena);
}
