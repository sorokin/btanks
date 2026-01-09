cmake_minimum_required(VERSION 3.28)

add_library(math INTERFACE)

target_sources(math PUBLIC
	FILE_SET HEADERS
	BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}"
	FILES
	math/binary.h
	math/matrix.h
	math/quad_tree.h
	math/range_list.h
	math/unary.h
	math/v2.h
	math/v3.h
	math/vector.h
)

target_link_libraries(math INTERFACE mrt)
