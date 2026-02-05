/*
 * AshEmu - WoW 1.12.1 Server Emulator
 * Copyright (C) 2025 AshEmu Team
 *
 * crypto.c - SRP6 authentication implementation using OpenSSL BIGNUM
 *
 * WoW uses little-endian byte order for BigIntegers.
 * OpenSSL BIGNUM uses big-endian, so we need conversion functions.
 */

#include "crypto.h"
#include <openssl/rand.h>
#include <ctype.h>

/* WoW-specific SRP6 parameters */
/* Prime modulus N (32 bytes, stored big-endian here for BIGNUM) */
static const uint8_t SRP6_N_BYTES[] = {
    0x89, 0x4B, 0x64, 0x5E, 0x89, 0xE1, 0x53, 0x5B,
    0xBD, 0xAD, 0x5B, 0x8B, 0x29, 0x06, 0x50, 0x53,
    0x08, 0x01, 0xB1, 0x8E, 0xBF, 0xBF, 0x5E, 0x8F,
    0xAB, 0x3C, 0x82, 0x87, 0x2A, 0x3E, 0x9B, 0xB7
};

/* Generator g */
#define SRP6_G 7

/* Multiplier k */
#define SRP6_K 3

/* Helper: Convert little-endian bytes to BIGNUM */
static BIGNUM *bn_from_le_bytes(const uint8_t *data, size_t len) {
    /* Reverse bytes for big-endian BIGNUM */
    uint8_t *reversed = (uint8_t*)malloc(len);
    if (!reversed) return NULL;

    for (size_t i = 0; i < len; i++) {
        reversed[i] = data[len - 1 - i];
    }

    BIGNUM *bn = BN_bin2bn(reversed, (int)len, NULL);
    free(reversed);
    return bn;
}

/* Helper: Convert BIGNUM to little-endian bytes with specified size */
static void bn_to_le_bytes(const BIGNUM *bn, uint8_t *out, size_t out_size) {
    memset(out, 0, out_size);

    int bn_size = BN_num_bytes(bn);
    uint8_t *temp = (uint8_t*)malloc(bn_size);
    if (!temp) return;

    BN_bn2bin(bn, temp);

    /* Reverse from big-endian to little-endian, padding if needed */
    size_t copy_size = (size_t)bn_size < out_size ? (size_t)bn_size : out_size;
    for (size_t i = 0; i < copy_size; i++) {
        out[i] = temp[bn_size - 1 - i];
    }

    free(temp);
}

/* Helper: Compute SHA1 */
void sha1(const uint8_t *data, size_t len, uint8_t hash[SHA_DIGEST_LENGTH]) {
    SHA_CTX ctx;
    SHA1_Init(&ctx);
    SHA1_Update(&ctx, data, len);
    SHA1_Final(hash, &ctx);
}

/* Helper: Generate random bytes */
void crypto_random_bytes(uint8_t *buf, size_t len) {
    RAND_bytes(buf, (int)len);
}

/* Create SRP6 context */
srp6_t *srp6_create(void) {
    srp6_t *srp = ALLOC(srp6_t);
    if (!srp) return NULL;

    /* Initialize N from big-endian bytes */
    srp->N = BN_bin2bn(SRP6_N_BYTES, sizeof(SRP6_N_BYTES), NULL);
    if (!srp->N) {
        free(srp);
        return NULL;
    }

    srp->g = BN_new();
    if (!srp->g) {
        BN_free(srp->N);
        free(srp);
        return NULL;
    }
    BN_set_word(srp->g, SRP6_G);

    srp->v = NULL;
    srp->b = NULL;
    srp->B = NULL;
    srp->has_session_key = false;
    memset(srp->username, 0, sizeof(srp->username));
    memset(srp->salt, 0, sizeof(srp->salt));
    memset(srp->session_key, 0, sizeof(srp->session_key));

    return srp;
}

/* Free SRP6 context */
void srp6_free(srp6_t *srp) {
    if (!srp) return;
    BN_free(srp->N);
    BN_free(srp->g);
    BN_free(srp->v);
    BN_free(srp->b);
    BN_free(srp->B);
    free(srp);
}

