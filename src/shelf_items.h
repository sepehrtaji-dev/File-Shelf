#pragma once
#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

void items_init(void);
void items_clear(void);
int  items_count(void);
void items_add(const wchar_t* path);
void items_remove(int index);
const wchar_t* items_get(int index);
void items_load(void);
void items_save(void);

#ifdef __cplusplus
}
#endif