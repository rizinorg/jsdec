// SPDX-FileCopyrightText: 2017-2021 Giovanni Dante Grazioli <deroad@libero.it>
// SPDX-License-Identifier: BSD-3-Clause

(function() { // lgtm [js/useless-expression]
	return {
		core: require('libdec/core'),
		archs: require('libdec/archs'),
		context: require('libdec/context'),
		supported: function() {
			return 'Supported architectures:\n    ' + Object.keys(this.archs).join(', ');
		}
	};
});