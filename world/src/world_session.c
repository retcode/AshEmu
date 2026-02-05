/*
 * AshEmu - WoW 1.12.1 Server Emulator
 * Copyright (C) 2025 AshEmu Team
 *
 * world_session.c - World session packet handlers
 */

#include "world.h"
#include "packet.h"
#include "opcodes.h"
#include "update.h"
#include "positions.h"
#include <openssl/sha.h>

world_session_t *world_session_create(client_t *client) {
    world_session_t *session = ALLOC(world_session_t);
    if (!session) return NULL;

    session->client = client;
    session->encryption_enabled = false;
    account_init(&session->account);
    session->has_player = false;
    session->state = WORLD_STATE_INIT;
    session->server_seed = (uint32_t)rand();
    session->time_sync_counter = 0;

    return session;
}

void world_session_free(world_session_t *session) {
    if (!session) return;
    client_free(session->client);
    free(session);
}

/* Send packet with proper header format */
static result_t send_packet(world_session_t *session, uint16_t opcode,
                                const uint8_t *data, size_t data_len) {
    /* Header: 2 bytes size (big-endian) + 2 bytes opcode (little-endian) */
    uint16_t size = (uint16_t)(data_len + 2);  /* +2 for opcode */
    uint8_t header[4];
    header[0] = (uint8_t)(size >> 8);
    header[1] = (uint8_t)(size & 0xFF);
    header[2] = (uint8_t)(opcode & 0xFF);
    header[3] = (uint8_t)((opcode >> 8) & 0xFF);

    /* Encrypt header if enabled */
    if (session->encryption_enabled) {
        worldcrypt_encrypt(&session->crypt, header, 4);
    }

    /* Send header */
    result_t result = client_send_all(session->client, header, 4);
    if (result != OK) return result;

    /* Send payload */
    if (data_len > 0) {
        result = client_send_all(session->client, data, data_len);
    }

    return result;
}

/* Send SMSG_AUTH_CHALLENGE */
static result_t send_auth_challenge(world_session_t *session) {
    packet_writer_t packet;
    writer_init(&packet);
    write_uint32(&packet, session->server_seed);

    result_t result = send_packet(session, SMSG_AUTH_CHALLENGE,
                                      writer_data(&packet), writer_size(&packet));
    writer_free(&packet);
    return result;
}

/* Handle CMSG_AUTH_SESSION */
static result_t handle_auth_session(world_session_t *session, const uint8_t *data, size_t len) {
    packet_reader_t reader;
    reader_init(&reader, data, len);

    uint32_t build = read_uint32(&reader);
    uint32_t server_id = read_uint32(&reader);
    (void)server_id;

    char username[MAX_USERNAME + 1];
    read_cstring(&reader, username, sizeof(username));
    to_upper(username);

    uint32_t client_seed = read_uint32(&reader);
    uint8_t client_digest[20];
    read_bytes(&reader, client_digest, 20);

    LOG_INFO("WorldServer", "Auth session from: %s (build %u)", username, build);

    /* Get account and session key */
    result_t result = database_get_account(username, &session->account);
    if (result != OK || !session->account.has_session_key) {
        LOG_ERROR("WorldServer", "No session key for: %s", username);
        packet_writer_t packet;
        writer_init(&packet);
        write_uint8(&packet, WORLD_AUTH_UNKNOWN_ACCOUNT);
        send_packet(session, SMSG_AUTH_RESPONSE, writer_data(&packet), writer_size(&packet));
        writer_free(&packet);
        return ERR_AUTH_FAILED;
    }

    /* Verify client digest: SHA1(username + 0000 + client_seed + server_seed + session_key) */
    SHA_CTX sha_ctx;
    SHA1_Init(&sha_ctx);
    SHA1_Update(&sha_ctx, username, strlen(username));
    uint32_t zero = 0;
    SHA1_Update(&sha_ctx, &zero, 4);
    SHA1_Update(&sha_ctx, &client_seed, 4);
    SHA1_Update(&sha_ctx, &session->server_seed, 4);
    SHA1_Update(&sha_ctx, session->account.session_key, SRP6_SESSION_KEY_SIZE);

    uint8_t expected_digest[20];
    SHA1_Final(expected_digest, &sha_ctx);

    if (memcmp(client_digest, expected_digest, 20) != 0) {
        LOG_ERROR("WorldServer", "Invalid digest for: %s", username);
        packet_writer_t packet;
        writer_init(&packet);
        write_uint8(&packet, WORLD_AUTH_FAILED);
        send_packet(session, SMSG_AUTH_RESPONSE, writer_data(&packet), writer_size(&packet));
        writer_free(&packet);
        return ERR_AUTH_FAILED;
    }

    /* Initialize encryption */
    worldcrypt_init(&session->crypt, session->account.session_key);
    session->encryption_enabled = true;

    LOG_INFO("WorldServer", "Auth successful: %s", username);

    /* Send auth response */
    packet_writer_t packet;
    writer_init(&packet);
    write_uint8(&packet, WORLD_AUTH_OK);
    write_uint32(&packet, 0);  /* BillingTimeRemaining */
    write_uint8(&packet, 0);   /* BillingPlanFlags */
    write_uint32(&packet, 0);  /* BillingTimeRested */

    send_packet(session, SMSG_AUTH_RESPONSE, writer_data(&packet), writer_size(&packet));
    writer_free(&packet);

    session->state = WORLD_STATE_AUTHED;
    return OK;
}

