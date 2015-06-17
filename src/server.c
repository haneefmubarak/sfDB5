#include "server.h"

server server_peer[MAX_SERVERS] = { 0 };
uint32_t server_alive_count = 0;
pthread_rwlock_t server_list_lock = PTHREAD_RWLOCK_INITIALIZER;
