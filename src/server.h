#pragma once

//===	Includes

#include <stdint.h>

#include <pthread.h>
#include <sys/socket.h>	// sockaddr_storage

#include "constants.h"
#include "rendezvous.h"

//===	Structures

typedef struct {
	__uint128_t		pool;		// nonzero server unique random ID
	struct sockaddr_storage	addr;		// socket address
	uint64_t		*shards;	// assigned shard IDs
	uint8_t			*name;		// server name (ideally in DNS)
	struct {
		double	in;	// latest received packet (in monotonic seconds)
		double	out;	// latest sent packet (in monotonic seconds)
	} heartbeat;
	uint32_t		shard_count;	// number of shard IDs
} server;

//===	Variables

extern server		server_peer[MAX_SERVERS];	// 0 is always self
extern uint32_t		server_alive_count;
extern pthread_rwlock_t	server_list_lock;
