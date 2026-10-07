#pragma once
#include <windows.h>

#define SHELF_PAD_TOP     38
#define SHELF_PAD_LEFT    12
#define SHELF_PAD_RIGHT   12
#define SHELF_PAD_BOTTOM  12
#define SHELF_ROW_H       36
#define SHELF_ROW_GAP     4
#define SHELF_ROUND       10

static inline RECT shelf_row_rect(RECT rc, int i) {
    RECT r;
    r.left   = rc.left   + SHELF_PAD_LEFT;
    r.right  = rc.right  - SHELF_PAD_RIGHT;
    r.top    = rc.top    + SHELF_PAD_TOP + i * (SHELF_ROW_H + SHELF_ROW_GAP);
    r.bottom = r.top + SHELF_ROW_H;
    return r;
}

static inline RECT shelf_hide_button_rect(RECT rc) {
    RECT r;
    r.right  = rc.right - 8;
    r.left   = r.right - 22;
    r.top    = 8;
    r.bottom = r.top + 22;
    return r;
}