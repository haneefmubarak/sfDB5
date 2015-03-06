#pragma once

//	====================================
//	============== WARNING =============
//	====================================
//
//	This queue does not preserve ordering
//	below `maxlen, the maximum length of
//	the subqueue.

//===	Includes

#include <stdint.h>
#include <stdlib.h>

#include <pthread.h>
#include <unistd.h>

#include "xm.h"

//=== Structures

typedef struct queue {
	pthread_rwlock_t	rwlock;
	pthread_mutex_t		mutex;
	struct {
		void			**items;
		volatile int64_t	count;
	} rq;
	struct {
		void	**items;
		int64_t	count;
	} wq;
	int maxlen;
} queue;

//===	Functions

queue xmattr_malloc *QueueCreate	(int maxlen);
void QueueFree				(queue *q);

void *QueueRead		(queue *q);	// will return NULL if subqueue is empty
void QueueMultiWrite	(queue *q, void **p, int n); // will block if subqueue is full
void QueueService	(queue *q);

void QueueWrite	(queue *q, void *p) {
	return QueueMultiWrite (q, &p, 1);
}
