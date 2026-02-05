/*
 * AshEmu - WoW 1.12.1 Server Emulator
 * Copyright (C) 2025 AshEmu Team
 *
 * worldcrypt.c - WorldCrypt header encryption (RC4-like cipher)
 */

#include "crypto.h"

void worldcrypt_init(worldcrypt_t *crypt, const uint8_t session_key[SRP6_SESSION_KEY_SIZE]) {
    memcpy(crypt->session_key, session_key, SRP6_SESSION_KEY_SIZE);
    crypt->send_i = 0;
    crypt->send_j = 0;
    crypt->recv_i = 0;
    crypt->recv_j = 0;
    crypt->initialized = true;
}

void worldcrypt_encrypt(worldcrypt_t *crypt, uint8_t *header, size_t len) {
    if (!crypt->initialized) return;

    for (size_t i = 0; i < len; i++) {
        crypt->send_i %= 40;
        uint8_t x = (header[i] ^ crypt->session_key[crypt->send_i]) + crypt->send_j;
        header[i] = x;
        crypt->send_j = x;
        crypt->send_i++;
    }
}

void worldcrypt_decrypt(worldcrypt_t *crypt, uint8_t *header, size_t len) {
    if (!crypt->initialized) return;

    for (size_t i = 0; i < len; i++) {
        crypt->recv_i %= 40;
        uint8_t original = header[i];
        uint8_t x = (header[i] - crypt->recv_j) ^ crypt->session_key[crypt->recv_i];
        crypt->recv_j = original;
        header[i] = x;
        crypt->recv_i++;
    }
}
