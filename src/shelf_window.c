#include "shelf_window.h"
#include <windowsx.h>
#include "ui_paint.h"
#include "glass.h"
#include "drop_in.h"
#include "shelf_items.h"
#include "shelf_layout.h"
#include "drag_out.h"
#include "util.h"

#define SHELF_CLASS L"ShelfWindowClass"
#define HOTKEY_ID   1
#define WM_TRAY     (WM_APP + 1)

extern void tray_menu_public(HWND hwnd);

static int   g_hover = -1;
static int   g_close_hover = -1;
static int   g_hide_hover = 0;
static BOOL  g_tracking = FALSE;
static BOOL  g_down_valid = FALSE;
static BOOL  g_dragging = FALSE;
static POINT g_down_pt = {0, 0};

static int hit_row(HWND hwnd, POINT pt, BOOL* on_close) {
    RECT rc; GetClientRect(hwnd, &rc);
    int n = items_count();
    for (int i = 0; i < n; ++i) {
        RECT rr = shelf_row_rect(rc, i);
        if (PtInRect(&rr, pt)) {
            if (on_close) {
                RECT close_zone = rr;
                close_zone.left = close_zone.right - 40;
                *on_close = PtInRect(&close_zone, pt);
            }
            return i;
        }
    }
    if (on_close) *on_close = FALSE;
    return -1;
}

static BOOL hit_hide(HWND hwnd, POINT pt) {
    RECT rc; GetClientRect(hwnd, &rc);
    RECT r = shelf_hide_button_rect(rc);
    return PtInRect(&r, pt) ? TRUE : FALSE;
}

static void redraw(HWND hwnd) {
    InvalidateRect(hwnd, NULL, FALSE);
    UpdateWindow(hwnd);
}

static void update_hover(HWND hwnd, POINT pt) {
    BOOL on_close = FALSE;
    int idx = hit_row(hwnd, pt, &on_close);
    int close_idx = on_close ? idx : -1;
    int hide_hover = hit_hide(hwnd, pt) ? 1 : 0;

    if (idx != g_hover || close_idx != g_close_hover || hide_hover != g_hide_hover) {
        g_hover = idx;
        g_close_hover = close_idx;
        g_hide_hover = hide_hover;
        ui_set_hover(g_hover, g_close_hover, g_hide_hover);
        redraw(hwnd);
    }
}

static LRESULT CALLBACK shelf_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {

    case WM_CREATE:
        glass_apply_acrylic(hwnd);
        return 0;

    case WM_PAINT:
        ui_paint(hwnd);
        return 0;

    case WM_ERASEBKGND:
        return 1;

    case WM_NCHITTEST: {
        POINT pt = { GET_X_LPARAM(lp), GET_Y_LPARAM(lp) };
        ScreenToClient(hwnd, &pt);
        if (hit_hide(hwnd, pt)) return HTCLIENT;
        BOOL dummy = FALSE;
        if (hit_row(hwnd, pt, &dummy) >= 0) return HTCLIENT;
        return HTCAPTION;
    }

    case WM_SETCURSOR:
        SetCursor(LoadCursor(NULL, IDC_ARROW));
        return TRUE;

    case WM_MOUSEMOVE: {
        if (!g_tracking) {
            TRACKMOUSEEVENT tme = { sizeof(tme), TME_LEAVE, hwnd, 0 };
            TrackMouseEvent(&tme);
            g_tracking = TRUE;
        }
        POINT pt = { GET_X_LPARAM(lp), GET_Y_LPARAM(lp) };

        if (g_down_valid && !g_dragging && (wp & MK_LBUTTON)) {
            int dx = pt.x - g_down_pt.x;
            int dy = pt.y - g_down_pt.y;
            if (dx * dx + dy * dy > 9) {
                g_dragging = TRUE;
                g_down_valid = FALSE;
                ReleaseCapture();
                drag_out_start(hwnd, DROPEFFECT_COPY | DROPEFFECT_MOVE | DROPEFFECT_LINK);
                return 0;
            }
        }
        update_hover(hwnd, pt);
        return 0;
    }

    case WM_MOUSELEAVE:
        g_tracking = FALSE;
        if (g_hover != -1 || g_hide_hover) {
            g_hover = -1; g_close_hover = -1; g_hide_hover = 0;
            ui_set_hover(-1, -1, 0);
            redraw(hwnd);
        }
        return 0;

    case WM_LBUTTONDOWN: {
        POINT pt = { GET_X_LPARAM(lp), GET_Y_LPARAM(lp) };

        if (hit_hide(hwnd, pt)) {
            ShowWindow(hwnd, SW_HIDE);
            return 0;
        }

        BOOL on_close = FALSE;
        int idx = hit_row(hwnd, pt, &on_close);
        if (idx < 0) return 0;

        if (on_close) {
            items_remove(idx);
            items_save();
            g_hover = -1; g_close_hover = -1;
            ui_set_hover(-1, -1, g_hide_hover);
            redraw(hwnd);
            return 0;
        }

        g_down_pt = pt;
        g_down_valid = TRUE;
        g_dragging = FALSE;
        SetCapture(hwnd);
        return 0;
    }

    case WM_LBUTTONUP:
        if (g_down_valid) {
            ReleaseCapture();
            g_down_valid = FALSE;
        }
        return 0;

    case WM_LBUTTONDBLCLK: {
        g_down_valid = FALSE;
        g_dragging = FALSE;
        ReleaseCapture();

        POINT pt = { GET_X_LPARAM(lp), GET_Y_LPARAM(lp) };
        BOOL on_close = FALSE;
        int idx = hit_row(hwnd, pt, &on_close);
        if (idx >= 0 && !on_close) {
            const wchar_t* p = items_get(idx);
            if (p) util_reveal_in_explorer(p);
        }
        return 0;
    }

    case WM_HOTKEY:
        if (wp == HOTKEY_ID) shelf_on_hotkey(hwnd);
        return 0;

    case WM_DROPFILES:
        drop_in_handle((HDROP)wp);
        items_save();
        redraw(hwnd);
        return 0;

    case WM_TRAY:
        if (LOWORD(lp) == WM_RBUTTONUP || LOWORD(lp) == WM_CONTEXTMENU)
            tray_menu_public(hwnd);
        else if (LOWORD(lp) == WM_LBUTTONUP)
            shelf_on_hotkey(hwnd);
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
    if (!wc.hIcon) wc.hIcon = LoadIconW(NULL, IDI_APPLICATION);
    wc.hIconSm       = wc.hIcon;
    return RegisterClassExW(&wc) != 0;
}

HWND shelf_create(HINSTANCE hInst, int nCmdShow) {
    HWND hwnd = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_ACCEPTFILES | WS_EX_COMPOSITED,
        SHELF_CLASS, L"Shelf",
        WS_POPUP | WS_CLIPCHILDREN,
        CW_USEDEFAULT, CW_USEDEFAULT, 320, 420,
        NULL, NULL, hInst, NULL);

    if (!hwnd) return NULL;
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);
    return hwnd;
}

void shelf_on_hotkey(HWND hwnd) {
    if (IsWindowVisible(hwnd)) ShowWindow(hwnd, SW_HIDE);
    else { ShowWindow(hwnd, SW_SHOW); SetForegroundWindow(hwnd); redraw(hwnd); }
}