/* Handle CMSG_CHAR_ENUM */
static result_t handle_char_enum(world_session_t *session) {
    character_list_t characters;
    database_get_characters(session->account.id, &characters);

    packet_writer_t packet;
    writer_init(&packet);
    write_uint8(&packet, (uint8_t)characters.count);

    for (int i = 0; i < characters.count; i++) {
        character_t *c = &characters.items[i];

        write_uint64(&packet, (uint64_t)c->id);  /* GUID */
        write_cstring(&packet, c->name);
        write_uint8(&packet, c->race);
        write_uint8(&packet, c->char_class);
        write_uint8(&packet, c->gender);
        write_uint8(&packet, c->skin);
        write_uint8(&packet, c->face);
        write_uint8(&packet, c->hair_style);
        write_uint8(&packet, c->hair_color);
        write_uint8(&packet, c->facial_hair);
        write_uint8(&packet, c->level);
        write_uint32(&packet, (uint32_t)c->map);  /* Zone ID (using map for simplicity) */
        write_uint32(&packet, (uint32_t)c->map);  /* Map ID */
        write_float(&packet, c->x);
        write_float(&packet, c->y);
        write_float(&packet, c->z);
        write_uint32(&packet, 0);  /* Guild ID */

        write_uint32(&packet, 0);  /* Character flags */
        write_uint8(&packet, 0);   /* First login (0 = no) */
        write_uint32(&packet, 0);  /* Pet display ID */
        write_uint32(&packet, 0);  /* Pet level */
        write_uint32(&packet, 0);  /* Pet family */

        /* Equipment (20 slots: display ID + inventory type) */
        for (int j = 0; j < 20; j++) {
            write_uint32(&packet, 0);  /* Display ID */
            write_uint8(&packet, 0);   /* Inventory type */
        }
    }

    send_packet(session, SMSG_CHAR_ENUM, writer_data(&packet), writer_size(&packet));
    writer_free(&packet);
    character_list_free(&characters);

    session->state = WORLD_STATE_CHAR_SELECT;
    return OK;
}

/* Handle CMSG_CHAR_CREATE */
static result_t handle_char_create(world_session_t *session, const uint8_t *data, size_t len) {
    packet_reader_t reader;
    reader_init(&reader, data, len);

    char name[MAX_CHARACTER_NAME + 1];
    read_cstring(&reader, name, sizeof(name));

    uint8_t race = read_uint8(&reader);
    uint8_t char_class = read_uint8(&reader);
    uint8_t gender = read_uint8(&reader);
    uint8_t skin = read_uint8(&reader);
    uint8_t face = read_uint8(&reader);
    uint8_t hair_style = read_uint8(&reader);
    uint8_t hair_color = read_uint8(&reader);
    uint8_t facial_hair = read_uint8(&reader);

    LOG_INFO("WorldServer", "Character create: %s (Race: %d, Class: %d)", name, race, char_class);

    /* Check if name exists */
    bool exists = false;
    database_character_name_exists(name, &exists);

    packet_writer_t packet;
    writer_init(&packet);

    if (exists) {
        write_uint8(&packet, CHAR_CREATE_NAME_IN_USE);
        send_packet(session, SMSG_CHAR_CREATE, writer_data(&packet), writer_size(&packet));
        writer_free(&packet);
        return OK;
    }

    /* Get starting position */
    const start_position_t *start_pos = get_start_position(race);

    character_t character;
    character_init(&character);
    character.account_id = session->account.id;
    safe_strncpy(character.name, name, sizeof(character.name));
    character.race = race;
    character.char_class = char_class;
    character.gender = gender;
    character.skin = skin;
    character.face = face;
    character.hair_style = hair_style;
    character.hair_color = hair_color;
    character.facial_hair = facial_hair;
    character.level = 1;
    character.map = start_pos->map;
    character.x = start_pos->x;
    character.y = start_pos->y;
    character.z = start_pos->z;
    character.orientation = start_pos->orientation;

    result_t result = database_create_character(&character);
    if (result != OK) {
        write_uint8(&packet, CHAR_CREATE_FAILED);
    } else {
        LOG_INFO("WorldServer", "Character created: %s", name);
        write_uint8(&packet, CHAR_CREATE_SUCCESS);
    }

    send_packet(session, SMSG_CHAR_CREATE, writer_data(&packet), writer_size(&packet));
    writer_free(&packet);
    return OK;
}

