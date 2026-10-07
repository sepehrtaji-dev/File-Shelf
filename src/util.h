#pragma once
#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

void  util_init_paths(void);
const wchar_t* util_appdata_dir(void);
const wchar_t* util_data_file(void);
BOOL  util_ensure_dir(const wchar_t* dir);
void  util_reveal_in_explorer(const wchar_t* path);

#ifdef __cplusplus
}
#endif