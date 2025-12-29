add_definitions(-DSDLXAPI=DLLEXPORT)

add_library(sdlx)
target_sources(sdlx PRIVATE
	cursor.cpp sdl_ex.cpp surface.cpp system.cpp joystick.cpp
	thread.cpp mutex.cpp semaphore.cpp color.cpp
	c_map.cpp font.cpp timer.cpp module.cpp file_rw.cpp
	gfx/SDL_rotozoom.c
)

if (WIN32)
	target_sources(sdlx PRIVATE wrappers/d3dsdl.cpp)
else()
	target_sources(sdlx PRIVATE wrappers/glSDL.c)
endif()

target_include_directories(sdlx PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/..")

target_compile_definitions(sdlx PUBLIC USE_GLSDL)

target_link_libraries(sdlx PUBLIC
	mrt
	SDL::SDL
	SDL_image
	Threads::Threads
)

if (WIN32)
	target_link_libraries(sdlx PUBLIC DirectXSDK::D3D9 DirectXSDK::D3DX9 Winmm User32 Gdi32 Advapi32)
	add_library(sdlx_main SDL_win32_main.c)
	target_link_libraries(sdlx_main PUBLIC
		SDL::SDL
	)
else ()
	target_link_libraries(sdlx PRIVATE rt X11)
endif ()


