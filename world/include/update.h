/*
 * AshEmu - WoW 2.4.3 Server Emulator (TBC)
 * Copyright (C) 2025 AshEmu Team
 *
 * update.h - Update packet builder and field indices
 */

#ifndef UPDATE_H
#define UPDATE_H

#include "common.h"
#include "player.h"
#include "packet.h"

/* ============================================================================
 * TBC 2.4.3 Update Field Indices
 * Based on CMaNGOS TBC UpdateFields.h
 * ============================================================================ */

/* Object fields (0x0000 - 0x0005) */
#define OBJECT_FIELD_GUID                     0x0000  /* 2 fields (uint64) */
#define OBJECT_FIELD_TYPE                     0x0002
#define OBJECT_FIELD_ENTRY                    0x0003
#define OBJECT_FIELD_SCALE_X                  0x0004
#define OBJECT_FIELD_PADDING                  0x0005
#define OBJECT_END                            0x0006

/* Item fields (0x0006 - 0x003F) - not needed for player login */
#define ITEM_END                              0x0040

/* Container fields (0x0040 - 0x0057) - not needed for player login */
#define CONTAINER_END                         0x0058

/* Unit fields (0x0006 - 0x008D for TBC) */
#define UNIT_FIELD_CHARM                      0x0006  /* 2 fields */
#define UNIT_FIELD_SUMMON                     0x0008  /* 2 fields */
#define UNIT_FIELD_CHARMEDBY                  0x000A  /* 2 fields */
#define UNIT_FIELD_SUMMONEDBY                 0x000C  /* 2 fields */
#define UNIT_FIELD_CREATEDBY                  0x000E  /* 2 fields */
#define UNIT_FIELD_TARGET                     0x0010  /* 2 fields */
#define UNIT_FIELD_PERSUADED                  0x0012  /* 2 fields */
#define UNIT_FIELD_CHANNEL_OBJECT             0x0014  /* 2 fields */
#define UNIT_FIELD_HEALTH                     0x0016
#define UNIT_FIELD_POWER1                     0x0017  /* Mana */
#define UNIT_FIELD_POWER2                     0x0018  /* Rage */
#define UNIT_FIELD_POWER3                     0x0019  /* Focus */
#define UNIT_FIELD_POWER4                     0x001A  /* Energy */
#define UNIT_FIELD_POWER5                     0x001B  /* Happiness */
#define UNIT_FIELD_MAXHEALTH                  0x001C
#define UNIT_FIELD_MAXPOWER1                  0x001D
#define UNIT_FIELD_MAXPOWER2                  0x001E
#define UNIT_FIELD_MAXPOWER3                  0x001F
#define UNIT_FIELD_MAXPOWER4                  0x0020
#define UNIT_FIELD_MAXPOWER5                  0x0021
#define UNIT_FIELD_LEVEL                      0x0022
#define UNIT_FIELD_FACTIONTEMPLATE            0x0023
#define UNIT_FIELD_BYTES_0                    0x0024  /* Race, Class, Gender, PowerType */
#define UNIT_VIRTUAL_ITEM_SLOT_DISPLAY        0x0025  /* 3 fields */
#define UNIT_VIRTUAL_ITEM_INFO                0x0028  /* 6 fields */
#define UNIT_FIELD_FLAGS                      0x002E
#define UNIT_FIELD_FLAGS_2                    0x002F  /* TBC addition */
#define UNIT_FIELD_AURA                       0x0030  /* 56 fields (was 48 in vanilla) */
#define UNIT_FIELD_AURAFLAGS                  0x0068  /* 14 fields (was 6) */
#define UNIT_FIELD_AURALEVELS                 0x0076  /* 14 fields (was 12) */
#define UNIT_FIELD_AURAAPPLICATIONS           0x0084  /* 14 fields (was 12) */
#define UNIT_FIELD_AURASTATE                  0x0092
#define UNIT_FIELD_BASEATTACKTIME             0x0093  /* 2 fields */
#define UNIT_FIELD_RANGEDATTACKTIME           0x0095
#define UNIT_FIELD_BOUNDINGRADIUS             0x0096
#define UNIT_FIELD_COMBATREACH                0x0097
#define UNIT_FIELD_DISPLAYID                  0x0098
#define UNIT_FIELD_NATIVEDISPLAYID            0x0099
#define UNIT_FIELD_MOUNTDISPLAYID             0x009A
#define UNIT_FIELD_MINDAMAGE                  0x009B
#define UNIT_FIELD_MAXDAMAGE                  0x009C
#define UNIT_FIELD_MINOFFHANDDAMAGE           0x009D
#define UNIT_FIELD_MAXOFFHANDDAMAGE           0x009E
#define UNIT_FIELD_BYTES_1                    0x009F  /* Standstate, etc. */
#define UNIT_FIELD_PETNUMBER                  0x00A0
#define UNIT_FIELD_PET_NAME_TIMESTAMP         0x00A1
#define UNIT_FIELD_PETEXPERIENCE              0x00A2
#define UNIT_FIELD_PETNEXTLEVELEXP            0x00A3
#define UNIT_DYNAMIC_FLAGS                    0x00A4
#define UNIT_CHANNEL_SPELL                    0x00A5
#define UNIT_MOD_CAST_SPEED                   0x00A6
#define UNIT_CREATED_BY_SPELL                 0x00A7
#define UNIT_NPC_FLAGS                        0x00A8
#define UNIT_NPC_EMOTESTATE                   0x00A9
#define UNIT_TRAINING_POINTS                  0x00AA
#define UNIT_FIELD_STAT0                      0x00AB  /* Strength */
#define UNIT_FIELD_STAT1                      0x00AC  /* Agility */
#define UNIT_FIELD_STAT2                      0x00AD  /* Stamina */
#define UNIT_FIELD_STAT3                      0x00AE  /* Intellect */
#define UNIT_FIELD_STAT4                      0x00AF  /* Spirit */
#define UNIT_FIELD_POSSTAT0                   0x00B0  /* TBC: Positive stat mods */
#define UNIT_FIELD_POSSTAT1                   0x00B1
#define UNIT_FIELD_POSSTAT2                   0x00B2
#define UNIT_FIELD_POSSTAT3                   0x00B3
#define UNIT_FIELD_POSSTAT4                   0x00B4
#define UNIT_FIELD_NEGSTAT0                   0x00B5  /* TBC: Negative stat mods */
#define UNIT_FIELD_NEGSTAT1                   0x00B6
#define UNIT_FIELD_NEGSTAT2                   0x00B7
#define UNIT_FIELD_NEGSTAT3                   0x00B8
#define UNIT_FIELD_NEGSTAT4                   0x00B9
#define UNIT_FIELD_RESISTANCES                0x00BA  /* 7 fields */
#define UNIT_FIELD_RESISTANCEBUFFMODSPOSITIVE 0x00C1  /* 7 fields */
#define UNIT_FIELD_RESISTANCEBUFFMODSNEGATIVE 0x00C8  /* 7 fields */
#define UNIT_FIELD_BASE_MANA                  0x00CF
#define UNIT_FIELD_BASE_HEALTH                0x00D0
#define UNIT_FIELD_BYTES_2                    0x00D1
#define UNIT_FIELD_ATTACK_POWER               0x00D2
#define UNIT_FIELD_ATTACK_POWER_MODS          0x00D3
#define UNIT_FIELD_ATTACK_POWER_MULTIPLIER    0x00D4
#define UNIT_FIELD_RANGED_ATTACK_POWER        0x00D5
#define UNIT_FIELD_RANGED_ATTACK_POWER_MODS   0x00D6
#define UNIT_FIELD_RANGED_ATTACK_POWER_MULT   0x00D7
#define UNIT_FIELD_MINRANGEDDAMAGE            0x00D8
#define UNIT_FIELD_MAXRANGEDDAMAGE            0x00D9
#define UNIT_FIELD_POWER_COST_MODIFIER        0x00DA  /* 7 fields */
#define UNIT_FIELD_POWER_COST_MULTIPLIER      0x00E1  /* 7 fields */
#define UNIT_FIELD_MAXHEALTHMODIFIER          0x00E8
#define UNIT_END                              0x00E9

