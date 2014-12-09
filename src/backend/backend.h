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
	int64_t		len;
	void		*data;
} kv_string;

//=== Variables

extern int xm_tlv kv_error;
extern kv_db_t xm_tlv *kv_db;

//=== Functions

// startup / shutdown
int kv_initialize	(size_t cacheSize);
void kv_thread_cleanup	(void);
void kv_terminate	(void);

// overall db ops
kv_db_t xmattr_malloc *kv_db_new	(const uint8_t *path);
kv_db_t xmattr_malloc *kv_db_open	(const uint8_t *path);
void kv_db_delete	(kv_db_t *db);
void kv_db_close	(kv_db_t *db);

// current db
static inline void kv_db_set (kv_db_t *db) {
	kv_db = db;
	return;
}

// batching for speed
void kv_batch_start	(void);
void kv_batch_end	(void);

// general functions
int kv_set	(kv_string key, kv_string value);
int kv_get	(kv_string key, kv_string *value);
int kv_del	(kv_string key);
