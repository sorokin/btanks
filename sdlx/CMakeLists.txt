add_library(sdlx)
target_sources(sdlx PRIVATE
	cursor.cpp sdl_ex.cpp surface.cpp system.cpp joystick.cpp
	color.cpp
	c_map.cpp font.cpp module.cpp file_rw.cpp
	gfx/SDL_rotozoom.c
)

if (WIN32)
	target_sources(sdlx PRIVATE wrappers/d3dsdl.cpp)
else()
	target_sources(sdlx PRIVATE wrappers/glSDL.c)
endif()

generate_export_header(sdlx
	EXPORT_FILE_NAME "${CMAKE_CURRENT_BINARY_DIR}/export_sdlx.h"
	EXPORT_MACRO_NAME SDLXAPI
)

target_include_directories(sdlx PUBLIC "${CMAKE_CURRENT_BINARY_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/..")

target_compile_definitions(sdlx PUBLIC USE_GLSDL)

target_link_libraries(sdlx PUBLIC
	mrt
	SDL::SDL
	SDL_image
)

if (WIN32)
	target_link_libraries(sdlx PUBLIC DirectXSDK::D3D9 DirectXSDK::D3DX9 Winmm User32 Gdi32 Advapi32)
	add_library(sdlx_main STATIC SDL_win32_main.c)
	target_link_libraries(sdlx_main PUBLIC
		SDL::SDL
	)
else ()
	target_link_libraries(sdlx PRIVATE X11::X11)
endif ()


