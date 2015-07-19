#include "test.h"
#include "../structure.h"

const char *key[4] = { "head", "number", "integer", "string" };
const char *text = "text";

void printstructure (const structure *s) {
	switch (s->type) {
		case STRUCTURE_TYPE_SUB: {
			printf ("Structure:\n"	\
				"\tType:\t%s\n"	\
				"\tKey:\t%s\n"	\
				"\tnumber Count:\t%i\n\n",
				"(Sub)Structure", s->key, s->count);
			break;
		}

		case STRUCTURE_TYPE_STRING: {
//			assert (s->len == (strlen (s->string) + 1));	// NULL terminator
			printf ("Structure:\n"	\
				"\tType:\t%s\n"	\
				"\tKey:\t%s\n"	\
				"\tValue:\t%s\n" \
				"\tLength:\t%i\n\n",
				"String", s->key, s->string, s->len);
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

		case STRUCTURE_TYPE_F64: {
			printf ("Structure:\n"	\
				"\tType:\t%s\n"	\
				"\tKey:\t%s\n"	\
				"\tValue:\t%f\n\n",
				"64-Bit FP Number", s->key, (float) s->f64);
			break;
		}
	}

	return;
}

void test (void) {
	// setup
	assert (!StructureInitialize ());

	structure *head, *number, *integer, *string;

	// basic allocations
	head = malloc (sizeof (structure));
	head->key = strdup (key[0]);
	head->type = STRUCTURE_TYPE_SUB;
	head->count = 0;

	number = malloc (sizeof (structure));
	number->key = strdup (key[1]);
	number->type = STRUCTURE_TYPE_F64;
	number->f64 = 3.1415;

	integer = malloc (sizeof (structure));
	integer->key = strdup (key[2]);
	integer->type = STRUCTURE_TYPE_I64;
	integer->i64 = -42;

	string = malloc (sizeof (structure));
	string->key = strdup (key[3]);
	string->type = STRUCTURE_TYPE_STRING;
	string->string = strdup (text);
	string->len = strlen (text) + 1;	// NULL terminator

	// lets see the current states
	printf ("\n---Structures Built---\n\n");
	printstructure (head);
	printstructure (number);
	printstructure (integer);
	printstructure (string);

	// lets put them together
	// head	+>	number
	//	->	integer
	//	->	string
	StructureAddChildren (head, integer, 1);
	StructureAddChildren (head, string, 1);
	StructureAddChildren (head, number, 1);

	// cleanup from nesting
	free (integer);
	free (string);
	free (number);

	// lets see how they are now
	printf ("\n---Structures Nested---\n\n");
	printstructure (head);
	printstructure (&head->children[0]);	// should be 'number'
	printstructure (&head->children[1]);	// should be 'integer'
	printstructure (&head->children[2]);	// should be 'string'

	// now lets pack it up
	kv_string packed = StructurePack (head);

	// free all the structures at once
	StructureFree (head);

	// unpack it
	structure *unpacked = StructureUnpack (packed);

	// they should match up with before
	printf ("\n---Structures Packed & Unpacked---\n\n");
	printstructure (unpacked);
	printstructure (&unpacked->children[0]);	// should be 'number'
	printstructure (&unpacked->children[1]);	// should be 'integer'
	printstructure (&unpacked->children[2]);	// should be 'string'

	// final cleanup
	free (packed.data);
	StructureFree (unpacked);
	StructureTerminate ();

	return;
}

#include "test.c"
