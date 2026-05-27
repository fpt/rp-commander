// SPDX-License-Identifier: MIT
// App and profile definitions for ardu-commander
//
// Platform: macOS (modifier key = LEFTGUI = Command).
// For Windows: replace MOD_META with KEYBOARD_MODIFIER_LEFTCTRL.

#include "profiles.h"
#include "icons.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// Modifier shortcuts
#define _C   KEYBOARD_MODIFIER_LEFTCTRL
#define _S   KEYBOARD_MODIFIER_LEFTSHIFT
#define _A   KEYBOARD_MODIFIER_LEFTALT
#define _G   KEYBOARD_MODIFIER_LEFTGUI    // Cmd on Mac

// Action constructors
#define NONE_ACT          {ACT_NONE, 0, 0, 0, "---"}
#define KEY(m,k,l)        {ACT_KEY,  (m), HID_KEY_##k, 0, (l)}
#define SCR(d,l)          {ACT_SCROLL, 0, 0, (d), (l)}

// ── Profiles (one per app) ────────────────────────────────────────────────────
// Order determines ENC1 cycle sequence.

profile_t g_profiles[] = {

    // ── Chrome ────────────────────────────────────────────────────────────────
    {
        .icon_idx   = ICON_CHROME,
        .name       = "Chrome",
        .enc2_cw    = SCR(+1, "Up"),
        .enc2_ccw   = SCR(-1, "Down"),
        .enc2_label = "Scroll",
        .btn_x      = KEY(_G|_A, ARROW_LEFT,   "<Tab"),
        .btn_y      = KEY(_G|_A, ARROW_RIGHT,  "Tab>"),
        .btn_a      = KEY(_G,   BRACKET_LEFT, "Back"),
        .btn_b      = KEY(_G,   BRACKET_RIGHT,"Fwd"),
        .btn_c      = KEY(_G,   R,            "Reload"),
        .btn_d      = KEY(_G,   T,            "New Tab"),
    },

    // ── Claude ────────────────────────────────────────────────────────────────
    {
        .icon_idx   = ICON_CLAUDE,
        .name       = "Claude",
        .enc2_cw    = SCR(+1, "Up"),
        .enc2_ccw   = SCR(-1, "Down"),
        .enc2_label = "Scroll",
        .btn_x      = KEY(_S,   TAB,    "S+Tab"),
        .btn_y      = KEY(0,    RETURN, "Enter"),
        .btn_a      = KEY(0,    1,      "1"),
        .btn_b      = KEY(0,    2,      "2"),
        .btn_c      = KEY(0,    3,      "3"),
        .btn_d      = KEY(0,    4,      "4"),
    },

    // ── Fusion 360 ────────────────────────────────────────────────────────────
    {
        .icon_idx   = ICON_FUSION,
        .name       = "Fusion",
        .enc2_cw    = SCR(-1, "Zoom+"),
        .enc2_ccw   = SCR(+1, "Zoom-"),
        .enc2_label = "Zoom",
        .btn_x      = KEY(0,     F,  "Fit"),
        .btn_y      = KEY(0,     S,  "Sketch"),
        .btn_a      = KEY(_G,    Z,  "Undo"),
        .btn_b      = KEY(_G|_S, Z,  "Redo"),
        .btn_c      = KEY(0,     E,  "Extrude"),
        .btn_d      = KEY(0,     L,  "Line"),
    },

    // ── KiCad ─────────────────────────────────────────────────────────────────
    {
        .icon_idx   = ICON_KICAD,
        .name       = "KiCad",
        .enc2_cw    = SCR(-1, "Zoom+"),
        .enc2_ccw   = SCR(+1, "Zoom-"),
        .enc2_label = "Zoom",
        .btn_x      = {ACT_KEY, _C, HID_KEY_0, 0, "Fit"},
        .btn_y      = KEY(0,   W, "Wire"),
        .btn_a      = KEY(_C,  Z, "Undo"),
        .btn_b      = KEY(_C,  Y, "Redo"),
        .btn_c      = KEY(0,   L, "Label"),
        .btn_d      = KEY(0,   A, "Add"),
    },

    // ── Krita ─────────────────────────────────────────────────────────────────
    {
        .icon_idx   = ICON_KRITA,
        .name       = "Krita",
        .enc2_cw    = KEY(0, BRACKET_RIGHT, "Size+"),
        .enc2_ccw   = KEY(0, BRACKET_LEFT,  "Size-"),
        .enc2_label = "Size",
        .btn_x      = KEY(0,     E,   "Eraser"),
        .btn_y      = KEY(0,     K,   "Eyedrop"),
        .btn_a      = KEY(_C,    Z,   "Undo"),
        .btn_b      = KEY(_C|_S, Z,   "Redo"),
        .btn_c      = KEY(0,     F,   "Fill"),
        .btn_d      = KEY(0,     TAB, "Canvas"),
    },
};

