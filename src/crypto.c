#include "crypto.h"

void *CryptoUpdater (void *p) {
	uint64_t last = 0;
	uint32_t rev = 0;
	struct timespec ts = { 0 };

	while (1) {
		clock_gettime (CLOCK_REALTIME, &ts);
		double t = ServerTimeToFloat64 (ts);

		// check if necessary to update - likely not
		if ((t - last) >= (1 << CRYPTO_UPDATE_PERIOD) || server_list_revision > rev) {
			pthread_rwlock_rdlock (&server_list_lock);

			rev = server_list_revision;

			// round down
			uint64_t s = t;
			s >>= CRYPTO_UPDATE_PERIOD;

			// store for next check - abridged version
			last = s << CRYPTO_UPDATE_PERIOD;

			// we actually can calculate next message and work backwards
			s++;

			// last part of prehash message is identical across servers
			uint8_t m[sizeof (__uint128_t) + sizeof (s)] = { 0 };
			memcpy (&m[sizeof (__uint128_t)], &s, sizeof (s));

			// we don't need to fetch ourselves repeatedly
			const __uint128_t self = server_peer[server_self].pool;

			int64_t x;
			for (x = 0; x < server_alive_count; x++) {
				const __uint128_t id = self ^ server_peer[x].pool;
				memcpy (&m, &id, sizeof (id));

				// don't let people read while we do changes
				pthread_rwlock_wrlock (&server_peer[x].crypto_key.lock);

				// move old keys
				memcpy (server_peer[x].crypto_key.last,
					server_peer[x].crypto_key.curr,
					KEY_LEN
				);
				memcpy (server_peer[x].crypto_key.curr,
					server_peer[x].crypto_key.next,
					KEY_LEN
				);

				// generate next key
				crypto_generichash_blake2b (
								server_peer[x].crypto_key.next,
								KEY_LEN,
								m,
								sizeof (m),
								server_cluster_key,
								sizeof (server_cluster_key)
				);

				pthread_rwlock_unlock (&server_peer[x].crypto_key.lock);
			}

			pthread_rwlock_unlock (&server_list_lock);
		}

		// rather catch it early
		sleep ((1 << CRYPTO_UPDATE_PERIOD) / 4);
	}

	// never returns
}
