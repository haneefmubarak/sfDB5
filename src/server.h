#pragma once

//===	Includes

#include <stdint.h>

#include <pthread.h>
#include <time.h>
#include <sys/socket.h>	// sockaddr_storage

#include <sodium.h>

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
		struct {
			uint64_t in;	// only accept if hearbeat is greater than in
			uint64_t out;	// always send heartbeat bigger than out and increment
		} ctr;
	} heartbeat;
	uint32_t		shard_count;	// number of shard IDs
	struct {	// key = blake2b (
			//		m=(sender{xor}receiver)+(CLOCK_REALTIME >> CRYPTO_UPDATE_PERIOD),
			//		k=server_cluster_key
			//	)
		uint8_t	last[KEY_LEN];
		uint8_t curr[KEY_LEN];
		uint8_t next[KEY_LEN];

		pthread_rwlock_t lock;
	} crypto_key;
} server;

//===	Variables

extern volatile server			server_peer[MAX_SERVERS];	// sorted by pool
extern volatile int			server_self;
extern volatile uint32_t		server_alive_count;		// servers not considered failed over
extern volatile pthread_rwlock_t	server_list_lock;		// don't sort while being read
extern volatile uint64_t		server_list_revision;		// increment if changed

extern uint8_t	server_cluster_saltpepper[SALT_LEN];
// key = scryptsalsa208sha256 (
//				pass = user_password <null terminated plaintext>,
//				salt = server_cluster_saltpepper <bytes>,
//				ops = SCRYPT_OPS <SENSITIVE>,
//				mem = SCRYPT_MEM <SENSITIVE>
//	)
extern uint8_t	server_cluster_key[KEY_LEN];	// takes a while to compute (a second or two)

//===	Functions

static inline struct timespec ServerTimeFromFloat64 (double f) {
	struct timespec t = { 0 };
	// float to int uses floor rounding (truncation)
	t.tv_sec = f;
	f -= t.tv_sec;
	f *= 1000 * 1000 * 1000;	// 1G nanoseconds = 1 second
	t.tv_nsec = f;

	return t;
}

static inline double ServerTimeToFloat64 (struct timespec t) {
	double f = 0;
	f = t.tv_nsec;
	f /= 1000 * 1000 * 1000;	// 1 G ns = 1 s
	f += t.tv_sec;

	return f;
}

int ServerPoolIDCompare (const void *a, const void *b);
