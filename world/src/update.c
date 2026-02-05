/*
 * AshEmu - WoW 1.12.1 Server Emulator
 * Copyright (C) 2025 AshEmu Team
 *
 * update.c - Update packet builder
 */

#include "update.h"
#include "opcodes.h"

void update_builder_init(update_builder_t *builder) {
    memset(builder->fields, 0, sizeof(builder->fields));
    memset(builder->field_set, 0, sizeof(builder->field_set));
    builder->max_field = -1;
}

void update_set_guid(update_builder_t *builder, int field, uint64_t value) {
    if (field < 0 || field + 1 >= MAX_UPDATE_FIELDS) return;

    builder->fields[field] = (uint32_t)(value & 0xFFFFFFFF);
    builder->fields[field + 1] = (uint32_t)(value >> 32);
    builder->field_set[field] = true;
    builder->field_set[field + 1] = true;

    if (field + 1 > builder->max_field) {
        builder->max_field = field + 1;
    }
}

void update_set_uint32(update_builder_t *builder, int field, uint32_t value) {
    if (field < 0 || field >= MAX_UPDATE_FIELDS) return;

    builder->fields[field] = value;
    builder->field_set[field] = true;

    if (field > builder->max_field) {
        builder->max_field = field;
    }
}

void update_set_int32(update_builder_t *builder, int field, int32_t value) {
    update_set_uint32(builder, field, (uint32_t)value);
}

void update_set_float(update_builder_t *builder, int field, float value) {
    union { uint32_t i; float f; } u;
    u.f = value;
    update_set_uint32(builder, field, u.i);
}

void update_set_byte(update_builder_t *builder, int field, int byte_index, uint8_t value) {
    if (field < 0 || field >= MAX_UPDATE_FIELDS) return;
    if (byte_index < 0 || byte_index > 3) return;

    uint32_t mask = (uint32_t)(0xFF << (byte_index * 8));
    builder->fields[field] = (builder->fields[field] & ~mask) | ((uint32_t)value << (byte_index * 8));
    builder->field_set[field] = true;

    if (field > builder->max_field) {
        builder->max_field = field;
    }
}

/* Write movement block for UPDATEFLAG_LIVING */
static void write_movement_block(packet_writer_t *packet, const player_t *player) {
    /* Movement flags (uint32) */
    write_uint32(packet, MOVEFLAG_NONE);

    /* Time (uint32) */
    write_uint32(packet, get_tick_count());

    /* Position (4 floats) */
    write_float(packet, player->x);
    write_float(packet, player->y);
    write_float(packet, player->z);
    write_float(packet, player->orientation);

    /* Fall time (uint32) */
    write_uint32(packet, 0);

    /* Speeds (6 values for vanilla 1.12.1) */
    write_float(packet, 2.5f);       /* Walk speed */
    write_float(packet, 7.0f);       /* Run speed */
    write_float(packet, 4.5f);       /* Run back speed */
    write_float(packet, 4.722222f);  /* Swim speed */
    write_float(packet, 2.5f);       /* Swim back speed */
    write_float(packet, 3.141593f);  /* Turn rate (pi rad/sec) */
}

/* Write update mask and field values */
static void write_update_fields(packet_writer_t *packet, const update_builder_t *builder) {
    int max_field = builder->max_field + 1;
    int mask_blocks = (max_field + 31) / 32;

    /* Write block count */
    write_uint8(packet, (uint8_t)mask_blocks);

    /* Build and write mask */
    uint32_t *mask = (uint32_t*)calloc(mask_blocks, sizeof(uint32_t));
    for (int i = 0; i <= builder->max_field; i++) {
        if (builder->field_set[i]) {
            mask[i / 32] |= (1u << (i % 32));
        }
    }

    for (int i = 0; i < mask_blocks; i++) {
        write_uint32(packet, mask[i]);
    }

    /* Write field values in order */
    for (int i = 0; i < max_field; i++) {
        if (builder->field_set[i]) {
            write_uint32(packet, builder->fields[i]);
        }
    }

    free(mask);
}

