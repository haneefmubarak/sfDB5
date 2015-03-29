#include "kv_rocksdb.h"

int xm_tlv kv_error = 0;
static char xm_tlv *internal__kv_error;

kv_db_t *kv_db;

static int xm_tlv kv_batch_active = 0;
static rocksdb_writebatch_t xm_tlv *kv_batch = NULL;

static size_t kv_cachesize;
static size_t kv_cores;
static rocksdb_writeoptions_t	*kv_write_options;
static rocksdb_readoptions_t	*kv_read_options;
static int kv_init = 0;

static kv_db_t xmattr_malloc *internal__kv_open (const uint8_t *path, uint64_t flags);

int KVInitialize (size_t cacheSize) {
	kv_cachesize	= cacheSize;				// caches size in bytes
	kv_cores	= sysconf (_SC_NPROCESSORS_ONLN);	// online core count
	kv_init		= 1;	// mark as initialized

	// set default global write and read options
	kv_write_options	= rocksdb_writeoptions_create ();
	kv_read_options		= rocksdb_readoptions_create ();

	// enable threading
	rocksdb_env_t *env = rocksdb_create_default_env ();
	rocksdb_env_set_background_threads (env, kv_cores / 2);
	rocksdb_env_set_high_priority_background_threads (env, kv_cores / 2);
	rocksdb_env_destroy (env);

	return 0;
}

void KVThreadCleanup (void) {
	rocksdb_writebatch_destroy (kv_batch);

	return;
}

void KVTerminate (void) {
	rocksdb_writeoptions_destroy (kv_write_options);
	rocksdb_readoptions_destroy (kv_read_options);

	kv_init = 0;	// mark as deinitialized
}

#define	KV_OPEN_OPTION_NONE	0x00
#define	KV_OPEN_OPTION_NEW	0x01

kv_db_t xmattr_malloc *KVDBNew (const uint8_t *path) {
	return internal__kv_open (path, KV_OPEN_OPTION_NEW);
}

kv_db_t xmattr_malloc *KVDBOpen (const uint8_t *path) {
	return internal__kv_open (path, KV_OPEN_OPTION_NONE);
}

static kv_db_t xmattr_malloc *internal__kv_open (const uint8_t *path, uint64_t flags) {
	if (!path)
		kv_error = KV_ERR_NPTR;
	if (!kv_init)
		kv_error = KV_ERR_INIT;
	if (kv_error)
		return NULL;

	kv_rocksdb *db = malloc (sizeof (kv_rocksdb));
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

	// options etc.
	db->options = rocksdb_options_create ();
	rocksdb_options_set_max_background_compactions (db->options, kv_cores / 2);
	rocksdb_options_set_max_background_flushes (db->options, kv_cores / 2);
	rocksdb_options_set_compaction_style (db->options, rocksdb_universal_compaction);
	rocksdb_options_set_max_open_files (db->options, -1);	// keep all files open
	rocksdb_options_set_allow_os_buffer (db->options, 1);	// Linux spare block cache
	rocksdb_options_set_allow_mmap_reads (db->options, 1);
	rocksdb_options_set_allow_mmap_writes (db->options, 1);
	rocksdb_options_set_compression (db->options, rocksdb_snappy_compression);	// faster IO
	db->table_options = rocksdb_block_based_options_create ();
	rocksdb_block_based_options_set_block_size (db->table_options, 64 * 1024);	// 64 kB
	rocksdb_options_set_block_based_table_factory (db->options, db->table_options);
	if (flags & KV_OPEN_OPTION_NEW) {
		rocksdb_options_set_create_if_missing (db->options, 1);
	}

	db->handle = rocksdb_open (db->options, db->path, &internal__kv_error);

	kv_db_t *vptr = db;
	return vptr;
}

static int internal__kv_rmrf_callback (const char *path, const struct stat *b,	\
					int c, struct FTW *d) {
	return remove (path);
}

void KVDBDelete (kv_db_t *vptr) {
	kv_rocksdb *db = vptr;

	// keep a copy of path without calling malloc ()
	uint8_t *path = alloca (strlen (db->path));
	strcpy (path, db->path);

	// close the db
	KVDBClose (vptr);

	// remove all db related files
	// 256: pick a nice looking number
	nftw (path, internal__kv_rmrf_callback, 256, FTW_DEPTH | FTW_PHYS | FTW_MOUNT);
	sync ();

	return;
}

void KVDBClose (kv_db_t *vptr) {
	kv_rocksdb *db = vptr;

	rocksdb_close (db->handle);

	// clean the rest up
	rocksdb_block_based_options_destroy (db->table_options);
	rocksdb_options_destroy (db->options);
	free (db->path);
	free (db);

	return;
}

void KVBatchStart (void) {
	if (kv_batch_active)
		rocksdb_writebatch_clear (kv_batch);
	else
		kv_batch = rocksdb_writebatch_create ();

	kv_batch_active = 1;
	return;
}

void KVBatchEnd (void) {
	if (!kv_batch_active)
		kv_error = KV_ERR_NOBATCH;
	else
		rocksdb_write (kv_db, kv_write_options, kv_batch, &internal__kv_error);

	// mark as inactive
	kv_batch_active = 0;
	return;
}

int KVSet (kv_string key, kv_string value) {
	if (kv_batch_active)
		rocksdb_writebatch_put (kv_batch, key.data,	\
				key.len, value.data, value.len);
	else
		rocksdb_put (kv_db, kv_write_options, key.data,	\
				key.len, value.data, value.len, &internal__kv_error);

	return 0;
}

int KVGet (kv_string key, kv_string *value) {
	value->data = rocksdb_get (kv_db, kv_read_options, key.data,	\
					key.len, &value->lens, &internal__kv_error);

	return 0;
}

int KVDelete (kv_string key) {
	if (kv_batch_active)
		rocksdb_writebatch_delete (kv_batch, key.data, key.len);
	else
		rocksdb_delete (kv_db, kv_write_options, key.data, key.len, &internal__kv_error);

	return 0;
}
