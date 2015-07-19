#include "structure.h"

void StructureFree (structure *s) {
	internal__structure_free (*s);
	free (s);
	return;
}

static int internal__structure_get_size (const structure *s, uint16_t keylen, uint8_t depth,
						uint32_t *entries, uint32_t *tbloffset) {
	if (s->type == STRUCTURE_TYPE_NULL)
		return 0;
	else if (s->type > STRUCTURE_TYPE_UNIXTIME)
		return 0;

	int error = 0;

	if (depth == 0)
		keylen += strlen (s->key);
	else
		keylen += 1 + strlen (s->key);	// "."

	if (*entries > (1024 * 1024) || keylen > 4096)
		return 1;

	(*entries)++;
	(*tbloffset) += sizeof (uint16_t) + keylen;

	if (s->type == STRUCTURE_TYPE_SUB) {
		depth++;
		int x;
		for (x = 0; x < s->count; x++) {
			error = internal__structure_get_size (&s->children[x], keylen, depth,
								entries, tbloffset);

			if (error)
				return error;
		}
	}

	return 0;
}

static int internal__structure_pack (structure *s, uint8_t *key, const uint8_t depth,
					kv_string *packed, uint32_t *offset, uint32_t *tbloffset) {
	if (s->type == STRUCTURE_TYPE_NULL)
		return 0;
	else if (s->type > STRUCTURE_TYPE_UNIXTIME)
		return 0;

	int error = 0;

	if (depth == 0)
		key = s->key;
	else {
		key = asprintf ("%s.%s", key, s->key);
		if (!key)
			return 1;
	}

	uint16_t keylen = strlen (key);

	memcpy (&packed->data[*offset], keylen, sizeof (keylen));
	*offset += sizeof (keylen);
	memcpy (&packed->data[*offset], key, keylen);
	*offset += keylen;

	switch (s->type) {
		case STRUCTURE_TYPE_SUB: {
			memcpy (&packed->data[*tbloffset], &s->type, sizeof (s->type));
			*tbloffset += sizeof (s->type);

			depth++;
			int x;
			for (x = 0; x < s->count; x++) {
				error = internal__structure_pack (s, key, depth, packed,
									offset, tbloffset);

				if (error) {
					free (key);
					return error;
				}
			}

			break;
		}

		case STRUCTURE_TYPE_I64:
		case STRUCTURE_TYPE_U64:
		case STRUCTURE_TYPE_H64:
		case STRUCTURE_TYPE_F64:
		case STRUCTURE_TYPE_TIME:
		case STRUCTURE_TYPE_UNIXTIME: {
			if ((*tbloffset + sizeof (uint8_t) + sizeof (uint64_t)) > packed->len) {
				packed->len *= 2;
				void *p = realloc (packed->data, packed->len);

				if (!p)
					return -1;

				packed->data = p;
			}

			memcpy (&packed->data[*tbloffset], &s->type, sizeof (s->type));
			*tbloffset += sizeof (s->type);
			memcpy (*packed->data[*tbloffset], &s->u64, sizeof (s->u64));
			*tbloffset += sizeof (s->u64);

			break;
		}

		case STRUCTURE_TYPE_BLOB:
		case STRUCTURE_TYPE_STRING: {
			if ((*tbloffset + sizeof (uint8_t) + sizeof (uint16_t) + s->len)
					> packed->len)
				packed->len *= 2;
				void *p = realloc (packed->data, packed->len);

				if (!p)
					return -1;

				packed->data = p;
			}

			memcpy (&packed->data[*tbloffset], &s->type, sizeof (s->type));
			*tbloffset += sizeof (s->type);
			memcpy (&packed->data[*tbloffset], &s->len, sizeof (s->len));
			*tbloffset += sizeof (s->len);
			memcpy (&packed->data[*tbloffset], s->blob, s->len);
			*tbloffset += s->len;

			break;
		}
	}

	free (key);
	return 0;
}

