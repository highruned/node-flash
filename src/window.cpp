#include <stdlib.h>
#include <stdio.h>

#include "gameswf/gameswf.h"

#include "base/utility.h"
#include "base/container.h"
#include "base/tu_file.h"
#include "base/tu_types.h"
#include "base/tu_timer.h"
#include "gameswf/gameswf_log.h"
#include "gameswf/gameswf_types.h"
#include "gameswf/gameswf_impl.h"
#include "gameswf/gameswf_root.h"
#include "gameswf/gameswf_freetype.h"
#include "gameswf/gameswf_player.h"

#include "gameswf/gameswf_as_classes/as_array.h"

#include "window.h"

static int	s_width = 0;
static int	s_height = 0;
static float s_scale = 1;
static int s_view_width = 0;
static int s_view_height = 0;
static int s_x0 = 0;
static int s_y0 = 0;
static Uint32	start_ticks = 0;
static Uint32	last_ticks = 0;
static tu_string flash_vars;
static int s_aa_level = 1;
static bool s_antialiased = false;
static bool s_allow_http = false;
static const char* infile = NULL;
static int s_bit_depth = 0;

static gameswf::gc_ptr<gameswf::player> player;
static gameswf::gc_ptr<gameswf::root>	m;

static array<mouse_event> s_mouse_event;

gameswf::sound_handler*	sound = NULL;

static void	message_log(const char* message)
// Process a log message.
{
	if (gameswf::get_verbose_parse())
	{
		fputs(message, stdout);
		fflush(stdout);
	}
}

static void	log_callback(bool error, const char* message)
// Error callback for handling gameswf messages.
{
	if (error)
	{
		// Log, and also print to stderr.
		message_log(message);
		fputs(message, stderr);
		fflush(stderr);
	}
	else
	{
		message_log(message);
	}
}

static tu_file*	file_opener(const char* url)
// Callback function.  This opens files for the gameswf library.
{
	assert(url);
	if (strncasecmp(url, "http://", 7) == 0)
	{
		const char* slash = strchr(url + 7, '/');
		if (slash == NULL)
		{
			return NULL;
		}
		
		char host[256];
		memset(host, 0, 256);
		strncpy(host, url + 7, slash - url - 7);
		
		int len = 0;
		const void* data = NULL;
		tu_string swfile;
		tu_string hisfile;
		
		swfile = "/start.swf";
		hisfile = "/his.txt";
		{
			tu_file fi(swfile.c_str(), "rb");
			len = fi.size();
			data = malloc(len);
			len = fi.read_bytes((void*)data, len);
		}
		
		// write swf
		{
			tu_file fi(swfile.c_str(), "wb");
			fi.write_bytes(data, len);
		}
		
		// update history
		
		// read history
		array<tu_string> his;
		{
			tu_file fi(hisfile.c_str(), "r");
			if (fi.get_error() == TU_FILE_NO_ERROR)
			{
				char buf[1024];
				while (fi.read_string(buf, 1024) > 1)	// !!!!
				{
					his.push_back(buf);
				}
			}
		}
		
		// test for same
		bool is_new = true;
		for (int i = 0; i < his.size(); i++)
		{
			if (his[i] == url + 7)
			{
				is_new = false;
				break;
			}
		}
		
		// append new item
		if (is_new)
		{
			his.push_back(url + 7);
		}
		
		// truncate to 20
		while (his.size() > 20)
		{
			his.remove(0);
		}
		
		// save history
		{
			tu_file fi(hisfile.c_str(), "w");
			for (int i = 0; i < his.size(); i++)
			{
				fi.write_string(his[i].c_str());
				fi.write_string("\n");
			}
		}
		//	printf("%s as %s\n", url, swfile.c_str());
		return new tu_file(swfile.c_str(), "rb");
	}
	return new tu_file(url, "rb");
}

static void	fs_callback(gameswf::character* movie, const char* command, const char* args)
// For handling notification callbacks from ActionScript.
{
	assert(movie);
	gameswf::gc_ptr<gameswf::player> player = movie->get_player();
	
	if (strcasecmp(command, "loadHistory") == 0)
	{
		
		tu_string hisfile;
		
		hisfile = "/his.txt";
		
		// for debugging
		//remove(hisfile.c_str());	
		
		array<tu_string> his;
		{
			tu_file fi(hisfile.c_str(), "r");
			if (fi.get_error() == TU_FILE_NO_ERROR)
			{
				char buf[1024];
				while (fi.read_string(buf, 1024) > 1)	// !!!!
				{
					his.push_back(buf);
				}
			}
		}
		
		gameswf::as_array* as_his = new gameswf::as_array(player);
		for (int i = 0; i < his.size(); i++)
		{
			as_his->push(his[i].c_str());
		}
		player->get_root_movie()->set_member("his", as_his);
	}
}

