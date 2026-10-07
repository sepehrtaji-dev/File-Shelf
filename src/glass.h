#pragma once
#include <windows.h>

// Applies Win11 acrylic backdrop. Call after window creation.
void glass_apply_acrylic(HWND hwnd);

// Win11 rounded corners + dark mode titlebar control.
void glass_apply_round_corners(HWND hwnd, int corner_pref);
void glass_apply_dark_titlebar(HWND hwnd, BOOL dark);

// Corner preference values for DwmSetWindowAttribute
#define SHELF_CORNER_DEFAULT    0
#define SHELF_CORNER_DONOTROUND 1
#define SHELF_CORNER_ROUND      2
#define SHELF_CORNER_ROUNDSMALL 3