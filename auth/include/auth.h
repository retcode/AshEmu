/*
 * AshEmu - WoW 1.12.1 Server Emulator
 * Copyright (C) 2025 AshEmu Team
 *
 * auth.h - Auth server types and functions
 */

#ifndef AUTH_H
#define AUTH_H

#include "common.h"
#include "crypto.h"
#include "network.h"
#include "database.h"

/* Auth server port */
#define AUTH_SERVER_PORT 3724

/* Auth session state */
typedef enum {
    AUTH_STATE_INIT,
    AUTH_STATE_CHALLENGED,
    AUTH_STATE_AUTHENTICATED
} auth_state_t;

/* Auth session context */
typedef struct {
    client_t *client;
    srp6_t *srp6;
    account_t account;
    auth_state_t state;
} auth_session_t;

/* Create auth session */
auth_session_t *auth_session_create(client_t *client);

/* Free auth session */
void auth_session_free(auth_session_t *session);

/* Handle auth session (blocking, runs until disconnect) */
void auth_session_handle(auth_session_t *session);

/* Auth server functions */

/* Start auth server (blocking call) */
result_t auth_server_start(void);

/* Stop auth server */
void auth_server_stop(void);

#endif /* AUTH_H */