/* Player fields (0x00E9 onwards for TBC) */
#define PLAYER_DUEL_ARBITER                   0x00E9  /* 2 fields */
#define PLAYER_FLAGS                          0x00EB
#define PLAYER_GUILDID                        0x00EC
#define PLAYER_GUILDRANK                      0x00ED
#define PLAYER_BYTES                          0x00EE  /* Skin, Face, HairStyle, HairColor */
#define PLAYER_BYTES_2                        0x00EF  /* FacialHair, ?, ?, RestState */
#define PLAYER_BYTES_3                        0x00F0  /* Gender, drunk state */
#define PLAYER_DUEL_TEAM                      0x00F1
#define PLAYER_GUILD_TIMESTAMP                0x00F2
#define PLAYER_QUEST_LOG_1_1                  0x00F3  /* Quest log: 20 quests, 4 fields each = 80 */
#define PLAYER_QUEST_LOG_LAST_3               0x0142
#define PLAYER_VISIBLE_ITEM_1_CREATOR         0x0143  /* 19 slots, 16 fields each = 304 */
#define PLAYER_VISIBLE_ITEM_LAST_PAD          0x0272
#define PLAYER_FIELD_INV_SLOT_HEAD            0x0273  /* 23 slots, 2 fields each = 46 */
#define PLAYER_FIELD_PACK_SLOT_1              0x02A1  /* 16 slots, 2 each = 32 */
#define PLAYER_FIELD_BANK_SLOT_1              0x02C1  /* 28 slots, 2 each = 56 */
#define PLAYER_FIELD_BANKBAG_SLOT_1           0x02F9  /* 7 slots, 2 each = 14 */
#define PLAYER_FIELD_VENDORBUYBACK_SLOT_1     0x0307  /* 12 slots, 2 each = 24 */
#define PLAYER_FIELD_KEYRING_SLOT_1           0x031F  /* 32 slots, 2 each = 64 */
#define PLAYER_FIELD_VANITYPET_SLOT_1         0x035F  /* TBC: 18 vanity pet slots */
#define PLAYER_FARSIGHT                       0x0383  /* 2 fields */
#define PLAYER_FIELD_KNOWN_TITLES             0x0385  /* 2 fields */
#define PLAYER_XP                             0x0387
#define PLAYER_NEXT_LEVEL_XP                  0x0388
#define PLAYER_SKILL_INFO_1_1                 0x0389  /* 384 fields for skills */
#define PLAYER_CHARACTER_POINTS1              0x0509
#define PLAYER_CHARACTER_POINTS2              0x050A
#define PLAYER_TRACK_CREATURES                0x050B
#define PLAYER_TRACK_RESOURCES                0x050C
#define PLAYER_BLOCK_PERCENTAGE               0x050D
#define PLAYER_DODGE_PERCENTAGE               0x050E
#define PLAYER_PARRY_PERCENTAGE               0x050F
#define PLAYER_EXPERTISE                      0x0510  /* TBC addition */
#define PLAYER_OFFHAND_EXPERTISE              0x0511  /* TBC addition */
#define PLAYER_CRIT_PERCENTAGE                0x0512
#define PLAYER_RANGED_CRIT_PERCENTAGE         0x0513
#define PLAYER_OFFHAND_CRIT_PERCENTAGE        0x0514  /* TBC addition */
#define PLAYER_SPELL_CRIT_PERCENTAGE1         0x0515  /* 7 fields */
#define PLAYER_SHIELD_BLOCK                   0x051C
#define PLAYER_EXPLORED_ZONES_1               0x051D  /* 128 fields */
#define PLAYER_REST_STATE_EXPERIENCE          0x059D
#define PLAYER_FIELD_COINAGE                  0x059E
#define PLAYER_FIELD_MOD_DAMAGE_DONE_POS      0x059F  /* 7 fields */
#define PLAYER_FIELD_MOD_DAMAGE_DONE_NEG      0x05A6  /* 7 fields */
#define PLAYER_FIELD_MOD_DAMAGE_DONE_PCT      0x05AD  /* 7 fields */
#define PLAYER_FIELD_MOD_HEALING_DONE_POS     0x05B4
#define PLAYER_FIELD_MOD_TARGET_RESISTANCE    0x05B5
#define PLAYER_FIELD_MOD_TARGET_PHYS_RESIST   0x05B6
#define PLAYER_FIELD_BYTES                    0x05B7
#define PLAYER_AMMO_ID                        0x05B8
#define PLAYER_SELF_RES_SPELL                 0x05B9
#define PLAYER_FIELD_PVP_MEDALS               0x05BA
#define PLAYER_FIELD_BUYBACK_PRICE_1          0x05BB  /* 12 fields */
#define PLAYER_FIELD_BUYBACK_TIMESTAMP_1      0x05C7  /* 12 fields */
#define PLAYER_FIELD_KILLS                    0x05D3
#define PLAYER_FIELD_TODAY_CONTRIBUTION       0x05D4
#define PLAYER_FIELD_YESTERDAY_CONTRIBUTION   0x05D5
#define PLAYER_FIELD_LIFETIME_HONORABLE_KILLS 0x05D6
#define PLAYER_FIELD_BYTES2                   0x05D7
#define PLAYER_FIELD_WATCHED_FACTION_INDEX    0x05D8
#define PLAYER_FIELD_COMBAT_RATING_1          0x05D9  /* 24 fields (TBC has more ratings) */
#define PLAYER_FIELD_ARENA_TEAM_INFO_1_1      0x05F1  /* 18 fields */
#define PLAYER_FIELD_HONOR_CURRENCY           0x0603  /* TBC addition */
#define PLAYER_FIELD_ARENA_CURRENCY           0x0604  /* TBC addition (arena points) */
#define PLAYER_FIELD_MOD_MANA_REGEN           0x0605
#define PLAYER_FIELD_MOD_MANA_REGEN_INTERRUPT 0x0606
#define PLAYER_FIELD_MAX_LEVEL                0x0607
#define PLAYER_FIELD_DAILY_QUESTS_1           0x0608  /* 25 fields */
#define PLAYER_END                            0x0621

