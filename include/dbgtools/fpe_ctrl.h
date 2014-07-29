/*
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
 */

#ifndef DBGTOOLS_FPE_CTRL_INCLUDED
#define DBGTOOLS_FPE_CTRL_INCLUDED

#if defined( __cplusplus )
extern "C" {
#endif

#if defined( _MSC_VER )
#include <float.h>
#else
#include <fenv.h>
#endif

/**
 * Enumeration used together with fpe_* functions.
 */
enum fp_exception_type
{
#if defined( _MSC_VER )
	fp_exception_invalid     = _EM_INVALID,    ///< ...
	fp_exception_div_by_zero = _EM_ZERODIVIDE, ///< ...
	fp_exception_overflow    = _EM_OVERFLOW,   ///< ...
	fp_exception_underflow   = _EM_UNDERFLOW,  ///< ...
	fp_exception_inexact     = _EM_INEXACT,    ///< ...
#else
	fp_exception_invalid     = FE_INVALID,   ///< ...
	fp_exception_div_by_zero = FE_DIVBYZERO, ///< ...
	fp_exception_overflow    = FE_OVERFLOW,  ///< ...
	fp_exception_underflow   = FE_UNDERFLOW, ///< ...
	fp_exception_inexact     = FE_INEXACT,   ///< ...
#endif

	fp_exception_all = fp_exception_invalid |
					   fp_exception_div_by_zero |
					   fp_exception_overflow |
					   fp_exception_underflow |
					   fp_exception_inexact
};

// get/set fp exception flag.

/**
 * Clear the floating point exception register according to bitmask.
 * @param except bits to clear in floating point register.
 * @return 0 on success, otherwise -1
 */
int fpe_clear( unsigned int except );

/**
 * Try to raise floating point exceptions specified by except.
 * @note order exceptions are raised is undefined if except has multiple exceptions set.
 * @return 0 on success, otherwise -1
 */
int fpe_raise( unsigned int except );

/**
 * Return the bitwise or between except and currently set bits in floating point exception register.
 */
unsigned int fpe_test( unsigned int except );

// enable/disable interrupt.

/**
 * Enables trap by raising SIGFPE when an floating point exception is raised.
 *
 * @param except bitfield of fp_exception_type to enable.
 * @return 0 on success, otherwise -1
 */
int fpe_enable_trap( unsigned int except );

/**
 * Disables trap by raising SIGFPE when an floating point exception is raised.
 *
 * @param except bitfield of fp_exception_type to disable.
 * @return 0 on success, otherwise -1
 */
int fpe_disable_trap( unsigned int except );

/**
 * Set what floating point exceptions that is trapped by raising SIGFPE.
 *
 * @param except bitfield of fp_exception_type to set, the non-set are cleared.
 * @return 0 on success, otherwise -1
 */
int fpe_set_trapped( unsigned int except );

/**
 * Return a bitfield of the currently trapped floating point exceptions.
 */
unsigned int fpe_get_trapped();

#if defined( __cplusplus )
}
#endif


#if defined( __cplusplus )

/**
 * Enable trap of floating point exception while object of this type is alive.
 *
 * @example
 *
 * {
 *     // ... not capturing divide by zero ...
 *
 *     {
 *         fpe_enable_scope es( fp_exception_div_by_zero );
 *
 *         // ... capturing divide by zero exceptions ...
 *     }
 *
 *     // ... not capturing divide by zero ...
 * }
 */
class fpe_enable_scope
{
	unsigned int old_except;

	/**
	 * @param except_flags mask of floating point exceptions to trap in scope.
	 */
	fpe_enable_scope( unsigned int except_flags )
		: old_except( fpe_get_trapped() )
	{
		fpe_enable_trap( except_flags );
	}

	~fpe_enable_scope()
	{
		fpe_set_trapped( old_except );
	}
};

/**
 * Disable trap of floating point exception while object of this type is alive.
 *
 * @example
 *
 * {
 *     // ... capturing divide by zero ...
 *
 *     {
 *         fpe_disable_scope es( fp_exception_div_by_zero );
 *
 *         // ... not capturing divide by zero exceptions ...
 *     }
 *
 *     // ... capturing divide by zero ...
 * }
 */
class fpe_disable_scope
{
	unsigned int old_except;

	/**
	 *
	 * @param except_flags mask of floating point exceptions to not trap in scope.
	 */
	fpe_disable_scope( unsigned int except_flags )
		: old_except( fpe_get_trapped() )
	{
		fpe_disable_trap( except_flags );
	}

	~fpe_disable_scope()
	{
		fpe_set_trapped( old_except );
	}
};

#endif // defined( __cplusplus )

#endif // DBGTOOLS_FPE_CTRL_INCLUDED
