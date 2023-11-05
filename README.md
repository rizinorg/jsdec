![Build Status](https://github.com/rizinorg/jsdec/workflows/continuous-tests/badge.svg)
[![Language grade: JavaScript](https://img.shields.io/lgtm/grade/javascript/g/rizinorg/jsdec.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/rizinorg/jsdec/context:javascript)
[![Total alerts](https://img.shields.io/lgtm/alerts/g/rizinorg/jsdec.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/rizinorg/jsdec/alerts/)
[![License](https://img.shields.io/badge/License-BSD_3--Clause-blue.svg)](https://opensource.org/licenses/BSD-3-Clause)

![jsdec](https://raw.githubusercontent.com/rizinorg/jsdec/master/.github/logo.png)

Converts asm to pseudo-C code.

# Software Requirements

Requires [rizin](https://github.com/rizinorg/rizin).

# Install in home folder (or in other paths)

If you want to install in the HOME folder or in other non standard paths, just
set the `prefix` to a different value
 - Run ``meson setup build --prefix=~/.local``
 - Run `ninja -C build install`

# Install system wide

Follow the following steps to install jsdec
 - clone this repository
 - Run `meson setup build --prefix=/usr` to create the build folder
 - Run `ninja -C build install` to build the shared library and to install it 

# Usage

* Open your file with rizin
* Analyze the function you want to disassemble (`af`)
* Run the plugin via `pdd`

# Arguments

```
[0x00000000]> pdd?
Usage: pdd[?]   # Core plugin for jsdec
| pdd  # decompile current function
| pddt # lists the supported architectures
| pdda # decompile current function with side assembly
| pddA # decompile current function with annotation output
| pddb # decompile current function but show only scopes
| pddc # decompiled code is returned to rizin as 'file:line code' (via CL)
| pddf # decompile all functions
| pddi # generate issue data
| pddj # decompile current function as json
| pddo # decompile current function side by side with offsets
```

# Evaluable vars

You can use these in your `.rizinrc` file.

```
jsdec.asm           | if true, shows pseudo next to the assembly.
jsdec.blocks        | if true, shows only scopes blocks.
jsdec.casts         | if false, hides all casts in the pseudo code.
jsdec.debug         | do not catch exceptions in jsdec.
jsdec.paddr         | if true, all xrefs uses physical addresses compare.
jsdec.slow          | if true load all the data before to avoid multirequests to rizin.
jsdec.xrefs         | if true, shows all xrefs in the pseudo code.
e scr.html          | outputs html data instead of text.
e scr.color         | enables syntax colors.
```

# Report an Issue

* Open your file with rizin
* Analyze the function you want to disassemble (`af`)
* Give the data to the plugin via `pddi` or `pdd --issue`
* Insert the JSON returned by the previous command into the issue (you can also upload the output)

# Supported Arch

    6502 (experimental)
    8051
    ARM 16/32/64 bit
    AVR
    dalvik
    m68k
    MIPS
    PowerPC 32/64 bit (VLE included)
    RISC-V
    Sparc
    SuperH (experimental)
    v850
    WebAssembly (experimental)
    x86/x64

# Developing on jsdec

[Read DEVELOPERS.md](https://github.com/rizinorg/jsdec/blob/master/DEVELOPERS.md)

