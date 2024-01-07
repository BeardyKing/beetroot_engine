#ifndef BEETROOT_ENGINE_H
#define BEETROOT_ENGINE_H

#include <cstdint>

//===api=====================
bool engine_is_open();

void engine_system_create();
void engine_system_update();
void engine_system_cleanup();

void engine_register_system_create(uint32_t priority, void(*ptr)());
void engine_register_system_update(uint32_t priority, void(*ptr)());
void engine_register_system_cleanup(uint32_t priority, void(*ptr)());

//===init & shutdown=========
void engine_create();
void engine_cleanup();

#endif //BEETROOT_ENGINE_H
