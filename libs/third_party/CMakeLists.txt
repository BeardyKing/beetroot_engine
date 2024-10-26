#===GLM================================================================================================================
add_subdirectory(glm)
#======================================================================================================================

#===LIB_FMT============================================================================================================
add_subdirectory(fmt)
#======================================================================================================================

#===ISPC_COMPRESSOR====================================================================================================
#set(ISPC_COMPILER_LOCATION $ENV{ISPC_COMPILER_LOCATION})
#set(ISPC_COMPILER_TARGET "avx2")
#add_subdirectory(ispc_texture_compressor_cmake)
#======================================================================================================================

#===CGLTF==============================================================================================================
project(cgltf)
add_library(cgltf INTERFACE)
target_include_directories(cgltf INTERFACE cgltf/)
#======================================================================================================================

#===IMGUI==============================================================================================================
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

if (WIN32)
    list(APPEND IMGUI_BACKENDS
            imgui/backends/imgui_impl_win32.cpp
    )
endif ()

find_package(Vulkan REQUIRED)
target_link_libraries(imgui
        Vulkan::Vulkan
)

target_include_directories(imgui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/imgui)
target_include_directories(imgui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends)

target_sources(imgui PRIVATE ${IMGUI_BACKENDS})
#======================================================================================================================
