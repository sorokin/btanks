cmake_minimum_required(VERSION 3.28)
project(btanks)
set(CMAKE_USE_RELATIVE_PATHS TRUE)
set(MACOSX_RPATH TRUE)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

include(CheckLibraryExists)

add_definitions(-DUSE_GLSDL -DV3_DISABLE_Z -D_REENTRANT)
include_directories(${CMAKE_SOURCE_DIR})

if (WIN32)
	add_definitions(-D_WINDOWS -DWIN32)
endif(WIN32)

if (MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4305") # truncation double -> float, too noisy
endif()

find_package(SDL REQUIRED)
find_package(expat CONFIG REQUIRED)
find_package(ZLIB REQUIRED)

add_subdirectory(SDL_image)
add_subdirectory(mrt)
add_subdirectory(sdlx)
add_subdirectory(clunk)
add_subdirectory(engine)
add_subdirectory(objects)
add_subdirectory(editor)

#include_directories(engine)

add_executable(btanks)
target_sources(btanks PRIVATE engine/src/main.cpp)
if (WIN32)
	target_sources(btanks PRIVATE engine/src/bt.rc)
endif()

target_link_libraries(btanks PRIVATE mrt sdlx btanks_engine bt_objects SDL::SDL sdlx_main user32 dxguid.lib)
