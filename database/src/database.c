/*
 * AshEmu - WoW 1.12.1 Server Emulator
 * Copyright (C) 2025 AshEmu Team
 *
 * database.c - SQLite database implementation
 */

#include "database.h"

/* Global database instance */
database_t *g_database = NULL;

/* SQL for creating tables */
static const char *CREATE_TABLES_SQL =
    "CREATE TABLE IF NOT EXISTS accounts ("
    "    id INTEGER PRIMARY KEY AUTOINCREMENT,"
    "    username TEXT NOT NULL UNIQUE COLLATE NOCASE,"
    "    salt BLOB NOT NULL,"
    "    verifier BLOB NOT NULL,"
    "    session_key BLOB"
    ");"
    ""
    "CREATE TABLE IF NOT EXISTS characters ("
    "    id INTEGER PRIMARY KEY AUTOINCREMENT,"
    "    account_id INTEGER NOT NULL,"
    "    name TEXT NOT NULL UNIQUE COLLATE NOCASE,"
    "    race INTEGER NOT NULL,"
    "    class INTEGER NOT NULL,"
    "    gender INTEGER NOT NULL,"
    "    skin INTEGER DEFAULT 0,"
    "    face INTEGER DEFAULT 0,"
    "    hair_style INTEGER DEFAULT 0,"
    "    hair_color INTEGER DEFAULT 0,"
    "    facial_hair INTEGER DEFAULT 0,"
    "    level INTEGER DEFAULT 1,"
    "    map INTEGER DEFAULT 0,"
    "    x REAL NOT NULL,"
    "    y REAL NOT NULL,"
    "    z REAL NOT NULL,"
    "    orientation REAL DEFAULT 0,"
    "    FOREIGN KEY (account_id) REFERENCES accounts(id)"
    ");";

result_t database_init(const char *db_path) {
    if (g_database) {
        return ERR_ALREADY_EXISTS;
    }

    g_database = ALLOC(database_t);
    if (!g_database) return ERR_MEMORY;

    safe_strncpy(g_database->path, db_path, sizeof(g_database->path));

    int rc = sqlite3_open(db_path, &g_database->db);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Database", "Failed to open database: %s", sqlite3_errmsg(g_database->db));
        FREE(g_database);
        return ERR_DATABASE;
    }

    /* Create tables */
    char *err_msg = NULL;
    rc = sqlite3_exec(g_database->db, CREATE_TABLES_SQL, NULL, NULL, &err_msg);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Database", "Failed to create tables: %s", err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(g_database->db);
        FREE(g_database);
        return ERR_DATABASE;
    }

    LOG_INFO("Database", "Initialized");
    return OK;
}

void database_shutdown(void) {
    if (g_database) {
        if (g_database->db) {
            sqlite3_close(g_database->db);
        }
        FREE(g_database);
    }
}

result_t database_get_account(const char *username, account_t *account) {
    account_init(account);

    const char *sql = "SELECT id, username, salt, verifier, session_key FROM accounts WHERE username = ? COLLATE NOCASE";

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(g_database->db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Database", "Failed to prepare statement: %s", sqlite3_errmsg(g_database->db));
        return ERR_DATABASE;
    }

    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        account->id = sqlite3_column_int(stmt, 0);
        safe_strncpy(account->username, (const char*)sqlite3_column_text(stmt, 1), sizeof(account->username));

        const void *salt = sqlite3_column_blob(stmt, 2);
        int salt_size = sqlite3_column_bytes(stmt, 2);
        if (salt && salt_size == SRP6_SALT_SIZE) {
            memcpy(account->salt, salt, SRP6_SALT_SIZE);
        }

        const void *verifier = sqlite3_column_blob(stmt, 3);
        int verifier_size = sqlite3_column_bytes(stmt, 3);
        if (verifier && verifier_size == SRP6_VERIFIER_SIZE) {
            memcpy(account->verifier, verifier, SRP6_VERIFIER_SIZE);
        }

        const void *session_key = sqlite3_column_blob(stmt, 4);
        int session_key_size = sqlite3_column_bytes(stmt, 4);
        if (session_key && session_key_size == SRP6_SESSION_KEY_SIZE) {
            memcpy(account->session_key, session_key, SRP6_SESSION_KEY_SIZE);
            account->has_session_key = true;
        }

        sqlite3_finalize(stmt);
        return OK;
    }

    sqlite3_finalize(stmt);
    return ERR_NOT_FOUND;
}

