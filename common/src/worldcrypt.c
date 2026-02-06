/*
 * AshEmu - WoW 2.4.3 Server Emulator (TBC)
 * Copyright (C) 2025 AshEmu Team
 *
 * worldcrypt.c - WorldCrypt header encryption (RC4-like cipher)
 *
 * TBC 2.4.3 uses HMAC-SHA1 to derive a 20-byte key from the session key.
 */

#include "crypto.h"
#include <openssl/hmac.h>

/* TBC encryption seed for HMAC-SHA1 key derivation */
static const uint8_t TBC_ENCRYPTION_SEED[16] = {
    0x38, 0xA7, 0x83, 0x15, 0xF8, 0x92, 0x25, 0x30,
    0x71, 0x98, 0x67, 0xB1, 0x8C, 0x04, 0xE2, 0xAA
};

void worldcrypt_init(worldcrypt_t *crypt, const uint8_t session_key[SRP6_SESSION_KEY_SIZE]) {
    /* TBC: Derive 20-byte key using HMAC-SHA1(seed, session_key) */
    unsigned int key_len = WORLDCRYPT_KEY_SIZE;
    HMAC(EVP_sha1(), TBC_ENCRYPTION_SEED, sizeof(TBC_ENCRYPTION_SEED),
         session_key, SRP6_SESSION_KEY_SIZE, crypt->key, &key_len);

    crypt->send_i = 0;
    crypt->send_j = 0;
    crypt->recv_i = 0;
    crypt->recv_j = 0;
    crypt->initialized = true;
}

void worldcrypt_encrypt(worldcrypt_t *crypt, uint8_t *header, size_t len) {
    if (!crypt->initialized) return;

    for (size_t i = 0; i < len; i++) {
        crypt->send_i %= WORLDCRYPT_KEY_SIZE;
        uint8_t x = (header[i] ^ crypt->key[crypt->send_i]) + crypt->send_j;
        header[i] = x;
        crypt->send_j = x;
        crypt->send_i++;
    }
}

void worldcrypt_decrypt(worldcrypt_t *crypt, uint8_t *header, size_t len) {
    if (!crypt->initialized) return;

    for (size_t i = 0; i < len; i++) {
        crypt->recv_i %= WORLDCRYPT_KEY_SIZE;
        uint8_t original = header[i];
        uint8_t x = (header[i] - crypt->recv_j) ^ crypt->key[crypt->recv_i];
        crypt->recv_j = original;
        header[i] = x;
        crypt->recv_i++;
    }
}
