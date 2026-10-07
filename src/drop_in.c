#include "drop_in.h"
#include "shelf_items.h"

void drop_in_handle(HDROP hdrop) {
    UINT n = DragQueryFileW(hdrop, 0xFFFFFFFF, NULL, 0);
    for (UINT i = 0; i < n; ++i) {
        wchar_t path[MAX_PATH * 2];
        if (DragQueryFileW(hdrop, i, path, MAX_PATH * 2)) items_add(path);
    }
    DragFinish(hdrop);
}