result_t database_create_account(const char *username, const uint8_t salt[SRP6_SALT_SIZE],
                                         const uint8_t verifier[SRP6_VERIFIER_SIZE], account_t *account) {
    const char *sql = "INSERT INTO accounts (username, salt, verifier) VALUES (?, ?, ?)";

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(g_database->db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Database", "Failed to prepare statement: %s", sqlite3_errmsg(g_database->db));
        return ERR_DATABASE;
    }

    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
    sqlite3_bind_blob(stmt, 2, salt, SRP6_SALT_SIZE, SQLITE_STATIC);
    sqlite3_bind_blob(stmt, 3, verifier, SRP6_VERIFIER_SIZE, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        LOG_ERROR("Database", "Failed to create account: %s", sqlite3_errmsg(g_database->db));
        return ERR_DATABASE;
    }

    /* Fill in account struct */
    account_init(account);
    account->id = (int)sqlite3_last_insert_rowid(g_database->db);
    safe_strncpy(account->username, username, sizeof(account->username));
    memcpy(account->salt, salt, SRP6_SALT_SIZE);
    memcpy(account->verifier, verifier, SRP6_VERIFIER_SIZE);

    return OK;
}

result_t database_update_session_key(int account_id, const uint8_t session_key[SRP6_SESSION_KEY_SIZE]) {
    const char *sql = "UPDATE accounts SET session_key = ? WHERE id = ?";

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(g_database->db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Database", "Failed to prepare statement: %s", sqlite3_errmsg(g_database->db));
        return ERR_DATABASE;
    }

    sqlite3_bind_blob(stmt, 1, session_key, SRP6_SESSION_KEY_SIZE, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, account_id);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        LOG_ERROR("Database", "Failed to update session key: %s", sqlite3_errmsg(g_database->db));
        return ERR_DATABASE;
    }

    return OK;
}

result_t database_get_characters(int account_id, character_list_t *list) {
    character_list_init(list);

    const char *sql =
        "SELECT id, account_id, name, race, class, gender, skin, face, "
        "hair_style, hair_color, facial_hair, level, map, x, y, z, orientation "
        "FROM characters WHERE account_id = ?";

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(g_database->db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Database", "Failed to prepare statement: %s", sqlite3_errmsg(g_database->db));
        return ERR_DATABASE;
    }

    sqlite3_bind_int(stmt, 1, account_id);

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        character_t character;
        character_init(&character);

        character.id = sqlite3_column_int(stmt, 0);
        character.account_id = sqlite3_column_int(stmt, 1);
        safe_strncpy(character.name, (const char*)sqlite3_column_text(stmt, 2), sizeof(character.name));
        character.race = (uint8_t)sqlite3_column_int(stmt, 3);
        character.char_class = (uint8_t)sqlite3_column_int(stmt, 4);
        character.gender = (uint8_t)sqlite3_column_int(stmt, 5);
        character.skin = (uint8_t)sqlite3_column_int(stmt, 6);
        character.face = (uint8_t)sqlite3_column_int(stmt, 7);
        character.hair_style = (uint8_t)sqlite3_column_int(stmt, 8);
        character.hair_color = (uint8_t)sqlite3_column_int(stmt, 9);
        character.facial_hair = (uint8_t)sqlite3_column_int(stmt, 10);
        character.level = (uint8_t)sqlite3_column_int(stmt, 11);
        character.map = sqlite3_column_int(stmt, 12);
        character.x = (float)sqlite3_column_double(stmt, 13);
        character.y = (float)sqlite3_column_double(stmt, 14);
        character.z = (float)sqlite3_column_double(stmt, 15);
        character.orientation = (float)sqlite3_column_double(stmt, 16);

        character_list_add(list, &character);
    }

    sqlite3_finalize(stmt);
    return OK;
}

