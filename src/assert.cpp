/*
	dbgtools - platform independent wrapping of "nice to have" debug functions.
	
	https://github.com/wc-duck/dbgtools

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
 */

#ifdef DBG_TOOLS_ASSERT_ENABLE
#include <dbgtools/assert.h>

#include <stdio.h>
#include <stdarg.h>

assert_callback_t g_assert_callback = 0x0;
void* g_assert_callback_data = 0x0;

void assert_register_callback( assert_callback_t callback, void* user_data )
{
	g_assert_callback = callback;
	g_assert_callback_data = user_data;
}

assert_action assert_call_trampoline(const char* file, unsigned int line, const char* cond)
{
	if( g_assert_callback != 0x0 )
		return g_assert_callback(cond, "", file, line, g_assert_callback_data);
	return ASSERT_ACTION_BREAK;
}

assert_action assert_call_trampoline(const char* file, unsigned int line, const char* cond, const char* fmt, ...)
{

	if( g_assert_callback == 0x0 )
		return ASSERT_ACTION_BREAK;
	char buffer[2048];
	va_list list;
	va_start( list, fmt );
	vsnprintf(buffer, 2048, fmt, list);
	va_end(list);
	buffer[2048 - 1] = 0;
	return g_assert_callback( cond, buffer, file, line, g_assert_callback_data );
}
#endif // DBG_TOOLS_ASSERT_ENABLE
