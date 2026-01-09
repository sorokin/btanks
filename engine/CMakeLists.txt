add_library(btanks_engine)
target_sources(btanks_engine PRIVATE
	ai/base.cpp
	ai/buratino.cpp
	ai/herd.cpp
	ai/old_school.cpp
	ai/rush.cpp
	ai/targets.cpp
	ai/traits.cpp
	ai/trooper.cpp
	ai/waypoints.cpp

	controls/control_method.cpp
	controls/joyplayer.cpp
	controls/keyplayer.cpp
	controls/mouse_control.cpp
	controls/simple_joy_bindings.cpp

	menu/box.cpp
	menu/button.cpp
	menu/campaign_menu.cpp
	menu/chat.cpp
	menu/checkbox.cpp
	menu/chooser.cpp
	menu/container.cpp
	menu/control.cpp
	menu/control_picker.cpp
	menu/grid.cpp
	menu/host_item.cpp
	menu/host_list.cpp
	menu/image.cpp
	menu/image_view.cpp
	menu/join_server_menu.cpp
	menu/join_team.cpp
	menu/label.cpp
	menu/main_menu.cpp
	menu/map_desc.cpp
	menu/map_details.cpp
	menu/map_picker.cpp
	menu/medals.cpp
	menu/menu.cpp
	menu/menu_config.cpp
	menu/menu_item.cpp
	menu/mode_panel.cpp
	menu/network_status.cpp
	menu/new_profile_dialog.cpp
	menu/notepad.cpp
	menu/number_control.cpp
	menu/options_menu.cpp
	menu/player_name_control.cpp
	menu/player_picker.cpp
	menu/popup_menu.cpp
	menu/profiles_menu.cpp
	menu/prompt.cpp
	menu/redefine_keys.cpp
	menu/scroll_list.cpp
	menu/shop.cpp
	menu/shop_item.cpp
	menu/simple_gamepad_setup.cpp
	menu/slider.cpp
	menu/start_server_menu.cpp
	menu/text_control.cpp
	menu/tooltip.cpp
	menu/upper_box.cpp
	menu/video_control.cpp
	menu/video_control_disabled.cpp

	net/client.cpp
	net/connection.cpp
	net/message.cpp
	net/monitor.cpp
	net/scanner.cpp
	net/server.cpp

	sound/mixer.cpp
	sound/ogg_ex.cpp
	sound/ogg_stream.cpp

	src/alarm.cpp
	src/animation_model.cpp
	src/base_object.cpp
	src/campaign.cpp
	src/cheater.cpp
	src/config.cpp
	src/console.cpp
	src/credits.cpp
	src/finder.cpp
	src/game.cpp
	src/game_monitor.cpp
	src/hud.cpp
	src/i18n.cpp
	src/logo.cpp
	src/netstats.cpp
	src/nickname.cpp
	src/notifying_xml_parser.cpp
	src/object.cpp
	src/player_manager.cpp
	src/player_slot.cpp
	src/player_state.cpp
	src/profiler.cpp
	src/registrar.cpp
	src/resource_manager.cpp
	src/rotating_object.cpp
	src/rt_config.cpp
	src/special_zone.cpp
	src/team.cpp
	src/var.cpp
	src/variants.cpp
	src/vehicle_traits.cpp
	src/version.cpp
	src/window.cpp
	src/world.cpp
	src/xml_parser.cpp
	src/zbox.cpp

	tmx/generator.cpp
	tmx/generator_object.cpp
	tmx/layer.cpp
	tmx/map.cpp
	tmx/tileset.cpp
	tmx/tileset_list.cpp
)

generate_export_header(btanks_engine
	EXPORT_FILE_NAME "${CMAKE_CURRENT_BINARY_DIR}/export_btanks.h"
	EXPORT_MACRO_NAME BTANKSAPI
)

set_source_files_properties(src/version.cpp PROPERTIES COMPILE_DEFINITIONS "VERSION=\"git\"")

target_compile_definitions(btanks_engine PUBLIC ENABLE_LUA)
target_sources(btanks_engine PRIVATE luaxx/state.cpp luaxx/lua_hooks.cpp)

target_include_directories(btanks_engine PUBLIC "${CMAKE_CURRENT_BINARY_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/src" "${LUA_INCLUDE_DIR}")
target_link_libraries(btanks_engine PUBLIC SDL::SDL mrt sdlx Ogg::ogg Vorbis::vorbisfile clunk ${LUA_LIBRARIES} Threads::Threads)

if (WIN32)
	target_link_libraries(btanks_engine PRIVATE Ws2_32 User32)
endif ()
