/*
 * AshEmu - WoW 2.4.3 Server Emulator
 * Copyright (C) 2025 AshEmu Team
 *
 * packet.c - Binary packet reader/writer implementation
 */

#include "packet.h"

/* Initial writer capacity */
#define INITIAL_CAPACITY 256

/* Reader functions */

void reader_init(packet_reader_t *reader, const uint8_t *data, size_t size) {
    reader->data = data;
    reader->size = size;
    reader->pos = 0;
}

size_t reader_remaining(const packet_reader_t *reader) {
    return reader->size - reader->pos;
}

void reader_skip(packet_reader_t *reader, size_t count) {
    if (reader->pos + count <= reader->size) {
        reader->pos += count;
    } else {
        reader->pos = reader->size;
    }
}

uint8_t read_uint8(packet_reader_t *reader) {
    if (reader->pos >= reader->size) return 0;
    return reader->data[reader->pos++];
}

uint16_t read_uint16(packet_reader_t *reader) {
    if (reader->pos + 2 > reader->size) return 0;
    uint16_t value = reader->data[reader->pos] |
                     ((uint16_t)reader->data[reader->pos + 1] << 8);
    reader->pos += 2;
    return value;
}

uint32_t read_uint32(packet_reader_t *reader) {
    if (reader->pos + 4 > reader->size) return 0;
    uint32_t value = reader->data[reader->pos] |
                     ((uint32_t)reader->data[reader->pos + 1] << 8) |
                     ((uint32_t)reader->data[reader->pos + 2] << 16) |
                     ((uint32_t)reader->data[reader->pos + 3] << 24);
    reader->pos += 4;
    return value;
}

uint64_t read_uint64(packet_reader_t *reader) {
    if (reader->pos + 8 > reader->size) return 0;
    uint64_t value = 0;
    for (int i = 0; i < 8; i++) {
        value |= ((uint64_t)reader->data[reader->pos + i]) << (i * 8);
    }
    reader->pos += 8;
    return value;
}

float read_float(packet_reader_t *reader) {
    union { uint32_t i; float f; } u;
    u.i = read_uint32(reader);
    return u.f;
}

void read_bytes(packet_reader_t *reader, uint8_t *dst, size_t count) {
    if (reader->pos + count > reader->size) {
        count = reader->size - reader->pos;
    }
    memcpy(dst, reader->data + reader->pos, count);
    reader->pos += count;
}

void read_bytes_reverse(packet_reader_t *reader, uint8_t *dst, size_t count) {
    if (reader->pos + count > reader->size) {
        count = reader->size - reader->pos;
    }
    for (size_t i = 0; i < count; i++) {
        dst[count - 1 - i] = reader->data[reader->pos + i];
    }
    reader->pos += count;
}

size_t read_cstring(packet_reader_t *reader, char *dst, size_t dst_size) {
    size_t len = 0;
    while (reader->pos < reader->size && reader->data[reader->pos] != 0) {
        if (len < dst_size - 1) {
            dst[len] = (char)reader->data[reader->pos];
        }
        len++;
        reader->pos++;
    }
    /* Skip null terminator */
    if (reader->pos < reader->size) {
        reader->pos++;
    }
    /* Null terminate */
    if (dst_size > 0) {
        dst[len < dst_size ? len : dst_size - 1] = '\0';
    }
    return len + 1;
}

uint64_t read_packed_guid(packet_reader_t *reader) {
    uint8_t mask = read_uint8(reader);
    if (mask == 0) return 0;

    uint64_t guid = 0;
    for (int i = 0; i < 8; i++) {
        if (mask & (1 << i)) {
            guid |= (uint64_t)read_uint8(reader) << (i * 8);
        }
    }
    return guid;
}

/* Writer functions */

static result_t writer_ensure_capacity(packet_writer_t *writer, size_t additional) {
    size_t needed = writer->size + additional;
    if (needed <= writer->capacity) return OK;

    if (needed > PACKET_MAX_SIZE) return ERR_BUFFER_OVERFLOW;

    size_t new_capacity = writer->capacity * 2;
    if (new_capacity < needed) new_capacity = needed;
    if (new_capacity > PACKET_MAX_SIZE) new_capacity = PACKET_MAX_SIZE;

    uint8_t *new_data = (uint8_t*)realloc(writer->data, new_capacity);
    if (!new_data) return ERR_MEMORY;

    writer->data = new_data;
    writer->capacity = new_capacity;
    return OK;
}

