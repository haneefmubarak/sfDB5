#include "kv_sophia.h"

int xm_tlv kv_error = 0;

static kv_sophia db = { 0 };

static int xm_tlv internal__kv_batch_active = 0;
static void xm_tlv *internal__kv_batch = NULL;


static int internal__kv_init = 0;
static size_t internal__kv_cachesize;

int KVInitialize (size_t cacheSize) {
	if (internal__kv_init)
		goto KVI_FAIL0;

	int cores = sysconf (_SC_NPROCESSORS_ONLN);

	// sophia uses strings, even for numbers
	uint8_t *memory_limit, *sched_threads;
	memory_limit	= alloca (21);	// u64 + NULL = ceil (log_10 (2^64 - 1)) + 1
	sched_threads	= alloca (11);	// u32 + NULL = ceil (log_10 (2^32  -1)) + 1
	snprintf (memory_limit, 21, "%" PRId64, cacheSize);
	snprintf (sched_threads, 11, "%" PRIu32, cores);

	db.env = sp_env ();
	if (!db.env)
		goto KVI_FAIL0;
	db.ctl = sp_ctl (db.env);
	if (!db.ctl)
		goto KVI_FAIL1;

	int rc = 0;
	rc |= sp_set (db.ctl, "memory.limit", memory_limit);
	rc |= sp_set (db.ctl, "scheduler.threads", sched_threads);
	if (rc)
		goto KVI_FAIL1;

	db.db = NULL;

	internal__kv_cachesize = cacheSize;
	internal__kv_init = 1;
	return 0;

	KVI_FAIL1:
		sp_destroy (db.env);
	KVI_FAIL0:
		return -1;
}

void KVTerminate (void) {
	if (!internal__kv_init)
		return;

	sp_destroy (db.env);

	db.env		= NULL;
	db.ctl		= NULL;
	db.db		= NULL;
	internal__kv_init = 0;
}


int KVDBNew (const uint8_t *path) {
	return KVDBOpen (path);
}

int KVDBOpen (const uint8_t *path) {
	if (!path)
		return KV_ERR_NPTR;
	if (!internal__kv_init)
		return KV_ERR_INIT;

	sp_set (db.ctl, "sophia.path", path);
	sp_set (db.ctl, "db", "default");
	db.db = sp_get (db.ctl, "db.default");
	if (sp_open (db.env))
		goto KVDBO_FAIL0;

	return 0;

	KVDBO_FAIL0:
		db.db = NULL;
		return -1;
}

static int internal__rmrf_callback (const char *path, const struct stat *b,	\
					int c, struct FTW *d) {
	return remove (path);
}

void KVDBDelete (void) {
	void *object = sp_get (db.ctl, "sophia.path");
	const char *path = sp_get (object, "value", NULL);

	uint8_t *fullpath = alloca (strlen (path) + strlen ("/default") + 1);
	strcpy (fullpath, path);
	strcat (fullpath, "/default");

	sp_destroy (object);	// free sophia's internally allocated memory

	KVDBClose ();

	// remove up to 256 files at a time while purging
	nftw (path, internal__rmrf_callback, 256, FTW_DEPTH | FTW_PHYS | FTW_MOUNT);
	sync ();

	return;
}


void KVDBClose (void) {
	db.db = NULL;

	KVTerminate ();
	KVInitialize (internal__kv_cachesize);

	return;
}

void KVBatchStart (void) {
	if (internal__kv_batch_active)
		KVBatchCancel ();

	internal__kv_batch = sp_begin (db.env);

	internal__kv_batch_active = 1;
	return;
}

void KVBatchCancel (void) {
	if (internal__kv_batch_active)
		sp_destroy (internal__kv_batch);

	internal__kv_batch_active = 0;
	return;
}

void KVBatchEnd (void) {
	if (!internal__kv_batch_active)
		return;

	kv_error = sp_commit (internal__kv_batch);

	internal__kv_batch_active = 0;
	return;
}

int KVSet (kv_string key, kv_string value) {
	void *target;
	int r = 0;

	if (internal__kv_batch_active)
		target = internal__kv_batch;
	else
		target = db.db;

	void *object = sp_object (db.db);
	r |= sp_set (object, "key", key.data, key.len);
	r |= sp_set (object, "value", value.data, value.len);

	// commit to db or add to transaction
	r |= sp_set (target, object);

	return r;
}

int KVGet (kv_string key, kv_string *value) {
	void *object = sp_object (db.db);

	sp_set (object, "key", key.data, key.len);
	void *result = sp_get (db.db, object);

	if (result) {
		int vsize;
		uint8_t *v = sp_get (result, "value", &vsize);

		// make a copy for the caller to keep
		value->data = malloc (vsize);
		if (!value->data)
			return -1;

		memcpy (value->data, v, vsize);
		value->len = vsize;

		sp_destroy (result);
	} else
		return 1;

	return 0;
}

int KVDelete (kv_string key) {
	void *target;
	int r = 0;


	if (internal__kv_batch_active)
		target = internal__kv_batch;
	else
		target = db.db;

	void *object = sp_object (db.db);
	r |= sp_set (object, "key", key.data, key.len);

	// commit deletion to database or add to transaction
	r |= sp_delete (target, object);

	return r;
}
