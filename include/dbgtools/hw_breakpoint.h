/*
	dbgtools - platform independent wrapping of "nice to have" debug functions.

	version 0.1, october, 2013

	https://github.com/wc-duck/dbgtools

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

#ifndef DBGTOOLS_HWBREAKPOINT_INCLUDED
#define DBGTOOLS_HWBREAKPOINT_INCLUDED

/**
 * Breakpoint type used in hw_breakpoint_set()
 */
enum hw_breakpoint_type
{
	HW_BREAKPOINT_READ,
	HW_BREAKPOINT_WRITE,
	HW_BREAKPOINT_READWRITE,
};

/**
 * Error codes from hw_breakpoint_set()
 */
#define HW_BREAKPOINT_ERROR_INVALID_ARG   (-1) // invalid argument was passed to hw_breakpoint_set such as a bad size.
#define HW_BREAKPOINT_ERROR_OUT_OF_SLOTS  (-2) // to many hardware breakpoints are already set.
#define HW_BREAKPOINT_ERROR_UNKNOWN       (-3) // unknown error! ( please report bug-report or bug-fix :) )
#define HW_BREAKPOINT_ERROR_NOT_SUPPORTED (-4) // hardware breakpoints aren't supported on this platform.

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

/**
 * Set a hardware breakpoint at the specified memory-address that will trigger a SIGSEGV on access.
 * @param address to break on.
 * @param size of bytes to monitor, valid values are 1, 2, 4, 8
 * @param type type of hw-breakpoint.
 * @return value >= 0 on success, otherwise errorcode.
 */
int hw_breakpoint_set( void* address, unsigned int size, enum hw_breakpoint_type type );

/**
 * Clear a previously set hardware breakpoint set with hw_breakpoint_set().
 * @param hwbp value returned by hw_breakpoint_set().
 */
void hw_breakpoint_clear( int hwbp );

#ifdef __cplusplus
}
#endif  // __cplusplus

#if defined(__cplusplus)

/**
 * Utility class to handle scoped hw_breakpoint_set/hw_breakpoint_clear
 */
class hw_breakpoint_enable_scope
{
	int hwbp;

	hw_breakpoint_enable_scope( void* address, unsigned int size, enum hw_breakpoint_type type )
		: hwbp( hw_breakpoint_set( address, size, type ) )
	{
	}

	~hw_breakpoint_enable_scope()
	{
		if( hwbp >= 0 )
			hw_breakpoint_clear( hwbp );
	}
};

#endif // defined(__cplusplus)

#endif // DBGTOOLS_HWBREAKPOINT_INCLUDED

