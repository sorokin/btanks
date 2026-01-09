cmake_minimum_required(VERSION 3.28)

add_library(mrt)
target_sources(mrt PRIVATE
	b64.cpp
	b64.h
	base_directory.cpp
	base_directory.h
	base_file.cpp
	base_file.h
	calendar.cpp
	calendar.h
	chunk.cpp
	chunk.h
	dict_serializator.cpp
	dict_serializator.h
	directory.cpp
	directory.h
	exception.cpp
	exception.h
	file.cpp
	file.h
	fmt.cpp
	fmt.h
	fs_node.cpp
	fs_node.h
	gzip.cpp
	gzip.h
	ioexception.cpp
	ioexception.h
	lang.cpp
	lang.h
	logger.cpp
	logger.h
	memory_info.cpp
	memory_info.h
	net_exception.cpp
	net_exception.h
	random.cpp
	random.h
	scoped_ptr.h
	serializable.cpp
	serializable.h
	serializator.cpp
	serializator.h
	singleton.h
	socket_set.cpp
	socket_set.h
	sys_socket.cpp
	sys_socket.h
	tcp_socket.cpp
	tcp_socket.h
	timer.cpp
	timer.h
	udp_socket.cpp
	udp_socket.h
	utf8_utils.cpp
	utf8_utils.h
	xml.cpp
	xml.h
	zip_dir.cpp
	zip_dir.h
	zip_file.cpp
	zip_file.h
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
