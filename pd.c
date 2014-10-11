#include "pd.h"

xm_tlv void *pd_arena;

void pd_init (size_t len, uint8_t *map) {
	int x;
	size_t const block = len;			// arena physical size
	size_t const size = (block / ABLKLEN) * ABLKLEN;	// arena useable size
	arenaheader *header;

	header = (void *) map;
	header->freelist = NULL;		// no free blocks because all are free
	header->size	= size;		// useable size
	header->bounds	= ABLKLEN;	// current bounds

	return;
}

xmattr_malloc void *pd_mallocBK	(void) {
	arenaheader *header = pd_arena;
	uint8_t *ptr;

	if (xm_unlikely (header->freelist)) {	// there's a sitting free block
		ptr = header->freelist;	// return the free block
		void **next = (void **) ptr;
		header->freelist = *next;	// update the free list
	} else if (xm_likely (header->bounds < header->size)) {	// no free blocks
		ptr	= pd_arena;
		ptr	+= header->bounds;
		header->bounds += ABLKLEN;
	} else {	// no more blocks
		ptr	= NULL;
	}

	return ptr;
}


xmattr_malloc void *pd_callocBK	(void) {
	void *ptr = pd_mallocBK ();

	if (xm_likely (ptr))	// allocation was successful
		memset (ptr, 0, ABLKLEN);

	return ptr;
}

void pd_freeBK (void *ptr) {
	arenaheader *header = pd_arena;

	if (xm_likely (ptr)) {	// non-NULL ptr
		void *next = header->freelist;	// get current top of stack
		void **this = ptr;
		*this = next;	// move address of current top of stack to ptr
		header->freelist = ptr;	// push ptr to stack
	}

	return;
}
