#define _CRT_SECURE_NO_WARNINGS
#include "shelf_items.h"
#include "util.h"
#include <shlwapi.h>
#include <stdio.h>
#include <wchar.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ITEMS 256

static wchar_t* g_items[MAX_ITEMS];
static int      g_count = 0;

void items_init(void) { g_count = 0; }

void items_clear(void) {
    for (int i = 0; i < g_count; ++i) { free(g_items[i]); g_items[i] = NULL; }
    g_count = 0;
}

int items_count(void) { return g_count; }

const wchar_t* items_get(int i) {
    return (i >= 0 && i < g_count) ? g_items[i] : NULL;
}

void items_add(const wchar_t* path) {
    if (!path || g_count >= MAX_ITEMS) return;
    for (int i = 0; i < g_count; ++i)
        if (_wcsicmp(g_items[i], path) == 0) return;

    size_t n = wcslen(path) + 1;
    wchar_t* copy = (wchar_t*)malloc(n * sizeof(wchar_t));
    if (!copy) return;
    wcsncpy_s(copy, n, path, _TRUNCATE);
    g_items[g_count++] = copy;
}

void items_remove(int i) {
    if (i < 0 || i >= g_count) return;
    free(g_items[i]);
    for (int j = i; j < g_count - 1; ++j) g_items[j] = g_items[j + 1];
    g_count--;
}

void items_load(void) {
    items_clear();
    FILE* f = _wfopen(util_data_file(), L"r, ccs=UTF-8");
    if (!f) return;
    static wchar_t line[32768];
    while (fgetws(line, 32768, f)) {
        size_t n = wcslen(line);
        while (n && (line[n-1] == L'\n' || line[n-1] == L'\r')) line[--n] = 0;
        if (n) items_add(line);
    }
    fclose(f);
}

void items_save(void) {
    FILE* f = _wfopen(util_data_file(), L"w, ccs=UTF-8");
    if (!f) return;
    for (int i = 0; i < g_count; ++i) fwprintf(f, L"%ls\n", g_items[i]);
    fclose(f);
}