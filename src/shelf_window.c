#include "shelf_window.h"
#include "ui_paint.h"
#include "glass.h"
#include "drop_in.h"
#include "shelf_items.h"

#define SHELF_CLASS L"ShelfWindowClass"
#define HOTKEY_ID   1
#define WM_TRAY     (WM_APP + 1)

// Implemented in main.c
extern void tray_menu_public(HWND hwnd);

static LRESULT CALLBACK shelf_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {

    case WM_CREATE:
        glass_apply_dark_titlebar(hwnd, TRUE);
        glass_apply_round_corners(hwnd, SHELF_CORNER_ROUND);
        glass_apply_acrylic(hwnd);
        return 0;

    case WM_PAINT:
        ui_paint(hwnd);
        return 0;

    case WM_ERASEBKGND:
        return 1;

    case WM_HOTKEY:
        if (wp == HOTKEY_ID) shelf_on_hotkey(hwnd);
        return 0;

    case WM_DROPFILES:
        drop_in_handle((HDROP)wp);
        InvalidateRect(hwnd, NULL, TRUE);
        return 0;

    case WM_TRAY:
        if (LOWORD(lp) == WM_RBUTTONUP || LOWORD(lp) == WM_CONTEXTMENU) {
            tray_menu_public(hwnd);
        } else if (LOWORD(lp) == WM_LBUTTONUP) {
            shelf_on_hotkey(hwnd);
        }
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

BOOL shelf_register_class(HINSTANCE hInst) {
    WNDCLASSEXW wc = {0};
    wc.cbSize        = sizeof(wc);
    wc.style         = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wc.lpfnWndProc   = shelf_proc;
    wc.hInstance     = hInst;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszClassName = SHELF_CLASS;
    wc.hIcon         = LoadIconW(hInst, MAKEINTRESOURCEW(1));
    wc.hIconSm       = wc.hIcon;
    return RegisterClassExW(&wc) != 0;
}

HWND shelf_create(HINSTANCE hInst, int nCmdShow) {
    HWND hwnd = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        SHELF_CLASS, L"Shelf",
        WS_POPUP | WS_THICKFRAME | WS_CLIPCHILDREN,
        CW_USEDEFAULT, CW_USEDEFAULT, 320, 420,
        NULL, NULL, hInst, NULL);

    if (!hwnd) return NULL;
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);
    return hwnd;
}

void shelf_on_hotkey(HWND hwnd) {
    if (IsWindowVisible(hwnd)) {
        ShowWindow(hwnd, SW_HIDE);
    } else {
        ShowWindow(hwnd, SW_SHOW);
        SetForegroundWindow(hwnd);
    }
}