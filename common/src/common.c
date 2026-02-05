/*
 * AshEmu - WoW 1.12.1 Server Emulator
 * Copyright (C) 2025 AshEmu Team
 *
 * common.c - Platform utilities implementation
 */

#include "common.h"
#include <ctype.h>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <sys/time.h>
#endif

static bool g_network_initialized = false;

int network_init(void) {
#ifdef _WIN32
    if (!g_network_initialized) {
        WSADATA wsa_data;
        int result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
        if (result != 0) {
            return ERR_NETWORK;
        }
        g_network_initialized = true;
    }
#else
    g_network_initialized = true;
#endif
    return OK;
}

void network_cleanup(void) {
#ifdef _WIN32
    if (g_network_initialized) {
        WSACleanup();
        g_network_initialized = false;
    }
#endif
}

uint32_t get_tick_count(void) {
#ifdef _WIN32
    return GetTickCount();
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint32_t)(tv.tv_sec * 1000 + tv.tv_usec / 1000);
#endif
}

void to_upper(char *str) {
    if (!str) return;
    while (*str) {
        *str = (char)toupper((unsigned char)*str);
        str++;
    }
}

void safe_strncpy(char *dst, const char *src, size_t dst_size) {
    if (!dst || !src || dst_size == 0) return;

    size_t i;
    for (i = 0; i < dst_size - 1 && src[i] != '\0'; i++) {
        dst[i] = src[i];
    }
    dst[i] = '\0';
}
