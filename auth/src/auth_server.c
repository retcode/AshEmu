/*
 * AshEmu - WoW 2.4.3 Server Emulator
 * Copyright (C) 2025 AshEmu Team
 *
 * auth_server.c - Auth server main loop
 */

#include "auth.h"
#include "network.h"

static server_t *g_auth_server = NULL;

/* Client handler callback */
static void auth_client_handler(client_t *client, void *userdata) {
    (void)userdata;

    auth_session_t *session = auth_session_create(client);
    if (!session) {
        LOG_ERROR("AuthServer", "Failed to create session");
        client_free(client);
        return;
    }

    auth_session_handle(session);
    auth_session_free(session);
}

result_t auth_server_start(void) {
    g_auth_server = server_create(AUTH_SERVER_PORT, "AuthServer");
    if (!g_auth_server) {
        return ERR_MEMORY;
    }

    return server_run(g_auth_server, auth_client_handler, NULL);
}

void auth_server_stop(void) {
    if (g_auth_server) {
        server_stop(g_auth_server);
    }
}
