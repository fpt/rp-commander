// SPDX-License-Identifier: MIT
// TinyUSB configuration for ardu-commander (RP2040, HID keyboard + mouse)

#ifndef _TUSB_CONFIG_H_
#define _TUSB_CONFIG_H_

#define CFG_TUSB_MCU  OPT_MCU_RP2040
#define CFG_TUSB_OS   OPT_OS_PICO

// Full-speed device on USB port 0 (native RP2040 USB)
#define CFG_TUSB_RHPORT0_MODE (OPT_MODE_DEVICE | OPT_MODE_FULL_SPEED)

// Composite HID + CDC device
#define CFG_TUD_HID     1
#define CFG_TUD_CDC     1
#define CFG_TUD_MSC     0
#define CFG_TUD_MIDI    0
#define CFG_TUD_VENDOR  0

#define CFG_TUD_CDC_RX_BUFSIZE  256
#define CFG_TUD_CDC_TX_BUFSIZE  512

#define CFG_TUD_HID_EP_BUFSIZE  16

// RP2040 native USB is on port 0
#ifndef BOARD_TUD_RHPORT
#define BOARD_TUD_RHPORT  0
#endif

#endif // _TUSB_CONFIG_H_
