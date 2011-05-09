import sys
import TaskGen

srcdir = "."
blddir = "bin/build"
VERSION = "0.0.1"

def set_options(opt):
	opt.tool_options("compiler_cxx")

def configure(conf):
	conf.check_tool("compiler_cxx")
	conf.check_tool("node_addon")

	conf.env.append_value('CCFLAGS', ['-O3'])

def build(bld):
	if sys.platform == 'darwin':
		TaskGen.task_gen.mappings['.m'] = TaskGen.task_gen.mappings['.cpp']
		TaskGen.task_gen.mappings['.mm'] = TaskGen.task_gen.mappings['.cc']

	print bld.new_task_gen(
		features = ['cxx', 'cshlib', 'node_addon'],
		target = 'flash',
		libpath = ['/usr/lib'],
		lib = ['pthread', 'SDL', 'SDLmain', 'jpeg'], #, 'glu32', 'opengl32', 'SDL'],
		cxxflags = ['-ObjC++', '-Wall', '-ansi', '-lz'],
		#cxxflags = ['-Wall', '-ansi', '-lz', '-lSDLmain', '-lSDL', '-lSDL_sound -lSDL_image -lSDL_net', '-lglu32', '-lopengl32', '-c99'],
		includes = [bld.srcnode.abspath(), bld.srcnode.abspath() + '/lib', bld.srcnode.abspath() + '/src'],
		uselib = ['SDL', 'GL'],
		source = """
./lib/base/component_hackery.cpp
./lib/base/configvars.cpp
./lib/base/container.cpp
./lib/base/demo.cpp
./lib/base/ear_clip_triangulate_float.cpp
./lib/base/ear_clip_triangulate_sint16.cpp
./lib/base/file_util.cpp
./lib/base/image.cpp
./lib/base/image_filters.cpp
./lib/base/jpeg.cpp
./lib/base/logger.cpp
./lib/base/membuf.cpp
./lib/base/ogl.cpp
./lib/base/png_helper.cpp
./lib/base/postscript.cpp
./lib/base/triangulate_float.cpp
./lib/base/triangulate_sint32.cpp
./lib/base/tu_file.cpp
./lib/base/tu_file_SDL.cpp
./lib/base/tu_gc_singlethreaded_marksweep.cpp
./lib/base/tu_gc_test.cpp
./lib/base/tu_loadlib.cpp
./lib/base/tu_random.cpp
./lib/base/tu_timer.cpp
./lib/base/tu_types.cpp
./lib/base/utf8.cpp
./lib/base/utility.cpp
./lib/base/zlib_adapter.cpp
./lib/gameswf/gameswf_abc.cpp
./lib/gameswf/gameswf_action.cpp
./lib/gameswf/gameswf_render_handler_ogl.cpp
./lib/gameswf/gameswf_as_classes/as_array.cpp
./lib/gameswf/gameswf_as_classes/as_boolean.cpp
./lib/gameswf/gameswf_as_classes/as_broadcaster.cpp
./lib/gameswf/gameswf_as_classes/as_class.cpp
./lib/gameswf/gameswf_as_classes/as_color.cpp
./lib/gameswf/gameswf_as_classes/as_color_transform.cpp
./lib/gameswf/gameswf_as_classes/as_date.cpp
./lib/gameswf/gameswf_as_classes/as_event.cpp
./lib/gameswf/gameswf_as_classes/as_flash.cpp
./lib/gameswf/gameswf_as_classes/as_geom.cpp
./lib/gameswf/gameswf_as_classes/as_global.cpp
./lib/gameswf/gameswf_as_classes/as_key.cpp
./lib/gameswf/gameswf_as_classes/as_loadvars.cpp
./lib/gameswf/gameswf_as_classes/as_math.cpp
./lib/gameswf/gameswf_as_classes/as_matrix.cpp
./lib/gameswf/gameswf_as_classes/as_mcloader.cpp
./lib/gameswf/gameswf_as_classes/as_mouse.cpp
./lib/gameswf/gameswf_as_classes/as_mouse_event.cpp
./lib/gameswf/gameswf_as_classes/as_netconnection.cpp
./lib/gameswf/gameswf_as_classes/as_netstream.cpp
./lib/gameswf/gameswf_as_classes/as_number.cpp
./lib/gameswf/gameswf_as_classes/as_point.cpp
./lib/gameswf/gameswf_as_classes/as_selection.cpp
./lib/gameswf/gameswf_as_classes/as_sharedobject.cpp
./lib/gameswf/gameswf_as_classes/as_sound.cpp
./lib/gameswf/gameswf_as_classes/as_string.cpp
./lib/gameswf/gameswf_as_classes/as_textformat.cpp
./lib/gameswf/gameswf_as_classes/as_transform.cpp
./lib/gameswf/gameswf_as_classes/as_xml.cpp
./lib/gameswf/gameswf_as_classes/as_xmlsocket.cpp
./lib/gameswf/gameswf_as_sprite.cpp
./lib/gameswf/gameswf_avm2.cpp
./lib/gameswf/gameswf_avm2_jit.cpp
./lib/gameswf/gameswf_button.cpp
./lib/gameswf/gameswf_canvas.cpp
./lib/gameswf/gameswf_character.cpp
./lib/gameswf/gameswf_disasm.cpp
./lib/gameswf/gameswf_dlist.cpp
./lib/gameswf/gameswf_environment.cpp
./lib/gameswf/gameswf_filters.cpp
./lib/gameswf/gameswf_font.cpp
./lib/gameswf/gameswf_fontlib.cpp
./lib/gameswf/gameswf_freetype.cpp
./lib/gameswf/gameswf_function.cpp
./lib/gameswf/gameswf_impl.cpp
./lib/gameswf/gameswf_jit.cpp
./lib/gameswf/gameswf_jit_opcode.cpp
./lib/gameswf/gameswf_listener.cpp
./lib/gameswf/gameswf_log.cpp
./lib/gameswf/gameswf_morph2.cpp
./lib/gameswf/gameswf_movie_def.cpp
./lib/gameswf/gameswf_mutex.cpp
./lib/gameswf/gameswf_object.cpp
./lib/gameswf/gameswf_player.cpp
./lib/gameswf/gameswf_render.cpp
./lib/gameswf/gameswf_root.cpp
./lib/gameswf/gameswf_shape.cpp
./lib/gameswf/gameswf_sound.cpp
./lib/gameswf/gameswf_sound_handler_sdl.cpp
./lib/gameswf/gameswf_sprite.cpp
./lib/gameswf/gameswf_sprite_def.cpp
./lib/gameswf/gameswf_stream.cpp
./lib/gameswf/gameswf_styles.cpp
./lib/gameswf/gameswf_tesselate.cpp
./lib/gameswf/gameswf_text.cpp
./lib/gameswf/gameswf_tools.cpp
./lib/gameswf/gameswf_types.cpp
./lib/gameswf/gameswf_value.cpp
./lib/gameswf/gameswf_video_impl.cpp
./src/main.cpp
./src/window.cpp
"""
	)
