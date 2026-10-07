#pragma once
#include <windows.h>

void items_init(void);
void items_clear(void);
int  items_count(void);
void items_add(const wchar_t* path);
void items_remove(int index);
const wchar_t* items_get(int index);
void items_load(void);
void items_save(void);