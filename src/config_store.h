// SPDX-License-Identifier: MIT
#pragma once
#include <stdint.h>
#include <stddef.h>

// Load config JSON from flash. Returns length on success, -1 if no valid config.
int config_store_load(char *out_json, size_t out_size);

// Save config JSON to flash. Returns 0 on success, -1 on error.
int config_store_save(const uint8_t *data, size_t len);
