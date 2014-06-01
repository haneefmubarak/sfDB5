#pragma once

#include <stdint.h>

#define ARENA_MAX_READER_COUNT 16

#define ARENA_OP_NONE	0
#define ARENA_OP_READER	1
#define ARENA_OP_WRITER	2

typedef struct {
	int access;
	struct {
		uint8_t w;
		uint8_t r;
	} wait;
	int8_t	reader_count;
	uint8_t	op;
} arena_lock;

int lock_reader (arena_lock *);	// shared, lo prio
int lock_writer (arena_lock *);	// exclsv, hi prio

int unlock_reader (arena_lock *);	// shared, lo prio
int unlock_writer (arena_lock *);	// exclsv, hi prio
