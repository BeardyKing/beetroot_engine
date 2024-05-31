cmake_minimum_required(VERSION 3.15)
project(beetroot)

if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

set(BEET_CMAKE_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

#====ENGINE LIBS==========
add_subdirectory(beet_engine)

#====OFFLINE PIPELINE=====
add_subdirectory(beet_pipeline)

#====EXES=================
add_subdirectory(beet_runtime)