/* Compute x = SHA1(salt || SHA1(credentials)) */
static BIGNUM *compute_x(const char *username, const char *password, const uint8_t salt[SRP6_SALT_SIZE]) {
    /* Compute SHA1(credentials) where credentials = UPPER(username):UPPER(password) */
    char credentials[256];
    snprintf(credentials, sizeof(credentials), "%s:%s", username, password);
    to_upper(credentials);

    uint8_t cred_hash[SHA_DIGEST_LENGTH];
    sha1((uint8_t*)credentials, strlen(credentials), cred_hash);

    /* Compute SHA1(salt || cred_hash) */
    uint8_t x_input[SRP6_SALT_SIZE + SHA_DIGEST_LENGTH];
    memcpy(x_input, salt, SRP6_SALT_SIZE);
    memcpy(x_input + SRP6_SALT_SIZE, cred_hash, SHA_DIGEST_LENGTH);

    uint8_t x_hash[SHA_DIGEST_LENGTH];
    sha1(x_input, sizeof(x_input), x_hash);

    return bn_from_le_bytes(x_hash, SHA_DIGEST_LENGTH);
}

/* Compute verifier: v = g^x mod N */
result_t srp6_compute_verifier(
    const char *username,
    const char *password,
    uint8_t salt[SRP6_SALT_SIZE],
    uint8_t verifier[SRP6_VERIFIER_SIZE]
) {
    /* Generate random salt */
    crypto_random_bytes(salt, SRP6_SALT_SIZE);

    /* Compute x */
    BIGNUM *x = compute_x(username, password, salt);
    if (!x) return ERR_CRYPTO;

    /* Load N and g */
    BIGNUM *N = BN_bin2bn(SRP6_N_BYTES, sizeof(SRP6_N_BYTES), NULL);
    BIGNUM *g = BN_new();
    BN_set_word(g, SRP6_G);

    /* v = g^x mod N */
    BIGNUM *v = BN_new();
    BN_CTX *ctx = BN_CTX_new();

    BN_mod_exp(v, g, x, N, ctx);

    /* Convert to little-endian bytes */
    bn_to_le_bytes(v, verifier, SRP6_VERIFIER_SIZE);

    BN_CTX_free(ctx);
    BN_free(v);
    BN_free(x);
    BN_free(g);
    BN_free(N);

    return OK;
}

/* Initialize SRP6 session for authentication */
result_t srp6_init(
    srp6_t *srp,
    const char *username,
    const uint8_t salt[SRP6_SALT_SIZE],
    const uint8_t verifier[SRP6_VERIFIER_SIZE]
) {
    /* Store username (uppercase) */
    safe_strncpy(srp->username, username, sizeof(srp->username));
    to_upper(srp->username);

    /* Store salt */
    memcpy(srp->salt, salt, SRP6_SALT_SIZE);

    /* Convert verifier from little-endian */
    srp->v = bn_from_le_bytes(verifier, SRP6_VERIFIER_SIZE);
    if (!srp->v) return ERR_CRYPTO;

    /* Generate random private key b (19 bytes) */
    uint8_t b_bytes[19];
    crypto_random_bytes(b_bytes, sizeof(b_bytes));
    srp->b = bn_from_le_bytes(b_bytes, sizeof(b_bytes));
    if (!srp->b) return ERR_CRYPTO;

    /* Compute B = (k*v + g^b) mod N */
    BN_CTX *ctx = BN_CTX_new();
    BIGNUM *kv = BN_new();
    BIGNUM *gb = BN_new();
    srp->B = BN_new();

    /* kv = k * v */
    BIGNUM *k = BN_new();
    BN_set_word(k, SRP6_K);
    BN_mod_mul(kv, k, srp->v, srp->N, ctx);

    /* gb = g^b mod N */
    BN_mod_exp(gb, srp->g, srp->b, srp->N, ctx);

    /* B = (kv + gb) mod N */
    BN_mod_add(srp->B, kv, gb, srp->N, ctx);

    BN_free(k);
    BN_free(kv);
    BN_free(gb);
    BN_CTX_free(ctx);

    srp->has_session_key = false;
    return OK;
}

/* Get server public key B */
result_t srp6_get_B(const srp6_t *srp, uint8_t B[SRP6_KEY_SIZE]) {
    if (!srp->B) return ERR_INVALID_PARAM;
    bn_to_le_bytes(srp->B, B, SRP6_KEY_SIZE);
    return OK;
}