result_t database_get_character(int character_id, character_t *character) {
    character_init(character);

    const char *sql =
        "SELECT id, account_id, name, race, class, gender, skin, face, "
        "hair_style, hair_color, facial_hair, level, map, x, y, z, orientation "
        "FROM characters WHERE id = ?";

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(g_database->db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Database", "Failed to prepare statement: %s", sqlite3_errmsg(g_database->db));
        return ERR_DATABASE;
    }

    sqlite3_bind_int(stmt, 1, character_id);

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        character->id = sqlite3_column_int(stmt, 0);
        character->account_id = sqlite3_column_int(stmt, 1);
        safe_strncpy(character->name, (const char*)sqlite3_column_text(stmt, 2), sizeof(character->name));
        character->race = (uint8_t)sqlite3_column_int(stmt, 3);
        character->char_class = (uint8_t)sqlite3_column_int(stmt, 4);
        character->gender = (uint8_t)sqlite3_column_int(stmt, 5);
        character->skin = (uint8_t)sqlite3_column_int(stmt, 6);
        character->face = (uint8_t)sqlite3_column_int(stmt, 7);
        character->hair_style = (uint8_t)sqlite3_column_int(stmt, 8);
        character->hair_color = (uint8_t)sqlite3_column_int(stmt, 9);
        character->facial_hair = (uint8_t)sqlite3_column_int(stmt, 10);
        character->level = (uint8_t)sqlite3_column_int(stmt, 11);
        character->map = sqlite3_column_int(stmt, 12);
        character->x = (float)sqlite3_column_double(stmt, 13);
        character->y = (float)sqlite3_column_double(stmt, 14);
        character->z = (float)sqlite3_column_double(stmt, 15);
        character->orientation = (float)sqlite3_column_double(stmt, 16);

        sqlite3_finalize(stmt);
        return OK;
    }

    sqlite3_finalize(stmt);
    return ERR_NOT_FOUND;
}

result_t database_character_name_exists(const char *name, bool *exists) {
    const char *sql = "SELECT COUNT(*) FROM characters WHERE name = ? COLLATE NOCASE";

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(g_database->db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Database", "Failed to prepare statement: %s", sqlite3_errmsg(g_database->db));
        return ERR_DATABASE;
    }

    sqlite3_bind_text(stmt, 1, name, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        *exists = sqlite3_column_int(stmt, 0) > 0;
    } else {
        *exists = false;
    }

    sqlite3_finalize(stmt);
    return OK;
}

result_t database_create_character(character_t *character) {
    const char *sql =
        "INSERT INTO characters (account_id, name, race, class, gender, skin, face, "
        "hair_style, hair_color, facial_hair, level, map, x, y, z, orientation) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(g_database->db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Database", "Failed to prepare statement: %s", sqlite3_errmsg(g_database->db));
        return ERR_DATABASE;
    }

    sqlite3_bind_int(stmt, 1, character->account_id);
    sqlite3_bind_text(stmt, 2, character->name, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, character->race);
    sqlite3_bind_int(stmt, 4, character->char_class);
    sqlite3_bind_int(stmt, 5, character->gender);
    sqlite3_bind_int(stmt, 6, character->skin);
    sqlite3_bind_int(stmt, 7, character->face);
    sqlite3_bind_int(stmt, 8, character->hair_style);
    sqlite3_bind_int(stmt, 9, character->hair_color);
    sqlite3_bind_int(stmt, 10, character->facial_hair);
    sqlite3_bind_int(stmt, 11, character->level);
    sqlite3_bind_int(stmt, 12, character->map);
    sqlite3_bind_double(stmt, 13, character->x);
    sqlite3_bind_double(stmt, 14, character->y);
    sqlite3_bind_double(stmt, 15, character->z);
    sqlite3_bind_double(stmt, 16, character->orientation);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        LOG_ERROR("Database", "Failed to create character: %s", sqlite3_errmsg(g_database->db));
        return ERR_DATABASE;
    }

    character->id = (int)sqlite3_last_insert_rowid(g_database->db);
    return OK;
}

result_t database_update_character_position(int character_id, int map, float x, float y, float z, float orientation) {
    const char *sql = "UPDATE characters SET map = ?, x = ?, y = ?, z = ?, orientation = ? WHERE id = ?";

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(g_database->db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Database", "Failed to prepare statement: %s", sqlite3_errmsg(g_database->db));
        return ERR_DATABASE;
    }

    sqlite3_bind_int(stmt, 1, map);
    sqlite3_bind_double(stmt, 2, x);
    sqlite3_bind_double(stmt, 3, y);
    sqlite3_bind_double(stmt, 4, z);
    sqlite3_bind_double(stmt, 5, orientation);
    sqlite3_bind_int(stmt, 6, character_id);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        LOG_ERROR("Database", "Failed to update position: %s", sqlite3_errmsg(g_database->db));
        return ERR_DATABASE;
    }

    return OK;
}

result_t database_delete_character(int character_id) {
    const char *sql = "DELETE FROM characters WHERE id = ?";

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(g_database->db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        LOG_ERROR("Database", "Failed to prepare statement: %s", sqlite3_errmsg(g_database->db));
        return ERR_DATABASE;
    }

    sqlite3_bind_int(stmt, 1, character_id);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        LOG_ERROR("Database", "Failed to delete character: %s", sqlite3_errmsg(g_database->db));
        return ERR_DATABASE;
    }

    return OK;
}