/* Handle CMSG_CHAR_DELETE */
static result_t handle_char_delete(world_session_t *session, const uint8_t *data, size_t len) {
    packet_reader_t reader;
    reader_init(&reader, data, len);
    uint64_t guid = read_uint64(&reader);

    database_delete_character((int)guid);

    packet_writer_t packet;
    writer_init(&packet);
    write_uint8(&packet, CHAR_DELETE_SUCCESS);
    send_packet(session, SMSG_CHAR_DELETE, writer_data(&packet), writer_size(&packet));
    writer_free(&packet);
    return OK;
}

/* Send login sequence packets */
static result_t send_login_verify_world(world_session_t *session) {
    packet_writer_t packet;
    writer_init(&packet);
    write_uint32(&packet, (uint32_t)session->player.map);
    write_float(&packet, session->player.x);
    write_float(&packet, session->player.y);
    write_float(&packet, session->player.z);
    write_float(&packet, session->player.orientation);
    send_packet(session, SMSG_LOGIN_VERIFY_WORLD, writer_data(&packet), writer_size(&packet));
    writer_free(&packet);
    return OK;
}

static result_t send_account_data_times(world_session_t *session) {
    packet_writer_t packet;
    writer_init(&packet);
    /* 32 uint32 values */
    for (int i = 0; i < 32; i++) {
        write_uint32(&packet, 0);
    }
    send_packet(session, SMSG_ACCOUNT_DATA_TIMES, writer_data(&packet), writer_size(&packet));
    writer_free(&packet);
    return OK;
}

static result_t send_tutorial_flags(world_session_t *session) {
    packet_writer_t packet;
    writer_init(&packet);
    /* 8 uint32 values - all 0xFFFFFFFF = tutorials complete */
    for (int i = 0; i < 8; i++) {
        write_uint32(&packet, 0xFFFFFFFF);
    }
    send_packet(session, SMSG_TUTORIAL_FLAGS, writer_data(&packet), writer_size(&packet));
    writer_free(&packet);
    return OK;
}

static result_t send_login_set_time_speed(world_session_t *session) {
    packet_writer_t packet;
    writer_init(&packet);

    /* Game time: packed as (minutes | (hours << 6) | (weekday << 11) | (day << 14) | (month << 20) | ((year-2000) << 24)) */
    time_t now = time(NULL);
    struct tm *tm = localtime(&now);
    uint32_t game_time = tm->tm_min | (tm->tm_hour << 6) | (tm->tm_wday << 11) |
                         ((tm->tm_mday - 1) << 14) | (tm->tm_mon << 20) |
                         ((tm->tm_year + 1900 - 2000) << 24);

    write_uint32(&packet, game_time);
    write_float(&packet, 0.01666667f);  /* Game speed (1/60 for real-time) */

    send_packet(session, SMSG_LOGIN_SETTIMESPEED, writer_data(&packet), writer_size(&packet));
    writer_free(&packet);
    return OK;
}

static result_t send_initial_spells(world_session_t *session) {
    packet_writer_t packet;
    writer_init(&packet);
    write_uint8(&packet, 0);   /* Unknown */
    write_uint16(&packet, 0);  /* Spell count */
    write_uint16(&packet, 0);  /* Cooldown count */
    send_packet(session, SMSG_INITIAL_SPELLS, writer_data(&packet), writer_size(&packet));
    writer_free(&packet);
    return OK;
}

