cmake_minimum_required(VERSION 3.28)

add_definitions(-DMRTAPI=DLLEXPORT)
include_directories(.)

set(mrt_SOURCES
	exception.cpp fmt.cpp ioexception.cpp logger.cpp chunk.cpp random.cpp
	base_file.cpp file.cpp xml.cpp b64.cpp gzip.cpp serializator.cpp
	serializable.cpp sys_socket.cpp tcp_socket.cpp socket_set.cpp directory.cpp
	fs_node.cpp lang.cpp utf8_utils.cpp udp_socket.cpp
	memory_info.cpp calendar.cpp base_directory.cpp zip_dir.cpp zip_file.cpp
	net_exception.cpp dict_serializator.cpp
)

add_library(mrt ${mrt_SOURCES})
target_link_libraries(mrt PUBLIC expat::expat ZLIB::ZLIB ws2_32.lib)
