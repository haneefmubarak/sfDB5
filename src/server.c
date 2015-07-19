#include "server.h"

volatile server server_peer[MAX_SERVERS] = {{ 0 }};
volatile int server_self;
volatile uint32_t server_alive_count = 0;
volatile pthread_rwlock_t server_list_lock = PTHREAD_RWLOCK_INITIALIZER;
volatile uint64_t server_list_revision = 0;
uint8_t server_cluster_saltpepper[SALT_LEN];
uint8_t server_cluster_key[KEY_LEN];

int ServerPoolIDCompare (const void *a, const void *b) {
	// this trick works because of server list struct layout
	const __uint128_t *lhs = a;
	const __uint128_t *rhs = b;
	return (lhs < rhs) ? -1 : (lhs > rhs);
}
