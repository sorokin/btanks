cmake_minimum_required(VERSION 3.28)

add_library(mrt)
target_sources(mrt PRIVATE
	mrt/b64.cpp
	mrt/base_directory.cpp
	mrt/base_file.cpp
	mrt/calendar.cpp
	mrt/chunk.cpp
	mrt/dict_serializator.cpp
	mrt/directory.cpp
	mrt/exception.cpp
	mrt/file.cpp
	mrt/fmt.cpp
	mrt/fs_node.cpp
	mrt/gzip.cpp
	mrt/ioexception.cpp
	mrt/lang.cpp
	mrt/logger.cpp
	mrt/memory_info.cpp
	mrt/net_exception.cpp
	mrt/random.cpp
	mrt/serializable.cpp
	mrt/serializator.cpp
	mrt/socket_set.cpp
	mrt/sys_socket.cpp
	mrt/tcp_socket.cpp
	mrt/timer.cpp
	mrt/udp_socket.cpp
	mrt/utf8_utils.cpp
	mrt/xml.cpp
	mrt/zip_dir.cpp
	mrt/zip_file.cpp
)

target_sources(mrt PUBLIC
	FILE_SET HEADERS
	BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}"
	FILES
	mrt/b64.h
	mrt/base_directory.h
	mrt/base_file.h
	mrt/calendar.h
	mrt/chunk.h
	mrt/dict_serializator.h
	mrt/directory.h
	mrt/exception.h
	mrt/file.h
	mrt/fmt.h
	mrt/fs_node.h
	mrt/gzip.h
	mrt/ioexception.h
	mrt/lang.h
	mrt/logger.h
	mrt/memory_info.h
	mrt/net_exception.h
	mrt/random.h
	mrt/serializable.h
	mrt/serializator.h
	mrt/singleton.h
	mrt/socket_set.h
	mrt/sys_socket.h
	mrt/tcp_socket.h
	mrt/timer.h
	mrt/udp_socket.h
	mrt/utf8_utils.h
	mrt/xml.h
	mrt/zip_dir.h
	mrt/zip_file.h
	"${CMAKE_CURRENT_BINARY_DIR}/mrt/export_mrt.h"
)

generate_export_header(mrt
	EXPORT_FILE_NAME "${CMAKE_CURRENT_BINARY_DIR}/mrt/export_mrt.h"
	EXPORT_MACRO_NAME MRTAPI
)

target_link_libraries(mrt PUBLIC expat::expat ZLIB::ZLIB)
if (WIN32)
	target_link_libraries(mrt PUBLIC Ws2_32)
endif()
