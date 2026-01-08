option(BTANKS_ENABLE_VENDORED_VCPKG "Download and use vcpkg automatically" ON)
if (BTANKS_ENABLE_VENDORED_VCPKG AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
	include(FetchContent)

	message(STATUS "Downloading vcpkg...")
	FetchContent_Declare(
		vcpkg
		GIT_REPOSITORY https://github.com/microsoft/vcpkg.git
		GIT_TAG 84bab45d415d22042bd0b9081aea57f362da3f35)
	FetchContent_MakeAvailable(vcpkg)

	set(CMAKE_TOOLCHAIN_FILE "${vcpkg_SOURCE_DIR}/scripts/buildsystems/vcpkg.cmake" CACHE FILEPATH "Vcpkg toolchain file")
	if (NOT DEFINED VCPKG_TARGET_TRIPLET AND WIN32)
		set(VCPKG_TARGET_TRIPLET "x64-windows-static")
		set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
		set(BUILD_SHARED_LIBS OFF)
	endif()
endif()
