#include <node.h>
#include <node_events.h>
#include <ev.h>
#include <eio.h>
#include <fcntl.h>
#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>
#include <iostream>
#include <stdio.h>

#include "main.h"
#include "window.h"

#include "gameswf/gameswf.h"
#include "gameswf/gameswf_log.h"

using namespace gameswf;

bool done = 0;
bool started = 0;
const int RUN_GAME_LOOP = 1;
SDL_Surface* screen;

class application;

struct loop_callback_type {
	void operator() (application* ga) {
		log_msg("-");
	}
} loop_callback;


Uint32 loop_timer(Uint32 interval, void* param) {
	SDL_Event event;
	
	event.type = SDL_USEREVENT;
	event.user.code = RUN_GAME_LOOP;
	event.user.data1 = &loop_callback;
	event.user.data2 = param;
	
	SDL_PushEvent(&event);
	
	return interval;
}

class application {
public:
	void init() {
		SDL_TimerID timer;
		timer = SDL_AddTimer(20, &loop_timer, this);
	}

	void run() {
		log_msg(".");
		
		advance_gameswf();
	}
	
	void event_loop(void) {
		SDL_Event event;
		
		while((!done) && (SDL_WaitEvent(&event))) {
			switch(event.type) {
				case SDL_ACTIVEEVENT:
		      if(event.active.state & SDL_APPACTIVE) {
						if(event.active.gain) {
						  printf("App activated\n");
						} else {
						  printf("App iconified\n");
						}
		      }
				break;
			
				case SDL_USEREVENT:
					handle_user_event(&event);
				break;
					
				case SDL_KEYDOWN:
					// v8::Handle any key presses here.
				break;
	
				case SDL_MOUSEBUTTONDOWN:
					// v8::Handle mouse clicks here.
					printf("The %s key was pressed!\n",
                   SDL_GetKeyName(event.key.keysym.sym));
                   
		      Uint8 *keys;
		
		      keys = SDL_GetKeyState(NULL);
		      if(keys[SDLK_ESCAPE] == SDL_PRESSED) {
						printf("Bye...\n");
						
						done = true;
		      }
				break;
				
        case SDL_MOUSEMOTION:
            //g_MouseX = event.motion.x;
            //g_MouseY = event.motion.y;
        break;
        
				case SDL_QUIT:
					done = true;
				break;
					
				default:
				break;
			}
		}
	}
	
	void handle_user_event(SDL_Event* event) {
		switch (event->user.code) {
			case RUN_GAME_LOOP:
				break;
				
			default:
				break;
		}
	}
};



class window : node::EventEmitter {
public:
		static v8::Handle<v8::Value> New(const v8::Arguments& args) {
				window* w1 = new window;
				w1->Wrap(args.This());
		
				return args.This();
		}
	
		static v8::Handle<v8::Value> listen(const v8::Arguments& args) {

		}
};



#ifdef OS_MAC
/* The main class of the application, the application's delegate */
@implementation SDLMain

static NSString *getApplicationName(void)
{
    const NSDictionary *dict;
    NSString *appName = 0;

    /* Determine the application name */
    dict = (const NSDictionary *)CFBundleGetInfoDictionary(CFBundleGetMainBundle());
    if (dict)
        appName = [dict objectForKey: @"CFBundleName"];
    
    if (![appName length])
        appName = [[NSProcessInfo processInfo] processName];

    return appName;
}

static void setApplicationMenu(void)
{
    /* warning: this code is very odd */
    NSMenu *appleMenu;
    NSMenuItem *menuItem;
    NSString *title;
    NSString *appName;
    
    appName = getApplicationName();
    appleMenu = [[NSMenu alloc] initWithTitle:@""];
    
    /* Add menu items */
    title = [@"About " stringByAppendingString:appName];
    [appleMenu addItemWithTitle:title action:@selector(orderFrontStandardAboutPanel:) keyEquivalent:@""];

    [appleMenu addItem:[NSMenuItem separatorItem]];

    title = [@"Hide " stringByAppendingString:appName];
    [appleMenu addItemWithTitle:title action:@selector(hide:) keyEquivalent:@"h"];

    menuItem = (NSMenuItem *)[appleMenu addItemWithTitle:@"Hide Others" action:@selector(hideOtherApplications:) keyEquivalent:@"h"];
    [menuItem setKeyEquivalentModifierMask:(NSAlternateKeyMask|NSCommandKeyMask)];

    [appleMenu addItemWithTitle:@"Show All" action:@selector(unhideAllApplications:) keyEquivalent:@""];

    [appleMenu addItem:[NSMenuItem separatorItem]];

    title = [@"Quit " stringByAppendingString:appName];
    [appleMenu addItemWithTitle:title action:@selector(terminate:) keyEquivalent:@"q"];

    
    /* Put menu into the menubar */
    menuItem = [[NSMenuItem alloc] initWithTitle:@"" action:nil keyEquivalent:@""];
    [menuItem setSubmenu:appleMenu];
    [[NSApp mainMenu] addItem:menuItem];

    /* Tell the application object that this is now the application menu */
    [NSApp setAppleMenu:appleMenu];

    /* Finally give up our references to the objects */
    [appleMenu release];
    [menuItem release];
}


