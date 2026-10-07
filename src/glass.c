#include "glass.h"
#include <dwmapi.h>

// Undocumented Win11 backdrop attribute
#ifndef DWMWA_SYSTEMBACKDROP_TYPE
#define DWMWA_SYSTEMBACKDROP_TYPE 38
#endif
#ifndef DWMWA_WINDOW_CORNER_PREFERENCE
#define DWMWA_WINDOW_CORNER_PREFERENCE 33
#endif
#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif

#define DWMSBT_TRANSIENTWINDOW 3   // Acrylic
#define DWMSBT_MAINWINDOW      2   // Mica

void glass_apply_acrylic(HWND hwnd) {
    int backdrop = DWMSBT_TRANSIENTWINDOW;
    DwmSetWindowAttribute(hwnd, DWMWA_SYSTEMBACKDROP_TYPE, &backdrop, sizeof(backdrop));

    // Extend DWM frame so the acrylic fills the client area.
    MARGINS m = { -1, -1, -1, -1 };
    DwmExtendFrameIntoClientArea(hwnd, &m);
}

void glass_apply_round_corners(HWND hwnd, int corner_pref) {
    DwmSetWindowAttribute(hwnd, DWMWA_WINDOW_CORNER_PREFERENCE,
                          &corner_pref, sizeof(corner_pref));
}

void glass_apply_dark_titlebar(HWND hwnd, BOOL dark) {
    BOOL v = dark;
    DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &v, sizeof(v));
}