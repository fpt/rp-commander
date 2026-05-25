// SPDX-License-Identifier: MIT
#include "config_store.h"
#include "hardware/flash.h"
#include "hardware/sync.h"
#include "pico/stdlib.h"
#include <string.h>

#define MAGIC         0x52504346u   // "RPCF"
#define CONFIG_OFFSET (PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE)

// Header: magic(4) + crc32(4) + len(4) = 12 bytes
#define HEADER_SIZE   12
#define MAX_DATA_SIZE (FLASH_SECTOR_SIZE - HEADER_SIZE)

static uint32_t crc32(const uint8_t *data, size_t len) {
    uint32_t crc = 0xFFFFFFFFu;
    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (int b = 0; b < 8; b++)
            crc = (crc >> 1) ^ (crc & 1 ? 0xEDB88320u : 0u);
    }
    return ~crc;
}

int config_store_load(char *out_json, size_t out_size) {
    const uint8_t *flash = (const uint8_t *)(XIP_BASE + CONFIG_OFFSET);

    uint32_t magic, stored_crc, len;
    memcpy(&magic,      flash,      4);
    memcpy(&stored_crc, flash + 4,  4);
    memcpy(&len,        flash + 8,  4);

    if (magic != MAGIC) return -1;
    if (len == 0 || len > MAX_DATA_SIZE) return -1;

    const uint8_t *data = flash + HEADER_SIZE;
    if (crc32(data, len) != stored_crc) return -1;

    size_t copy = len < out_size ? len : out_size - 1;
    memcpy(out_json, data, copy);
    out_json[copy] = '\0';
    return (int)copy;
}

int config_store_save(const uint8_t *data, size_t len) {
    if (len == 0 || len > MAX_DATA_SIZE) return -1;

    static uint8_t page[FLASH_SECTOR_SIZE];
    memset(page, 0xFF, sizeof(page));

    uint32_t magic = MAGIC;
    uint32_t crc   = crc32(data, len);
    uint32_t dlen  = (uint32_t)len;

    memcpy(page,      &magic, 4);
    memcpy(page + 4,  &crc,   4);
    memcpy(page + 8,  &dlen,  4);
    memcpy(page + HEADER_SIZE, data, len);

    uint32_t save = save_and_disable_interrupts();
    flash_range_erase(CONFIG_OFFSET, FLASH_SECTOR_SIZE);
    flash_range_program(CONFIG_OFFSET, page, FLASH_SECTOR_SIZE);
    restore_interrupts(save);

    return 0;
}
