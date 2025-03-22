include_directories(.)
include_directories(../mrt)
add_definitions(-DSDLXAPI=DLLEXPORT)

set(sdlx_SOURCES 
	cursor.cpp sdl_ex.cpp surface.cpp system.cpp joystick.cpp
	thread.cpp mutex.cpp semaphore.cpp color.cpp
	c_map.cpp font.cpp timer.cpp module.cpp file_rw.cpp
	gfx/SDL_rotozoom.c
)

if (WIN32)
	list(APPEND sdlx_SOURCES wrappers/d3dsdl.cpp)
else(WIN32)
	list(APPEND sdlx_SOURCES wrappers/glSDL.c)
endif(WIN32)

add_library(sdlx ${sdlx_SOURCES})
target_link_libraries(sdlx PRIVATE
	mrt
	SDL::SDL
)

if (WIN32)
	target_link_libraries(sdlx PUBLIC DirectXSDK::D3D9 DirectXSDK::D3DX9 SDL_image winmm user32 gdi32 advapi32)
	add_library(sdlx_main SDL_win32_main.c)
	target_link_libraries(sdlx_main PUBLIC
		SDL::SDL
	)
else ()
	target_link_libraries(sdlx rt X11)
endif ()


