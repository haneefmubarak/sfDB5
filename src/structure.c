#include "structure.h"

int StructureAddChildren (structure *parent, const structure *children, int count) {
	if (parent->type != STRUCTURE_TYPE_SUB)
		return 1;	// yeah lets not cause a memory issue

	// realloc () may lose data
	structure *tmp = malloc ((parent->count + count) * sizeof (structure *));
	if (!tmp)
		return -1;	// memory failure

	// copy over the old children
	memcpy (tmp, parent->children, parent->count * sizeof (structure));
	free (parent->children);
	parent->children = tmp;

	// copy over the new children
	memcpy (&parent->children[parent->count], children, count * sizeof (structure));
	parent->count += count;

	// resort the array to allow bsearch() to find members
	structure_binary_sort (parent->children, parent->count);

	return 0;
}

static void internal_structure_free (structure s) {
	switch (s.type) {
		case STRUCTURE_TYPE_SUB: {
			int x;
			for (x = 0; x < s.count; x++)
				internal_structure_free (s.children[x]);
			// fallthough to next code


		// contains a pointer to the blob or string (or substructures if fallthrough)
		case STRUCTURE_TYPE_BLOB:
		case STRUCTURE_TYPE_STRING: {
			free (s.blob);
			// fallthrough to next code
		}

		// no other pointers contained other than key
		default:
		case STRUCTURE_TYPE_NULL:
		case STRUCTURE_TYPE_I64:
		case STRUCTURE_TYPE_U64:
		case STRUCTURE_TYPE_H64:
		case STRUCTURE_TYPE_F64:
		case STRUCTURE_TYPE_TIME:
		case STRUCTURE_TYPE_UNIXTIME: {
			free (s.key);
			break;
		}
	}

	return;
}

void StructureFree (structure *s) {
	internal_structure_free (*s);
	free (s);
	return;
}

static int internal_serialize_getlen (const structure *s) {
	int len = 0;
	switch (s->type) {
		// this structure is invalid
		default:
		case STRUCTURE_TYPE_NULL: {
			return 0;
		}

		// structure containing data
		case STRUCTURE_TYPE_SUB: {
			int x;
			for (x = 0; x < s->count; x++)
				len += internal_serialize_getlen (&s->children[x]);
			goto BASESIZE_ISGL;
		}

		case STRUCTURE_TYPE_BLOB:
		case STRUCTURE_TYPE_STRING: {
			len += s->len;
			goto BASESIZE_ISGL;
		}

		BASESIZE_ISGL:	// common sizes to all valid structures
		// all of these are flat
		case STRUCTURE_TYPE_I64:
		case STRUCTURE_TYPE_U64:
		case STRUCTURE_TYPE_H64:
		case STRUCTURE_TYPE_F64:
		case STRUCTURE_TYPE_TIME:
		case STRUCTURE_TYPE_UNIXTIME: {
			len += strlen (s->key) + 1;	// NULL terminator
			len += sizeof (structure);
			break;
		}
	}

	return len;
}

static void internal_serialize_pack (const structure *s, uint8_t *stream, int *pos) {
	switch (s->type) {
		default:
		case STRUCTURE_TYPE_NULL: {
			return;	// skip
		}

		case STRUCTURE_TYPE_SUB: {
			structure *t = (void *) &stream[*pos];
			*t = *s;	// copy the data over
			// increase compressibility
			t->key	= NULL;
			t->children = NULL;
			*pos += sizeof (structure);

			int len = strlen (s->key) + 1;	// NULL terminator
			memcpy (&stream[*pos], s->key, len);	// we need the len anyways
			*pos += len;

			// we already know how many substructures there are (s->count)
			int x;
			for (x = 0; x < s->count; x++)
				internal_serialize_pack (&s->children[x], stream, pos);

			break;
		}

		case STRUCTURE_TYPE_BLOB:
		case STRUCTURE_TYPE_STRING: {
			structure *t = (void *) &stream[*pos];
			*t = *s;	// copy the data over
			// increase compressibility
			t->key	= NULL;
			t->blob	= NULL;
			*pos += sizeof (structure);

			int len = strlen (s->key) + 1;	// NULL terminator
			memcpy (&stream[*pos], s->key, len);	// we need the len anyways
			*pos += len;

			// we already know how large the payload is (s->len)
			memcpy (&stream[*pos], s->blob, s->len);
			*pos += s->len;

			break;
		}

		// all of these are flat
		case STRUCTURE_TYPE_I64:
		case STRUCTURE_TYPE_U64:
		case STRUCTURE_TYPE_H64:
		case STRUCTURE_TYPE_F64:
		case STRUCTURE_TYPE_TIME:
		case STRUCTURE_TYPE_UNIXTIME: {
			structure *t = (void *) &stream[*pos];
			*t = *s;	// copy the data over
			// increase compressibility
			t->key	= NULL;
			*pos += sizeof (structure);

			int len = strlen (s->key) + 1;	// NULL terminator
			memcpy (&stream[*pos], s->key, len);	// we need the len anyways
			*pos += len;

			break;
		}

	}

	return;
}

kv_string *StructurePack (const structure *s) {
	int len = internal_serialize_getlen (s);	// Pass I: get length of structures
	if (!len)
		return NULL;
	kv_string *packed = malloc (sizeof (kv_string));
	if (!packed)
		return NULL;
	packed->data = malloc (len);
	if (!packed->data) {
		free (packed);
		return NULL;
	}
	packed->len = len + sizeof (checksum_t);

	// pack it up
	int pos = 0;
	internal_serialize_pack (s, packed->data, &pos);	// Pass II: pack it together
	checksum_t *sum = (void *) packed->data[packed->len - sizeof (checksum_t)];
	*sum = checksum (packed->data, packed->len - sizeof (checksum_t));	// detect bit errors

	return packed;
}

static structure *internal_deserialize_unpack (const kv_string *packed, int *pos) {
	const int len = packed->len - sizeof (checksum_t);
	if (*pos >= len)
		return NULL;
	structure *s = (void *) &packed->data[*pos];
	structure *r = malloc (sizeof (structure);
	if (!r)
		return NULL;

	switch (s->type) {
		default:
		case: STRUCTURE_TYPE_NULL: {	// this kind of structure really shouldn't be here
			free (r);
			return NULL;
		}

		case: STRUCTURE_TYPE_SUB: {
			*r = *s;	// copy the data over
			*pos += sizeof (structure);

			// copy the key over
			r->key = strndup (&packed->data[*pos],	\	// max keysize of 255 chars
						xm_min (packed->len - *pos, 255));
			if (!r->key) {
				free (r);
				return NULL;
			}
			*pos += strlen (r->key) + 1;	// NULL terminator

			// when building up substructures, clean up if we get a bad one
			r->count = 0;	// adding structures needs to increment this
			for (x = 0; x < s->count; x++) {
				structure *t = internal_deserialize_unpack (packed, *pos);
				if (!t) {
					StructureFree (r);
					return NULL;
				}
				StructureAddChildren (r, t, 1);
			}

			break;
		}


		case: STRUCTURE_TYPE_BLOB:
		case: STRUCTURE_TYPE_STRING: {
			*r = *s;	// copy the data over
			*pos += sizeof (structure);

			// copy the key over
			r->key = strndup (&packed->data[*pos],	\	// max keysize of 255 chars
						xm_min (packed->len - *pos, 255));
			if (!r->key) {
				free (r);
				return NULL;
			}
			*pos += strlen (r->key) + 1;	// NULL terminator

			// copy the blob / string over
			r->blob = malloc (r->len);
			if (!r->blob) {
				free (r->key);
				free (r);
				return NULL;
			}
			memcpy (r->blob, &packed->data[*pos], r->len);
			*pos += r->len;

			break;
		}

		// all of these are flat
		case STRUCTURE_TYPE_I64:
		case STRUCTURE_TYPE_U64:
		case STRUCTURE_TYPE_H64:
		case STRUCTURE_TYPE_F64:
		case STRUCTURE_TYPE_TIME:
		case STRUCTURE_TYPE_UNIXTIME: {
			*r = *s;	// copy the data over
			*pos += sizeof (structure);

			// copy the key over
			r->key = strndup (&packed->data[*pos],	\	// max keysize of 255 chars
						xm_min (packed->len - *pos, 255));
			if (!r->key) {
				free (r);
				return NULL;
			}
			*pos += strlen (r->key) + 1;	// NULL terminator

			break;
		}

	}

	return r;
}


structure *StructureUnpack (const kv_string *packed) {
	// validate checksum
	checksum_t *sum = (void *) packed->data[packed->len - sizeof (checksum_t)];
	if (*sum != checksum (packed->data, packed->len - sizeof (checksum_t)))
		return NULL;	// checksum does not match

	int pos = 0;
	structure *r = internal_deserialize_unpack (packed, &pos);
	return r;	// if NULL - that'll get returned too
}
