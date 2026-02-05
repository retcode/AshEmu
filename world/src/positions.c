/*
 * AshEmu - WoW 1.12.1 Server Emulator
 * Copyright (C) 2025 AshEmu Team
 *
 * positions.c - Race starting positions
 */

#include "positions.h"

/* Race IDs: Human=1, Orc=2, Dwarf=3, NightElf=4, Undead=5, Tauren=6, Gnome=7, Troll=8 */
static const start_position_t START_POSITIONS[] = {
    /* Index 0 unused (race IDs start at 1) */
    { 0, 0.0f, 0.0f, 0.0f, 0.0f },

    /* Human - Northshire Valley (Race 1) */
    { 0, -8949.95f, -132.493f, 83.5312f, 0.0f },

    /* Orc - Valley of Trials (Race 2) */
    { 1, -618.518f, -4251.67f, 38.718f, 0.0f },

    /* Dwarf - Coldridge Valley (Race 3) */
    { 0, -6240.32f, 331.033f, 382.758f, 6.17716f },

    /* Night Elf - Shadowglen (Race 4) */
    { 1, 10311.3f, 832.463f, 1326.41f, 5.69632f },

    /* Undead - Deathknell (Race 5) */
    { 0, 1676.71f, 1678.31f, 121.67f, 2.70526f },

    /* Tauren - Camp Narache (Race 6) */
    { 1, -2917.58f, -257.98f, 52.9968f, 0.0f },

    /* Gnome - Coldridge Valley (same as Dwarf) (Race 7) */
    { 0, -6240.32f, 331.033f, 382.758f, 6.17716f },

    /* Troll - Valley of Trials (same as Orc) (Race 8) */
    { 1, -618.518f, -4251.67f, 38.718f, 0.0f }
};

#define NUM_RACES 8

const start_position_t *get_start_position(uint8_t race) {
    if (race < 1 || race > NUM_RACES) {
        return &START_POSITIONS[1];  /* Default to Human */
    }
    return &START_POSITIONS[race];
}
