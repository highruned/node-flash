var assert = require('assert');
var flash = require("../bin/build/default/flash");

var window = flash.start('/Users/eric/Dropbox/repo/node-flash/test/start.swf');

var abc = function() {
	flash.run();
	setTimeout(abc, 1);
};

abc();