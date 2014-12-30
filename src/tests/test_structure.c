#include "test.h"
#include "../structure.h"

const char *key[4] = { "head", "child", "number", "string" };
const char *text = "text";

void printstructure (const structure *s) {
	switch (s->type) {
		case STRUCTURE_TYPE_SUB: {
			printf ("Structure:\n"	\
				"\tType:\t%s\n"	\
				"\tKey:\t%s\n"	\
				"\tChild Count:\t%i\n\n",
				"(Sub)Structure", s->key, s->count);
			break;
		}
		case STRUCTURE_TYPE_I64: {
			printf ("Structure:\n"	\
				"\tType:\t%s\n"	\
				"\tKey:\t%s\n"	\
				"\tValue:\t%i\n\n",
				"64-Bit Signed Integer", s->key, (int) s->i64);
			break;
		}

		case STRUCTURE_TYPE_STRING: {
			assert (s->len == (strlen (s->string) + 1));	// NULL terminator
			printf ("Structure:\n"	\
				"\tType:\t%s\n"	\
				"\tKey:\t%s\n"	\
				"\tValue:\t%s\n" \
				"\tLength:\t%i\n\n",
				"String", s->key, s->string, s->len);
			break;
		}
	}

	return;
}

void test (void) {
	structure *head, *child, *number, *string;

	// basic allocations
	head = malloc (sizeof (structure));
	head->key = strdup (key[0]);
	head->type = STRUCTURE_TYPE_SUB;
	head->count = 0;

	child = malloc (sizeof (structure));
	child->key = strdup (key[1]);
	child->type = STRUCTURE_TYPE_SUB;
	child->count = 0;

	number = malloc (sizeof (structure));
	number->key = strdup (key[2]);
	number->type = STRUCTURE_TYPE_I64;
	number->i64 = -42;

	string = malloc (sizeof (structure));
	string->key = strdup (key[3]);
	string->type = STRUCTURE_TYPE_STRING;
	string->string = strdup (text);
	string->len = strlen (text) + 1;	// NULL terminator

	// lets see the current states
	printf ("\n---Structures Built---\n\n");
	printstructure (head);
	printstructure (child);
	printstructure (number);
	printstructure (string);

	// lets try some nesting
	// head	+>	child ->	number
	//	->	string
	StructureAddChildren (head, string, 1);
	StructureAddChildren (head, child, 1);
	StructureAddChildren (child, number, 1);

	// cleanup from nesting
	free (number);
	free (string);
	free (child);

	// lets see how they are now
	printf ("\n---Structures Nested---\n\n");
	printstructure (head);
	printstructure (&head->children[0]);	// should be 'child'
	printstructure (&head->children[1]);	// should be 'string'
	printstructure (&head->children[0].children[0]);	// should be 'number'

	// now lets pack it up
	kv_string *packed = StructurePack (head);

	// free all the structures at once
	StructureFree (head);

	// unpack it
	structure *unpacked = StructureUnpack (packed);

	// they should match up with before
	printf ("\n---Structures Packed & Unpacked---\n\n");
	printstructure (unpacked);
	printstructure (&unpacked->children[0]);	// should be 'child'
	printstructure (&unpacked->children[1]);	// should be 'string'
	printstructure (&unpacked->children[0].children[0]);	// should be 'number'

	// final cleanup
	free (packed);
	StructureFree (unpacked);

	return;
}

#include "test.c"