/* Object type flags */
#define TYPE_OBJECT     0x0001
#define TYPE_ITEM       0x0002
#define TYPE_CONTAINER  0x0004
#define TYPE_UNIT       0x0008
#define TYPE_PLAYER     0x0010
#define TYPE_GAMEOBJECT 0x0020
#define TYPE_DYNAMICOBJECT 0x0040
#define TYPE_CORPSE     0x0080

/* Maximum number of update fields we track (increased for TBC) */
#define MAX_UPDATE_FIELDS 1600

/* Update builder structure */
typedef struct {
    uint32_t fields[MAX_UPDATE_FIELDS];
    bool field_set[MAX_UPDATE_FIELDS];
    int max_field;
} update_builder_t;

/* Initialize update builder */
void update_builder_init(update_builder_t *builder);

/* Set a GUID field (2 uint32 fields) */
void update_set_guid(update_builder_t *builder, int field, uint64_t value);

/* Set uint32 field */
void update_set_uint32(update_builder_t *builder, int field, uint32_t value);

/* Set int32 field */
void update_set_int32(update_builder_t *builder, int field, int32_t value);

/* Set float field */
void update_set_float(update_builder_t *builder, int field, float value);

/* Set a byte within a uint32 field (byteIndex 0-3) */
void update_set_byte(update_builder_t *builder, int field, int byte_index, uint8_t value);