void	print_usage()
// Brief instructions.
{
	printf(
		   "gameswf_test_ogl -- a test player for the gameswf library.\n"
		   "\n"
		   "This program has been donated to the Public Domain.\n"
		   "See http://tulrich.com/geekstuff/gameswf.html for more info.\n"
		   "\n"
		   "usage: gameswf_test_ogl [options] movie_file.swf\n"
		   "\n"
		   "Plays a SWF (Shockwave Flash) movie, using OpenGL and the\n"
		   "gameswf library.\n"
		   "\n"
		   "options:\n"
		   "\n"
		   "  -h          Print this info.\n"
		   "  -c          Produce a core file instead of letting SDL trap it\n"
		   "  -d num      Number of milliseconds to delay in main loop\n"
		   "  -a <level>  Specify the antialiasing level (0,1,2,4,8,16,...)\n"
		   "  -v          Be verbose; i.e. print log messages to stdout\n"
		   "  -va         Be verbose about movie Actions\n"
		   "  -vp         Be verbose about parsing the movie\n"
		   "  -ml <bias>  Specify the texture LOD bias (float, default is -1)\n"
		   "  -p          Run full speed (no sleep) and log frame rate\n"
		   "  -1          Play once; exit when/if movie reaches the last frame\n"
		   "  -r <0|1|2>  0 disables rendering & sound (good for batch tests)\n"
		   "              1 enables rendering & sound (default setting)\n"
		   "              2 enables rendering & disables sound\n"
		   "  -t <sec>    Timeout and exit after the specified number of seconds\n"
		   "  -b <bits>   Bit depth of output window (16 or 32, default is 16)\n"
		   "  -n          Allow use of network to try to open resource URLs\n"
		   "  -u          Allow pass user variables to Flash\n"
		   "  -k          Disables cursor\n"
		   "  -w <w>x<h>  Specify the window size, for example 1024x768\n"
		   "  -f          Force realtime framerate\n"
		   "  -i          Grub bitmaps from swf file\n"
		   "\n"
		   "keys:\n"
		   "  CTRL-Q          Quit/Exit\n"
		   "  CTRL-W          Quit/Exit\n"
		   "  ESC             Quit/Exit\n"
		   "  CTRL-P          Toggle Pause\n"
		   "  CTRL-[ or kp-   Step back one frame\n"
		   "  CTRL-] or kp+   Step forward one frame\n"
		   "  CTRL-A          Toggle antialiasing\n"
		   "  CTRL-T          Debug.  Test the set_variable() function\n"
		   "  CTRL-G          Debug.  Test the get_variable() function\n"
		   "  CTRL-M          Debug.  Test the call_method() function\n"
		   "  CTRL-B          Toggle background color\n"
		   );
}

bool init_gameswf(int argc, char *argv[])
{
	gameswf::log_msg("5\n");
	for (int arg = 1; arg < argc; arg++)
	{
		if (argv[arg][0] == '-')
		{
			// Looks like an option.
			
			if (argv[arg][1] == 'h')
			{
				// Help.
				print_usage();
				exit(1);
			}
			if (argv[arg][1] == 'u')
			{
				arg++;
				if (arg < argc)
				{
					flash_vars = argv[arg];
				}
				else
				{
					fprintf(stderr, "-u arg must be followed string like myvar=x&myvar2=y and so on\n");
					print_usage();
					exit(1);
				}
				
			}
			else if (argv[arg][1] == 'w')
			{
				arg++;
				if (arg < argc)
				{
					s_width = atoi(argv[arg]);
					const char* x = strstr(argv[arg], "x");
					if (x)
					{
						s_height = atoi(x + 1);
					}
				}
				
				if (s_width <=0 || s_height <= 0)
				{
					fprintf(stderr, "-w arg must be followed by the window size\n");
					print_usage();
					exit(1);
				}
			}
			else if (argv[arg][1] == 'a')
			{
				// Set antialiasing on or off.
				arg++;
				if (arg < argc)
				{
					s_aa_level = atoi(argv[arg]);
					s_antialiased = s_aa_level > 0 ? true : false;
				}
				else
				{
					fprintf(stderr, "-a arg must be followed by the antialiasing level\n");
					print_usage();
					exit(1);
				}
			}
			else if (argv[arg][1] == 'b')
			{
				// Set default bit depth.
				arg++;
				if (arg < argc)
				{
					s_bit_depth = atoi(argv[arg]);
					if (s_bit_depth != 16 && s_bit_depth != 24 && s_bit_depth != 32)
					{
						fprintf(stderr, "Command-line supplied bit depth %d, but it must be 16, 24 or 32", s_bit_depth);
						print_usage();
						exit(1);
					}
				}
				else
				{
					fprintf(stderr, "-b arg must be followed by 16 or 32 to set bit depth\n");
					print_usage();
					exit(1);
				}
			}
			else if (argv[arg][1] == 'v')
			{
				// Be verbose; i.e. print log messages to stdout.
				if (argv[arg][2] == 'a')
				{
					// Enable spew re: action.
					player->verbose_action(true);
				}
				else if (argv[arg][2] == 'p')
				{
					// Enable parse spew.
					player->verbose_parse(true);
				}
				// ...
			}
			else if (argv[arg][1] == 'n')
			{
				s_allow_http = true;
			}
			else if (argv[arg][1] == 'i')
			{
				player->set_separate_thread(false);
				player->set_log_bitmap_info(true);
			}
		}
		else
		{
			infile = argv[arg];
		}
	}
	gameswf::log_msg("%s\n", infile);
	if (infile == NULL)
	{
		printf("no input file\n");
		print_usage();
		return false;
	}
	gameswf::log_msg("1\n");
	gameswf::register_file_opener_callback(file_opener);
	gameswf::register_fscommand_callback(fs_callback);
	if (gameswf::get_verbose_parse())
	{
		gameswf::register_log_callback(log_callback);
	}
	gameswf::log_msg("2\n");
	sound = gameswf::create_sound_handler_sdl();
	gameswf::set_sound_handler(sound);
	
	gameswf::render_handler*	render = gameswf::create_render_handler_ogl();
	gameswf::set_render_handler(render);
	
	gameswf::set_glyph_provider(gameswf::create_glyph_provider_tu());
	gameswf::log_msg("3\n");
	player = new gameswf::player();
	m = player->load_file(infile);
	if (m == NULL)
	{gameswf::log_msg("4\n");
		printf("can't open input file %s", infile);
		return false;
	}
	gameswf::log_msg("4^\n");
	
	return true;
}