/* Create a window menu */
static void setupWindowMenu(void)
{
    NSMenu      *windowMenu;
    NSMenuItem  *windowMenuItem;
    NSMenuItem  *menuItem;

    windowMenu = [[NSMenu alloc] initWithTitle:@"Window"];
    
    /* "Minimize" item */
    menuItem = [[NSMenuItem alloc] initWithTitle:@"Minimize" action:@selector(performMiniaturize:) keyEquivalent:@"m"];
    [windowMenu addItem:menuItem];
    [menuItem release];
    
    /* Put menu into the menubar */
    windowMenuItem = [[NSMenuItem alloc] initWithTitle:@"Window" action:nil keyEquivalent:@""];
    [windowMenuItem setSubmenu:windowMenu];
    [[NSApp mainMenu] addItem:windowMenuItem];
    
    /* Tell the application object that this is now the window menu */
    [NSApp setWindowsMenu:windowMenu];

    /* Finally give up our references to the objects */
    [windowMenu release];
    [windowMenuItem release];
}

@end
#endif



v8::Handle<v8::Value>
start(const v8::Arguments& args) {
	#ifdef OS_MAC
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
	#endif
	
	v8::HandleScope scope;
	v8::String::AsciiValue abc(args[0]->ToString());

	
	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	
	Uint32 flags;
	
	flags = SDL_OPENGL;//SDL_SWSURFACESDL_OPENGL | SDL_HWPALETTE; // | SDL_FULLSCREEN;
	screen = SDL_SetVideoMode(1024, 768, 0, flags);
	
	SDL_WM_SetCaption("Flash", NULL);
	
	SDL_GL_SwapBuffers();
	
	application ga;
	ga.init();
	
	char* argv[2];
	argv[0] = NULL;
	argv[1] = *abc;

	log_msg("5^^");
	init_gameswf(2, argv);
	log_msg("7");
	log_msg("8");
	
	v8::Handle<v8::String> result = v8::String::New("Success");
	
	#ifdef OS_MAC
	//started = 1;
  SDLMain* sdlMain;

  /* Ensure the application object is initialised */
  [NSApplication sharedApplication];

  /* Set up the menubar */
  [NSApp setMainMenu:[[NSMenu alloc] init]];
  setApplicationMenu();
  setupWindowMenu();

  /* Create SDLMain and make it the app delegate */
  sdlMain = [[SDLMain alloc] init];
  [NSApp setDelegate:sdlMain];
  
  /* Start the main event loop */
  [NSApp run];
  
  [sdlMain release];
  [pool release];
  #endif
	
	return scope.Close(result);
}


v8::Handle<v8::Value> 
run(const v8::Arguments& args) {
	#ifdef OS_MAC
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
	#endif
	
	v8::HandleScope scope;
	
	if(started) {
		SDL_Event event;
		
		while(SDL_PollEvent(&event)) {
	 		switch(event.type) {
	 			case SDL_USEREVENT: {
	 				application* ga = static_cast<application*>(event.user.data2);
	 				ga->run();
	 				break;
	 			}
	 		}
	 		
	 		SDL_Flip(screen);
      SDL_Delay(1000 / 25);
	 	}
	}
 	
	#ifdef OS_MAC
	[pool release];
	#endif
	
	return scope.Close(v8::Boolean::New(true));
}

v8::Handle<v8::Value> 
stop(const v8::Arguments& args) {
	v8::HandleScope scope;

	v8::String::Utf8Value a(args[0]->ToString());
	v8::String::Utf8Value b(args[1]->ToString());
	
	v8::Handle<v8::Boolean> result = v8::Boolean::New(true);
	
	return scope.Close(result);
}

void initialize(v8::Handle<v8::Object> target) {
	#ifdef OS_MAC
	NSApplicationLoad();
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
	#endif
	
	v8::HandleScope scope;
	
	target->Set(v8::String::New("start"), v8::FunctionTemplate::New(start)->GetFunction());
	target->Set(v8::String::New("stop"), v8::FunctionTemplate::New(stop)->GetFunction());
	target->Set(v8::String::New("run"), v8::FunctionTemplate::New(run)->GetFunction());
		
	v8::Local<v8::FunctionTemplate> t = v8::FunctionTemplate::New(window::New);
	
	t->InstanceTemplate()->SetInternalFieldCount(1);
	t->Inherit(node::EventEmitter::constructor_template);

	NODE_SET_PROTOTYPE_METHOD(t, "listen", window::listen);

	target->Set(v8::String::NewSymbol("window"), t->GetFunction());
	
	#ifdef OS_MAC
	[pool release];
	#endif
}

NODE_MODULE(flash, initialize);