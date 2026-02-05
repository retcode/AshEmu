/*
 * AshEmu - WoW 1.12.1 Server Emulator
 * Copyright (C) 2025 AshEmu Team
 *
 * auth_session.c - Auth session handling (SRP6 authentication)
 */

#include "auth.h"
#include "packet.h"
#include "opcodes.h"

/* WoW-specific N parameter (little-endian for protocol) */
static const uint8_t N_BYTES_LE[] = {
    0xB7, 0x9B, 0x3E, 0x2A, 0x87, 0x82, 0x3C, 0xAB,
    0x8F, 0x5E, 0xBF, 0xBF, 0x8E, 0xB1, 0x01, 0x08,
    0x53, 0x50, 0x06, 0x29, 0x8B, 0x5B, 0xAD, 0xBD,
    0x5B, 0x53, 0xE1, 0x89, 0x5E, 0x64, 0x4B, 0x89
};

auth_session_t *auth_session_create(client_t *client) {
    auth_session_t *session = ALLOC(auth_session_t);
    if (!session) return NULL;

    session->client = client;
    session->srp6 = srp6_create();
    if (!session->srp6) {
        free(session);
        return NULL;
    }

    account_init(&session->account);
    session->state = AUTH_STATE_INIT;

    return session;
}

void auth_session_free(auth_session_t *session) {
    if (!session) return;
    srp6_free(session->srp6);
    client_free(session->client);
    free(session);
}

/* Handle AUTH_LOGON_CHALLENGE */
static result_t handle_logon_challenge(auth_session_t *session, const uint8_t *data, size_t len) {
    /* Minimum packet size: opcode(1) + error(1) + size(2) + gamename(4) + version(3)
       + build(2) + platform(4) + os(4) + locale(4) + timezone(4) + ip(4) + username_len(1) */
    if (len < 34) {
        LOG_ERROR("AuthServer", "Challenge packet too short");
        return ERR_INVALID_PARAM;
    }

    uint8_t username_len = data[33];
    if (len < 34 + username_len) {
        LOG_ERROR("AuthServer", "Challenge packet username truncated");
        return ERR_INVALID_PARAM;
    }

    /* Extract username */
    char username[MAX_USERNAME + 1];
    size_t copy_len = username_len < MAX_USERNAME ? username_len : MAX_USERNAME;
    memcpy(username, data + 34, copy_len);
    username[copy_len] = '\0';
    to_upper(username);

    LOG_INFO("AuthServer", "Login challenge from: %s", username);

    /* Get or create account */
    result_t result = database_get_account(username, &session->account);
    if (result == ERR_NOT_FOUND) {
        /* Auto-create account with username as password */
        uint8_t salt[SRP6_SALT_SIZE];
        uint8_t verifier[SRP6_VERIFIER_SIZE];
        srp6_compute_verifier(username, username, salt, verifier);

        result = database_create_account(username, salt, verifier, &session->account);
        if (result != OK) {
            LOG_ERROR("AuthServer", "Failed to create account for: %s", username);
            return result;
        }
        LOG_INFO("AuthServer", "Auto-created account for: %s", username);
    } else if (result != OK) {
        return result;
    }

    /* Initialize SRP6 */
    result = srp6_init(session->srp6, username, session->account.salt, session->account.verifier);
    if (result != OK) return result;

    /* Build response */
    packet_writer_t response;
    writer_init(&response);

    write_uint8(&response, AUTH_LOGON_CHALLENGE);
    write_uint8(&response, 0);  /* unknown */
    write_uint8(&response, AUTH_SUCCESS);

    /* B (32 bytes) */
    uint8_t B[SRP6_KEY_SIZE];
    srp6_get_B(session->srp6, B);
    write_bytes(&response, B, SRP6_KEY_SIZE);

    /* g length + g */
    write_uint8(&response, 1);
    write_uint8(&response, 7);

    /* N length + N (little-endian) */
    write_uint8(&response, 32);
    write_bytes(&response, N_BYTES_LE, 32);

    /* salt (32 bytes) */
    write_bytes(&response, session->account.salt, SRP6_SALT_SIZE);

    /* unknown/checksum (16 bytes) */
    write_zeros(&response, 16);

    /* security flags */
    write_uint8(&response, 0);

    client_send_all(session->client, writer_data(&response), writer_size(&response));
    writer_free(&response);

    session->state = AUTH_STATE_CHALLENGED;
    return OK;
}

