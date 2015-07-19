#include "structure.h"

static uint8_t internal__structure_proto[] = {
#include "structure.format"
};

static struct pbc_env *pbc_env;

int StructureInitialize (void) {
	if (pbc_env)
		return 0;

	struct pbc_slice slice;
	slice.buffer = internal__structure_proto;
	slice.len = strlen (internal__structure_proto);

	pbc_env = pbc_new ();
	if (!pbc_env)
		return -1;

	return pbc_register (pbc_env, &slice);
}

void StructureTerminate (void) {
	if (!pbc_env)
		return;

	pbc_delete (pbc_env);
	pbc_env = NULL;

	return;
}

int StructureAddChildren (structure *parent, const structure *children, int count) {
	if (parent->type != STRUCTURE_TYPE_SUB)
		return 1;	// yeah lets not cause a memory issue

	// realloc () may lose data
	structure *tmp = malloc ((parent->count + count) * sizeof (structure));
	if (!tmp)
		return -1;	// memory failure

	// copy over the old children
	memcpy (tmp, parent->children, parent->count * sizeof (structure));
	if (parent->count > 0)
		free (parent->children);
	parent->children = tmp;

	// copy over the new children
	memcpy (&parent->children[parent->count], children, count * sizeof (structure));
	parent->count += count;

	// resort the array to allow bsearch() to find members
	structure_tim_sort (parent->children, parent->count);

	return 0;
}

void StructureFree (structure *s) {
	switch (s->type) {
		case STRUCTURE_TYPE_SUB: {
			int x;
			for (x = 0; x < s->count; x++) {
				switch (s->children[x].type) {
					case STRUCTURE_TYPE_BLOB:
					case STRUCTURE_TYPE_STRING: {
						free (s->children[x].blob);

						break;
					}

					default: {
						continue;
					}
				}
			}

			// fallthrough
		}

		case STRUCTURE_TYPE_BLOB:
		case STRUCTURE_TYPE_STRING: {
			free (s->blob);

			// fallthrough
		}

		case STRUCTURE_TYPE_NULL:
		case STRUCTURE_TYPE_I64:
		case STRUCTURE_TYPE_U64:
		case STRUCTURE_TYPE_H64:
		case STRUCTURE_TYPE_F64:
		case STRUCTURE_TYPE_TIME:
		case STRUCTURE_TYPE_UNIXTIME:
		default: {
			free (s->key);
			free (s);

			break;
		}
	}

	return;
}

