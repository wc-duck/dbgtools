--[[
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
--]]

BUILD_PATH = "local"

platform = "linux_x86_64"
if family == "windows" then
    platform = "winx64"
end
config   = "debug"

local settings = NewSettings()

local output_path = PathJoin( BUILD_PATH, PathJoin( config, platform ) )
local output_func = function(settings, path) return PathJoin(output_path, PathFilename(PathBase(path)) .. settings.config_ext) end
settings.cc.Output = output_func
settings.lib.Output = output_func
settings.link.Output = output_func

settings.cc.defines:Add("DBG_TOOLS_ASSERT_ENABLE")

if family ~= "windows" then
	settings.cc.flags:Add( "-Wconversion", "-Wextra", "-Wall", "-Werror", "-Wstrict-aliasing=2" )
	settings.link.flags:Add( '-rdynamic' )
else
	settings.link.flags:Add( "/NODEFAULTLIB:LIBCMT.LIB" );
	settings.link.libs:Add( "Dbghelp" );
	settings.cc.defines:Add("_ITERATOR_DEBUG_LEVEL=0")
end

settings.cc.includes:Add( 'include' )

local debugger_obj  = Compile( settings, 'src/debugger.cpp' )
local callstack_obj = Compile( settings, 'src/callstack.cpp' )
local assert_obj    = Compile( settings, 'src/assert.cpp' )

Compile( settings, 'test/test_static_assert.c' )
Compile( settings, 'test/test_static_assert_cpp.cpp' )

Link( settings, 'test_debugger',      debugger_obj,  Compile( settings, 'test/test_debugger_present.c' ) )
Link( settings, 'test_callstack',     callstack_obj, Compile( settings, 'test/test_callstack.c' ) )
Link( settings, 'test_callstack_cpp', callstack_obj, Compile( settings, 'test/test_callstack_cpp.cpp' ) )
Link( settings, 'test_assert',        assert_obj,    Compile( settings, 'test/test_assert.cpp' ) )
