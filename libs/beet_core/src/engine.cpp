#include <beet_core/engine.h>
#include <beet_shared/assert.h>

#include <map>

//TODO: REFACTOR TO POD i.e. remove std::map and replace with custom ordered map
//===INTERNAL_STRUCTS===================================================================================================
struct Engine {
    std::map<uint32_t, void (*)()> createSystems;
    std::map<uint32_t, void (*)()> cleanupSystems;
    std::map<uint32_t, void (*)()> updateSystems;
};
static Engine *s_engine;
//======================================================================================================================

//===API================================================================================================================
void engine_register_system_create(uint32_t priority, void(*ptr)()) {
    ASSERT_MSG(s_engine->createSystems.count(priority) == 0, "Err: create system already exists with this priority");
    s_engine->createSystems[priority] = ptr;
}

void engine_register_system_update(uint32_t priority, void(*ptr)()) {
    ASSERT_MSG(s_engine->updateSystems.count(priority) == 0, "Err: update system already exists with this priority");
    s_engine->updateSystems[priority] = ptr;
}

void engine_register_system_cleanup(uint32_t priority, void(*ptr)()) {
    ASSERT_MSG(s_engine->cleanupSystems.count(priority) == 0, "Err: cleanup system already exists with this priority");
    s_engine->cleanupSystems[priority] = ptr;
}


void engine_system_create() {
    for (const auto &sys: s_engine->createSystems) {
        sys.second();
    }
}

void engine_system_update() {
    for (const auto &sys: s_engine->updateSystems) {
        sys.second();
    }
}

void engine_system_cleanup() {
    // iterate over the cleanup systems in reverse.
    auto &cleanupSystems = s_engine->cleanupSystems;
    for (auto sys = cleanupSystems.rbegin(); sys != cleanupSystems.rend(); sys++) {
        sys->second();
    }
}
//======================================================================================================================

//===INIT_&_SHUTDOWN====================================================================================================
void engine_create() {
    s_engine = new Engine;
}

void engine_cleanup() {
    delete s_engine;
    s_engine = nullptr;
}
//======================================================================================================================

