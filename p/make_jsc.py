# SPDX-FileCopyrightText: 2021 Giovanni Dante Grazioli <deroad@libero.it>
# SPDX-License-Identifier: BSD-3-Clause

import sys
import glob
import os
import json

def const_var_name(name):
	return 'jsc_' + name.replace(os.sep, '_').replace('.', '_').replace('-', '_')

def main(argc, argv):
	if argc != 2:
		print("usage: {} <path/to/js/files>".format(argv[0]))
		sys.exit(1)
	path = os.path.join(argv[1])
	path_len = len(path)

	js_files = glob.glob(os.path.join(path, '**/*.js'), recursive=True)
	js_files.remove(os.path.join(path, 'jsdec-test.js'))

	for file in js_files:
		vname = const_var_name(file.replace(path, ""))
		code = ''
		count = 0
		with open(file, "rb") as f:
			raw = f.read()
			for byte in raw:
				if count > 0 and count % 32 == 0:
					code += "\n\t"
				code += "{}, ".format(byte)
				count += 1
		print('const unsigned char ' + vname + '[' + str(count + 1) + '] = {\n\t' + code + ' 0\n};\n')

	print('\n#define RZ_JSC_SIZE ({})\n\n'.format(len(js_files)))
	print('const RzJSC rz_jsc_file[RZ_JSC_SIZE] = {')
	for file in js_files:
		name = file.replace(path, "").replace(os.sep, '/')
		if name.startswith("/"):
			name = name[1:]
		vname = const_var_name(file.replace(path, ""))
		print('\t{ .name = "' + name + '", .code = (const char *)' + vname + ' },')
	print('};')

if __name__ == '__main__':
	main(len(sys.argv), sys.argv)
