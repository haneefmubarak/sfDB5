#include "test.h"
#include "../kv/backend.h"

#include <stdio.h>

uint8_t *path = "./storage";
uint8_t *hw[] = { "hello", "world" };

void test (void) {
	// Initialize KV environment and open DB
	assert (!KVInitialize (16 * 1024 * 1024));	// 16 MB Cache
	assert (!KVDBNew (path));

	// basic test values
	kv_string key, value;
	key.data	= hw[0];
	key.len		= strlen (key.data);
	value.data	= hw[1];
	value.len	= strlen (value.data);

	// test basic set and get
	assert (!KVSet (key, value));

	kv_string retval;

	assert (!KVGet (key, &retval));
	assert (retval.len == value.len);
	assert (!memcmp (retval.data, value.data, retval.len));

	free (retval.data);

	// test basic persistence
	KVDBClose ();

	assert (!KVDBOpen (path));

	retval.data = NULL;
	retval.len = 0;

	assert (!KVGet (key, &retval));
	assert (retval.len == value.len);
	assert (!memcmp (retval.data, value.data, retval.len));

	free (retval.data);

	// test batch set and get
	KVBatchStart ();

	int x;
	for (x = 0; x < 4; x++) {
		uint8_t v[11];
		snprintf (v, 11, "%i", x);

		key.data = v;
		key.len = strlen (key.data);
		value.data = v;
		value.len = strlen (value.data);

		assert (!KVSet (key, value));
	}

	KVBatchEnd ();

	for (x = 0; x < 4; x++) {
		uint8_t v[11];
		snprintf (v, 11, "%i", x);

		key.data = v;
		key.len = strlen (key.data);

		assert (!KVGet (key, &retval));
		assert (retval.len == key.len);
		assert (!memcmp (retval.data, key.data, retval.len));

		free (retval.data);
	}

	// test batch cancellation
	KVBatchStart ();

	for (x = 0; x < 4; x++) {
		uint8_t v[11];
		snprintf (v, 11, "%i", 16 - x);

		key.data = v;
		key.len = strlen (key.data);
		value.data = v;
		value.len = strlen (value.data);

		assert (!KVSet (key, value));
	}

	KVBatchCancel ();

	// check that results did not change
	for (x = 0; x < 4; x++) {
		uint8_t v[11];
		snprintf (v, 11, "%i", x);

		key.data = v;
		key.len = strlen (key.data);

		assert (!KVGet (key, &retval));
		assert (retval.len == key.len);
		assert (!memcmp (retval.data, key.data, retval.len));

		free (retval.data);
	}


	// test deletion
	key.data = hw[0];
	key.len = strlen (key.data);
	assert (!KVDelete (key));

	int r = KVGet (key, &retval);
	assert (r || !retval.data);

	// test batch deletion
	KVBatchStart ();

	for (x = 0; x < 4; x++) {
		uint8_t v[11];
		snprintf (v, 11, "%i", x);

		key.data = v;
		key.len = strlen (key.data);

		assert (!KVDelete (key));
	}

	KVBatchEnd ();

	for (x = 0; x < 4; x++) {
		uint8_t v[11];
		snprintf (v, 11, "%i", x);

		key.data = v;
		key.len = strlen (key.data);

		r = KVGet (key, &retval);
		assert (r || !retval.data);
	}

	// cleanup
	KVDBDelete ();
	KVTerminate ();

	return;
}

#include "test.c"
