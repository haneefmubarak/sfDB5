#include "queue.h"

queue xmattr_malloc *QueueCreate (int maxlen) {
	queue *q = malloc (sizeof (queue));
	if (!q)
		return NULL;

	q->rq.items = malloc (maxlen * sizeof (void *));
	if (!q->rq.items) {
		free (q);
		return NULL;
	}

	q->wq.items = malloc (maxlen * sizeof (void *));
	if (!q->wq.items) {
		free (q->rq.items);
		free (q);
		return NULL;
	}

	// initialize pthread locks
	if (!pthread_rwlock_init (&q->rwlock, NULL)) {
		free (q->wq.items);
		free (q->rq.items);
		free (q);
	}
	if (!pthread_mutex_init (&q->mutex, NULL)) {
		pthread_rwlock_destroy (&q->rwlock);
		free (q->wq.items);
		free (q->rq.items);
		free (q);
	}

	// initialize remaining values
	q->wq.count = 0;
	q->rq.count = 0;
	q->maxlen = maxlen;

	return q;
}

void QueueFree (queue *q) {
	pthread_mutex_destroy (&q->mutex);
	pthread_rwlock_destroy (&q->rwlock);
	free (q->wq.items);
	free (q->rq.items);
	free (q);

	return;
}

void *QueueRead (queue *q) {
	pthread_rwlock_rdlock (&q->rwlock);

	int64_t item = __sync_fetch_and_sub (&q->rq.count);
	if (item < 0) {	// subqueue is empty
		pthread_rwlock_unlock (&q->rwlock);
		return NULL;
	}

	// fetch pointer from queue
	void *p = q->rq.items[item];
	pthread_rwlock_unlock (&q->rwlock);

	return p;
}

static void internal__queue_service (queue *q) {
	pthread_rwlock_wrlock (&q->rwlock);

	// swap subqueues and zero write count
	{
		void **p = q->rq.items;

		q->rq.items = q.wq.items;
		q->rq.count = q.wq.count;

		q->wq.items = p;
		q->wq.count = 0;
	}

	pthread_rwlock_unlock (&q->rwlock);

	return;
}

void QueueMultiWrite (queue *q, void **p, int n) {
	pthread_mutex_lock (&q->mutex);

	int x;
	for (x = 0; x < n; x++) {
		// check for empty read queue or full write queue
		do {
			if (q->rq.count < 0)	// service empty queue
				internal__queue_service (q);
			else if (q->wq.count == q->maxlen)	// prevent waiting CPU starvation
				usleep (1000);
		} while (q->wq.count == q->maxlen);

		// add item
		q->wq.items[q->wq.count] = p[x];
		q->wq.count++;
	}

	pthread_mutex_unlock (&q->mutex);
	return;
}

void QueueService (queue *q) {
	pthread_mutex_lock (&q->mutex);
	internal_queue_service (q);
	pthread_mutex_unlock (&q->mutex);
}
