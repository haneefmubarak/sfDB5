#include "kv_rocksdb.h"

int xm_tlv kv_error = 0;

static size_t kv_cachesize;
static size_t kv_cores;
static int kv_init = 0;

int kv_initialize (size_t cacheSize) {
	kv_cachesize	= cacheSize;				// caches size in bytes
	kv_cores	= sysconf (_SC_NPROCESSORS_ONLN);	// online core count
	kv_init		= 1;	// mark as initialized

	return 0;
}

void kv_terminate (void) {
	kv_init = 0;	// mark as deinitialized
}

kv_db_t xmattr_malloc *kv_db_new (const uint8_t *path) {
	if (!path)
		kv_error = KV_ERR_NPTR;
	if (!kv_init)
		kv_error = KV_ERR_INIT;
	if (kv_error)
		return NULL;

	kv_rocksdb *db = malloc (sizeof kv_rocksdb);
	if (!db) {
		kv_error = KV_ERR_MEM;
		return NULL;
	}
	db->path = strdup (path);
	if (!db->path) {
		kv_error = KV_ERR_MEM;
		free (db);
		return NULL;
	}
