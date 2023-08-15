// SPDX-FileCopyrightText: 2017-2021 Giovanni Dante Grazioli <deroad@libero.it>
// SPDX-License-Identifier: BSD-3-Clause

(function() { // lgtm [js/useless-expression]
	return {
		'6502': require('libdec/arch/6502'),
		'8051': require('libdec/arch/8051'),
		arm: require('libdec/arch/arm'),
		avr: require('libdec/arch/avr'),
		dalvik: require('libdec/arch/dalvik'),
		m68k: require('libdec/arch/m68k'),
		mips: require('libdec/arch/mips'),
		ppc: require('libdec/arch/ppc'),
		riscv: require('libdec/arch/riscv'),
		sh: require('libdec/arch/sh'),
		sparc: require('libdec/arch/sparc'),
		v850: require('libdec/arch/v850'),
		wasm: require('libdec/arch/wasm'),
		x86: require('libdec/arch/x86')
	};
});