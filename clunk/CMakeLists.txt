project(clunk)
cmake_minimum_required(VERSION 3.28)

option(BUILD_TEST "Build simple test application" false)

find_package(SDL REQUIRED)

set (CMAKE_CXX_STANDARD 11)
set(clunk_VERSION_MAJOR 1)
set(clunk_VERSION_MINOR 3)


if(WIN32)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_WINDOWS)
endif()

add_definitions(-DCLUNKAPI=DLLEXPORT)
set(CMAKE_INCLUDE_CURRENT_DIR TRUE)

add_library(clunk)
target_sources(clunk PRIVATE
	clunk/buffer.cpp
	clunk/clunk_ex.cpp
	clunk/context.cpp
	clunk/distance_model.cpp
	clunk/hrtf.cpp
	clunk/kemar.c
	clunk/logger.cpp
	clunk/object.cpp
	clunk/sample.cpp
	clunk/source.cpp
	clunk/stream.cpp
	clunk/wav_file.cpp
#	clunk/clunk_c.cpp
)

set(CLUNK_BACKEND_SDL 1)
target_sources(clunk PRIVATE
	clunk/backend/sdl/sdl_ex.cpp
	clunk/backend/sdl/backend.cpp
)

set(PUBLIC_HEADERS
	clunk/buffer.h
	clunk/clunk.h
	clunk/clunk_assert.h
	clunk/context.h
	clunk/distance_model.h
	clunk/export_clunk.h
	clunk/fft_context.h
	clunk/hrtf.h
	clunk/kemar.h
	clunk/locker.h
	clunk/logger.h
	clunk/mdct_context.h
	clunk/object.h
	clunk/ref_mdct_context.h
	clunk/sample.h
	clunk/source.h
	clunk/sse_fft_context.h
	clunk/stream.h
	clunk/v3.h
	clunk/clunk_c.h
	clunk/window_function.h
	${CMAKE_CURRENT_BINARY_DIR}/clunk/config.h
)

list(APPEND PUBLIC_HEADERS
	clunk/backend/sdl/backend.h
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/clunk/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/clunk/config.h)

target_include_directories(clunk PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(clunk PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

install(TARGETS clunk DESTINATION lib)
install(FILES ${PUBLIC_HEADERS} DESTINATION include/clunk)

target_link_libraries(clunk PRIVATE SDL::SDL Threads::Threads)

if(BUILD_TEST)
	add_executable(clunk-test test.cpp)
	target_link_libraries(clunk-test clunk)

	add_executable(clunk-mdct clunk-mdct.cpp)
	target_link_libraries(clunk-mdct clunk)
endif()
