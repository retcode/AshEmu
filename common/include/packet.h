/*
 * AshEmu - WoW 1.12.1 Server Emulator
 * Copyright (C) 2025 AshEmu Team
 *
 * packet.h - Binary packet reader/writer with packed GUID support
 */

#ifndef PACKET_H
#define PACKET_H

#include "common.h"

/* Maximum packet size */
#define PACKET_MAX_SIZE 65536

/* Packet reader structure */
typedef struct {
    const uint8_t *data;
    size_t size;
    size_t pos;
} packet_reader_t;

/* Packet writer structure */
typedef struct {
    uint8_t *data;
    size_t size;
    size_t capacity;
} packet_writer_t;

/* Reader functions */
void reader_init(packet_reader_t *reader, const uint8_t *data, size_t size);
size_t reader_remaining(const packet_reader_t *reader);
void reader_skip(packet_reader_t *reader, size_t count);

uint8_t read_uint8(packet_reader_t *reader);
uint16_t read_uint16(packet_reader_t *reader);
uint32_t read_uint32(packet_reader_t *reader);
uint64_t read_uint64(packet_reader_t *reader);
float read_float(packet_reader_t *reader);
void read_bytes(packet_reader_t *reader, uint8_t *dst, size_t count);
void read_bytes_reverse(packet_reader_t *reader, uint8_t *dst, size_t count);

/* Read null-terminated string, returns length written (including null) */
size_t read_cstring(packet_reader_t *reader, char *dst, size_t dst_size);

/* Read variable-length packed GUID */
uint64_t read_packed_guid(packet_reader_t *reader);

/* Writer functions */
result_t writer_init(packet_writer_t *writer);
void writer_free(packet_writer_t *writer);
void writer_reset(packet_writer_t *writer);
const uint8_t *writer_data(const packet_writer_t *writer);
size_t writer_size(const packet_writer_t *writer);

result_t write_uint8(packet_writer_t *writer, uint8_t value);
result_t write_uint16(packet_writer_t *writer, uint16_t value);
result_t write_uint32(packet_writer_t *writer, uint32_t value);
result_t write_uint64(packet_writer_t *writer, uint64_t value);
result_t write_float(packet_writer_t *writer, float value);
result_t write_bytes(packet_writer_t *writer, const uint8_t *src, size_t count);
result_t write_bytes_reverse(packet_writer_t *writer, const uint8_t *src, size_t count);

/* Write null-terminated string */
result_t write_cstring(packet_writer_t *writer, const char *str);

/* Write N zero bytes */
result_t write_zeros(packet_writer_t *writer, size_t count);

/* Write variable-length packed GUID */
result_t write_packed_guid(packet_writer_t *writer, uint64_t guid);

#endif /* PACKET_H */
