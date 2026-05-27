// SPDX-License-Identifier: MIT
#include "cdc_config.h"
#include "profiles.h"
#include "config_store.h"
#include "tusb.h"
#include "hardware/watchdog.h"
#include "pico/stdlib.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static char    cmd_buf[64];
static int     cmd_len   = 0;
static char    json_buf[20480];  // large enough for profiles + 5 × 32×32 icons in base64

// SET state: how many bytes remain to read for the incoming JSON body
static int     set_remaining = 0;
static int     set_received  = 0;

static void send(const char *s) {
    tud_cdc_write_str(s);
    tud_cdc_write_flush();
}

static void handle_command(const char *cmd) {
    if (strcmp(cmd, "HELLO") == 0) {
        send("rp-commander 1.0\nOK\n");

    } else if (strcmp(cmd, "GET") == 0) {
        int n = profiles_to_json(json_buf, sizeof(json_buf));
        if (n < 0) { send("ERR serialize\nFAIL\n"); return; }
        json_buf[n] = '\n';
        tud_cdc_write(json_buf, (uint32_t)(n + 1));
        send("OK\n");

    } else if (strncmp(cmd, "SET ", 4) == 0) {
        int len = atoi(cmd + 4);
        if (len <= 0 || len >= (int)sizeof(json_buf)) {
            send("ERR bad length\nFAIL\n");
            return;
        }
        set_remaining = len;
        set_received  = 0;

    } else if (strcmp(cmd, "SAVE") == 0) {
        int n = profiles_to_json(json_buf, sizeof(json_buf));
        if (n < 0) { send("ERR serialize\nFAIL\n"); return; }
        if (config_store_save((const uint8_t *)json_buf, (size_t)n) != 0) {
            send("ERR flash\nFAIL\n");
            return;
        }
        send("OK\n");

    } else if (strcmp(cmd, "RESET") == 0) {
        send("OK\n");
        tud_cdc_write_flush();
        watchdog_enable(100, 1);
        while (1) {}

    } else {
        send("ERR unknown\nFAIL\n");
    }
}

void cdc_config_task(void) {
    if (!tud_cdc_connected()) return;

    // Accumulate SET body bytes
    if (set_remaining > 0) {
        uint32_t avail = tud_cdc_available();
        while (avail > 0 && set_remaining > 0) {
            uint32_t want = (uint32_t)set_remaining;
            if (want > avail) want = avail;
            int space = (int)sizeof(json_buf) - set_received - 1;
            if (space <= 0) { send("ERR overflow\nFAIL\n"); set_remaining = 0; return; }
            if ((int)want > space) want = (uint32_t)space;
            uint32_t got = tud_cdc_read((uint8_t *)json_buf + set_received, want);
            set_received  += (int)got;
            set_remaining -= (int)got;
            avail -= got;
        }
        if (set_remaining == 0) {
            json_buf[set_received] = '\0';
            if (profiles_from_json(json_buf) != 0) {
                send("ERR parse\nFAIL\n");
            } else {
                send("OK\n");
            }
            set_received = 0;
        }
        return;
    }

    // Accumulate command line characters
    uint32_t avail = tud_cdc_available();
    while (avail > 0) {
        char ch;
        tud_cdc_read((uint8_t *)&ch, 1);
        avail--;
        if (ch == '\r') continue;
        if (ch == '\n') {
            cmd_buf[cmd_len] = '\0';
            if (cmd_len > 0) handle_command(cmd_buf);
            cmd_len = 0;
        } else {
            if (cmd_len < (int)sizeof(cmd_buf) - 1)
                cmd_buf[cmd_len++] = ch;
        }
    }
}