kv_string StructurePack (const structure *s) {
	const kv_string empty = { .data = NULL, .len = 0 };

	struct pbc_wmessage *msg = pbc_wmessage_new (pbc_env, "sfDB5.Structure");
	if (xm_unlikely (!msg))
		return empty;

	if (xm_unlikely (s->type == STRUCTURE_TYPE_NULL)) {
		pbc_wmessage_delete (msg);

		return empty;
	} else if (s->type != STRUCTURE_TYPE_SUB) {	// single type structure
		int err = 0;

		err |= pbc_wmessage_string (msg, "name", s->key, strnlen (s->key, 255));
		err |= pbc_wmessage_integer (msg, "type", s->type, 0);

		switch (s->type) {
			case STRUCTURE_TYPE_BLOB:
			case STRUCTURE_TYPE_STRING: {
				err |= pbc_wmessage_string (msg, "value", s->blob, s->len);

				break;
			}

			case STRUCTURE_TYPE_I64:
			case STRUCTURE_TYPE_U64:
			case STRUCTURE_TYPE_H64:
			case STRUCTURE_TYPE_F64:
			case STRUCTURE_TYPE_TIME:
			case STRUCTURE_TYPE_UNIXTIME: {
				err |= pbc_wmessage_string (msg, "value", (uint8_t *) &s->i64, sizeof (s->i64));

				break;
			}

			default: {
				err |= 1;
				break;
			}
		}

		// cleanup all at once
		if (xm_unlikely (err)) {
			pbc_wmessage_delete (msg);

			return empty;
		}

		struct pbc_slice slice;
		pbc_wmessage_buffer (msg, &slice);

		if (slice.len > STRUCTURE_PACK_MAX_SIZE) {
			pbc_wmessage_delete (msg);

			return empty;
		}

		kv_string packed;
		packed.len = slice.len;
		packed.data = malloc (packed.len);
		if (!packed.data) {
			pbc_wmessage_delete (msg);

			return empty;
		}

		memcpy (packed.data, slice.buffer, packed.len);
		pbc_wmessage_delete (msg);

		return packed;
	}

	// composite structure
	int err = 0;

	err |= pbc_wmessage_string (msg, "name", s->key, strnlen (s->key, 255));
	err |= pbc_wmessage_integer (msg, "type", s->type, 0);

	int x;
	for (x = 0; x < s->count; x++) {
		// only allow flat types
		switch (s->children[x].type) {
			case STRUCTURE_TYPE_BLOB:
			case STRUCTURE_TYPE_STRING: {
				struct pbc_wmessage *msg_sub = pbc_wmessage_message (msg, "entries");
				if (xm_unlikely (!msg_sub)) {
					pbc_wmessage_delete (msg);

					return empty;
				}

				err |= pbc_wmessage_string (
								msg_sub,
								"name",
								s->children[x].key,
								strnlen (s->children[x].key, 255)
								);
				err |= pbc_wmessage_integer (
								msg_sub,
								"type",
								s->children[x].type,
								0
								);
				err |= pbc_wmessage_string (
								msg_sub,
								"value",
								s->children[x].blob,
								s->children[x].len
								);

				break;
			}

			case STRUCTURE_TYPE_I64:
			case STRUCTURE_TYPE_U64:
			case STRUCTURE_TYPE_H64:
			case STRUCTURE_TYPE_F64:
			case STRUCTURE_TYPE_TIME:
			case STRUCTURE_TYPE_UNIXTIME: {
				struct pbc_wmessage *msg_sub = pbc_wmessage_message (msg, "entries");
				if (xm_unlikely (!msg_sub)) {
					pbc_wmessage_delete (msg);

					return empty;
				}

				err |= pbc_wmessage_string (
								msg_sub,
								"name",
								s->children[x].key,
								strnlen (s->children[x].key, 255)
								);
				err |= pbc_wmessage_integer (
								msg_sub,
								"type",
								s->children[x].type,
								0
								);
				err |= pbc_wmessage_string (
								msg_sub,
								"value",
								(uint8_t *) &s->children[x].i64,
								sizeof (s->children[x].i64)
								);

				break;
			}

			default: {
				continue;	// jump to next item
			}
		}

	}

	if (xm_unlikely (err)) {
		pbc_wmessage_delete (msg);

		return empty;
	}

	struct pbc_slice slice;
	pbc_wmessage_buffer (msg, &slice);

	if (slice.len > STRUCTURE_PACK_MAX_SIZE) {
		pbc_wmessage_delete (msg);

		return empty;
	}


	kv_string packed;
	packed.len = slice.len;
	packed.data = malloc (packed.len);
	if (!packed.data) {
		pbc_wmessage_delete (msg);

		return empty;
	}

	memcpy (packed.data, slice.buffer, packed.len);
	pbc_wmessage_delete (msg);

	return packed;
}

