// SPDX-FileCopyrightText: 2019-2023 Giovanni Dante Grazioli <deroad@libero.it>
// SPDX-License-Identifier: BSD-3-Clause

import JSONex from './JSONex.js';
import Long from './long.js';

function rzcustom(value, regex, function_fix) {
	var x = rizin.command(value) || "";
	if (regex) {
		x = x.replace(regex, '');
	}
	return function_fix ? function_fix(x.trim()) : x.trim();
}

function rzstr(value, multiline) {
	var x = rizin.command(value) || "";
	if (multiline) {
		x = x.replace(/\n/g, '');
	}
	return x.trim();
}

function rzjson(m, def) {
	var x = rzstr(m, true);
	try {
		return x.length > 0 ? JSONex.parse(x) : def;
	} catch(e){}
	return def;
}

function rzint(value, def) {
	var x = rzstr(value);
	if (x != '') {
		try {
			return parseInt(x);
		} catch (e) {}
	}
	return def || 0;
}

function rzlong(value, def) {
	var x = rzstr(value);
	if (x != '') {
		try {
			return Long.from(x, true);
		} catch (e) {}
	}
	return def || Long.UZERO;
}

function rzbool(value) {
	var x = rzstr(value);
	return x == 'true' || x == '1';
}

export default {
	custom: rzcustom,
	string: rzstr,
	json: rzjson,
	int: rzint,
	long: rzlong,
	bool: rzbool,
};
