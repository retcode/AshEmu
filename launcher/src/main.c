/*
 * AshEmu - WoW 1.12.1 Server Emulator
 * Copyright (C) 2025 AshEmu Team
 *
 * launcher/main.c - Combined launcher that runs both auth and world servers
 */

#include "common.h"
#include "database.h"
#include "auth.h"
#include "world.h"

#ifdef _WIN32
    #include <windows.h>
    #include <process.h>
#else
    #include <pthread.h>
#endif

#include <signal.h>

static volatile bool g_running = true;

static void signal_handler(int sig) {
    (void)sig;
    g_running = false;
    auth_server_stop();
    world_server_stop();
}

#ifdef _WIN32
/* Windows thread function for auth server */
static unsigned __stdcall auth_server_thread(void *arg) {
    (void)arg;
    auth_server_start();
    return 0;
}

/* Windows thread function for world server */
static unsigned __stdcall world_server_thread(void *arg) {
    (void)arg;
    world_server_start();
    return 0;
}
#else
/* POSIX thread function for auth server */
static void *auth_server_thread(void *arg) {
    (void)arg;
    auth_server_start();
    return NULL;
}

/* POSIX thread function for world server */
static void *world_server_thread(void *arg) {
    (void)arg;
    world_server_start();
    return NULL;
}
#endif

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    printf("===========================================\n");
    printf("  AshEmu - WoW 1.12.1 Server Emulator\n");
    printf("  Auth Server: Port %d\n", AUTH_SERVER_PORT);
    printf("  World Server: Port %d\n", WORLD_SERVER_PORT);
    printf("===========================================\n\n");

    /* Initialize networking */
    if (network_init() != OK) {
        LOG_ERROR("AshEmu", "Failed to initialize networking");
        return 1;
    }

    /* Initialize database */
    if (database_init("ashemu.db") != OK) {
        LOG_ERROR("AshEmu", "Failed to initialize database");
        network_cleanup();
        return 1;
    }

    /* Setup signal handler */
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    LOG_INFO("AshEmu", "Starting servers...");

#ifdef _WIN32
    /* Start auth server thread */
    HANDLE auth_thread = (HANDLE)_beginthreadex(NULL, 0, auth_server_thread, NULL, 0, NULL);
    if (auth_thread == NULL) {
        LOG_ERROR("AshEmu", "Failed to start auth server thread");
        database_shutdown();
        network_cleanup();
        return 1;
    }

    /* Start world server thread */
    HANDLE world_thread = (HANDLE)_beginthreadex(NULL, 0, world_server_thread, NULL, 0, NULL);
    if (world_thread == NULL) {
        LOG_ERROR("AshEmu", "Failed to start world server thread");
        auth_server_stop();
        WaitForSingleObject(auth_thread, INFINITE);
        CloseHandle(auth_thread);
        database_shutdown();
        network_cleanup();
        return 1;
    }

    /* Wait for threads to complete */
    WaitForSingleObject(auth_thread, INFINITE);
    WaitForSingleObject(world_thread, INFINITE);
    CloseHandle(auth_thread);
    CloseHandle(world_thread);
#else
    pthread_t auth_thread, world_thread;

    /* Start auth server thread */
    if (pthread_create(&auth_thread, NULL, auth_server_thread, NULL) != 0) {
        LOG_ERROR("AshEmu", "Failed to start auth server thread");
        database_shutdown();
        network_cleanup();
        return 1;
    }

    /* Start world server thread */
    if (pthread_create(&world_thread, NULL, world_server_thread, NULL) != 0) {
        LOG_ERROR("AshEmu", "Failed to start world server thread");
        auth_server_stop();
        pthread_join(auth_thread, NULL);
        database_shutdown();
        network_cleanup();
        return 1;
    }

    /* Wait for threads to complete */
    pthread_join(auth_thread, NULL);
    pthread_join(world_thread, NULL);
#endif

    /* Cleanup */
    database_shutdown();
    network_cleanup();

    LOG_INFO("AshEmu", "Server stopped");
    return 0;
}