static result_t send_action_buttons(world_session_t *session) {
    packet_writer_t packet;
    writer_init(&packet);
    /* 120 action button slots (each 4 bytes) */
    for (int i = 0; i < 120; i++) {
        write_uint32(&packet, 0);
    }
    send_packet(session, SMSG_ACTION_BUTTONS, writer_data(&packet), writer_size(&packet));
    writer_free(&packet);
    return OK;
}

static result_t send_initialize_factions(world_session_t *session) {
    packet_writer_t packet;
    writer_init(&packet);
    write_uint32(&packet, 0x00000040);  /* Faction count (64) */

    /* 64 faction entries (flags uint8, standing uint32) */
    for (int i = 0; i < 64; i++) {
        write_uint8(&packet, 0);   /* Flags */
        write_uint32(&packet, 0);  /* Standing */
    }

    send_packet(session, SMSG_INITIALIZE_FACTIONS, writer_data(&packet), writer_size(&packet));
    writer_free(&packet);
    return OK;
}

static result_t send_update_object(world_session_t *session) {
    update_builder_t builder;
    update_builder_init(&builder);

    packet_writer_t packet;
    writer_init(&packet);

    update_build_create_packet(&builder, &session->player, true, &packet);

    send_packet(session, SMSG_UPDATE_OBJECT, writer_data(&packet), writer_size(&packet));
    writer_free(&packet);
    return OK;
}

static result_t send_time_sync_request(world_session_t *session) {
    packet_writer_t packet;
    writer_init(&packet);
    write_uint32(&packet, session->time_sync_counter++);
    send_packet(session, SMSG_TIME_SYNC_REQ, writer_data(&packet), writer_size(&packet));
    writer_free(&packet);
    return OK;
}

/* Handle CMSG_PLAYER_LOGIN */
static result_t handle_player_login(world_session_t *session, const uint8_t *data, size_t len) {
    packet_reader_t reader;
    reader_init(&reader, data, len);
    uint64_t guid = read_uint64(&reader);

    character_t character;
    result_t result = database_get_character((int)guid, &character);
    if (result != OK) {
        LOG_ERROR("WorldServer", "Character not found: %llu", (unsigned long long)guid);
        return result;
    }

    player_init(&session->player, &character);
    session->has_player = true;

    LOG_INFO("WorldServer", "Player login: %s", character.name);

    /* Send login sequence in order */
    send_login_verify_world(session);
    send_account_data_times(session);
    send_tutorial_flags(session);
    send_login_set_time_speed(session);
    send_initial_spells(session);
    send_action_buttons(session);
    send_initialize_factions(session);
    send_update_object(session);
    send_time_sync_request(session);

    session->state = WORLD_STATE_IN_WORLD;
    return OK;
}

/* Handle CMSG_PING */
static result_t handle_ping(world_session_t *session, const uint8_t *data, size_t len) {
    packet_reader_t reader;
    reader_init(&reader, data, len);
    uint32_t ping = read_uint32(&reader);
    /* uint32_t latency = */ read_uint32(&reader);

    packet_writer_t packet;
    writer_init(&packet);
    write_uint32(&packet, ping);
    send_packet(session, SMSG_PONG, writer_data(&packet), writer_size(&packet));
    writer_free(&packet);
    return OK;
}

/* Handle CMSG_NAME_QUERY */
static result_t handle_name_query(world_session_t *session, const uint8_t *data, size_t len) {
    packet_reader_t reader;
    reader_init(&reader, data, len);
    uint64_t guid = read_uint64(&reader);

    character_t character;
    result_t result = database_get_character((int)guid, &character);

    packet_writer_t packet;
    writer_init(&packet);
    write_uint64(&packet, guid);

    if (result == OK) {
        write_cstring(&packet, character.name);
        write_uint8(&packet, 0);  /* Realm name (empty = same realm) */
        write_uint32(&packet, (uint32_t)character.race);
        write_uint32(&packet, (uint32_t)character.gender);
        write_uint32(&packet, (uint32_t)character.char_class);
    } else {
        write_cstring(&packet, "Unknown");
        write_uint8(&packet, 0);
        write_uint32(&packet, 0);
        write_uint32(&packet, 0);
        write_uint32(&packet, 0);
    }

    send_packet(session, SMSG_NAME_QUERY_RESPONSE, writer_data(&packet), writer_size(&packet));
    writer_free(&packet);
    return OK;
}

