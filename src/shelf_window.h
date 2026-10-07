#pragma once
#include <windows.h>

#define WM_SHELF_REFRESH (WM_APP + 2)

#ifdef __cplusplus
extern "C" {
#endif

BOOL shelf_register_class(HINSTANCE hInst);
HWND shelf_create(HINSTANCE hInst, int nCmdShow);
void shelf_on_hotkey(HWND hwnd);

#ifdef __cplusplus
}
#endif