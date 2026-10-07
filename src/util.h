#pragma once
#include <windows.h>

void  util_init_paths(void);
const wchar_t* util_appdata_dir(void);   // %APPDATA%\Shelf
const wchar_t* util_data_file(void);     // %APPDATA%\Shelf\shelf.txt
BOOL  util_ensure_dir(const wchar_t* dir);