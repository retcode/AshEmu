/*
 * AshEmu - WoW 1.12.1 Server Emulator
 * Copyright (C) 2025 AshEmu Team
 *
 * models.h - Account and Character data structures
 */

#ifndef MODELS_H
#define MODELS_H

#include "common.h"
#include "crypto.h"

/* Account structure */
typedef struct {
    int id;
    char username[MAX_USERNAME + 1];
    uint8_t salt[SRP6_SALT_SIZE];
    uint8_t verifier[SRP6_VERIFIER_SIZE];
    uint8_t session_key[SRP6_SESSION_KEY_SIZE];
    bool has_session_key;
} account_t;

/* Character structure */
typedef struct {
    int id;
    int account_id;
    char name[MAX_CHARACTER_NAME + 1];
    uint8_t race;
    uint8_t char_class;  /* 'class' is reserved in C++ */
    uint8_t gender;
    uint8_t skin;
    uint8_t face;
    uint8_t hair_style;
    uint8_t hair_color;
    uint8_t facial_hair;
    uint8_t level;
    int map;
    float x;
    float y;
    float z;
    float orientation;
} character_t;

/* Character list (dynamic array) */
typedef struct {
    character_t *items;
    int count;
    int capacity;
} character_list_t;

/* Initialize account structure */
void account_init(account_t *account);

/* Initialize character structure */
void character_init(character_t *character);

/* Initialize character list */
void character_list_init(character_list_t *list);

/* Free character list */
void character_list_free(character_list_t *list);

/* Add character to list */
result_t character_list_add(character_list_t *list, const character_t *character);

#endif /* MODELS_H */
