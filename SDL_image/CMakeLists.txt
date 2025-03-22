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
target_include_directories(SDL_image PUBLIC include)

target_compile_definitions(SDL_image PRIVATE
	LOAD_BMP
	LOAD_GIF
	LOAD_JPG
#	LOAD_JPG_DYNAMIC="libjpeg-8.dll"
	LOAD_LBM
	LOAD_PCX
	LOAD_PNG
#	LOAD_PNG_DYNAMIC="libpng15-15.dll"
	LOAD_PNM
	LOAD_TGA
#	LOAD_TIF
#	LOAD_TIF_DYNAMIC="libtiff-5.dll"
#	LOAD_WEBP
#	LOAD_WEBP_DYNAMIC="libwebp-7.dll"
	LOAD_XPM
	LOAD_XV
#	PNG_USE_DLL
#	ZLIB_DLL
)

target_link_libraries(SDL_image PUBLIC SDL::SDL PNG::PNG JPEG::JPEG)
