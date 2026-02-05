/*
 * AshEmu - WoW 1.12.1 Server Emulator
 * Copyright (C) 2025 AshEmu Team
 *
 * auth/main.c - Standalone auth server entry point
 */

#include "auth.h"
#include "database.h"
#include <signal.h>

static volatile bool g_running = true;

static void signal_handler(int sig) {
    (void)sig;
    g_running = false;
    auth_server_stop();
}

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    LOG_INFO("AshEmu", "Auth Server starting...");

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

    /* Start auth server (blocking) */
    result_t result = auth_server_start();

    /* Cleanup */
    database_shutdown();
    network_cleanup();

    LOG_INFO("AshEmu", "Auth Server stopped");
    return result == OK ? 0 : 1;
}
