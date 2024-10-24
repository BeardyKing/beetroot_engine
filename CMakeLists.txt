cmake_minimum_required(VERSION 3.15)
project(beetroot)

if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

set(BEET_CMAKE_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

#====THIRD_PARTY_LIBS=====
add_subdirectory(libs/third_party)

#====LIBS=================
add_subdirectory(libs/beet_math)
add_subdirectory(libs/beet_shared)
add_subdirectory(libs/beet_gfx)
add_subdirectory(libs/beet_converter)
add_subdirectory(libs/beet_core)

#====PROJECTS=============
add_subdirectory(projects/beet_pipeline)
add_subdirectory(projects/beet_runtime)