/* Handle CMSG_LOGOUT_REQUEST */
static result_t handle_logout_request(world_session_t *session) {
    packet_writer_t packet;
    writer_init(&packet);
    write_uint32(&packet, 0);  /* Reason (0 = success) */
    write_uint8(&packet, 1);   /* Instant logout flag */
    send_packet(session, SMSG_LOGOUT_RESPONSE, writer_data(&packet), writer_size(&packet));
    writer_free(&packet);

    /* Send logout complete */
    packet_writer_t complete;
    writer_init(&complete);
    send_packet(session, SMSG_LOGOUT_COMPLETE, writer_data(&complete), writer_size(&complete));
    writer_free(&complete);

    session->state = WORLD_STATE_CHAR_SELECT;
    session->has_player = false;
    return OK;
}

/* Handle movement packets */
static void handle_movement(world_session_t *session, uint16_t opcode, const uint8_t *data, size_t len) {
    (void)opcode;
    if (!session->has_player || len < 24) return;

    packet_reader_t reader;
    reader_init(&reader, data, len);

    /* uint32_t move_flags = */ read_uint32(&reader);
    /* uint32_t time = */ read_uint32(&reader);
    float x = read_float(&reader);
    float y = read_float(&reader);
    float z = read_float(&reader);
    float orientation = read_float(&reader);

    /* Update player position */
    session->player.x = x;
    session->player.y = y;
    session->player.z = z;
    session->player.orientation = orientation;
}

/* Handle a single packet */
static result_t handle_packet(world_session_t *session, uint16_t opcode, const uint8_t *data, size_t len) {
    switch (opcode) {
        case CMSG_AUTH_SESSION:
            return handle_auth_session(session, data, len);
        case CMSG_CHAR_ENUM:
            return handle_char_enum(session);
        case CMSG_CHAR_CREATE:
            return handle_char_create(session, data, len);
        case CMSG_CHAR_DELETE:
            return handle_char_delete(session, data, len);
        case CMSG_PLAYER_LOGIN:
            return handle_player_login(session, data, len);
        case CMSG_PING:
            return handle_ping(session, data, len);
        case CMSG_NAME_QUERY:
            return handle_name_query(session, data, len);
        case CMSG_LOGOUT_REQUEST:
            return handle_logout_request(session);
        case CMSG_TIME_SYNC_RESP:
        case CMSG_STANDSTATECHANGE:
        case CMSG_SET_SELECTION:
            /* Silently ignore */
            return OK;
        default:
            if (is_movement_opcode(opcode)) {
                handle_movement(session, opcode, data, len);
                return OK;
            }
            /* Unknown opcode - silently ignore */
            return OK;
    }
}

void world_session_handle(world_session_t *session) {
    LOG_INFO("WorldServer", "Client connected: %s", client_get_address(session->client));

    /* Send auth challenge immediately */
    send_auth_challenge(session);

    uint8_t header_buf[6];  /* 2 size + 4 opcode */

    while (client_is_connected(session->client)) {
        /* Read header */
        result_t result = client_recv_exact(session->client, header_buf, 6);
        if (result != OK) break;

        /* Decrypt header if enabled */
        if (session->encryption_enabled) {
            worldcrypt_decrypt(&session->crypt, header_buf, 6);
        }

        /* Parse header (big-endian size, little-endian opcode) */
        uint16_t size = ((uint16_t)header_buf[0] << 8) | header_buf[1];
        uint16_t opcode = header_buf[2] | ((uint16_t)header_buf[3] << 8) |
                          ((uint16_t)header_buf[4] << 16) | ((uint16_t)header_buf[5] << 24);

        /* Size includes opcode bytes, so subtract 4 */
        size_t payload_size = size > 4 ? size - 4 : 0;

        /* Read payload */
        uint8_t *payload = NULL;
        if (payload_size > 0) {
            payload = (uint8_t*)malloc(payload_size);
            if (!payload) break;

            result = client_recv_exact(session->client, payload, payload_size);
            if (result != OK) {
                free(payload);
                break;
            }
        }

        /* Handle packet */
        handle_packet(session, opcode, payload, payload_size);
        free(payload);
    }

    /* Save position on disconnect */
    if (session->has_player) {
        database_update_character_position(
            session->player.character.id,
            session->player.map,
            session->player.x,
            session->player.y,
            session->player.z,
            session->player.orientation
        );
    }

    LOG_INFO("WorldServer", "Client disconnected: %s", client_get_address(session->client));
}