const int g_num_profiles = (int)(sizeof(g_profiles) / sizeof(g_profiles[0]));

// ── Base64 encode/decode ──────────────────────────────────────────────────────

static const char B64ENC[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static int b64_encode(const uint8_t *src, size_t srclen, char *dst, size_t dstsize) {
    size_t out = 0;
    for (size_t i = 0; i < srclen; i += 3) {
        uint32_t v = (uint32_t)src[i] << 16;
        int rem = (int)(srclen - i);
        if (rem > 1) v |= (uint32_t)src[i+1] << 8;
        if (rem > 2) v |= src[i+2];
        if (out + 5 >= dstsize) return -1;
        dst[out++] = B64ENC[(v >> 18) & 63];
        dst[out++] = B64ENC[(v >> 12) & 63];
        dst[out++] = rem > 1 ? B64ENC[(v >>  6) & 63] : '=';
        dst[out++] = rem > 2 ? B64ENC[ v        & 63] : '=';
    }
    dst[out] = '\0';
    return (int)out;
}

static int b64_decode(const char *src, size_t srclen, uint8_t *dst, size_t dstsize) {
    size_t out = 0;
    for (size_t i = 0; i + 1 < srclen; i += 4) {
        uint8_t c[4] = {0, 0, 0, 0};
        int n = 0;
        for (int j = 0; j < 4 && i + (size_t)j < srclen; j++) {
            char ch = src[i + j];
            if (ch == '=') break;
            uint8_t v;
            if      (ch >= 'A' && ch <= 'Z') v = (uint8_t)(ch - 'A');
            else if (ch >= 'a' && ch <= 'z') v = (uint8_t)(ch - 'a' + 26);
            else if (ch >= '0' && ch <= '9') v = (uint8_t)(ch - '0' + 52);
            else if (ch == '+') v = 62;
            else if (ch == '/') v = 63;
            else break;
            c[n++] = v;
        }
        if (n < 2) break;
        if (out < dstsize) dst[out++] = (uint8_t)((c[0] << 2) | (c[1] >> 4));
        if (n >= 3 && out < dstsize) dst[out++] = (uint8_t)((c[1] << 4) | (c[2] >> 2));
        if (n >= 4 && out < dstsize) dst[out++] = (uint8_t)((c[2] << 6) |  c[3]);
    }
    return (int)out;
}

static const char *jfind_b64(const char *json, const char *key, size_t *out_len) {
    char pat[32];
    snprintf(pat, sizeof(pat), "\"%s\":\"", key);
    const char *p = strstr(json, pat);
    if (!p) { *out_len = 0; return NULL; }
    p += strlen(pat);
    const char *start = p;
    while (*p && *p != '"') p++;
    *out_len = (size_t)(p - start);
    return start;
}

// ── JSON serialization ────────────────────────────────────────────────────────

static int action_to_json(char *buf, size_t sz, const action_t *a) {
    if (a->type == ACT_NONE)
        return snprintf(buf, sz, "{\"t\":0}");
    if (a->type == ACT_KEY)
        return snprintf(buf, sz, "{\"t\":1,\"m\":%u,\"k\":%u,\"l\":\"%s\"}",
                        a->mod, a->key, a->label);
    // ACT_SCROLL
    return snprintf(buf, sz, "{\"t\":2,\"d\":%d,\"l\":\"%s\"}",
                    (int)a->scroll, a->label);
}

int profiles_to_json(char *buf, size_t bufsize) {
    int pos = 0;
    int r;
#define APPEND(fmt, ...) do { \
    r = snprintf(buf + pos, bufsize - (size_t)pos, fmt, ##__VA_ARGS__); \
    if (r < 0) return -1; pos += r; } while (0)

    APPEND("{\"v\":1,\"profiles\":[");
    for (int i = 0; i < g_num_profiles; i++) {
        const profile_t *p = &g_profiles[i];
        if (i > 0) APPEND(",");
        APPEND("{\"icon\":%u,\"name\":\"%s\",\"el\":\"%s\",",
               p->icon_idx, p->name, p->enc2_label);
        APPEND("\"ew\":");
        pos += action_to_json(buf + pos, bufsize - (size_t)pos, &p->enc2_cw);
        APPEND(",\"eccw\":");
        pos += action_to_json(buf + pos, bufsize - (size_t)pos, &p->enc2_ccw);
        APPEND(",\"x\":");
        pos += action_to_json(buf + pos, bufsize - (size_t)pos, &p->btn_x);
        APPEND(",\"y\":");
        pos += action_to_json(buf + pos, bufsize - (size_t)pos, &p->btn_y);
        APPEND(",\"a\":");
        pos += action_to_json(buf + pos, bufsize - (size_t)pos, &p->btn_a);
        APPEND(",\"b\":");
        pos += action_to_json(buf + pos, bufsize - (size_t)pos, &p->btn_b);
        APPEND(",\"c\":");
        pos += action_to_json(buf + pos, bufsize - (size_t)pos, &p->btn_c);
        APPEND(",\"d\":");
        pos += action_to_json(buf + pos, bufsize - (size_t)pos, &p->btn_d);
        APPEND("}");
    }
    APPEND("]");

    // Append runtime icons as base64-encoded RGB565 byte arrays
    static const char *icon_names[] = {"chrome","claude","fusion","kicad","krita"};
    int first_icon = 1;
    for (int i = 0; i < 5; i++) {
        const uint16_t *idata = icon_get_runtime(i);
        if (!idata) continue;
        if (first_icon) { APPEND(",\"icons\":{"); first_icon = 0; }
        else            { APPEND(","); }
        APPEND("\"%s\":\"", icon_names[i]);
        int n = b64_encode((const uint8_t *)idata,
                           ICON_RT_SIZE * ICON_RT_SIZE * sizeof(uint16_t),
                           buf + pos, bufsize - (size_t)pos);
        if (n < 0) return -1;
        pos += n;
        APPEND("\"");
    }
    if (!first_icon) APPEND("}");

    APPEND("}");
#undef APPEND
    return pos;
}

// ── JSON parsing ──────────────────────────────────────────────────────────────

// Finds "key":N and returns N, or def if not found.
static int jfind_int(const char *json, const char *key, int def) {
    char pat[32];
    snprintf(pat, sizeof(pat), "\"%s\":", key);
    const char *p = strstr(json, pat);
    if (!p) return def;
    p += strlen(pat);
    while (*p == ' ') p++;
    int v = def;
    sscanf(p, "%d", &v);
    return v;
}

// Finds "key":"value" and copies value into buf (null-terminated).
static void jfind_str(const char *json, const char *key, char *buf, size_t bufsize) {
    char pat[32];
    snprintf(pat, sizeof(pat), "\"%s\":\"", key);
    const char *p = strstr(json, pat);
    if (!p) { buf[0] = '\0'; return; }
    p += strlen(pat);
    size_t i = 0;
    while (*p && *p != '"' && i + 1 < bufsize) buf[i++] = *p++;
    buf[i] = '\0';
}

// Finds "key":{...} and returns pointer into a static buffer holding the object substring.
static const char *jfind_obj(const char *json, const char *key) {
    static char objbuf[256];
    char pat[32];
    snprintf(pat, sizeof(pat), "\"%s\":{", key);
    const char *p = strstr(json, pat);
    if (!p) return NULL;
    p += strlen(pat) - 1; // points at '{'
    int depth = 0;
    const char *start = p;
    const char *end   = p;
    while (*p) {
        if (*p == '{') depth++;
        else if (*p == '}') { depth--; if (depth == 0) { end = p; break; } }
        p++;
    }
    size_t len = (size_t)(end - start + 1);
    if (len >= sizeof(objbuf)) len = sizeof(objbuf) - 1;
    memcpy(objbuf, start, len);
    objbuf[len] = '\0';
    return objbuf;
}

static void parse_action(const char *obj, action_t *a) {
    if (!obj) { a->type = ACT_NONE; return; }
    int t = jfind_int(obj, "t", 0);
    a->type = (act_type_t)t;
    if (t == 1) {
        a->mod    = (uint8_t)jfind_int(obj, "m", 0);
        a->key    = (uint8_t)jfind_int(obj, "k", 0);
        a->scroll = 0;
        jfind_str(obj, "l", a->label, sizeof(a->label));
    } else if (t == 2) {
        a->mod    = 0;
        a->key    = 0;
        a->scroll = (int8_t)jfind_int(obj, "d", 0);
        jfind_str(obj, "l", a->label, sizeof(a->label));
    } else {
        a->mod = 0; a->key = 0; a->scroll = 0;
        snprintf(a->label, sizeof(a->label), "---");
    }
}

int profiles_from_json(const char *json) {
    if (!json) return -1;
    const char *arr = strstr(json, "\"profiles\":[");
    if (!arr) return -1;
    arr += strlen("\"profiles\":[");

    int idx = 0;
    while (idx < g_num_profiles) {
        // Skip to next '{'
        while (*arr && *arr != '{' && *arr != ']') arr++;
        if (!*arr || *arr == ']') break;

        // Extract the profile object
        const char *obj_start = arr;
        int depth = 0;
        while (*arr) {
            if (*arr == '{') depth++;
            else if (*arr == '}') { depth--; if (depth == 0) { arr++; break; } }
            arr++;
        }
        // arr now points just past the closing '}'
        size_t obj_len = (size_t)(arr - obj_start);

        static char pbuf[512];
        if (obj_len >= sizeof(pbuf)) obj_len = sizeof(pbuf) - 1;
        memcpy(pbuf, obj_start, obj_len);
        pbuf[obj_len] = '\0';

        profile_t *p = &g_profiles[idx];
        p->icon_idx = (uint8_t)jfind_int(pbuf, "icon", p->icon_idx);
        jfind_str(pbuf, "name", p->name, sizeof(p->name));
        jfind_str(pbuf, "el",   p->enc2_label, sizeof(p->enc2_label));

        parse_action(jfind_obj(pbuf, "ew"),   &p->enc2_cw);
        parse_action(jfind_obj(pbuf, "eccw"), &p->enc2_ccw);
        parse_action(jfind_obj(pbuf, "x"),    &p->btn_x);
        parse_action(jfind_obj(pbuf, "y"),    &p->btn_y);
        parse_action(jfind_obj(pbuf, "a"),    &p->btn_a);
        parse_action(jfind_obj(pbuf, "b"),    &p->btn_b);
        parse_action(jfind_obj(pbuf, "c"),    &p->btn_c);
        parse_action(jfind_obj(pbuf, "d"),    &p->btn_d);

        idx++;
    }

    // Parse runtime icons (optional "icons" object)
    static const char *icon_names[] = {"chrome","claude","fusion","kicad","krita"};
    static uint8_t icon_decode_buf[ICON_RT_SIZE * ICON_RT_SIZE * sizeof(uint16_t)];
    const char *icons_sec = strstr(json, "\"icons\":{");
    if (icons_sec) {
        icons_sec += strlen("\"icons\":{") - 1;
        // Find end of icons object
        int depth = 0;
        const char *p = icons_sec;
        while (*p) {
            if (*p == '{') depth++;
            else if (*p == '}') { if (--depth == 0) break; }
            p++;
        }
        // Null-terminate a local copy for searching within
        size_t olen = (size_t)(p - icons_sec + 1);
        static char icons_obj[16384];
        if (olen < sizeof(icons_obj)) {
            memcpy(icons_obj, icons_sec, olen);
            icons_obj[olen] = '\0';
            for (int i = 0; i < 5; i++) {
                size_t b64len;
                const char *b64 = jfind_b64(icons_obj, icon_names[i], &b64len);
                if (!b64 || b64len == 0) continue;
                int dec = b64_decode(b64, b64len,
                                     icon_decode_buf, sizeof(icon_decode_buf));
                if (dec == ICON_RT_SIZE * ICON_RT_SIZE * (int)sizeof(uint16_t))
                    icon_set_runtime(i, (const uint16_t *)icon_decode_buf);
            }
        }
    }

    return 0;
}
