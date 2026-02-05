/*
 * AshEmu - WoW 1.12.1 Server Emulator
 * Copyright (C) 2025 AshEmu Team
 *
 * world.h - World server types and functions
 */

#ifndef WORLD_H
#define WORLD_H

#include "common.h"
#include "crypto.h"
#include "network.h"
#include "database.h"
#include "player.h"

/* World server port */
#define WORLD_SERVER_PORT 8085

/* World session state */
typedef enum {
    WORLD_STATE_INIT,
    WORLD_STATE_AUTHED,
    WORLD_STATE_CHAR_SELECT,
    WORLD_STATE_IN_WORLD
} world_state_t;

/* World session context */
typedef struct {
    client_t *client;
    worldcrypt_t crypt;
    bool encryption_enabled;
    account_t account;
    player_t player;
    bool has_player;
    world_state_t state;
    uint32_t server_seed;
    uint32_t time_sync_counter;
} world_session_t;

/* Create world session */
world_session_t *world_session_create(client_t *client);

/* Free world session */
void world_session_free(world_session_t *session);

/* Handle world session (blocking, runs until disconnect) */
void world_session_handle(world_session_t *session);

/* World server functions */

/* Start world server (blocking call) */
result_t world_server_start(void);

/* Stop world server */
void world_server_stop(void);

#endif /* WORLD_H */
