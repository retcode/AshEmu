/*
 * AshEmu - WoW 2.4.3 Server Emulator (TBC)
 * Copyright (C) 2025 AshEmu Team
 *
 * positions.c - Race starting positions
 */

#include "positions.h"

/* Race IDs:
 * 1=Human, 2=Orc, 3=Dwarf, 4=NightElf, 5=Undead, 6=Tauren, 7=Gnome, 8=Troll
 * 9=Goblin (unused in TBC)
 * 10=BloodElf (TBC), 11=Draenei (TBC)
 */
static const start_position_t START_POSITIONS[] = {
    /* Index 0 unused (race IDs start at 1) */
    { 0, 12, 9, 0.0f, 0.0f, 0.0f, 0.0f },

    /* Human - Northshire Valley (Race 1) */
    { 0, 12, 9, -8949.95f, -132.493f, 83.5312f, 0.0f },

    /* Orc - Valley of Trials (Race 2) */
    { 1, 14, 363, -618.518f, -4251.67f, 38.718f, 0.0f },

    /* Dwarf - Coldridge Valley (Race 3) */
    { 0, 1, 132, -6240.32f, 331.033f, 382.758f, 6.17716f },

    /* Night Elf - Shadowglen (Race 4) */
    { 1, 141, 188, 10311.3f, 832.463f, 1326.41f, 5.69632f },

    /* Undead - Deathknell (Race 5) */
    { 0, 85, 154, 1676.71f, 1678.31f, 121.67f, 2.70526f },

    /* Tauren - Camp Narache (Race 6) */
    { 1, 215, 222, -2917.58f, -257.98f, 52.9968f, 0.0f },

    /* Gnome - Coldridge Valley (same as Dwarf) (Race 7) */
    { 0, 1, 132, -6240.32f, 331.033f, 382.758f, 6.17716f },

    /* Troll - Valley of Trials (same as Orc) (Race 8) */
    { 1, 14, 363, -618.518f, -4251.67f, 38.718f, 0.0f },

    /* Goblin (Race 9 - unused placeholder) */
    { 0, 12, 9, 0.0f, 0.0f, 0.0f, 0.0f },

    /* Blood Elf - Sunstrider Isle, Eversong Woods (Race 10) - Map 530 = Outland */
    { 530, 3430, 3431, 10349.6f, -6357.29f, 33.4026f, 5.31605f },

    /* Draenei - Ammen Vale, Azuremyst Isle (Race 11) - Map 530 = Outland */
    { 530, 3524, 3526, -3961.64f, -13931.2f, 100.615f, 2.08364f }
};

const start_position_t *get_start_position(uint8_t race) {
    if (race < 1 || race > NUM_RACES || race == 9) {
        return &START_POSITIONS[1];  /* Default to Human */
    }
    return &START_POSITIONS[race];
}
