#pragma once

//===	Includes

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "../xm.h"

//===	Defines

#define	KV_ERR_NPTR	1
#define	KV_ERR_INIT	2
#define KV_ERR_MEM	3
#define	KV_ERR_NOBATCH	4

//===	Types

typedef void kv_db_t;

typedef struct {
	union {
		int64_t	len;
		size_t	lens;
	};
	uint8_t		*data;
} kv_string;

//=== Variables
extern int xm_tlv kv_error;

//=== Functions

// startup / shutdown
// (global)
int KVInitialize	(size_t cacheSize);	// in bytes
void KVTerminate	(void);

// overall db ops
// (global)
int KVDBNew	(const uint8_t *path);		// relative or absolute is okay
int KVDBOpen	(const uint8_t *path);		// same
void KVDBDelete	(void);
void KVDBClose	(void);

// batching for speed
// (thread)
void KVBatchStart	(void);
void KVBatchCancel	(void);
void KVBatchEnd		(void);

// general functions
// (thread)
int KVSet	(kv_string key, kv_string value);
int KVGet	(kv_string key, kv_string *value);
int KVDelete	(kv_string key);
