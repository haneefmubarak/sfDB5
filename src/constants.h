#pragma once

//===	Includes

#include <stdint.h>

#include <sodium.h>

#include "xm.h"

//===	Defines

// SI multipliers
#define	SI_K	(1000)		// may exceed range:	2^20 > int8_t (2^7), uint8_t (2^8),
//								int16_t (2^15), uint16_t (2^16)
#define	SI_M	(SI_K * SI_K)	// may lose precision:	2^30 > float (2^24)
#define	SI_G	(SI_M * SI_K)	// may exceed range:	2^40 > int32_t (2^31), uint32_t (2^32)
#define	SI_T	(SI_G * SI_K)
#define	SI_P	(SI_T * SI_K)	// may lose precision:	2^60 > double (2^53)

// CS multipliers
#define	CS_K	(1024)		// may exceed range:	2^20 > int8_t (2^7), uint8_t (2^8),
//								int16_t (2^15), uint16_t (2^16)
#define	CS_M	(CS_K * CS_K)	// may lose precision:	2^30 > float (2^24)
#define	CS_G	(CS_M * CS_K)	// may exceed range:	2^40 > int32_t (2^31), uint32_t (2^32)
#define	CS_T	(CS_G * CS_K)
#define	CS_P	(CS_T * CS_K)	// may lose precision:	2^60 > double (2^53)

// cluster shit
#define MAX_SERVERS	(4 * CS_K)			// maximum total servers
#define	MIN_SHARDS	(CS_K / 4)			// minimum probabilistic shards / server
#define	SHARDS		(MAX_SERVERS * MIN_SHARDS)	// total shards

// heartbeat timing (latency vs bandwidth)
#define	HEARTBEAT_PERIOD	(2.5 * SI_M)		// how often should we pulse (in ns)
#define	HEARTBEAT_FREQ		(SI_G / HEARTBEAT_PERIOD)	// f (in Hz) = 1 / t

// crypto
#define	CRYPTO_UPDATE_PERIOD	(6)	// 64 = 2^6
#define	KEY_LEN			(crypto_aead_chacha20poly1305_KEYBYTES)
#define	NONCE_LEN		(crypto_aead_chacha20poly1305_NPUBBYTES)
#define	AEAD_LEN		(crypto_aead_chacha20poly1305_ABYTES)
#define	SALT_LEN		(crypto_pwhash_scryptsalsa208sha256_SALTBYTES)
#define	SCRYPT_OPS		(crypto_pwhash_scryptsalsa208sha256_OPSLIMIT_SENSITIVE)
#define	SCRYPT_MEM		(crypto_pwhash_scryptsalsa208sha256_MEMLIMIT_SENSITIVE)

// structures
#define	STRUCTURE_PACK_MAX_SIZE	(1 << 24)	// 16 MB
