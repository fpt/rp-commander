// SPDX-License-Identifier: MIT
// TinyUSB USB descriptors — composite HID: keyboard (ID 1) + mouse (ID 2)

#include "tusb.h"
#include "pico/unique_id.h"

// ── Report IDs ────────────────────────────────────────────────────────────────

#define REPORT_ID_KEYBOARD  1
#define REPORT_ID_MOUSE     2

// ── HID report descriptor ─────────────────────────────────────────────────────

static const uint8_t hid_report_desc[] = {
    TUD_HID_REPORT_DESC_KEYBOARD(HID_REPORT_ID(REPORT_ID_KEYBOARD)),
    TUD_HID_REPORT_DESC_MOUSE(HID_REPORT_ID(REPORT_ID_MOUSE)),
};

// ── Device descriptor ─────────────────────────────────────────────────────────

#define USB_VID  0x2E8A   // Raspberry Pi
#define USB_PID  0x1001   // (unofficial, change for production)
#define USB_BCD  0x0100

tusb_desc_device_t const desc_device = {
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = 0x0200,
    .bDeviceClass       = 0x00,
    .bDeviceSubClass    = 0x00,
    .bDeviceProtocol    = 0x00,
    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor           = USB_VID,
    .idProduct          = USB_PID,
    .bcdDevice          = USB_BCD,
    .iManufacturer      = 0x01,
    .iProduct           = 0x02,
    .iSerialNumber      = 0x03,
    .bNumConfigurations = 0x01,
};

uint8_t const *tud_descriptor_device_cb(void) {
    return (uint8_t const *)&desc_device;
}

// ── Configuration descriptor ──────────────────────────────────────────────────

enum { ITF_HID = 0, ITF_COUNT };

#define CONFIG_TOTAL_LEN  (TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN)
#define EPNUM_HID  0x81

uint8_t const desc_configuration[] = {
    TUD_CONFIG_DESCRIPTOR(1, ITF_COUNT, 0, CONFIG_TOTAL_LEN,
                          TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100),
    TUD_HID_DESCRIPTOR(ITF_HID, 0, HID_ITF_PROTOCOL_NONE,
                       sizeof(hid_report_desc), EPNUM_HID,
                       CFG_TUD_HID_EP_BUFSIZE, 10),
};

uint8_t const *tud_descriptor_configuration_cb(uint8_t index) {
    (void)index;
    return desc_configuration;
}

// ── String descriptors ────────────────────────────────────────────────────────

static char serial_str[17];

static const char *string_desc[] = {
    "\x09\x04",           // 0: Language = English
    "RP2040-Zero",        // 1: Manufacturer
    "Commander",          // 2: Product
    serial_str,           // 3: Serial (filled from flash unique ID)
};

uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
    (void)langid;

    static uint16_t buf[32];
    uint8_t len;

    if (index == 0) {
        buf[1] = 0x0409;
        len = 1;
    } else {
        if (index == 3 && serial_str[0] == '\0') {
            pico_unique_board_id_t id;
            pico_get_unique_board_id(&id);
            for (int i = 0; i < 8; i++) {
                static const char hex[] = "0123456789ABCDEF";
                serial_str[i * 2]     = hex[id.id[i] >> 4];
                serial_str[i * 2 + 1] = hex[id.id[i] & 0xF];
            }
            serial_str[16] = '\0';
        }
        if (index >= sizeof(string_desc) / sizeof(string_desc[0])) return NULL;
        const char *s = string_desc[index];
        len = (uint8_t)strlen(s);
        if (len > 31) len = 31;
        for (uint8_t i = 0; i < len; i++) buf[i + 1] = s[i];
    }

    buf[0] = (uint16_t)((TUSB_DESC_STRING << 8) | (2 * len + 2));
    return buf;
}

// ── HID callbacks (required by TinyUSB) ──────────────────────────────────────

uint8_t const *tud_hid_descriptor_report_cb(uint8_t itf) {
    (void)itf;
    return hid_report_desc;
}

uint16_t tud_hid_get_report_cb(uint8_t itf, uint8_t report_id,
                                hid_report_type_t report_type,
                                uint8_t *buf, uint16_t reqlen) {
    (void)itf; (void)report_id; (void)report_type; (void)buf; (void)reqlen;
    return 0;
}

void tud_hid_set_report_cb(uint8_t itf, uint8_t report_id,
                            hid_report_type_t report_type,
                            uint8_t const *buf, uint16_t bufsize) {
    (void)itf; (void)report_id; (void)report_type; (void)buf; (void)bufsize;
}
