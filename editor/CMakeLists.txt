set(bted_SOURCES
	base_brush.cpp
	editor.cpp
	open_map_dialog.cpp
	tileset_dialog.cpp
	layer_item.cpp
	layer_list_dialog.cpp
	command.cpp
	add_tileset_dialog.cpp
	add_object_dialog.cpp
	object_properties.cpp
	tilebox_brush.cpp
	morph_dialog.cpp
	resize_dialog.cpp
)

add_executable(bted ${bted_SOURCES})
target_link_libraries(bted PUBLIC mrt sdlx btanks_engine expat::expat bt_objects)

if (WIN32)
	target_link_libraries(bted PRIVATE SDL::SDL sdlx_main user32 dxguid.lib)
else(WIN32)
	target_link_libraries(bted PRIVATE ${SDLMAIN_LIBRARY})
endif(WIN32)
