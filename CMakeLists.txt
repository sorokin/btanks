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
	if (NOT DEFINED VCPKG_TARGET_TRIPLET AND WIN32)
		set(VCPKG_TARGET_TRIPLET "x64-windows-static")
		set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
	endif()
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
project(btanks)

find_package(expat CONFIG REQUIRED)
find_package(Ogg CONFIG REQUIRED)
find_package(Lua51 REQUIRED)
find_package(SDL REQUIRED)
find_package(Threads REQUIRED)
find_package(Vorbis CONFIG REQUIRED)
find_package(ZLIB REQUIRED)
if (WIN32)
	find_package(DirectXSDK REQUIRED)
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
	# too noisy on SDL1
	add_compile_options("-Wno-pragma-pack" "-Wno-language-extension-token")
endif()

add_definitions(-DUSE_GLSDL)
if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
	add_definitions(-DDEBUG)
endif()
include_directories(${CMAKE_SOURCE_DIR})

if (WIN32)
	add_definitions(-D_WINDOWS -DWIN32)
endif()

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

target_link_libraries(btanks PRIVATE mrt sdlx btanks_engine bt_objects SDL::SDL)
if (WIN32)
	target_link_libraries(btanks PRIVATE sdlx_main User32 DxGuid)
endif()
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

target_link_libraries(bted PRIVATE mrt sdlx btanks_engine expat::expat bt_objects)

target_link_libraries(bted PRIVATE SDL::SDL)
if (WIN32)
	target_link_libraries(bted PRIVATE sdlx_main User32 DxGuid)
endif()
add_dependencies(bted data_copy)
