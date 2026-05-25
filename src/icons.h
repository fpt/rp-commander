// SPDX-License-Identifier: MIT
// App icon draw functions — renders to RGB565 framebuffer at (x, y), size×size px

#ifndef _ICONS_H_
#define _ICONS_H_

#include <stdint.h>
#include <stdbool.h>

// Draw an app icon at pixel (x,y), size pixels square.
// selected: draw with highlight (white border), else dim.
void icon_draw(uint16_t *fb, int x, int y, int size, int icon_idx, bool selected);

// Animated Claude mascot (frame 0-7, ~120 ms per frame → ~1 Hz bounce).
void icon_claude_anim(uint16_t *fb, int x, int y, int sz, bool sel, int frame);

#endif // _ICONS_H_
