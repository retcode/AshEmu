/*
 * AshEmu - WoW 1.12.1 Server Emulator
 * Copyright (C) 2025 AshEmu Team
 *
 * update.h - Update packet builder and field indices
 */

#ifndef UPDATE_H
#define UPDATE_H

#include "common.h"
#include "player.h"
#include "packet.h"

/* Update field indices (1.12.1) */

/* Object fields */
#define UF_OBJECT_FIELD_GUID              0x0000  /* 2 fields (uint64) */
#define UF_OBJECT_FIELD_TYPE              0x0002
#define UF_OBJECT_FIELD_ENTRY             0x0003
#define UF_OBJECT_FIELD_SCALE_X           0x0004
#define UF_OBJECT_FIELD_PADDING           0x0005

/* Unit fields */
#define UF_UNIT_FIELD_CHARM               0x0006  /* 2 fields */
#define UF_UNIT_FIELD_SUMMON              0x0008  /* 2 fields */
#define UF_UNIT_FIELD_CHARMEDBY           0x000A  /* 2 fields */
#define UF_UNIT_FIELD_SUMMONEDBY          0x000C  /* 2 fields */
#define UF_UNIT_FIELD_CREATEDBY           0x000E  /* 2 fields */
#define UF_UNIT_FIELD_TARGET              0x0010  /* 2 fields */
#define UF_UNIT_FIELD_PERSUADED           0x0012  /* 2 fields */
#define UF_UNIT_FIELD_CHANNEL_OBJECT      0x0014  /* 2 fields */
#define UF_UNIT_FIELD_HEALTH              0x0016
#define UF_UNIT_FIELD_POWER1              0x0017  /* Mana */
#define UF_UNIT_FIELD_POWER2              0x0018  /* Rage */
#define UF_UNIT_FIELD_POWER3              0x0019  /* Focus */
#define UF_UNIT_FIELD_POWER4              0x001A  /* Energy */
#define UF_UNIT_FIELD_POWER5              0x001B  /* Happiness */
#define UF_UNIT_FIELD_MAXHEALTH           0x001C
#define UF_UNIT_FIELD_MAXPOWER1           0x001D
#define UF_UNIT_FIELD_MAXPOWER2           0x001E
#define UF_UNIT_FIELD_MAXPOWER3           0x001F
#define UF_UNIT_FIELD_MAXPOWER4           0x0020
#define UF_UNIT_FIELD_MAXPOWER5           0x0021
#define UF_UNIT_FIELD_LEVEL               0x0022
#define UF_UNIT_FIELD_FACTIONTEMPLATE     0x0023
#define UF_UNIT_FIELD_BYTES_0             0x0024  /* Race, Class, Gender, PowerType */
#define UF_UNIT_VIRTUAL_ITEM_SLOT_DISPLAY 0x0025  /* 3 fields */
#define UF_UNIT_VIRTUAL_ITEM_INFO         0x0028  /* 6 fields */
#define UF_UNIT_FIELD_FLAGS               0x002E
#define UF_UNIT_FIELD_AURA                0x002F  /* 48 fields */
#define UF_UNIT_FIELD_AURAFLAGS           0x005F  /* 6 fields */
#define UF_UNIT_FIELD_AURALEVELS          0x0065  /* 12 fields */
#define UF_UNIT_FIELD_AURAAPPLICATIONS    0x0071  /* 12 fields */
#define UF_UNIT_FIELD_AURASTATE           0x007D
#define UF_UNIT_FIELD_BASEATTACKTIME      0x007E  /* 2 fields */
#define UF_UNIT_FIELD_RANGEDATTACKTIME    0x0080
#define UF_UNIT_FIELD_BOUNDINGRADIUS      0x0081
#define UF_UNIT_FIELD_COMBATREACH         0x0082
#define UF_UNIT_FIELD_DISPLAYID           0x0083
#define UF_UNIT_FIELD_NATIVEDISPLAYID     0x0084
#define UF_UNIT_FIELD_MOUNTDISPLAYID      0x0085
#define UF_UNIT_FIELD_MINDAMAGE           0x0086
#define UF_UNIT_FIELD_MAXDAMAGE           0x0087
#define UF_UNIT_FIELD_MINOFFHANDDAMAGE    0x0088
#define UF_UNIT_FIELD_MAXOFFHANDDAMAGE    0x0089
#define UF_UNIT_FIELD_BYTES_1             0x008A  /* Standstate, etc. */
#define UF_UNIT_FIELD_PETNUMBER           0x008B
#define UF_UNIT_FIELD_PET_NAME_TIMESTAMP  0x008C
#define UF_UNIT_FIELD_PETEXPERIENCE       0x008D
#define UF_UNIT_FIELD_PETNEXTLEVELEXP     0x008E
#define UF_UNIT_DYNAMIC_FLAGS             0x008F
#define UF_UNIT_CHANNEL_SPELL             0x0090
#define UF_UNIT_MOD_CAST_SPEED            0x0091
#define UF_UNIT_CREATED_BY_SPELL          0x0092
#define UF_UNIT_NPC_FLAGS                 0x0093
#define UF_UNIT_NPC_EMOTESTATE            0x0094
#define UF_UNIT_TRAINING_POINTS           0x0095
#define UF_UNIT_FIELD_STAT0               0x0096  /* Strength */
#define UF_UNIT_FIELD_STAT1               0x0097  /* Agility */
#define UF_UNIT_FIELD_STAT2               0x0098  /* Stamina */
#define UF_UNIT_FIELD_STAT3               0x0099  /* Intellect */
#define UF_UNIT_FIELD_STAT4               0x009A  /* Spirit */
#define UF_UNIT_FIELD_RESISTANCES         0x009B  /* 7 fields */
#define UF_UNIT_FIELD_BASE_MANA           0x00A2
#define UF_UNIT_FIELD_BASE_HEALTH         0x00A3
#define UF_UNIT_FIELD_BYTES_2             0x00A4
#define UF_UNIT_FIELD_ATTACK_POWER        0x00A5
#define UF_UNIT_FIELD_ATTACK_POWER_MODS   0x00A6
#define UF_UNIT_END                       0x00BB

/* Player fields */
#define UF_PLAYER_DUEL_ARBITER            0x00BB  /* 2 fields */
#define UF_PLAYER_FLAGS                   0x00BD
#define UF_PLAYER_GUILDID                 0x00BE
#define UF_PLAYER_GUILDRANK               0x00BF
#define UF_PLAYER_BYTES                   0x00C0  /* Skin, Face, HairStyle, HairColor */
#define UF_PLAYER_BYTES_2                 0x00C1  /* FacialHair, ?, ?, RestState */
#define UF_PLAYER_BYTES_3                 0x00C2  /* Gender, ? */
#define UF_PLAYER_DUEL_TEAM               0x00C3
#define UF_PLAYER_GUILD_TIMESTAMP         0x00C4
#define UF_PLAYER_END                     0x0501

/* Object type flags */
#define TYPE_OBJECT     0x0001
#define TYPE_ITEM       0x0002
#define TYPE_CONTAINER  0x0004
#define TYPE_UNIT       0x0008
#define TYPE_PLAYER     0x0010
#define TYPE_GAMEOBJECT 0x0020
#define TYPE_DYNAMICOBJECT 0x0040
#define TYPE_CORPSE     0x0080

/* Maximum number of update fields we track */
#define MAX_UPDATE_FIELDS 256

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

#endif /* UPDATE_H */