/* Build SMSG_UPDATE_OBJECT packet for player creation */
result_t update_build_create_packet(update_builder_t *builder,
                                            const player_t *player,
                                            bool self,
                                            packet_writer_t *packet);

/* Backwards compatibility macros for old field names */
#define UF_OBJECT_FIELD_GUID              OBJECT_FIELD_GUID
#define UF_OBJECT_FIELD_TYPE              OBJECT_FIELD_TYPE
#define UF_OBJECT_FIELD_ENTRY             OBJECT_FIELD_ENTRY
#define UF_OBJECT_FIELD_SCALE_X           OBJECT_FIELD_SCALE_X
#define UF_OBJECT_FIELD_PADDING           OBJECT_FIELD_PADDING
#define UF_UNIT_FIELD_CHARM               UNIT_FIELD_CHARM
#define UF_UNIT_FIELD_SUMMON              UNIT_FIELD_SUMMON
#define UF_UNIT_FIELD_CHARMEDBY           UNIT_FIELD_CHARMEDBY
#define UF_UNIT_FIELD_SUMMONEDBY          UNIT_FIELD_SUMMONEDBY
#define UF_UNIT_FIELD_CREATEDBY           UNIT_FIELD_CREATEDBY
#define UF_UNIT_FIELD_TARGET              UNIT_FIELD_TARGET
#define UF_UNIT_FIELD_PERSUADED           UNIT_FIELD_PERSUADED
#define UF_UNIT_FIELD_CHANNEL_OBJECT      UNIT_FIELD_CHANNEL_OBJECT
#define UF_UNIT_FIELD_HEALTH              UNIT_FIELD_HEALTH
#define UF_UNIT_FIELD_POWER1              UNIT_FIELD_POWER1
#define UF_UNIT_FIELD_POWER2              UNIT_FIELD_POWER2
#define UF_UNIT_FIELD_POWER3              UNIT_FIELD_POWER3
#define UF_UNIT_FIELD_POWER4              UNIT_FIELD_POWER4
#define UF_UNIT_FIELD_POWER5              UNIT_FIELD_POWER5
#define UF_UNIT_FIELD_MAXHEALTH           UNIT_FIELD_MAXHEALTH
#define UF_UNIT_FIELD_MAXPOWER1           UNIT_FIELD_MAXPOWER1
#define UF_UNIT_FIELD_MAXPOWER2           UNIT_FIELD_MAXPOWER2
#define UF_UNIT_FIELD_MAXPOWER3           UNIT_FIELD_MAXPOWER3
#define UF_UNIT_FIELD_MAXPOWER4           UNIT_FIELD_MAXPOWER4
#define UF_UNIT_FIELD_MAXPOWER5           UNIT_FIELD_MAXPOWER5
#define UF_UNIT_FIELD_LEVEL               UNIT_FIELD_LEVEL
#define UF_UNIT_FIELD_FACTIONTEMPLATE     UNIT_FIELD_FACTIONTEMPLATE
#define UF_UNIT_FIELD_BYTES_0             UNIT_FIELD_BYTES_0
#define UF_UNIT_VIRTUAL_ITEM_SLOT_DISPLAY UNIT_VIRTUAL_ITEM_SLOT_DISPLAY
#define UF_UNIT_VIRTUAL_ITEM_INFO         UNIT_VIRTUAL_ITEM_INFO
#define UF_UNIT_FIELD_FLAGS               UNIT_FIELD_FLAGS
#define UF_UNIT_FIELD_AURA                UNIT_FIELD_AURA
#define UF_UNIT_FIELD_AURAFLAGS           UNIT_FIELD_AURAFLAGS
#define UF_UNIT_FIELD_AURALEVELS          UNIT_FIELD_AURALEVELS
#define UF_UNIT_FIELD_AURAAPPLICATIONS    UNIT_FIELD_AURAAPPLICATIONS
#define UF_UNIT_FIELD_AURASTATE           UNIT_FIELD_AURASTATE
#define UF_UNIT_FIELD_BASEATTACKTIME      UNIT_FIELD_BASEATTACKTIME
#define UF_UNIT_FIELD_RANGEDATTACKTIME    UNIT_FIELD_RANGEDATTACKTIME
#define UF_UNIT_FIELD_BOUNDINGRADIUS      UNIT_FIELD_BOUNDINGRADIUS
#define UF_UNIT_FIELD_COMBATREACH         UNIT_FIELD_COMBATREACH
#define UF_UNIT_FIELD_DISPLAYID           UNIT_FIELD_DISPLAYID
#define UF_UNIT_FIELD_NATIVEDISPLAYID     UNIT_FIELD_NATIVEDISPLAYID
#define UF_UNIT_FIELD_MOUNTDISPLAYID      UNIT_FIELD_MOUNTDISPLAYID
#define UF_UNIT_FIELD_MINDAMAGE           UNIT_FIELD_MINDAMAGE
#define UF_UNIT_FIELD_MAXDAMAGE           UNIT_FIELD_MAXDAMAGE
#define UF_UNIT_FIELD_MINOFFHANDDAMAGE    UNIT_FIELD_MINOFFHANDDAMAGE
#define UF_UNIT_FIELD_MAXOFFHANDDAMAGE    UNIT_FIELD_MAXOFFHANDDAMAGE
#define UF_UNIT_FIELD_BYTES_1             UNIT_FIELD_BYTES_1
#define UF_UNIT_FIELD_PETNUMBER           UNIT_FIELD_PETNUMBER
#define UF_UNIT_FIELD_PET_NAME_TIMESTAMP  UNIT_FIELD_PET_NAME_TIMESTAMP
#define UF_UNIT_FIELD_PETEXPERIENCE       UNIT_FIELD_PETEXPERIENCE
#define UF_UNIT_FIELD_PETNEXTLEVELEXP     UNIT_FIELD_PETNEXTLEVELEXP
#define UF_UNIT_DYNAMIC_FLAGS             UNIT_DYNAMIC_FLAGS
#define UF_UNIT_CHANNEL_SPELL             UNIT_CHANNEL_SPELL
#define UF_UNIT_MOD_CAST_SPEED            UNIT_MOD_CAST_SPEED
#define UF_UNIT_CREATED_BY_SPELL          UNIT_CREATED_BY_SPELL
#define UF_UNIT_NPC_FLAGS                 UNIT_NPC_FLAGS
#define UF_UNIT_NPC_EMOTESTATE            UNIT_NPC_EMOTESTATE
#define UF_UNIT_TRAINING_POINTS           UNIT_TRAINING_POINTS
#define UF_UNIT_FIELD_STAT0               UNIT_FIELD_STAT0
#define UF_UNIT_FIELD_STAT1               UNIT_FIELD_STAT1
#define UF_UNIT_FIELD_STAT2               UNIT_FIELD_STAT2
#define UF_UNIT_FIELD_STAT3               UNIT_FIELD_STAT3
#define UF_UNIT_FIELD_STAT4               UNIT_FIELD_STAT4
#define UF_UNIT_FIELD_RESISTANCES         UNIT_FIELD_RESISTANCES
#define UF_UNIT_FIELD_BASE_MANA           UNIT_FIELD_BASE_MANA
#define UF_UNIT_FIELD_BASE_HEALTH         UNIT_FIELD_BASE_HEALTH
#define UF_UNIT_FIELD_BYTES_2             UNIT_FIELD_BYTES_2
#define UF_UNIT_FIELD_ATTACK_POWER        UNIT_FIELD_ATTACK_POWER
#define UF_UNIT_FIELD_ATTACK_POWER_MODS   UNIT_FIELD_ATTACK_POWER_MODS
#define UF_UNIT_END                       UNIT_END
#define UF_PLAYER_DUEL_ARBITER            PLAYER_DUEL_ARBITER
#define UF_PLAYER_FLAGS                   PLAYER_FLAGS
#define UF_PLAYER_GUILDID                 PLAYER_GUILDID
#define UF_PLAYER_GUILDRANK               PLAYER_GUILDRANK
#define UF_PLAYER_BYTES                   PLAYER_BYTES
#define UF_PLAYER_BYTES_2                 PLAYER_BYTES_2
#define UF_PLAYER_BYTES_3                 PLAYER_BYTES_3
#define UF_PLAYER_DUEL_TEAM               PLAYER_DUEL_TEAM
#define UF_PLAYER_GUILD_TIMESTAMP         PLAYER_GUILD_TIMESTAMP
#define UF_PLAYER_END                     PLAYER_END

#endif /* UPDATE_H */