structure *StructureUnpack (const kv_string packed) {
	// Packed Format:
	//	u8:	structure name length
	//	u8[]:	structure name
	//	u8:	structure type
	//	either (
	//		u16:	number of entries
	//		[
	//			u8:	member name length
	//			u8[]:	member name
	//			u8:	member type
	//			#u16:	member value length (if needed)
	//			u8[]:	member value
	//		]
	//	) or (
	//		#u16:	structure value length (if needed)
	//		u8[]:	structure value
	//	)

	if (xm_unlikely (packed.lens > STRUCTURE_PACK_MAX_SIZE))
		return NULL;

	struct pbc_slice slice;
	slice.buffer = packed.data;
	slice.len = packed.len;

	struct pbc_rmessage *msg = pbc_rmessage_new (pbc_env, "sfDB5.Structure", &slice);
	if (xm_unlikely (!msg))
		return NULL;

	structure *unpacked = calloc (1, sizeof (structure));
	if (!unpacked) {
		pbc_rmessage_delete (msg);

		return NULL;
	}

	uint8_t *ts;
	int tslen;
	ts = pbc_rmessage_string (msg, "name", 0, &tslen);
	if (xm_unlikely (tslen <= 0)) {
		pbc_rmessage_delete (msg);
		free (unpacked);

		return NULL;
	}

	unpacked->key = strndup (ts, xm_min (tslen, 255));
	if (xm_unlikely (!unpacked->key)) {
		pbc_rmessage_delete (msg);

		free (unpacked);

		return NULL;
	}

	unpacked->type = pbc_rmessage_integer (msg, "type", 0, NULL);

	if (unpacked->type != STRUCTURE_TYPE_SUB) {	// flat structure
		switch (unpacked->type) {
			case STRUCTURE_TYPE_BLOB:
			case STRUCTURE_TYPE_STRING: {
				ts = pbc_rmessage_string (msg, "value", 0, &tslen);
				if (xm_unlikely (tslen <= 0)) {
					pbc_rmessage_delete (msg);

					free (unpacked->key);
					free (unpacked);

					return NULL;
				}

				unpacked->len = tslen;
				unpacked->blob = malloc (tslen);
				if (!unpacked->blob) {
					pbc_rmessage_delete (msg);

					free (unpacked->key);
					free (unpacked);

					return NULL;
				}

				memcpy (unpacked->blob, ts, unpacked->len);

				pbc_rmessage_delete (msg);

				return unpacked;
			}

			case STRUCTURE_TYPE_I64:
			case STRUCTURE_TYPE_U64:
			case STRUCTURE_TYPE_H64:
			case STRUCTURE_TYPE_F64:
			case STRUCTURE_TYPE_TIME:
			case STRUCTURE_TYPE_UNIXTIME: {
				ts = pbc_rmessage_string (msg, "value", 0, &tslen);
				if (xm_unlikely (tslen != sizeof (unpacked->i64))) {
					pbc_rmessage_delete (msg);

					free (unpacked->key);
					free (unpacked);

					return NULL;
				}

				memcpy (&unpacked->i64, ts, tslen);

				return unpacked;
			}

			default: {
				pbc_rmessage_delete (msg);

				free (unpacked->key);
				free (unpacked);

				return NULL;
			}
		}
	}

	// composite structure
	tslen = pbc_rmessage_size (msg, "entries");
	if (xm_unlikely (tslen <= 0)) {
		pbc_rmessage_delete (msg);

		free (unpacked->key);
		free (unpacked);

		return NULL;
	}

	unpacked->count = tslen;
	unpacked->children = calloc (unpacked->count, sizeof (structure));
	if (!unpacked->children) {
		pbc_rmessage_delete (msg);

		free (unpacked->key);
		free (unpacked);

		return NULL;
	}

	int x;
	for (x = 0; x < unpacked->count; x++) {
		struct pbc_rmessage *msg_sub = pbc_rmessage_message (msg, "entries", x);

		ts = pbc_rmessage_string (msg_sub, "name", x, &tslen);
		if (xm_unlikely (tslen <= 0)) {
			pbc_rmessage_delete (msg);

			// we can free everything but this since this has nothing allocated inside
			unpacked->count = x;
			StructureFree (unpacked);

			return NULL;
		}

		unpacked->children[x].key = strndup (ts, xm_min (tslen, 255));
		if (!unpacked->children[x].key) {
			pbc_rmessage_delete (msg);

			unpacked->count = x;
			StructureFree (unpacked);

			return NULL;
		}

		unpacked->children[x].type = pbc_rmessage_integer (msg_sub, "type", x, NULL);

		switch (unpacked->children[x].type) {
			case STRUCTURE_TYPE_BLOB:
			case STRUCTURE_TYPE_STRING: {
				ts = pbc_rmessage_string (msg_sub, "value", x, &tslen);
				if (tslen <= 0) {
					pbc_rmessage_delete (msg);

					// hack to free the key
					unpacked->children[x].type = STRUCTURE_TYPE_I64;
					unpacked->count = x + 1;
					StructureFree (unpacked);

					return NULL;
				}

				unpacked->children[x].len = tslen;
				unpacked->children[x].blob = malloc (unpacked->children[x].len);
				if (!unpacked->children[x].blob) {
					pbc_rmessage_delete (msg);

					unpacked->children[x].type = STRUCTURE_TYPE_I64;
					unpacked->count = x + 1;
					StructureFree (unpacked);

					return NULL;
				}

				memcpy (unpacked->children[x].blob, ts, unpacked->children[x].len);

				// on to the next one
				continue;
			}

			// encoding should have shipped only purely flat types
			// and authentication should ensure that no injections occurred
			default: {
				ts = pbc_rmessage_string (msg_sub, "value", x, &tslen);
				if (tslen != sizeof (unpacked->children[x].i64)) {
					pbc_rmessage_delete (msg);

					unpacked->count = x + 1;
					StructureFree (unpacked);

					return NULL;
				}

				memcpy (&unpacked->children[x].i64, ts, sizeof (unpacked->children[x].i64));

				// and onto the next one
				continue;
			}
		}
	}

	// libpbc cleanup
	pbc_rmessage_delete (msg);

	// get all of them in order
	StructureSortChildren (unpacked);

	// OMG we made it!
	return unpacked;
}
