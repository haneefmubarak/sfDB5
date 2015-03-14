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
	structure_binary_insertion_sort (parent->children, parent->count);

	return 0;
}

static void internal__structure_free (structure s) {
	switch (s.type) {
		case STRUCTURE_TYPE_SUB: {
			int x;
			for (x = 0; x < s.count; x++)
				internal__structure_free (s.children[x]);
			// fallthough to next code
		}


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
	internal__structure_free (*s);
	free (s);
	return;
}

struct i64_pair {
	int64_t	len;
	int64_t	count;
};

static struct i64_pair internal__structure_traverse_prepare (const structure *s, int64_t keylen, int64_t depth) {
	struct i64_pair r = { 0 };

	if (depth == 0)
		keylen += strlen (s->key);
	else
		keylen += 1 + strlen (s->key);	// '.' + key

	switch (s->type) {
		case STRUCTURE_TYPE_NULL:
		default: {
			return r;	// we don't need this
		}

		case STRUCTURE_TYPE_SUB: {
			int x;
			struct i64_pair t;
			depth++;
			for (x = 0; x < s->count; x++) {
				t = internal__structure_traverse_prepare (&s->children[x], keylen, depth);
				r.len	+= t.len;
				r.count	+= t.count;
			}

			return r;
		}

		case STRUCTURE_TYPE_BLOB:
		case STRUCTURE_TYPE_STRING: {
			r.count	= 1;
			r.len	= s->len;
			r.len	+= keylen + strlen (s->key) + 1;	// NULL terminator
			r.len	+= 1 + 2;	// u8 type + u16 len

			return r;
		}

		case STRUCTURE_TYPE_I64:
		case STRUCTURE_TYPE_U64:
		case STRUCTURE_TYPE_H64:
		case STRUCTURE_TYPE_F64:
		case STRUCTURE_TYPE_TIME:
		case STRUCTURE_TYPE_UNIXTIME: {
			r.count	= 1;
			r.len	= sizeof (int64_t);
			r.len	+= keylen + strlen (s->key) + 1;	// NULL terminator
			r.len	+= 1 + 2;	// u8 type + u16 len

			return r;
		}
	}
}

static void internal__structure_traverse_pack (const structure *s, uint32_t *offset,	\
						kv_string stream, const uint8_t *k,
						int64_t depth) {
	uint8_t *key;

	switch (s->type) {
		case STRUCTURE_TYPE_NULL:
		default: {
			return;
		}

		case STRUCTURE_TYPE_SUB:
		case STRUCTURE_TYPE_I64:
		case STRUCTURE_TYPE_U64:
		case STRUCTURE_TYPE_H64:
		case STRUCTURE_TYPE_F64:
		case STRUCTURE_TYPE_BLOB:
		case STRUCTURE_TYPE_STRING:
		case STRUCTURE_TYPE_TIME:
		case STRUCTURE_TYPE_UNIXTIME: {
			if (xm_unlikely (depth == 0))
				key = xm_strdupa (s->key);
			else {
				// max size checked earlier
				int tlen = strlen (k) + 1 + strlen (s->key) + 1; // k + '.' + key + NULL
				key = alloca (tlen);

				snprintf (key, tlen, "%s.%s", k, s->key);
			}

			break;
		}
	}

	switch (s->type) {
		case STRUCTURE_TYPE_SUB: {
			int x;
			depth++;
			for (x = 0; x < s->count; x++)
				internal__structure_traverse_pack (&s->children[x],	\
									offset, stream,	\
									key, depth);
			return;
		}

		case STRUCTURE_TYPE_BLOB:
		case STRUCTURE_TYPE_STRING: {
			// type
			memcpy (&stream.data[*offset], &s->type, sizeof (s->type));
			*offset += sizeof (s->type);
			// key
			strcpy (&stream.data[*offset], key);
			*offset += strlen (key) + 1;	// key + NULL

			// len
			memcpy (&stream.data[*offset], &s->len, sizeof (s->len));
			*offset += sizeof (s->len);
			// data
			memcpy (&stream.data[*offset], s->blob, s->len);
			*offset += s->len;

			return;
		}

		case STRUCTURE_TYPE_I64:
		case STRUCTURE_TYPE_U64:
		case STRUCTURE_TYPE_H64:
		case STRUCTURE_TYPE_F64:
		case STRUCTURE_TYPE_TIME:
		case STRUCTURE_TYPE_UNIXTIME: {
			// type
			memcpy (&stream.data[*offset], &s->type, sizeof (s->type));
			*offset += sizeof (s->type);
			// key
			strcpy (&stream.data[*offset], key);
			*offset += strlen (key) + 1;	// key + NULL

			// len
			uint16_t len = sizeof (int64_t);
			memcpy (&stream.data[*offset], &len, sizeof (len));
			*offset += sizeof (len);
			// data
			memcpy (&stream.data[*offset], &s->i64, sizeof (int64_t));
			*offset += sizeof (int64_t);

			return;
		}
	}
}

kv_string StructurePack (const structure *s) {
	// pseudo structure result:
	//
	//	packed {
	//		u32	len;	// packed total len (including this)
	//		u16	count;	// field count
	//		quad[count] {
	//				u8	type;
	//				str	key;	// NULL terminated
	//				u16	len;	// data length (max 16M-1)
	//				type	data;
	//		}
	//	}

	const kv_string empty = { .data = NULL, .len = 0 };

	struct i64_pair t = internal__structure_traverse_prepare (s, 0, 0);
	t.len += 4 + 2;	// u32 len + u16 count
	if (t.count > ((1 << 16) - 1) || t.len > ((1 << 24) - 1))
		return empty;
	const uint32_t len	= t.len;
	const uint16_t count	= t.count;

	// initialize kv_string structure
	kv_string packed;
	packed.data = malloc (len);
	if (!packed.data)
		return empty;
	packed.len = len;

	uint32_t offset = 0;
	memcpy (&packed.data[offset], &len, sizeof (len));
	offset += sizeof (len);
	memcpy (&packed.data[offset], &count, sizeof (count));
	offset += sizeof (count);

	const uint8_t nullterm = 0;
	internal__structure_traverse_pack (s, &offset, packed, &nullterm, 0);

	return packed;
}
