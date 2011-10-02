# Flash

Library which runs .swf Flash files natively in Node.js. Inspired by Scaleform (embedded hardware-accelerated vector rending engine). Currently only supports SDL. 

## WIP

I'm not sure what I want to do with this. I just felt like laying it out.  
Warning: Not entirely working, especially on Mac OS X (issues with Cocoa/SDL). Doesn't spawn the rendering window most of the time. Need to fix it up.

## Install

1) Download and install OpenGL (included in Xcode on Mac OS X).  
2) Download and install SDL into your compiler's global library directory (/usr/lib) - required for linking.  
On Mac OS X using Homebrew:  
`brew install sdl`
`brew install jpeg`

1) Clone the repository.  
2) Go to the directory with flash library.  
3) Execute `node-waf configure build`.  
4) Get module from `./build/default/flash.node`.  
  
From the repository directory, you can use `var flash = require("build/default/flash");`  

You can then run `npm install .` and use `var flash = require("flash");` from any path.

## Status

Compiled on Mac. Still converting the `gameswf` library. Currently stuck at spawning an OpenGL window. Trying on Linux.

## API
### Functions
`flash.start(filename)` - Starts the interpreter.  
`flash.run()` - Moves the interpreter forward. Must be looped, with for instance setTimeout.  
`flash.stop()` - Stops the interpreter.  
### Classes
`flash.window()`

## Usage
Please see the example in `test/window.js`  

	var flash = require('flash');
	var window = new flash.window();

## Contribute

Generate .cpp list with: `find . -name \*.cpp -print`  
Remove files: 

	./lib/base/Stackwalker.cpp
	./lib/base/test_ogl.cpp
	./lib/gameswf/gameswf_render_handler_d3d.cpp
	./lib/gameswf/gameswf_render_handler_xbox.cpp
	./lib/gameswf/gameswf_render_handler_ogles.cpp
	./lib/gameswf/gameswf_sound_handler_openal.cpp
	./lib/gameswf/gameswf_parser.cpp
	./lib/gameswf/gameswf_processor.cpp
	./lib/gameswf/gameswf_test_ogl.cpp

## TODO

* Fix window bug (on Mac OS X).
* Check cross-compatibility.
* Clean up dependencies.


## Credit
gameswf by [Thatcher Ulrich](http://www.ijg.org/)  
libjpeg by [Independent JPEG Group](http://www.ijg.org/)  
node-flash is public domain  