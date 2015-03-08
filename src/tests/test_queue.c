#include "test.h"
#include "../queue.h"

const int subqueue_size = 8;

volatile int done = 0, rcvd = 0;

void *produce (void *p) {
	queue *q = p;

	int x, y;
	for (x = 0; x < 8; x++) {
		for (y = 0; y < subqueue_size; y++) {
			char *message;
			asprintf (&message, "Single Write:\tPack %i,\tMessage %i", x, y);
			assert (message);

			QueueWrite (q, message);
		}
	}

	while (!done) {
		QueueService (q);
	}

	return NULL;
}

void *consume (void *p) {
	queue *q = p;

	while (rcvd < (8 * subqueue_size)) {
		char *message = QueueRead (q);
		if (!message)
			continue;

		puts (message);
		free (message);
		__sync_add_and_fetch (&rcvd, 1);
	}


	done = 1;
	return NULL;
}

void test (void) {
	queue *q = QueueCreate (subqueue_size);	// subqueue can hold 4 messages
	assert (q);

	pthread_t producer;
	pthread_t consumers[2];
	assert (!pthread_create (&producer, NULL, produce, q));
	int x;
	for (x = 0; x < 2; x++) {
		assert (!pthread_create (&consumers[x], NULL, consume, q));
		assert (!pthread_detach (consumers[x]));
	}

	assert (!pthread_join (producer, NULL));
	QueueFree (q);

	return;
}

#include "test.c"