/* Compute interleaved session key from shared secret S */
static void compute_session_key(const uint8_t *S, size_t S_len, uint8_t session_key[SRP6_SESSION_KEY_SIZE]) {
    /* Find start index (skip leading zeros, align to even) */
    size_t start = 0;
    while (start < S_len && S[start] == 0) {
        start++;
    }
    if (start % 2 == 1) {
        start++;
    }

    size_t len = S_len - start;
    size_t half_len = len / 2;

    /* Split into even and odd bytes */
    uint8_t *even_bytes = (uint8_t*)malloc(half_len);
    uint8_t *odd_bytes = (uint8_t*)malloc(half_len);

    for (size_t i = 0; i < half_len; i++) {
        even_bytes[i] = S[start + i * 2];
        odd_bytes[i] = S[start + i * 2 + 1];
    }

    /* Hash each half */
    uint8_t even_hash[SHA_DIGEST_LENGTH];
    uint8_t odd_hash[SHA_DIGEST_LENGTH];
    sha1(even_bytes, half_len, even_hash);
    sha1(odd_bytes, half_len, odd_hash);

    /* Interleave the hashes */
    for (int i = 0; i < 20; i++) {
        session_key[i * 2] = even_hash[i];
        session_key[i * 2 + 1] = odd_hash[i];
    }

    free(even_bytes);
    free(odd_bytes);
}

/* Compute M1 = SHA1(H(N) XOR H(g) || H(username) || salt || A || B || K) */
static void compute_M1(
    const srp6_t *srp,
    const uint8_t A[SRP6_KEY_SIZE],
    const uint8_t B[SRP6_KEY_SIZE],
    uint8_t M1[SRP6_PROOF_SIZE]
) {
    /* H(N) - hash of N in little-endian */
    uint8_t N_le[SRP6_KEY_SIZE];
    bn_to_le_bytes(srp->N, N_le, SRP6_KEY_SIZE);
    uint8_t N_hash[SHA_DIGEST_LENGTH];
    sha1(N_le, SRP6_KEY_SIZE, N_hash);

    /* H(g) */
    uint8_t g_byte = SRP6_G;
    uint8_t g_hash[SHA_DIGEST_LENGTH];
    sha1(&g_byte, 1, g_hash);

    /* XOR them */
    uint8_t ng_xor[SHA_DIGEST_LENGTH];
    for (int i = 0; i < SHA_DIGEST_LENGTH; i++) {
        ng_xor[i] = N_hash[i] ^ g_hash[i];
    }

    /* H(username) */
    uint8_t user_hash[SHA_DIGEST_LENGTH];
    sha1((uint8_t*)srp->username, strlen(srp->username), user_hash);

    /* M1 = SHA1(ng_xor || user_hash || salt || A || B || K) */
    size_t m1_input_size = SHA_DIGEST_LENGTH + SHA_DIGEST_LENGTH + SRP6_SALT_SIZE +
                           SRP6_KEY_SIZE + SRP6_KEY_SIZE + SRP6_SESSION_KEY_SIZE;
    uint8_t *m1_input = (uint8_t*)malloc(m1_input_size);
    size_t offset = 0;

    memcpy(m1_input + offset, ng_xor, SHA_DIGEST_LENGTH); offset += SHA_DIGEST_LENGTH;
    memcpy(m1_input + offset, user_hash, SHA_DIGEST_LENGTH); offset += SHA_DIGEST_LENGTH;
    memcpy(m1_input + offset, srp->salt, SRP6_SALT_SIZE); offset += SRP6_SALT_SIZE;
    memcpy(m1_input + offset, A, SRP6_KEY_SIZE); offset += SRP6_KEY_SIZE;
    memcpy(m1_input + offset, B, SRP6_KEY_SIZE); offset += SRP6_KEY_SIZE;
    memcpy(m1_input + offset, srp->session_key, SRP6_SESSION_KEY_SIZE);

    sha1(m1_input, m1_input_size, M1);
    free(m1_input);
}

