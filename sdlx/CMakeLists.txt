add_library(sdlx)
target_sources(sdlx PRIVATE
	SDL_main.h
	c_map.cpp
	c_map.h
	color.cpp
	color.h
	cursor.cpp
	cursor.h
	file_rw.cpp
	file_rw.h
	font.cpp
	font.h
	gfx/SDL_rotozoom.c
	joystick.cpp
	joystick.h
	module.cpp
	module.h
	rect.h
	sdl_ex.cpp
	sdl_ex.h
	sdlx.h
	surface.cpp
	surface.h
	system.cpp
	system.h
)

if (WIN32)
	target_sources(sdlx PRIVATE wrappers/d3dsdl.cpp wrappers/d3dsdl.h)
else()
	target_sources(sdlx PRIVATE wrappers/glSDL.c wrappers/glSDL.h)
endif()

generate_export_header(sdlx
	EXPORT_FILE_NAME "${CMAKE_CURRENT_BINARY_DIR}/export_sdlx.h"
	EXPORT_MACRO_NAME SDLXAPI
)

target_include_directories(sdlx PUBLIC "${CMAKE_CURRENT_BINARY_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/..")

target_compile_definitions(sdlx PUBLIC USE_GLSDL)
target_compile_definitions(sdlx PRIVATE _USE_MATH_DEFINES)

target_link_libraries(sdlx PUBLIC
	mrt
	SDL::SDL
	SDL_image
)

target_link_libraries(sdlx PRIVATE math)

if (WIN32)
	target_link_libraries(sdlx PUBLIC DirectXSDK::D3D9 DirectXSDK::D3DX9 Winmm User32 Gdi32 Advapi32)
	add_library(sdlx_main STATIC SDL_win32_main.c)
	target_link_libraries(sdlx_main PUBLIC
		SDL::SDL
	)
else ()
	target_link_libraries(sdlx PRIVATE X11::X11)
endif ()


