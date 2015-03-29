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
extern kv_db_t *kv_db;

//=== Functions

// startup / shutdown
int KVInitialize	(size_t cacheSize);
void KVCleanup		(void);
void KVTerminate	(void);

// overall db ops
kv_db_t xmattr_malloc *KVDBNew	(const uint8_t *path);
kv_db_t xmattr_malloc *KVDBOpen	(const uint8_t *path);
void KVDBDelete	(kv_db_t *db);
void KVDBClose	(kv_db_t *db);

// current db
static inline void KVDBSet (kv_db_t *db) {
	kv_db = db;
	return;
}

// batching for speed
void KVBatchStart	(void);
void KVBatchEnd		(void);

// general functions
int KVSet	(kv_string key, kv_string value);
int KVget	(kv_string key, kv_string *value);
int KVDelete	(kv_string key);
