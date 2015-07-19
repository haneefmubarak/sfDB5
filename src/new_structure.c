#include "structure.h"

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

	const kv_string empty = { .data = NULL, .len = 0 };

	if (xm_unlikely (s->type == STRUCTURE_TYPE_NULL)) {
		return empty;
	} else if (s->type != STRUCTURE_TYPE_SUB) {	// single type structure
		uint8_t keylen = strlen (s->key);

		kv_string packed = { 0 };
		packed.lens += sizeof (keylen);
		packed.lens += keylen;
		packed.lens += sizeof (s->type);
		if (s->type == STRUCTURE_TYPE_BLOB || s->type == STRUCTURE_TYPE_STRING) {
			packed.lens += sizeof (s->len);
			packed.lens += s->len;
		} else {
			packed.lens += sizeof (s->i64);
		}

		if (packed.lens > STRUCTURE_PACK_MAX_SIZE)
			return empty;

		packed.data = malloc (packed.lens);
		if (!packed.data)
			return empty;

		uint32_t offset = 0;
		memcpy (&packed.data[offset], &keylen, sizeof (keylen));
		offset += sizeof (keylen);
		memcpy (&packed.data[offset], s->key, keylen);
		offset += keylen;
		memcpy (&packed.data[offset], &s->type, sizeof (s->type));
		offset += sizeof (s->type);

		switch (s->type) {
			case STRUCTURE_TYPE_BLOB:
			case STRUCTURE_TYPE_STRING: {
				memcpy (&packed.data[offset], &s->len, sizeof (s->len));
				offset += s->len;
				memcpy (&packed.data[offset], s->blob, s->len);

				return packed;
			}

			case STRUCTURE_TYPE_I64:
			case STRUCTURE_TYPE_U64:
			case STRUCTURE_TYPE_H64:
			case STRUCTURE_TYPE_F64:
			case STRUCTURE_TYPE_TIME:
			case STRUCTURE_TYPE_UNIXTIME: {
				memcpy (&packed.data[offset], &s->i64, sizeof (s->i64));

				return packed;
			}

			default: {
				free (packed.data);
				return empty;
			}
		}
	}

	// composite structure
	kv_string packed = { 0 };
	packed.lens += sizeof (uint8_t);
	packed.lens += strlen (s->key);
	packed.lens += sizeof (s->type);
	packed.lens += sizeof (s->count);
	int x;
	for (x = 0; x < s->count; x++) {
		// only allow flat types
		switch (s->children[x].type) {
			case STRUCTURE_TYPE_BLOB:
			case STRUCTURE_TYPE_STRING: {
				packed.lens += sizeof (uint8_t);
				packed.lens += strlen (s->children[x].key);
				packed.lens += sizeof (s->children[x].type);
				packed.lens += sizeof (s->children[x].len);
				packed.lens += s->children[x].len;

				break;
			}

			case STRUCTURE_TYPE_I64:
			case STRUCTURE_TYPE_U64:
			case STRUCTURE_TYPE_H64:
			case STRUCTURE_TYPE_F64:
			case STRUCTURE_TYPE_TIME:
			case STRUCTURE_TYPE_UNIXTIME: {
				packed.lens += sizeof (uint8_t);
				packed.lens += strlen (s->children[x].key);
				packed.lens += sizeof (s->children[x].type);
				packed.lens += sizeof (s->children[x].i64);

				break;
			}

			default: {
				continue;	// jump to next item
			}
		}

	}

	// should really figure out what's wrong witht the length calculation at some point
	packed.lens = x + (x / 4);	// x * 1.25

	uint16_t entries = x;

	if (packed.lens > STRUCTURE_PACK_MAX_SIZE)
		return empty;

	packed.data = malloc (packed.lens);
	if (!packed.data)
		return empty;

	uint32_t offset = 0;
	uint8_t keylen = strlen (s->key);
	memcpy (&packed.data[offset], &keylen, sizeof (keylen));
	offset += keylen;
	memcpy (&packed.data[offset], s->key, keylen);
	offset += keylen;
	memcpy (&packed.data[offset], &s->type, sizeof (s->type));
	offset += sizeof (s->type);
	memcpy (&packed.data[offset], &entries, sizeof (entries));
	offset += sizeof (entries);

	for (x = 0; x < s->count; x++) {
		// only allow flat types
		switch (s->children[x].type) {
			case STRUCTURE_TYPE_BLOB:
			case STRUCTURE_TYPE_STRING: {
				keylen = strlen (s->children[x].key);
				memcpy (&packed.data[offset], &keylen, sizeof (keylen));
				offset += sizeof (keylen);
				memcpy (&packed.data[offset], &s->children[x].key, keylen);
				offset += keylen;
				memcpy (&packed.data[offset], &s->children[x].type, sizeof (s->children[x].type));
				offset += sizeof (s->children[x].type);
				memcpy (&packed.data[offset], &s->children[x].len, sizeof (s->children[x].len));
				offset += sizeof (s->children[x].len);
				memcpy (&packed.data[offset], s->children[x].blob, s->children[x].len);
				offset += s->children[x].len;

				break;
			}

			case STRUCTURE_TYPE_I64:
			case STRUCTURE_TYPE_U64:
			case STRUCTURE_TYPE_H64:
			case STRUCTURE_TYPE_F64:
			case STRUCTURE_TYPE_TIME:
			case STRUCTURE_TYPE_UNIXTIME: {
				keylen = strlen (s->children[x].key);
				memcpy (&packed.data[offset], &keylen, sizeof (keylen));
				offset += sizeof (keylen);
				memcpy (&packed.data[offset], &s->children[x].key, keylen);
				offset += keylen;
				memcpy (&packed.data[offset], &s->children[x].type, sizeof (s->children[x].type));
				offset += sizeof (s->children[x].type);
				memcpy (&packed.data[offset], &s->children[x].i64, sizeof (s->children[x].i64));
				offset += sizeof (s->children[x].i64);

				break;
			}

			default: {
				continue;	// jump to next item
			}
		}

	}

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

	// structure namelen + name + type + smallest value
	if (xm_unlikely (packed.lens < (sizeof (uint8_t) * 3 + sizeof (uint64_t))))
		return NULL;

	structure *unpacked = calloc (1, sizeof (structure));
	if (!unpacked)
		return NULL;

	uint32_t offset = 0;
	uint8_t keylen;
	memcpy (&keylen, &packed.data[offset], sizeof (keylen));
	offset += sizeof (keylen);

	if (xm_unlikely ((offset + keylen) > packed.lens)) {
		free (unpacked);

		return NULL;
	}

	unpacked->key = strndup (&packed.data[offset], keylen);
	if (xm_unlikely (!unpacked->key)) {
		free (unpacked);

		return NULL;
	}
	offset += keylen;

	if (xm_unlikely ((offset + sizeof (unpacked->type)) > packed.lens)) {
		free (unpacked->key);
		free (unpacked);

		return NULL;
	}

	memcpy (&unpacked->type, &packed.data[offset], sizeof (unpacked->type));
	offset += sizeof (unpacked->type);

	if (unpacked->type != STRUCTURE_TYPE_SUB) {	// flat structure
		switch (unpacked->type) {
			case STRUCTURE_TYPE_BLOB:
			case STRUCTURE_TYPE_STRING: {
				if (xm_unlikely ((offset + sizeof (unpacked->len)) > packed.lens)) {
					free (unpacked->key);
					free (unpacked);

					return NULL;
				}

				memcpy (&unpacked->len, &packed.data[offset], sizeof (unpacked->len));
				offset += sizeof (unpacked->len);

				if (xm_unlikely ((offset + unpacked->len) > packed.lens)) {
					free (unpacked->key);
					free (unpacked);

					return NULL;
				}

				unpacked->blob = malloc (unpacked->len);
				if (!unpacked->blob) {
					free (unpacked->key);
					free (unpacked);

					return NULL;
				}

				memcpy (unpacked->blob, &packed.data[offset], unpacked->len);

				return unpacked;
			}

			case STRUCTURE_TYPE_I64:
			case STRUCTURE_TYPE_U64:
			case STRUCTURE_TYPE_H64:
			case STRUCTURE_TYPE_F64:
			case STRUCTURE_TYPE_TIME:
			case STRUCTURE_TYPE_UNIXTIME: {
				if (xm_unlikely ((offset + sizeof (unpacked->i64)) > packed.lens)) {
					free (unpacked->key);
					free (unpacked);

					return NULL;
				}

				memcpy (&unpacked->i64, &packed.data[offset], sizeof (unpacked->i64));

				return unpacked;
			}

			default: {
				free (unpacked->key);
				free (unpacked);

				return NULL;
			}
		}
	}

	// composite structure
	if (xm_unlikely ((offset + sizeof (unpacked->count)) > packed.lens)) {
		free (unpacked->key);
		free (unpacked);

		return NULL;
	}

	memcpy (&unpacked->count, &packed.data[offset], sizeof (unpacked->count));
	offset += sizeof (unpacked->count);

	unpacked->children = calloc (unpacked->count, sizeof (structure));
	if (!unpacked->children) {
		free (unpacked->key);
		free (unpacked);

		return NULL;
	}

	int x;
	for (x = 0; x < unpacked->count; x++) {
		uint8_t keylen;
		if (xm_unlikely ((offset + sizeof (keylen)) > packed.lens)) {
			unpacked->count = x;	// we don't need to handle this one
			StructureFree (unpacked);

			return NULL;
		}

		memcpy (&keylen, &packed.data[offset], sizeof (keylen));
		offset += keylen;

		if (xm_unlikely ((offset + keylen) > packed.lens)) {
			unpacked->count = x;
			StructureFree (unpacked);

			return NULL;
		}

		unpacked->children[x].key = strndup (&packed.data[offset], keylen);
		if (!unpacked->children[x].key) {
			unpacked->count = x;
			StructureFree (unpacked);

			return NULL;
		}
		offset += keylen;

		if (xm_unlikely ((offset + sizeof (unpacked->children[x].type)) > packed.lens)) {
			// we only have to free the key
			unpacked->children[x].type = STRUCTURE_TYPE_I64;

			unpacked->count = x + 1;	// loop semantics
			StructureFree (unpacked);

			return NULL;
		}

		memcpy (&unpacked->children[x].type, &packed.data[offset], sizeof (unpacked->children[x].type));
		offset += sizeof (unpacked->children[x].type);

		switch (unpacked->children[x].type) {
			case STRUCTURE_TYPE_BLOB:
			case STRUCTURE_TYPE_STRING: {
				if (xm_unlikely ((offset + sizeof (unpacked->children[x].len)) > packed.lens)) {
					// hack to free the key
					unpacked->children[x].type = STRUCTURE_TYPE_I64;

					unpacked->count = x + 1; // for loop semantics
					StructureFree (unpacked);

					return NULL;
				}

				int size = sizeof (unpacked->children[x].len);
				memcpy (&unpacked->children[x].len, &packed.data[offset], size);
				offset += size;

				unpacked->children[x].blob = malloc (unpacked->children[x].len);
				if (!unpacked->children[x].blob) {
					unpacked->children[x].type = STRUCTURE_TYPE_I64;

					unpacked->count = x + 1;
					StructureFree (unpacked);

					return NULL;
				}

				if (xm_unlikely ((offset + unpacked->children[x].len) > packed.lens)) {
					// it's okay to free our memory
					unpacked->count = x + 1; // for loop semantics
					StructureFree (unpacked);

					return NULL;
				}

				size = unpacked->children[x].len;
				memcpy (&unpacked->children[x].blob, &packed.data[offset], size);
				offset += size;

				// on to the next one
				continue;
			}

			// encoding should have shipped only purely flat types
			// and authentication should ensure that no injections occurred
			default: {
				if (xm_unlikely ((offset + sizeof (unpacked->children[x].i64)) > packed.lens)) {
					// type is already correct
					unpacked->count = x + 1;
					StructureFree (unpacked);

					return NULL;
				}

				int size = sizeof (unpacked->children[x].i64);
				memcpy (&unpacked->children[x].i64, &packed.data[offset], size);
				offset += size;

				// and onto the next one
				continue;
			}
		}
	}

	// OMG we made it!
	return unpacked;
}
