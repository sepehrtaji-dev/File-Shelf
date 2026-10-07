#include "glass.h"
#include <dwmapi.h>

typedef enum _WINDOWCOMPOSITIONATTRIB {
    WCA_ACCENT_POLICY = 19
} WINDOWCOMPOSITIONATTRIB;

typedef struct _WINDOWCOMPOSITIONATTRIBDATA {
    WINDOWCOMPOSITIONATTRIB Attrib;
    PVOID pvData;
    SIZE_T cbData;
} WINDOWCOMPOSITIONATTRIBDATA;

typedef enum _ACCENT_STATE {
    ACCENT_DISABLED                   = 0,
    ACCENT_ENABLE_GRADIENT            = 1,
    ACCENT_ENABLE_TRANSPARENTGRADIENT = 2,
    ACCENT_ENABLE_BLURBEHIND          = 3,
    ACCENT_ENABLE_ACRYLICBLURBEHIND   = 4,
    ACCENT_ENABLE_HOSTBACKDROP        = 5
} ACCENT_STATE;

typedef struct _ACCENT_POLICY {
    ACCENT_STATE AccentState;
    DWORD AccentFlags;
    DWORD GradientColor;
    DWORD AnimationId;
} ACCENT_POLICY;

typedef BOOL (WINAPI *pSetWindowCompositionAttribute)(HWND, WINDOWCOMPOSITIONATTRIBDATA*);

#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif
#ifndef DWMWA_WINDOW_CORNER_PREFERENCE
#define DWMWA_WINDOW_CORNER_PREFERENCE 33
#endif

void glass_apply_acrylic(HWND hwnd) {
    // Dark titlebar + rounded corners (Win11).
    BOOL dark = TRUE;
    DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &dark, sizeof(dark));

    int corner = 2; // DWMWCP_ROUND
    DwmSetWindowAttribute(hwnd, DWMWA_WINDOW_CORNER_PREFERENCE, &corner, sizeof(corner));

    // Real acrylic blur behind the window.
    HMODULE u32 = GetModuleHandleW(L"user32.dll");
    if (!u32) return;

    // Launder FARPROC -> typed function pointer via a union to avoid
    // -Wcast-function-type (GCC) / C4191 (MSVC).
    union {
        FARPROC raw;
        pSetWindowCompositionAttribute typed;
    } u;
    u.raw = GetProcAddress(u32, "SetWindowCompositionAttribute");
    if (!u.typed) return;

    ACCENT_POLICY policy = {0};
    policy.AccentState   = ACCENT_ENABLE_ACRYLICBLURBEHIND;
    policy.AccentFlags   = 2;               // draw borders
    policy.GradientColor = 0x8018181C;      // AABBGGRR — 50% dark grey-blue tint

    WINDOWCOMPOSITIONATTRIBDATA data = {0};
    data.Attrib = WCA_ACCENT_POLICY;
    data.pvData = &policy;
    data.cbData = sizeof(policy);
    u.typed(hwnd, &data);
}