/* Handle AUTH_LOGON_PROOF */
static result_t handle_logon_proof(auth_session_t *session, const uint8_t *data, size_t len) {
    /* opcode(1) + A(32) + M1(20) + crc(20) + keys(1) + securityFlags(1) = 75 bytes */
    if (len < 75) {
        LOG_ERROR("AuthServer", "Proof packet too short");
        return ERR_INVALID_PARAM;
    }

    const uint8_t *A = data + 1;    /* Client public key */
    const uint8_t *M1 = data + 33;  /* Client proof */

    uint8_t M2[SRP6_PROOF_SIZE];
    result_t result = srp6_verify_proof(session->srp6, A, M1, M2);

    packet_writer_t response;
    writer_init(&response);

    write_uint8(&response, AUTH_LOGON_PROOF);

    if (result != OK) {
        LOG_INFO("AuthServer", "Invalid proof from: %s", session->account.username);
        write_uint8(&response, AUTH_FAIL_INCORRECT_PASSWORD);
        client_send_all(session->client, writer_data(&response), writer_size(&response));
        writer_free(&response);
        return ERR_AUTH_FAILED;
    }

    LOG_INFO("AuthServer", "Login successful: %s", session->account.username);

    /* Save session key */
    const uint8_t *session_key = srp6_get_session_key(session->srp6);
    if (session_key) {
        database_update_session_key(session->account.id, session_key);
    }

    /* Build success response (1.12.1 format) */
    write_uint8(&response, AUTH_SUCCESS);
    write_bytes(&response, M2, SRP6_PROOF_SIZE);  /* Server proof */
    write_uint32(&response, 0);  /* unknown (required for 1.12.1) */

    client_send_all(session->client, writer_data(&response), writer_size(&response));
    writer_free(&response);

    session->state = AUTH_STATE_AUTHENTICATED;
    return OK;
}

/* Handle REALM_LIST */
static result_t handle_realm_list(auth_session_t *session) {
    LOG_INFO("AuthServer", "Realm list requested by: %s", session->account.username);

    /* Build realm data first */
    packet_writer_t realm_data;
    writer_init(&realm_data);

    write_uint32(&realm_data, 0);  /* unknown */
    write_uint16(&realm_data, 1);  /* realm count (uint16 per 1.12.1 protocol) */

    /* Realm entry (1.12.1 format) */
    write_uint8(&realm_data, 0);   /* realm type/icon (0 = Normal) */
    write_uint8(&realm_data, 0);   /* lock (0 = unlocked) */
    write_uint8(&realm_data, 0);   /* color (green) */
    write_uint8(&realm_data, 0);   /* flags (0 = online) */
    write_cstring(&realm_data, "AshEmu");  /* realm name */
    write_cstring(&realm_data, "127.0.0.1:8085");  /* address */
    write_float(&realm_data, 0.0f);  /* population */
    write_uint8(&realm_data, 0);   /* character count */
    write_uint8(&realm_data, 1);   /* timezone */
    write_uint8(&realm_data, 1);   /* realm ID (must be non-zero) */

    write_uint16(&realm_data, 0x10);  /* footer */

    /* Build full response */
    packet_writer_t response;
    writer_init(&response);

    write_uint8(&response, AUTH_REALM_LIST);
    write_uint16(&response, (uint16_t)writer_size(&realm_data));
    write_bytes(&response, writer_data(&realm_data), writer_size(&realm_data));

    client_send_all(session->client, writer_data(&response), writer_size(&response));

    writer_free(&realm_data);
    writer_free(&response);
    return OK;
}

void auth_session_handle(auth_session_t *session) {
    LOG_INFO("AuthServer", "Client connected: %s", client_get_address(session->client));

    uint8_t buffer[4096];

    while (client_is_connected(session->client)) {
        ssize_t bytes_read = client_recv(session->client, buffer, sizeof(buffer));
        if (bytes_read <= 0) break;

        auth_opcode_t opcode = (auth_opcode_t)buffer[0];

        switch (opcode) {
            case AUTH_LOGON_CHALLENGE:
                handle_logon_challenge(session, buffer, bytes_read);
                break;
            case AUTH_LOGON_PROOF:
                handle_logon_proof(session, buffer, bytes_read);
                break;
            case AUTH_REALM_LIST:
                handle_realm_list(session);
                break;
            default:
                LOG_INFO("AuthServer", "Unknown opcode: 0x%02X", opcode);
                break;
        }
    }

    LOG_INFO("AuthServer", "Client disconnected: %s", client_get_address(session->client));
}