result_t writer_init(packet_writer_t *writer) {
    writer->data = (uint8_t*)malloc(INITIAL_CAPACITY);
    if (!writer->data) return ERR_MEMORY;
    writer->size = 0;
    writer->capacity = INITIAL_CAPACITY;
    return OK;
}

void writer_free(packet_writer_t *writer) {
    FREE(writer->data);
    writer->size = 0;
    writer->capacity = 0;
}

void writer_reset(packet_writer_t *writer) {
    writer->size = 0;
}

const uint8_t *writer_data(const packet_writer_t *writer) {
    return writer->data;
}

size_t writer_size(const packet_writer_t *writer) {
    return writer->size;
}

result_t write_uint8(packet_writer_t *writer, uint8_t value) {
    result_t r = writer_ensure_capacity(writer, 1);
    if (r != OK) return r;
    writer->data[writer->size++] = value;
    return OK;
}

result_t write_uint16(packet_writer_t *writer, uint16_t value) {
    result_t r = writer_ensure_capacity(writer, 2);
    if (r != OK) return r;
    writer->data[writer->size++] = (uint8_t)(value & 0xFF);
    writer->data[writer->size++] = (uint8_t)((value >> 8) & 0xFF);
    return OK;
}

result_t write_uint32(packet_writer_t *writer, uint32_t value) {
    result_t r = writer_ensure_capacity(writer, 4);
    if (r != OK) return r;
    writer->data[writer->size++] = (uint8_t)(value & 0xFF);
    writer->data[writer->size++] = (uint8_t)((value >> 8) & 0xFF);
    writer->data[writer->size++] = (uint8_t)((value >> 16) & 0xFF);
    writer->data[writer->size++] = (uint8_t)((value >> 24) & 0xFF);
    return OK;
}

result_t write_uint64(packet_writer_t *writer, uint64_t value) {
    result_t r = writer_ensure_capacity(writer, 8);
    if (r != OK) return r;
    for (int i = 0; i < 8; i++) {
        writer->data[writer->size++] = (uint8_t)((value >> (i * 8)) & 0xFF);
    }
    return OK;
}

result_t write_float(packet_writer_t *writer, float value) {
    union { uint32_t i; float f; } u;
    u.f = value;
    return write_uint32(writer, u.i);
}

result_t write_bytes(packet_writer_t *writer, const uint8_t *src, size_t count) {
    result_t r = writer_ensure_capacity(writer, count);
    if (r != OK) return r;
    memcpy(writer->data + writer->size, src, count);
    writer->size += count;
    return OK;
}

result_t write_bytes_reverse(packet_writer_t *writer, const uint8_t *src, size_t count) {
    result_t r = writer_ensure_capacity(writer, count);
    if (r != OK) return r;
    for (size_t i = 0; i < count; i++) {
        writer->data[writer->size++] = src[count - 1 - i];
    }
    return OK;
}

result_t write_cstring(packet_writer_t *writer, const char *str) {
    size_t len = strlen(str);
    result_t r = writer_ensure_capacity(writer, len + 1);
    if (r != OK) return r;
    memcpy(writer->data + writer->size, str, len);
    writer->size += len;
    writer->data[writer->size++] = 0;
    return OK;
}

result_t write_zeros(packet_writer_t *writer, size_t count) {
    result_t r = writer_ensure_capacity(writer, count);
    if (r != OK) return r;
    memset(writer->data + writer->size, 0, count);
    writer->size += count;
    return OK;
}

result_t write_packed_guid(packet_writer_t *writer, uint64_t guid) {
    if (guid == 0) {
        return write_uint8(writer, 0);
    }

    uint8_t mask = 0;
    uint8_t bytes[8];
    int count = 0;

    for (int i = 0; i < 8; i++) {
        uint8_t b = (uint8_t)(guid >> (i * 8));
        if (b != 0) {
            mask |= (1 << i);
            bytes[count++] = b;
        }
    }

    result_t r = write_uint8(writer, mask);
    if (r != OK) return r;

    return write_bytes(writer, bytes, count);
}
