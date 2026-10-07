#pragma once
#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

void ui_init(void);
void ui_shutdown(void);
void ui_paint(HWND hwnd);
void ui_paint_background(HDC hdc, const RECT* rc);

#ifdef __cplusplus
}
#endif