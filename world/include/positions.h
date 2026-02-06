/*
 * AshEmu - WoW 2.4.3 Server Emulator (TBC)
 * Copyright (C) 2025 AshEmu Team
 *
 * positions.h - Race starting positions
 */

#ifndef POSITIONS_H
#define POSITIONS_H

#include "common.h"

/* Number of races in TBC (1-8 vanilla + 10-11 TBC, 9 unused) */
#define NUM_RACES 11

/* Starting position data */
typedef struct {
    int map;
    int zone_id;
    int area_id;
    float x;
    float y;
    float z;
    float orientation;
} start_position_t;

/* Get starting position for a race (1-8, 10-11) */
const start_position_t *get_start_position(uint8_t race);

#endif /* POSITIONS_H */
