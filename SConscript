import sys

Import('env')
Import('version')
Import('revision')
Import('debug')
Import('sigc_lib')
Import('al_lib')

env = env.Copy()

venv = env.Copy()
venv.Append(CPPDEFINES=['VERSION="\\"' + version + '\\""'])
venv.Append(CPPDEFINES=['REVISION=%d' % revision])


vobj = venv.Object('src/version.cpp')
bt_sources = 	[
	'src/alarm.cpp', 'src/base_object.cpp', 'src/notifying_xml_parser.cpp',
	
	'src/player_manager.cpp',
	
	'objects/bullet.cpp', 'objects/explosion.cpp', 'objects/single_pose.cpp',
	'objects/tank.cpp', 'objects/shilka.cpp', 'objects/launcher.cpp', 'objects/ai_tank.cpp',
	'objects/traffic_lights.cpp', 'objects/missiles_in_vehicle.cpp', 'objects/missile.cpp',
	'objects/corpse.cpp', 'objects/item.cpp', 'objects/mine.cpp', 'objects/dirt.cpp', 
	'objects/damage.cpp', 'objects/helicopter.cpp', 'objects/paratrooper.cpp', 'objects/kamikaze.cpp',
	'objects/machinegunner.cpp', 'objects/destructable_object.cpp', 'objects/submarine.cpp', 'objects/train.cpp',
	'objects/trooper.cpp', 'objects/fakemod.cpp', 'objects/car.cpp', 'objects/tooltip.cpp', 
	'objects/ai_launcher.cpp', 'objects/vehicle_traits.cpp', 'objects/barrack.cpp', 'objects/watchtower.cpp',
	'objects/cannon.cpp', 'objects/boat.cpp', 
	
	'src/player_state.cpp', 
	'controls/joyplayer.cpp', 'controls/keyplayer.cpp', 'controls/external_control.cpp', 'controls/mouse_control.cpp', 

	'src/object.cpp', 'src/animation_model.cpp', 
	'src/resource_manager.cpp', 'src/world.cpp',
	'tmx/map.cpp', 'tmx/layer.cpp', 
	'src/main.cpp', 'src/var.cpp', 'src/config.cpp', 
	
	'src/player_slot.cpp', 'src/hud.cpp', 'src/game.cpp',  'src/window.cpp', 
	'src/credits.cpp', 'src/cheater.cpp', 

	vobj
	]
	
vorbis = 'vorbisfile'
if debug and sys.platform == "win32": 
	vorbis = 'vorbisfile_d'

#fanncxx

bt_libs = ['bt_ai', 'bt_sound', 'bt_net', 'bt_menu', 'sdlx', 'mrt', sigc_lib, 'SDL_ttf', 'SDL_image', 'SDL', 'expat', 'z', vorbis, al_lib, 'alut']
if sys.platform == "win32":
	bt_rc = env.RES('src/bt.rc')
	bt_sources.append(bt_rc)

	bt_libs[0:0] = ['SDLmain']
	bt_libs.append('Ws2_32')
	bt_libs.append('opengl32')
	bt_libs.append('user32')
	#bt_libs.append('gdi32')
else: 
#	pass
	bt_libs.append('GL')

bt = env.Program('bt', bt_sources, LIBS=bt_libs, RPATH=['.'])
Install('#', bt)
