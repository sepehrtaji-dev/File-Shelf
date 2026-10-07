#pragma once
#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

void ui_init(void);
void ui_shutdown(void);
void ui_set_hover(int hover_idx, int close_hover_idx, int hide_hover);
void ui_paint(HWND hwnd);

#ifdef __cplusplus
}
#endif