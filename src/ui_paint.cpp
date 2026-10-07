#include "ui_paint.h"
#include <gdiplus.h>
using namespace Gdiplus;

static ULONG_PTR g_gdiplus_token = 0;

extern "C" void ui_init(void) {
    GdiplusStartupInput in;
    GdiplusStartup(&g_gdiplus_token, &in, NULL);
}

extern "C" void ui_shutdown(void) {
    if (g_gdiplus_token) GdiplusShutdown(g_gdiplus_token);
}

extern "C" void ui_paint_background(HDC hdc, const RECT* rc) {
    Graphics g(hdc);
    g.SetSmoothingMode(SmoothingModeAntiAlias);

    INT x = (INT)rc->left;
    INT y = (INT)rc->top;
    INT w = (INT)(rc->right  - rc->left);
    INT h = (INT)(rc->bottom - rc->top);

    SolidBrush fill(Color(0x33, 0xFF, 0xFF, 0xFF));
    g.FillRectangle(&fill, x, y, w, h);

    Pen border(Color(0x40, 0xFF, 0xFF, 0xFF), 1.0f);
    g.DrawRectangle(&border, x, y, w - 1, h - 1);
}

extern "C" void ui_paint(HWND hwnd) {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);

    RECT rc;
    GetClientRect(hwnd, &rc);

    HDC mem = CreateCompatibleDC(hdc);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
    HBITMAP old = (HBITMAP)SelectObject(mem, bmp);

    ui_paint_background(mem, &rc);

    BitBlt(hdc, 0, 0, rc.right, rc.bottom, mem, 0, 0, SRCCOPY);

    SelectObject(mem, old);
    DeleteObject(bmp);
    DeleteDC(mem);
    EndPaint(hwnd, &ps);
}