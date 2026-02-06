/*
 * AshEmu - WoW 2.4.3 Server Emulator
 * Copyright (C) 2025 AshEmu Team
 *
 * crypto.h - SRP6 authentication and WorldCrypt header encryption
 */

#ifndef CRYPTO_H
#define CRYPTO_H

#include "common.h"
#include <openssl/bn.h>
#include <openssl/sha.h>

/* SRP6 parameter sizes */
#define SRP6_SALT_SIZE 32
#define SRP6_VERIFIER_SIZE 32
#define SRP6_KEY_SIZE 32
#define SRP6_SESSION_KEY_SIZE 40
#define SRP6_PROOF_SIZE 20

/* SRP6 session context */
typedef struct {
    BIGNUM *N;          /* Prime modulus */
    BIGNUM *g;          /* Generator */
    BIGNUM *v;          /* Verifier */
    BIGNUM *b;          /* Server private ephemeral */
    BIGNUM *B;          /* Server public ephemeral */

    uint8_t salt[SRP6_SALT_SIZE];
    char username[MAX_USERNAME + 1];
    uint8_t session_key[SRP6_SESSION_KEY_SIZE];
    bool has_session_key;
} srp6_t;

/* Create SRP6 context */
srp6_t *srp6_create(void);

/* Free SRP6 context */
void srp6_free(srp6_t *srp);

/* Compute verifier for a new account (static) */
result_t srp6_compute_verifier(
    const char *username,
    const char *password,
    uint8_t salt[SRP6_SALT_SIZE],          /* OUT: generated salt */
    uint8_t verifier[SRP6_VERIFIER_SIZE]   /* OUT: computed verifier */
);

/* Initialize SRP6 session for authentication */
result_t srp6_init(
    srp6_t *srp,
    const char *username,
    const uint8_t salt[SRP6_SALT_SIZE],
    const uint8_t verifier[SRP6_VERIFIER_SIZE]
);

/* Get server public key B (32 bytes) */
result_t srp6_get_B(const srp6_t *srp, uint8_t B[SRP6_KEY_SIZE]);

/* Verify client proof and compute server proof */
result_t srp6_verify_proof(
    srp6_t *srp,
    const uint8_t client_public_key[SRP6_KEY_SIZE],  /* A from client */
    const uint8_t client_proof[SRP6_PROOF_SIZE],     /* M1 from client */
    uint8_t server_proof[SRP6_PROOF_SIZE]            /* M2 output */
);

/* Get session key after successful verification */
const uint8_t *srp6_get_session_key(const srp6_t *srp);

/* TBC WorldCrypt uses 20-byte HMAC-derived key */
#define WORLDCRYPT_KEY_SIZE 20

/* WorldCrypt header encryption context */
typedef struct {
    uint8_t key[WORLDCRYPT_KEY_SIZE];  /* TBC: HMAC-SHA1 derived key */
    uint8_t send_i;
    uint8_t send_j;
    uint8_t recv_i;
    uint8_t recv_j;
    bool initialized;
} worldcrypt_t;

/* Initialize WorldCrypt with session key */
void worldcrypt_init(worldcrypt_t *crypt, const uint8_t session_key[SRP6_SESSION_KEY_SIZE]);

/* Encrypt outgoing header (in-place) */
void worldcrypt_encrypt(worldcrypt_t *crypt, uint8_t *header, size_t len);

/* Decrypt incoming header (in-place) */
void worldcrypt_decrypt(worldcrypt_t *crypt, uint8_t *header, size_t len);

/* Helper: Generate random bytes */
void crypto_random_bytes(uint8_t *buf, size_t len);

/* Helper: Compute SHA1 hash */
void sha1(const uint8_t *data, size_t len, uint8_t hash[SHA_DIGEST_LENGTH]);

#endif /* CRYPTO_H */
