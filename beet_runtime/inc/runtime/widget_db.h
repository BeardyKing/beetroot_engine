#ifndef BEETROOT_WIDGET_DB_H
#define BEETROOT_WIDGET_DB_H

#include <cstdint>

//===PUBLIC_STRUCTS=====================================================================================================
enum SelectedPool : int32_t {
    SELECTED_POOL_NONE = -1,
    SELECTED_POOL_LIT_ENT = 0,
    SELECTED_POOL_CAMERA_ENT = 1,
};
//======================================================================================================================

//===API================================================================================================================
void widget_db_update(bool &enabled);

SelectedPool *get_selected_pool_type();
int32_t *get_selected_pool_index();
//======================================================================================================================

#endif //BEETROOT_WIDGET_DB_H
