#pragma once

//===	Includes

#include <stdint.h>

#include "xm.h"

//===	Types

typedef void kv_db_t;

typedef struct {
	int64_t		len;
	void		*data;
} kv_string;

//=== Variables

extern xm_tlv kv_error;

//=== Functions

// overall db ops
kv_db_t xmattr_malloc *kv_db_new	(const uint8_t *path);
kv_db_t xmattr_malloc *kv_db_open	(const uint8_t *path);
void kv_db_delete	(kv_db_t *db);
void kv_db_close	(kv_db_t *db);

// batching for speed
void kv_batch_begin	(kv_db_t *db);
void kv_batch_end	(kv_db_t *db);

// general functions
int kv_get	(kv_string key, kv_string value);
int kv_put	(kv_string key, kv_string *value);
int kv_delete	(kv_string key);
