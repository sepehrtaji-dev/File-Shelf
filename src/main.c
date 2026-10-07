#include <windows.h>
#include <shellapi.h>
#include <objbase.h>
#include "shelf_window.h"
#include "shelf_items.h"
#include "util.h"
#include "ui_paint.h"

#define HOTKEY_ID   1
#define TRAY_ID     1
#define WM_TRAY     (WM_APP + 1)

static HWND g_hwnd = NULL;
static NOTIFYICONDATAW g_nid = {0};

static void tray_add(HWND hwnd) {
    g_nid.cbSize = sizeof(g_nid);
    g_nid.hWnd   = hwnd;
    g_nid.uID    = TRAY_ID;
    g_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    g_nid.uCallbackMessage = WM_TRAY;
    g_nid.hIcon  = LoadIconW(GetModuleHandleW(NULL), MAKEINTRESOURCEW(1));
    if (!g_nid.hIcon) g_nid.hIcon = LoadIconW(NULL, IDI_APPLICATION);
    wcsncpy_s(g_nid.szTip, ARRAYSIZE(g_nid.szTip), L"Shelf", _TRUNCATE);
    Shell_NotifyIconW(NIM_ADD, &g_nid);
}

static void tray_remove(void) {
    Shell_NotifyIconW(NIM_DELETE, &g_nid);
}

void tray_menu_public(HWND hwnd) {
    HMENU m = CreatePopupMenu();
    AppendMenuW(m, MF_STRING, 1, L"Show / Hide");
    AppendMenuW(m, MF_STRING, 2, L"Clear shelf");
    AppendMenuW(m, MF_SEPARATOR, 0, NULL);
    AppendMenuW(m, MF_STRING, 3, L"Exit");

    POINT pt; GetCursorPos(&pt);
    SetForegroundWindow(hwnd);
    int cmd = TrackPopupMenu(m, TPM_RETURNCMD | TPM_RIGHTBUTTON,
                             pt.x, pt.y, 0, hwnd, NULL);
    DestroyMenu(m);

    switch (cmd) {
    case 1:
        shelf_on_hotkey(hwnd);
        break;
    case 2:
        items_clear();
        items_save();
        PostMessageW(hwnd, WM_SHELF_REFRESH, 0, 0);
        break;
    case 3:
        DestroyWindow(hwnd);
        break;
    }
}

int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE hPrev, PWSTR cmd, int show) {
    (void)hPrev; (void)cmd;
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    OleInitialize(NULL);

    util_init_paths();
    items_init();
    items_load();
    ui_init();

    if (!shelf_register_class(hInst)) return 1;

    g_hwnd = shelf_create(hInst, show);
    if (!g_hwnd) return 1;

    DragAcceptFiles(g_hwnd, TRUE);
    tray_add(g_hwnd);

    if (!RegisterHotKey(g_hwnd, HOTKEY_ID, MOD_CONTROL | MOD_ALT | MOD_SHIFT, 'S'))
        MessageBoxW(NULL, L"Could not register Ctrl+Alt+Shift+S", L"Shelf", MB_ICONWARNING);

    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    UnregisterHotKey(g_hwnd, HOTKEY_ID);
    tray_remove();
    items_save();
    ui_shutdown();
    OleUninitialize();
    return 0;
}