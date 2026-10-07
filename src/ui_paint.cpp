#include "ui_paint.h"
#include "shelf_layout.h"
#include "shelf_items.h"
#include <windows.h>
#include <gdiplus.h>
#include <shlwapi.h>

static ULONG_PTR g_token = 0;
static int g_hover = -1;
static int g_close_hover = -1;
static int g_hide_hover = 0;

extern "C" void ui_init(void) {
    Gdiplus::GdiplusStartupInput in;
    Gdiplus::GdiplusStartup(&g_token, &in, NULL);
}
extern "C" void ui_shutdown(void) {
    if (g_token) Gdiplus::GdiplusShutdown(g_token);
}
extern "C" void ui_set_hover(int h, int c, int hh) {
    g_hover = h; g_close_hover = c; g_hide_hover = hh;
}

static void rounded_path(Gdiplus::GraphicsPath& p, const Gdiplus::RectF& r,
                         Gdiplus::REAL radius) {
    Gdiplus::REAL d = radius * 2.0f;
    if (d > r.Width)  d = r.Width;
    if (d > r.Height) d = r.Height;
    p.AddArc(r.X,            r.Y,             d, d, 180.0f, 90.0f);
    p.AddArc(r.GetRight()-d, r.Y,             d, d, 270.0f, 90.0f);
    p.AddArc(r.GetRight()-d, r.GetBottom()-d, d, d,   0.0f, 90.0f);
    p.AddArc(r.X,            r.GetBottom()-d, d, d,  90.0f, 90.0f);
    p.CloseFigure();
}

static void draw_text(Gdiplus::Graphics& g, const WCHAR* s,
                      const Gdiplus::RectF& r, Gdiplus::REAL size,
                      Gdiplus::ARGB argb) {
    Gdiplus::FontFamily ff(L"Segoe UI");
    Gdiplus::Font font(&ff, size, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
    Gdiplus::SolidBrush brush{ Gdiplus::Color(argb) };
    Gdiplus::StringFormat fmt;
    fmt.SetAlignment(Gdiplus::StringAlignmentNear);
    fmt.SetLineAlignment(Gdiplus::StringAlignmentCenter);
    fmt.SetTrimming(Gdiplus::StringTrimmingEllipsisPath);
    g.DrawString(s, -1, &font, r, &fmt, &brush);
}

static void basename_of(const wchar_t* path, wchar_t* out, size_t n) {
    const wchar_t* slash = wcsrchr(path, L'\\');
    const wchar_t* fwd   = wcsrchr(path, L'/');
    const wchar_t* base  = slash > fwd ? slash : fwd;
    wcsncpy_s(out, n, base ? base + 1 : path, _TRUNCATE);
}
static void parent_of(const wchar_t* path, wchar_t* out, size_t n) {
    wcsncpy_s(out, n, path, _TRUNCATE);
    wchar_t* slash = wcsrchr(out, L'\\');
    if (slash) *slash = 0;
}

extern "C" void ui_paint(HWND hwnd) {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);

    RECT rc;
    GetClientRect(hwnd, &rc);
    int W = rc.right, H = rc.bottom;

    Gdiplus::Graphics g(hdc);
    g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    g.SetTextRenderingHint(Gdiplus::TextRenderingHintClearTypeGridFit);

    // ---- 1px highlight border ------------------------------------------
    Gdiplus::Pen border{ Gdiplus::Color(0xFF, 0x60, 0x60, 0x70), 1.0f };
    g.DrawRectangle(&border, 0, 0, W - 1, H - 1);

    // ---- hide button ----------------------------------------------------
    RECT hb = shelf_hide_button_rect(rc);
    Gdiplus::RectF hbf((Gdiplus::REAL)hb.left, (Gdiplus::REAL)hb.top,
                       (Gdiplus::REAL)(hb.right - hb.left),
                       (Gdiplus::REAL)(hb.bottom - hb.top));

    if (g_hide_hover) {
        Gdiplus::GraphicsPath hp;
        rounded_path(hp, hbf, 6.0f);
        Gdiplus::SolidBrush hbg{ Gdiplus::Color(0xFF, 0x55, 0x55, 0x60) };
        g.FillPath(&hbg, &hp);
    }
    {
        Gdiplus::Pen chev{ Gdiplus::Color(0xFF, 0xFF, 0xFF, 0xFF), 1.6f };
        Gdiplus::REAL cx = hbf.X + hbf.Width  * 0.5f;
        Gdiplus::REAL cy = hbf.Y + hbf.Height * 0.5f;
        g.DrawLine(&chev, (INT)(hbf.X + 6), (INT)(cy + 2),
                          (INT)cx,          (INT)(cy - 4));
        g.DrawLine(&chev, (INT)cx,          (INT)(cy - 4),
                          (INT)(hbf.GetRight() - 6), (INT)(cy + 2));
    }

    // ---- rows -----------------------------------------------------------
    int n = items_count();
    for (int i = 0; i < n; ++i) {
        RECT rr = shelf_row_rect(rc, i);
        Gdiplus::RectF rf((Gdiplus::REAL)rr.left, (Gdiplus::REAL)rr.top,
                          (Gdiplus::REAL)(rr.right - rr.left),
                          (Gdiplus::REAL)(rr.bottom - rr.top));

        if (i == g_hover) {
            Gdiplus::GraphicsPath rp;
            rounded_path(rp, rf, (Gdiplus::REAL)SHELF_ROUND - 2.0f);
            Gdiplus::SolidBrush rowbg{ Gdiplus::Color(0xFF, 0x55, 0x55, 0x60) };
            g.FillPath(&rowbg, &rp);
        }

        const wchar_t* src = items_get(i);
        wchar_t name[MAX_PATH], parent[MAX_PATH];
        basename_of(src, name, MAX_PATH);
        parent_of(src, parent, MAX_PATH);

        Gdiplus::REAL text_left  = rf.X + 14.0f;
        Gdiplus::REAL text_right = rf.GetRight() - (i == g_hover ? 40.0f : 14.0f);

        Gdiplus::RectF name_r(text_left, rf.Y + 4.0f,
                              text_right - text_left, 16.0f);
        draw_text(g, name, name_r, 13.5f, 0xFFFFFFFF);

        Gdiplus::RectF parent_r(text_left, rf.Y + 20.0f,
                                text_right - text_left, 13.0f);
        draw_text(g, parent, parent_r, 10.0f, 0xFFA8A8B0);

        if (i == g_hover) {
            Gdiplus::REAL bx = rf.GetRight() - 28.0f;
            Gdiplus::REAL by = rf.Y + (rf.Height - 18.0f) / 2.0f;
            Gdiplus::RectF br(bx, by, 18.0f, 18.0f);

            if (i == g_close_hover) {
                Gdiplus::GraphicsPath cp;
                rounded_path(cp, br, 6.0f);
                Gdiplus::SolidBrush hbb{ Gdiplus::Color(0xFF, 0xC0, 0x40, 0x40) };
                g.FillPath(&hbb, &cp);
            }
            Gdiplus::Pen xp{ Gdiplus::Color(0xFF, 0xFF, 0xFF, 0xFF), 1.4f };
            g.DrawLine(&xp, (INT)(br.X + 5),          (INT)(br.Y + 5),
                            (INT)(br.GetRight() - 5), (INT)(br.GetBottom() - 5));
            g.DrawLine(&xp, (INT)(br.GetRight() - 5), (INT)(br.Y + 5),
                            (INT)(br.X + 5),          (INT)(br.GetBottom() - 5));
        }
    }

    EndPaint(hwnd, &ps);
}