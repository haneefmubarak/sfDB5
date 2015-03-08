#pragma once

//	====================================
//	============== WARNING =============
//	====================================
//
//	This queue is not strongly ordered.
//	In other words, while the general
//	order is preserved to an extent in
//	the long run (ie: nothing should go
//	unread for extended periods of
//	time), in the short run, contents
//	may end up partially reordered
//	(usually reversed).

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
void QueueWrite		(queue *q, void *p);	// will block if subqueue is full
void QueueService	(queue *q);
