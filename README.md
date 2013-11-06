# About:
Platform independent wrapping of "nice to have" debug functions.

callstack.h - implements capturing of callstack/backtrace + translation of captured symbols into name, file, line and offset.
debugger.h  - implements debugger_present to check if a debugger is attached to the process.

# Design:
The files are designed to be able to be used by them self, only header and src should be needed by the user and all files
should work by them self and compile with all common compilers without to many specific fixes.

# Notes:
MSVC      - callstack_symbols() require linking against Dbghelp.lib.
GCC/Clang - callstack_symbols() require -rdynamic to be sepcified as link-flag to get valid symbols.

# Licence:

```
dbgtools - platform independent wrapping of "nice to have" debug functions.

version 0.1, october, 2013

Copyright (C) 2013- Fredrik Kihlander

This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.

Fredrik Kihlander
```
