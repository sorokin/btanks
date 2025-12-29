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
