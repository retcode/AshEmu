/*
 * AshEmu - WoW 1.12.1 Server Emulator
 * Copyright (C) 2025 AshEmu Team
 *
 * positions.h - Race starting positions
 */

#ifndef POSITIONS_H
#define POSITIONS_H

#include "common.h"

/* Starting position data */
typedef struct {
    int map;
    float x;
    float y;
    float z;
    float orientation;
} start_position_t;

/* Get starting position for a race (1-8) */
const start_position_t *get_start_position(uint8_t race);

#endif /* POSITIONS_H */