/* Compute M2 = SHA1(A || M1 || K) */
static void compute_M2(
    const uint8_t A[SRP6_KEY_SIZE],
    const uint8_t M1[SRP6_PROOF_SIZE],
    const uint8_t session_key[SRP6_SESSION_KEY_SIZE],
    uint8_t M2[SRP6_PROOF_SIZE]
) {
    size_t m2_input_size = SRP6_KEY_SIZE + SRP6_PROOF_SIZE + SRP6_SESSION_KEY_SIZE;
    uint8_t *m2_input = (uint8_t*)malloc(m2_input_size);

    memcpy(m2_input, A, SRP6_KEY_SIZE);
    memcpy(m2_input + SRP6_KEY_SIZE, M1, SRP6_PROOF_SIZE);
    memcpy(m2_input + SRP6_KEY_SIZE + SRP6_PROOF_SIZE, session_key, SRP6_SESSION_KEY_SIZE);

    sha1(m2_input, m2_input_size, M2);
    free(m2_input);
}

/* Verify client proof */
result_t srp6_verify_proof(
    srp6_t *srp,
    const uint8_t client_public_key[SRP6_KEY_SIZE],
    const uint8_t client_proof[SRP6_PROOF_SIZE],
    uint8_t server_proof[SRP6_PROOF_SIZE]
) {
    memset(server_proof, 0, SRP6_PROOF_SIZE);

    /* Convert A from little-endian */
    BIGNUM *A = bn_from_le_bytes(client_public_key, SRP6_KEY_SIZE);
    if (!A) return ERR_CRYPTO;

    /* Check A != 0 and A % N != 0 */
    BN_CTX *ctx = BN_CTX_new();
    BIGNUM *A_mod_N = BN_new();
    BN_mod(A_mod_N, A, srp->N, ctx);

    if (BN_is_zero(A) || BN_is_zero(A_mod_N)) {
        BN_free(A);
        BN_free(A_mod_N);
        BN_CTX_free(ctx);
        return ERR_AUTH_FAILED;
    }
    BN_free(A_mod_N);

    /* u = SHA1(A || B) */
    uint8_t A_bytes[SRP6_KEY_SIZE];
    uint8_t B_bytes[SRP6_KEY_SIZE];
    bn_to_le_bytes(A, A_bytes, SRP6_KEY_SIZE);
    bn_to_le_bytes(srp->B, B_bytes, SRP6_KEY_SIZE);

    uint8_t u_input[SRP6_KEY_SIZE * 2];
    memcpy(u_input, A_bytes, SRP6_KEY_SIZE);
    memcpy(u_input + SRP6_KEY_SIZE, B_bytes, SRP6_KEY_SIZE);

    uint8_t u_hash[SHA_DIGEST_LENGTH];
    sha1(u_input, sizeof(u_input), u_hash);

    BIGNUM *u = bn_from_le_bytes(u_hash, SHA_DIGEST_LENGTH);

    /* S = (A * v^u)^b mod N */
    BIGNUM *v_u = BN_new();    /* v^u mod N */
    BIGNUM *Av_u = BN_new();   /* A * v^u mod N */
    BIGNUM *S = BN_new();

    BN_mod_exp(v_u, srp->v, u, srp->N, ctx);
    BN_mod_mul(Av_u, A, v_u, srp->N, ctx);
    BN_mod_exp(S, Av_u, srp->b, srp->N, ctx);

    /* Convert S to bytes */
    uint8_t S_bytes[SRP6_KEY_SIZE];
    bn_to_le_bytes(S, S_bytes, SRP6_KEY_SIZE);

    /* Compute session key */
    compute_session_key(S_bytes, SRP6_KEY_SIZE, srp->session_key);
    srp->has_session_key = true;

    /* Compute expected M1 */
    uint8_t expected_M1[SRP6_PROOF_SIZE];
    compute_M1(srp, A_bytes, B_bytes, expected_M1);

    /* Compare proofs */
    bool match = (memcmp(client_proof, expected_M1, SRP6_PROOF_SIZE) == 0);

    if (match) {
        /* Compute M2 */
        compute_M2(A_bytes, client_proof, srp->session_key, server_proof);
    }

    /* Cleanup */
    BN_free(A);
    BN_free(u);
    BN_free(v_u);
    BN_free(Av_u);
    BN_free(S);
    BN_CTX_free(ctx);

    return match ? OK : ERR_AUTH_FAILED;
}

/* Get session key */
const uint8_t *srp6_get_session_key(const srp6_t *srp) {
    return srp->has_session_key ? srp->session_key : NULL;
}
