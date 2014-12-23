#include "structure.h"

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
		case STRUCTURE_TYPE_UNIXTIME:	{
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
		case STRUCTURE_TYPE_UNIXTIME:	{
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
	packed->len = len;

	int pos = 0;
	internal_serialize_pack (s, packed->data, &pos);	// Pass II: pack it together

	return packed;
}
