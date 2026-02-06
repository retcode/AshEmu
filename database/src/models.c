/*
 * AshEmu - WoW 2.4.3 Server Emulator
 * Copyright (C) 2025 AshEmu Team
 *
 * models.c - Model structure implementations
 */

#include "models.h"

void account_init(account_t *account) {
    memset(account, 0, sizeof(account_t));
    account->id = -1;
    account->has_session_key = false;
}

void character_init(character_t *character) {
    memset(character, 0, sizeof(character_t));
    character->id = -1;
    character->account_id = -1;
    character->level = 1;
}

void character_list_init(character_list_t *list) {
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
}

void character_list_free(character_list_t *list) {
    FREE(list->items);
    list->count = 0;
    list->capacity = 0;
}

result_t character_list_add(character_list_t *list, const character_t *character) {
    if (list->count >= list->capacity) {
        int new_capacity = list->capacity == 0 ? 4 : list->capacity * 2;
        character_t *new_items = (character_t*)realloc(list->items, new_capacity * sizeof(character_t));
        if (!new_items) return ERR_MEMORY;
        list->items = new_items;
        list->capacity = new_capacity;
    }
    list->items[list->count++] = *character;
    return OK;
}
