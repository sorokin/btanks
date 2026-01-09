cmake_minimum_required(VERSION 3.28)

find_package(SDL REQUIRED)
find_package(PNG REQUIRED)
find_package(JPEG REQUIRED)

add_library(SDL_image)
target_sources(SDL_image PRIVATE
	src/IMG.c
	src/IMG_bmp.c
	src/IMG_gif.c
	src/IMG_jpg.c
	src/IMG_lbm.c
	src/IMG_pcx.c
	src/IMG_png.c
	src/IMG_pnm.c
	src/IMG_tga.c
	src/IMG_tif.c
	src/IMG_webp.c
	src/IMG_xcf.c
	src/IMG_xpm.c
	src/IMG_xv.c
	src/IMG_xxx.c
)

target_sources(SDL_image PUBLIC
	FILE_SET HEADERS
	BASE_DIRS include
	FILES
	include/SDL_image.h
)

target_compile_definitions(SDL_image PRIVATE
	LOAD_BMP
	LOAD_GIF
	LOAD_JPG
	LOAD_LBM
	LOAD_PCX
	LOAD_PNG
	LOAD_PNM
	LOAD_TGA
	LOAD_XPM
	LOAD_XV
)

target_link_libraries(SDL_image PUBLIC SDL::SDL PNG::PNG JPEG::JPEG)