kv_string StructurePack (const structure *s) {
	// Packed Format:
	//
	//	u32:	number of entries
	//	u32:	key table byte size (member table offset)
	//	[
	//		u16:	length of member key
	//			u8[]:	member key
	//	]
	//	[
	//		u8:	type of member value
	//		#u16:	length of member value	(if needed)
	//			u8[]:	member value
	//	]

	const kv_string empty = { .data = NULL, .len = 0 };

	uint32_t entries = 0, tbloffset = 0;
	int error = internal__structure_get_size (s, 0, 0, &entries, &tbloffset);

	if (error)
		return empty;


	kv_string packed;
	tbloffset	+= 2 * sizeof (uint32_t)
	packed.len	= 2 * tbloffset;	// mul2 is for initial memval room
	packed.data	= malloc (packed.len);
	if (!packed.data)
		return empty;

	uint32_t offset = 0;
	memcpy (&packed.data[offset], &entries, sizeof (entries));
	offset += sizeof (entries);
	memcpy (&packed.data[offset], &tbloffset, sizeof (tbloffset));
	offset += sizeof (tbloffset);

	error = internal__structure_pack (s, NULL, 0, &packed, &offset, &tbloffset);

	if (error) {
		free (packed.data);
		return empty;
	}

	return packed;
}

structure *StructureUnpack (const kv_string packed) {
	uint32_t offset = 0, tbloffset = 0;

	uint32_t entries;
	memcpy (&entries, &packed.data[offset], sizeof (entries));
	offset += sizeof (entries);
	memcpy (&tbloffset, &packed.data[offset], sizeof (tbloffset));
	offset += sizeof (tbloffset);

	if (tbloffset <= packed.len) {
		return NULL;

	structure *unpacked = malloc (entries * sizeof (structure));
	if (!unpacked)
		return NULL;

	int x;
	structure *stack[256];
	uint8_t depth = 0;
	const uint8_t *prevpath[256] = { 0 };
	for (x = 0; x < entries; x++) {
		uint16_t pathlen;
		memcpy (&pathlen, &packed.data[offset], sizeof (pathlen));
		offset += sizeof (pathlen);
		if (xm_unlikely ((offset + pathlen) > tbloffset))
			goto StructureUnpackErrorCleanup;

		const uint8_t *path = calloc (1, pathlen + 1);
		if (!path)
			goto StructureUnpackErrorCleanup;
		memcpy (path, &packed.data[offset], pathlen);

		const uint8_t *curpath[256] = { 0 };
		// if we are on the first token
		if (x == 0) {
			if (strchr (path, '.')) {
				free (path);
				return NULL;
			}

			stack[0] = calloc (1, sizeof (structure));
			if (!stack[0]) {
				free (path);
				return NULL;
			}

			stack[0]->key = strdup (path);
			if (!stack[0]->key) {
				free (stack[0]);
				free (path);
				return NULL;
			}

			depth = 0;
		} else {
			// decompose the path into tokens
			int y = 0, l = 0, n = 0;
			while (path[y] != 0) {
				while (path[y] != '.' && path[y] != 0)
					y++;

				curpath[n] = strndup (&path[y], y - l);
				if (!curpath[n]) {
					for (y = 0; y < n; y++)
						free (curpath[y]);
					goto StructureUnpackErrorCleanup;
				}
			}

			// match the current path to the previous path
			const uint8_t curpathlen = y;
			while (
		}
	}

	return stack[0];

	StructureUnpackErrorCleanup:
	int y;
	for (y = 0; y < depth; y++)
		free (prevpath[y]);
	for (y = 0; y < x; y++) {
		switch (unpacked[y]->type) {
			case STRUCTURE_TYPE_BLOB:
			case STRUCTURE_TYPE_STRING: {
				free (unpacked[y]->blob);
				// fallthrough
			}

			default: {
				free (unpacked[y]);
				break;
			}
		}
	}

	return NULL;
}
