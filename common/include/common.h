/*
 * AshEmu - WoW 2.4.3 Server Emulator
 * Copyright (C) 2025 AshEmu Team
 *
 * common.h - Platform types, logging, and error codes
 */

#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef SOCKET socket_t;
    /* INVALID_SOCKET and SOCKET_ERROR defined by winsock2.h */
    #define socket_close closesocket
    #define socket_errno WSAGetLastError()
    /* ssize_t is not defined on Windows */
    #include <BaseTsd.h>
    typedef SSIZE_T ssize_t;
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <errno.h>
    typedef int socket_t;
    #define INVALID_SOCKET (-1)
    #define SOCKET_ERROR (-1)
    #define socket_close close
    #define socket_errno errno
#endif

/* Result codes */
typedef enum {
    OK = 0,
    ERR_MEMORY = -1,
    ERR_NETWORK = -2,
    ERR_DATABASE = -3,
    ERR_CRYPTO = -4,
    ERR_INVALID_PARAM = -5,
    ERR_NOT_FOUND = -6,
    ERR_ALREADY_EXISTS = -7,
    ERR_BUFFER_OVERFLOW = -8,
    ERR_AUTH_FAILED = -9,
    ERR_DISCONNECTED = -10
} result_t;

/* Logging macros */
#define LOG_INFO(component, fmt, ...) \
    do { fprintf(stdout, "[%s] " fmt "\n", component, ##__VA_ARGS__); fflush(stdout); } while(0)

#define LOG_ERROR(component, fmt, ...) \
    do { fprintf(stderr, "[%s] ERROR: " fmt "\n", component, ##__VA_ARGS__); fflush(stderr); } while(0)

#define LOG_DEBUG(component, fmt, ...) \
    do { fprintf(stdout, "[%s] DEBUG: " fmt "\n", component, ##__VA_ARGS__); fflush(stdout); } while(0)

/* Memory helpers */
#define ALLOC(type) ((type*)calloc(1, sizeof(type)))
#define ALLOC_ARRAY(type, count) ((type*)calloc((count), sizeof(type)))
#define FREE(ptr) do { if (ptr) { free(ptr); (ptr) = NULL; } } while(0)

/* String helpers */
#define MAX_USERNAME 32
#define MAX_CHARACTER_NAME 12
#define MAX_REALM_NAME 32
#define MAX_PATH 260

/* Initialize networking (call once at startup) */
int network_init(void);

/* Cleanup networking (call once at shutdown) */
void network_cleanup(void);

/* Get current tick count in milliseconds */
uint32_t get_tick_count(void);

/* Convert string to uppercase in-place */
void to_upper(char *str);

/* Safe string copy with null termination */
void safe_strncpy(char *dst, const char *src, size_t dst_size);

#endif /* COMMON_H */
