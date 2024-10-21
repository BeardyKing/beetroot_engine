project(imgui)

add_library(imgui
        imgui/imgui.h
        imgui/imgui.cpp
        imgui/imgui_demo.cpp
        imgui/imgui_draw.cpp
        imgui/imgui_widgets.cpp
        imgui/imgui_tables.cpp
        imgui/backends/imgui_impl_vulkan.cpp
)

# Check for platform and add respective backend
if (WIN32)
    list(APPEND IMGUI_BACKENDS
            imgui/backends/imgui_impl_win32.cpp
    )
endif ()

# Add Vulkan
find_package(Vulkan REQUIRED)

# Link libraries
target_link_libraries(imgui
        Vulkan::Vulkan
)

# Include directories
target_include_directories(imgui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/imgui)
target_include_directories(imgui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends)

# Add backend sources to the imgui library
target_sources(imgui PRIVATE ${IMGUI_BACKENDS})
