var assert = require('assert');
var flash = require('../bin/build/Release/flash');

var window = flash.start(__dirname + '/start.swf');

var abc = function() {
	flash.run();
	
	setTimeout(abc, 1); // let's do it again
};

abc();