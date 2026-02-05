/*
 * AshEmu - WoW 1.12.1 Server Emulator
 * Copyright (C) 2025 AshEmu Team
 *
 * player.h - Player data and helper functions
 */

#ifndef PLAYER_H
#define PLAYER_H

#include "common.h"
#include "models.h"

/* Player structure */
typedef struct {
    character_t character;
    uint64_t guid;
    int map;
    float x;
    float y;
    float z;
    float orientation;
} player_t;

/* Initialize player from character */
void player_init(player_t *player, const character_t *character);

/* Get display ID for race/gender combination */
int player_get_display_id(const player_t *player);

/* Get faction template for race */
int player_get_faction_template(const player_t *player);

/* Get power type for class (0=Mana, 1=Rage, 3=Energy) */
uint8_t player_get_power_type(const player_t *player);

/* Get current health */
int player_get_health(const player_t *player);

/* Get max health */
int player_get_max_health(const player_t *player);

/* Get current power (mana/rage/energy) */
int player_get_power(const player_t *player);

/* Get max power */
int player_get_max_power(const player_t *player);

#endif /* PLAYER_H */
