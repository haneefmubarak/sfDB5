#pragma once

//===	Includes

#include <stdint.h>

#include <pthread.h>
#include <time.h>
#include <unistd.h>

#include <sodium.h>


#include "constants.h"
#include "kv/backend.h"
#include "server.h"

//===	Functions

// updates interserver keys periodically
void *CryptoUpdater	(void *p);

// authenticate + encrypt/decrypt payloads
static inline int CryptoAEADServerSend (const server s, const kv_string m, kv_string *o) {
	o->len = NONCE_LEN + m.len + AEAD_LEN;
	o->data = malloc (o->len);
	if (!o->data)
		return -1;

	// o = nonce + ciphertext
	randombytes_buf (o->data, NONCE_LEN);
	return crypto_aead_chacha20poly1305_encrypt (
							&o->data[NONCE_LEN],
							NULL,
							m.data,
							m.len,
							server_cluster_saltpepper,
							SALT_LEN,
							NULL,
							o->data,
							s.crypto_key.curr
	);
}

static inline int CryptoAEADServerRecv (const server s, const kv_string m, kv_string *o) {
	o->len = m.len - (NONCE_LEN + AEAD_LEN);
	o->data = malloc (o->len);
	if (!o->data)
		return -1;

	// try the current key, then one older, then one newer, then fail
	if (crypto_aead_chacha20poly1305_decrypt (
							o->data,
							NULL,
							NULL,
							&m.data[NONCE_LEN],
							m.len,
							server_cluster_saltpepper,
							SALT_LEN,
							m.data,
							s.crypto_key.curr
	) == 0) {
		return 0;
	} else if (crypto_aead_chacha20poly1305_decrypt (
								o->data,
								NULL,
								NULL,
								&m.data[NONCE_LEN],
								m.len,
								server_cluster_saltpepper,
								SALT_LEN,
								m.data,
								s.crypto_key.last
	) == 0) {
		return 0;
	} else if (crypto_aead_chacha20poly1305_decrypt (
								o->data,
								NULL,
								NULL,
								&m.data[NONCE_LEN],
								m.len,
								server_cluster_saltpepper,
								SALT_LEN,
								m.data,
								s.crypto_key.next
	) == 0) {
		return 0;
	}

	free (o->data);
	return -1;
}
