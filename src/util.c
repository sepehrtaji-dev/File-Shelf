#include "util.h"
#include <shlobj.h>
#include <shlwapi.h>

static wchar_t g_dir[MAX_PATH];
static wchar_t g_file[MAX_PATH];

void util_init_paths(void) {
    PWSTR appdata = NULL;
    if (SUCCEEDED(SHGetKnownFolderPath(&FOLDERID_RoamingAppData, 0, NULL, &appdata))) {
        wcsncpy_s(g_dir, MAX_PATH, appdata, _TRUNCATE);
        CoTaskMemFree(appdata);
    } else {
        wcsncpy_s(g_dir, MAX_PATH, L".", _TRUNCATE);
    }
    PathAppendW(g_dir, L"Shelf");
    util_ensure_dir(g_dir);

    wcsncpy_s(g_file, MAX_PATH, g_dir, _TRUNCATE);
    PathAppendW(g_file, L"shelf.txt");
}

const wchar_t* util_appdata_dir(void) { return g_dir; }
const wchar_t* util_data_file(void)   { return g_file; }

BOOL util_ensure_dir(const wchar_t* dir) {
    return CreateDirectoryW(dir, NULL) || GetLastError() == ERROR_ALREADY_EXISTS;
}

void util_reveal_in_explorer(const wchar_t* path) {
    if (!path) return;
    PIDLIST_ABSOLUTE pidl = NULL;
    if (SUCCEEDED(SHParseDisplayName(path, NULL, &pidl, 0, NULL))) {
        SHOpenFolderAndSelectItems(pidl, 0, NULL, 0);
        CoTaskMemFree(pidl);
    }
}