void set_gameswf_window(int backingWidth, int backingHeight)
{	
	s_width = backingWidth;
	s_height = backingHeight;
	
	float scale_x = (float) s_width / m->get_movie_width();
	float scale_y = (float) s_height / m->get_movie_height();
	s_scale = fmin(scale_x, scale_y);
	
	// move movie to center
	s_view_width =  m->get_movie_width() * s_scale;
	s_view_height =  m->get_movie_height() * s_scale;
	s_x0 =  (s_width - s_view_width) >> 1;
	s_y0 =  (s_height - s_view_height) >> 1;
	
	start_ticks = tu_timer::get_ticks();
	last_ticks = start_ticks;
	
	printf("screen: %dx%d, movie: %dx%d\n", s_width, s_height, s_view_width, s_view_height);
	
}

void onMouseDown(float x, float y)
{
	// mouse move event
	mouse_event me;
	me.m_x = (x - s_x0) / s_scale;
	me.m_y = (y - s_y0) / s_scale;
	me.m_state = 0;
	s_mouse_event.push_back(me);
	
	// mouse down event
	me.m_state = 1;
	s_mouse_event.push_back(me);
}

void onMouseUp(float x, float y)
{
	// mouse up event
	mouse_event me;
	me.m_x = (x - s_x0) / s_scale;
	me.m_y = (y - s_y0) / s_scale;
	me.m_state = 0;
	s_mouse_event.push_back(me);
}

void onMouseMove(float x, float y)
{
	// mouse move event
	mouse_event me;
	me.m_x = (x - s_x0) / s_scale;
	me.m_y = (y - s_y0) / s_scale;
	me.m_state = 1;
	s_mouse_event.push_back(me);
}

void advance_gameswf()
{
	Uint32	ticks = tu_timer::get_ticks();
	int	delta_ticks = ticks - last_ticks;
	float	delta_t = delta_ticks / 1000.f;
	last_ticks = ticks;
	gameswf::log_msg("6\n");
	m->set_display_viewport(s_x0, s_y0, s_view_width, s_view_height);
	gameswf::log_msg("6\n");
	m->set_background_alpha(1.0f);
	gameswf::log_msg("6\n");
	if (s_mouse_event.size() > 0)
	{gameswf::log_msg("6\n");
		//	printf("notify= %d %d %d %d\n", s_mouse_event.size(), s_mouse_event[0].m_x, s_mouse_event[0].m_y, s_mouse_event[0].m_state);	
		m->notify_mouse_state(s_mouse_event[0].m_x, s_mouse_event[0].m_y, s_mouse_event[0].m_state);
		s_mouse_event.remove(0);
	}
	gameswf::log_msg("6\n");
	Uint32 t_advance = tu_timer::get_ticks();
	m->advance(delta_t);
	sound->advance(delta_t);
	t_advance = tu_timer::get_ticks() - t_advance;
	gameswf::log_msg("6\n");
	Uint32 t_display = tu_timer::get_ticks();
	m->display();
	t_display = tu_timer::get_ticks() - t_display;
	gameswf::log_msg("6\n");
	// for perfomance testing
	//printf("advance time: %d, display time %d\n", t_advance, t_display);
	
}

