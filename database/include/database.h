/*
 * AshEmu - WoW 1.12.1 Server Emulator
 * Copyright (C) 2025 AshEmu Team
 *
 * database.h - SQLite database wrapper
 */

#ifndef DATABASE_H
#define DATABASE_H

#include "common.h"
#include "models.h"
#include <sqlite3.h>

/* Database context */
typedef struct {
    sqlite3 *db;
    char path[MAX_PATH];
} database_t;

/* Global database instance (singleton pattern) */
extern database_t *g_database;

/* Initialize database (creates tables if needed) */
result_t database_init(const char *db_path);

/* Shutdown database */
void database_shutdown(void);

/* Account operations */
result_t database_get_account(const char *username, account_t *account);
result_t database_create_account(const char *username, const uint8_t salt[SRP6_SALT_SIZE],
                                         const uint8_t verifier[SRP6_VERIFIER_SIZE], account_t *account);
result_t database_update_session_key(int account_id, const uint8_t session_key[SRP6_SESSION_KEY_SIZE]);

/* Character operations */
result_t database_get_characters(int account_id, character_list_t *list);
result_t database_get_character(int character_id, character_t *character);
result_t database_character_name_exists(const char *name, bool *exists);
result_t database_create_character(character_t *character);
result_t database_update_character_position(int character_id, int map, float x, float y, float z, float orientation);
result_t database_delete_character(int character_id);

#endif /* DATABASE_H */
