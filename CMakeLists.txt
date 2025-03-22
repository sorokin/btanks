cmake_minimum_required(VERSION 3.28)

option(BTANKS_ENABLE_VENDORED_VCPKG "Download and use vcpkg automatically" ON)
if (BTANKS_ENABLE_VENDORED_VCPKG AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
	include(FetchContent)

	message(STATUS "Downloading vcpkg...")
	FetchContent_Declare(
		vcpkg
		GIT_REPOSITORY https://github.com/microsoft/vcpkg.git
		GIT_TAG a7d06b3a72d5ec48353bacb84152bd027ee9999b)
	FetchContent_MakeAvailable(vcpkg)

	set(CMAKE_TOOLCHAIN_FILE "${vcpkg_SOURCE_DIR}/scripts/buildsystems/vcpkg.cmake" CACHE FILEPATH "Vcpkg toolchain file")
	if (NOT DEFINED VCPKG_TARGET_TRIPLET)
		set(VCPKG_TARGET_TRIPLET "x64-windows-static")
	endif()
endif()

project(btanks)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(CMAKE_USE_RELATIVE_PATHS TRUE)
set(MACOSX_RPATH TRUE)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

include(CheckLibraryExists)

add_definitions(-DUSE_GLSDL -DV3_DISABLE_Z -D_REENTRANT)
include_directories(${CMAKE_SOURCE_DIR})

if (WIN32)
	add_definitions(-D_WINDOWS -DWIN32)
endif(WIN32)

find_package(SDL REQUIRED)
find_package(expat CONFIG REQUIRED)
find_package(ZLIB REQUIRED)
find_package(DirectXSDK REQUIRED)

add_subdirectory(SDL_image)
add_subdirectory(mrt)
add_subdirectory(sdlx)
add_subdirectory(clunk)
add_subdirectory(engine)
add_subdirectory(objects)


add_custom_target(data_copy
	COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/data"
        "$<TARGET_FILE_DIR:btanks>/data"
	COMMENT "Copying data directory"
)


add_executable(btanks)
target_sources(btanks PRIVATE engine/src/main.cpp)
if (WIN32)
	target_sources(btanks PRIVATE engine/src/bt.rc)
endif()

target_link_libraries(btanks PRIVATE mrt sdlx btanks_engine bt_objects SDL::SDL sdlx_main user32 dxguid.lib)
add_dependencies(btanks data_copy)


add_executable(bted)

target_sources(bted PRIVATE
	editor/base_brush.cpp
	editor/editor.cpp
	editor/open_map_dialog.cpp
	editor/tileset_dialog.cpp
	editor/layer_item.cpp
	editor/layer_list_dialog.cpp
	editor/command.cpp
	editor/add_tileset_dialog.cpp
	editor/add_object_dialog.cpp
	editor/object_properties.cpp
	editor/tilebox_brush.cpp
	editor/morph_dialog.cpp
	editor/resize_dialog.cpp
)

target_link_libraries(bted PUBLIC mrt sdlx btanks_engine expat::expat bt_objects)

if (WIN32)
	target_link_libraries(bted PRIVATE SDL::SDL sdlx_main user32 dxguid.lib)
else(WIN32)
	target_link_libraries(bted PRIVATE ${SDLMAIN_LIBRARY})
endif(WIN32)
add_dependencies(bted data_copy)
