option(BUILD_TEST "Build simple test application" false)
option(CLUNK_BACKEND_SDL "Enable SDL backend" true)
option(CLUNK_ENABLE_DEBUG_LOGGING "Enable debug-mode logging" false)

add_library(clunk)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/clunk/config.h.in" "${CMAKE_CURRENT_BINARY_DIR}/clunk/config.h")

generate_export_header(clunk
	EXPORT_FILE_NAME "${CMAKE_CURRENT_BINARY_DIR}/clunk/export_clunk.h"
	EXPORT_MACRO_NAME CLUNKAPI
)

target_sources(clunk PRIVATE
	clunk/backend/sdl/backend.cpp
	clunk/backend/sdl/sdl_ex.cpp
	clunk/buffer.cpp
	clunk/clunk_ex.cpp
	clunk/context.cpp
	clunk/distance_model.cpp
	clunk/hrtf.cpp
	clunk/kemar.c
	clunk/logger.cpp
	clunk/mixer.h
	clunk/object.cpp
	clunk/resample.h
	clunk/sample.cpp
	clunk/source.cpp
	clunk/stream.cpp
	clunk/wav_file.cpp
)

target_sources(clunk PUBLIC
	FILE_SET HEADERS
	BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}"
	FILES
	"${CMAKE_CURRENT_BINARY_DIR}/clunk/config.h"
	"${CMAKE_CURRENT_BINARY_DIR}/clunk/export_clunk.h"
	clunk/audio_spec.h
	clunk/backend/sdl/backend.h
	clunk/backend/sdl/locker.h
	clunk/backend/sdl/sdl_ex.h
	clunk/buffer.h
	clunk/clunk.h
	clunk/clunk_ex.h
	clunk/context.h
	clunk/distance_model.h
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
	clunk/stream.h
	clunk/types.h
	clunk/v3.h
	clunk/wav_file.h
	clunk/window_function.h
)

install(TARGETS clunk DESTINATION lib
	FILE_SET HEADERS DESTINATION include
)

target_link_libraries(clunk PRIVATE SDL::SDL Threads::Threads)

if(BUILD_TEST)
	add_executable(clunk-test test.cpp)
	target_link_libraries(clunk-test clunk)

	add_executable(clunk-mdct clunk-mdct.cpp)
	target_link_libraries(clunk-mdct clunk)
endif()