result_t update_build_create_packet(update_builder_t *builder,
                                            const player_t *player,
                                            bool self,
                                            packet_writer_t *packet) {
    /* Set all required fields */
    update_set_guid(builder, UF_OBJECT_FIELD_GUID, player->guid);
    update_set_uint32(builder, UF_OBJECT_FIELD_TYPE, TYPE_OBJECT | TYPE_UNIT | TYPE_PLAYER);
    update_set_float(builder, UF_OBJECT_FIELD_SCALE_X, 1.0f);

    /* Unit fields */
    update_set_int32(builder, UF_UNIT_FIELD_HEALTH, player_get_health(player));
    update_set_int32(builder, UF_UNIT_FIELD_MAXHEALTH, player_get_max_health(player));
    update_set_int32(builder, UF_UNIT_FIELD_POWER1, player_get_power(player));
    update_set_int32(builder, UF_UNIT_FIELD_MAXPOWER1, player_get_max_power(player));
    update_set_int32(builder, UF_UNIT_FIELD_LEVEL, player->character.level);
    update_set_int32(builder, UF_UNIT_FIELD_FACTIONTEMPLATE, player_get_faction_template(player));

    /* UNIT_FIELD_BYTES_0: race, class, gender, powertype */
    update_set_byte(builder, UF_UNIT_FIELD_BYTES_0, 0, player->character.race);
    update_set_byte(builder, UF_UNIT_FIELD_BYTES_0, 1, player->character.char_class);
    update_set_byte(builder, UF_UNIT_FIELD_BYTES_0, 2, player->character.gender);
    update_set_byte(builder, UF_UNIT_FIELD_BYTES_0, 3, player_get_power_type(player));

    update_set_int32(builder, UF_UNIT_FIELD_DISPLAYID, player_get_display_id(player));
    update_set_int32(builder, UF_UNIT_FIELD_NATIVEDISPLAYID, player_get_display_id(player));

    update_set_float(builder, UF_UNIT_FIELD_BOUNDINGRADIUS, 0.389f);
    update_set_float(builder, UF_UNIT_FIELD_COMBATREACH, 1.5f);

    update_set_float(builder, UF_UNIT_FIELD_MINDAMAGE, 1.0f);
    update_set_float(builder, UF_UNIT_FIELD_MAXDAMAGE, 2.0f);
    update_set_uint32(builder, UF_UNIT_FIELD_BASEATTACKTIME, 2000);
    update_set_uint32(builder, UF_UNIT_FIELD_BASEATTACKTIME + 1, 2000);

    update_set_float(builder, UF_UNIT_MOD_CAST_SPEED, 1.0f);

    /* Base stats */
    update_set_int32(builder, UF_UNIT_FIELD_STAT0, 20);  /* Strength */
    update_set_int32(builder, UF_UNIT_FIELD_STAT1, 20);  /* Agility */
    update_set_int32(builder, UF_UNIT_FIELD_STAT2, 20);  /* Stamina */
    update_set_int32(builder, UF_UNIT_FIELD_STAT3, 20);  /* Intellect */
    update_set_int32(builder, UF_UNIT_FIELD_STAT4, 20);  /* Spirit */

    update_set_int32(builder, UF_UNIT_FIELD_BASE_HEALTH, player_get_max_health(player));
    update_set_int32(builder, UF_UNIT_FIELD_BASE_MANA, player_get_max_power(player));

    /* UNIT_FIELD_BYTES_1: standstate (0=standing) */
    update_set_byte(builder, UF_UNIT_FIELD_BYTES_1, 0, 0);

    /* UNIT_FIELD_BYTES_2 */
    update_set_uint32(builder, UF_UNIT_FIELD_BYTES_2, 0);

    /* Player fields */
    update_set_uint32(builder, UF_PLAYER_FLAGS, 0);

    /* PLAYER_BYTES: skin, face, hairstyle, haircolor */
    update_set_byte(builder, UF_PLAYER_BYTES, 0, player->character.skin);
    update_set_byte(builder, UF_PLAYER_BYTES, 1, player->character.face);
    update_set_byte(builder, UF_PLAYER_BYTES, 2, player->character.hair_style);
    update_set_byte(builder, UF_PLAYER_BYTES, 3, player->character.hair_color);

    /* PLAYER_BYTES_2: facial hair */
    update_set_byte(builder, UF_PLAYER_BYTES_2, 0, player->character.facial_hair);

    /* PLAYER_BYTES_3: gender */
    update_set_byte(builder, UF_PLAYER_BYTES_3, 0, player->character.gender);

    /* Build the packet */

    /* Block count */
    write_uint32(packet, 1);  /* 1 update block */

    /* Has transport */
    write_uint8(packet, 0);

    /* Update type: CREATE_OBJECT2 for players */
    write_uint8(packet, UPDATETYPE_CREATE_OBJECT2);

    /* Packed GUID */
    write_packed_guid(packet, player->guid);

    /* Object type */
    write_uint8(packet, TYPEID_PLAYER);

    /* Update flags:
       Self: 0x61 = SELF(0x01) | LIVING(0x20) | HAS_POSITION(0x40)
       Others: 0x60 = LIVING(0x20) | HAS_POSITION(0x40) */
    uint8_t update_flags = self ? 0x61 : 0x60;
    write_uint8(packet, update_flags);

    /* Movement block */
    write_movement_block(packet, player);

    /* Update mask and values */
    write_update_fields(packet, builder);

    return OK;
}
