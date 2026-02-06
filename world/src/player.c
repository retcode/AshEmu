/*
 * AshEmu - WoW 2.4.3 Server Emulator (TBC)
 * Copyright (C) 2025 AshEmu Team
 *
 * player.c - Player helper functions
 */

#include "player.h"
#include "positions.h"

void player_init(player_t *player, const character_t *character) {
    player->character = *character;
    player->guid = (uint64_t)character->id;  /* Player GUID high part is 0 */
    player->map = character->map;
    player->x = character->x;
    player->y = character->y;
    player->z = character->z;
    player->orientation = character->orientation;

    /* Get zone/area from start position based on race */
    const start_position_t *start = get_start_position(character->race);
    player->zone_id = start->zone_id;
    player->area_id = start->area_id;
}

int player_get_display_id(const player_t *player) {
    uint8_t race = player->character.race;
    uint8_t gender = player->character.gender;

    /* Display IDs for each race/gender combination */
    switch (race) {
        case 1:  /* Human */
            return gender == 0 ? 49 : 50;
        case 2:  /* Orc */
            return gender == 0 ? 51 : 52;
        case 3:  /* Dwarf */
            return gender == 0 ? 53 : 54;
        case 4:  /* Night Elf */
            return gender == 0 ? 55 : 56;
        case 5:  /* Undead */
            return gender == 0 ? 57 : 58;
        case 6:  /* Tauren */
            return gender == 0 ? 59 : 60;
        case 7:  /* Gnome */
            return gender == 0 ? 1563 : 1564;
        case 8:  /* Troll */
            return gender == 0 ? 1478 : 1479;
        case 10: /* Blood Elf (TBC) */
            return gender == 0 ? 15476 : 15475;
        case 11: /* Draenei (TBC) */
            return gender == 0 ? 16125 : 16126;
        default:
            return 49;  /* Default to Human Male */
    }
}

int player_get_faction_template(const player_t *player) {
    switch (player->character.race) {
        case 1:  return 1;     /* Human - Stormwind */
        case 2:  return 2;     /* Orc - Orgrimmar */
        case 3:  return 3;     /* Dwarf - Ironforge */
        case 4:  return 4;     /* Night Elf - Darnassus */
        case 5:  return 5;     /* Undead - Undercity */
        case 6:  return 6;     /* Tauren - Thunder Bluff */
        case 7:  return 115;   /* Gnome - Gnomeregan */
        case 8:  return 116;   /* Troll - Darkspear */
        case 10: return 1610;  /* Blood Elf - Silvermoon (TBC) */
        case 11: return 1629;  /* Draenei - Exodar (TBC) */
        default: return 1;
    }
}

uint8_t player_get_power_type(const player_t *player) {
    /* Power types: Mana=0, Rage=1, Focus=2, Energy=3 */
    switch (player->character.char_class) {
        case 1:  return 1;  /* Warrior - Rage */
        case 2:  return 0;  /* Paladin - Mana */
        case 3:  return 0;  /* Hunter - Mana (Focus in later versions) */
        case 4:  return 3;  /* Rogue - Energy */
        case 5:  return 0;  /* Priest - Mana */
        case 7:  return 0;  /* Shaman - Mana */
        case 8:  return 0;  /* Mage - Mana */
        case 9:  return 0;  /* Warlock - Mana */
        case 11: return 0;  /* Druid - Mana */
        default: return 0;
    }
}

int player_get_health(const player_t *player) {
    (void)player;
    return 100;
}

int player_get_max_health(const player_t *player) {
    (void)player;
    return 100;
}

int player_get_power(const player_t *player) {
    /* Rage starts at 0, mana/energy starts at 100 */
    return player->character.char_class == 1 ? 0 : 100;
}

int player_get_max_power(const player_t *player) {
    /* Rage max is 1000 (displayed /10), others are 100 */
    return player->character.char_class == 1 ? 1000 : 100;
}
