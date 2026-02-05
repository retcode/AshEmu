/*
 * AshEmu - WoW 1.12.1 Server Emulator
 * Copyright (C) 2025 AshEmu Team
 *
 * world/main.c - Standalone world server entry point
 */

#include "world.h"
#include "database.h"
#include <signal.h>

static volatile bool g_running = true;

static void signal_handler(int sig) {
    (void)sig;
    g_running = false;
    world_server_stop();
}

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    LOG_INFO("AshEmu", "World Server starting...");

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

    /* Start world server (blocking) */
    result_t result = world_server_start();

    /* Cleanup */
    database_shutdown();
    network_cleanup();

    LOG_INFO("AshEmu", "World Server stopped");
    return result == OK ? 0 : 1;
}
