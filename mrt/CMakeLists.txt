cmake_minimum_required(VERSION 3.28)

add_library(mrt)
target_sources(mrt PRIVATE
	b64.cpp
	base_directory.cpp
	base_file.cpp
	calendar.cpp
	chunk.cpp
	dict_serializator.cpp
	directory.cpp
	exception.cpp
	file.cpp
	fmt.cpp
	fs_node.cpp
	gzip.cpp
	ioexception.cpp
	lang.cpp
	logger.cpp
	memory_info.cpp
	net_exception.cpp
	random.cpp
	serializable.cpp
	serializator.cpp
	socket_set.cpp
	sys_socket.cpp
	tcp_socket.cpp
	timer.cpp
	udp_socket.cpp
	utf8_utils.cpp
	xml.cpp
	zip_dir.cpp
	zip_file.cpp
)

generate_export_header(mrt
	EXPORT_FILE_NAME "${CMAKE_CURRENT_BINARY_DIR}/export_mrt.h"
	EXPORT_MACRO_NAME MRTAPI
)

target_include_directories(mrt PUBLIC "${CMAKE_CURRENT_BINARY_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/..")

target_link_libraries(mrt PUBLIC expat::expat ZLIB::ZLIB)
if (WIN32)
	target_link_libraries(mrt PUBLIC Ws2_32)
endif()
