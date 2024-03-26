project(imgui)

add_library(imgui
        imgui/imgui.h
        imgui/imgui.cpp
        imgui/imgui_demo.cpp
        imgui/imgui_draw.cpp
        imgui/imgui_widgets.cpp
        imgui/imgui_tables.cpp
        imgui/backends/imgui_impl_vulkan.cpp
        imgui/backends/imgui_impl_win32.cpp
)

find_package(Vulkan REQUIRED)
target_link_libraries(imgui
        Vulkan::Vulkan
)

target_include_directories(imgui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/imgui)
target_include_directories(imgui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends)