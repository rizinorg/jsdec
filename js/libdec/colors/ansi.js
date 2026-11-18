// SPDX-FileCopyrightText: 2018-2023 Giovanni Dante Grazioli <deroad@libero.it>
// SPDX-License-Identifier: BSD-3-Clause

var __colors = {
	black: [30, 39],
	red: [31, 39],
	green: [32, 39],
	yellow: [33, 39],
	blue: [34, 39],
	magenta: [35, 39],
	cyan: [36, 39],
	white: [37, 39],
	gray: [90, 39],
};

function rgbnum(rgb, n) {
	if (n > rgb.length) {
		return '0';
	}
	return rgb[n].toString();
}

var Color = function(name) {
	var fn = (function(){
		var that = function(x) {
			return that.open + x + that.close;
		};
		return that;
	})();
	if (name['rgb']) {
		var rgb = name['rgb'];
		const str = '38;2;' + rgbnum(rgb, 0) + ';' + rgbnum(rgb, 1) + ';' + rgbnum(rgb, 2);
		fn.open = '\u001b[' + str + 'm';
		fn.close = '\u001b[39m';
	} else {
		if (!__colors[name]) {
			throw new Error('Invalid name: ' + name);
		}
		fn.open = '\u001b[' + __colors[name][0] + 'm';
		fn.close = '\u001b[' + __colors[name][1] + 'm';
	}
	return fn;
};

Color.make = function(theme) {
	var g = {};
	for (var key in theme) {
		g[key] = Color(theme[key]);
	}
	return g;
};
export default Color;