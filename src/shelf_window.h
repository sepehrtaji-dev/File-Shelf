#pragma once
#include <windows.h>

BOOL shelf_register_class(HINSTANCE hInst);
HWND shelf_create(HINSTANCE hInst, int nCmdShow);
void shelf_on_hotkey(HWND hwnd);