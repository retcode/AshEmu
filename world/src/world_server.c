/*
 * AshEmu - WoW 1.12.1 Server Emulator
 * Copyright (C) 2025 AshEmu Team
 *
 * world_server.c - World server main loop
 */

#include "world.h"
#include "network.h"

static server_t *g_world_server = NULL;

/* Client handler callback */
static void world_client_handler(client_t *client, void *userdata) {
    (void)userdata;

    world_session_t *session = world_session_create(client);
    if (!session) {
        LOG_ERROR("WorldServer", "Failed to create session");
        client_free(client);
        return;
    }

    world_session_handle(session);
    world_session_free(session);
}

result_t world_server_start(void) {
    g_world_server = server_create(WORLD_SERVER_PORT, "WorldServer");
    if (!g_world_server) {
        return ERR_MEMORY;
    }

    return server_run(g_world_server, world_client_handler, NULL);
}

void world_server_stop(void) {
    if (g_world_server) {
        server_stop(g_world_server);
    }
}
