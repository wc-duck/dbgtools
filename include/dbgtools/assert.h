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

#ifndef DBGTOOLS_ASSERT_INCLUDED
#define DBGTOOLS_ASSERT_INCLUDED

/**
 * return value used by assert callback to determine action of assert.
 */
enum assert_action
{
	ASSERT_ACTION_NONE,  ///< continue execution after callback returns.
	ASSERT_ACTION_BREAK  ///< break into debugger when callstack returns.
};

/**
 * callback signature for callback to attach to assert.
 *
 * @param cond assert condition that failed as string.
 * @param msg assert message provided, or "" no one was provided.
 * @param file file where assert was triggered.
 * @param line line where assert was triggered.
 * @param userdata pointer passed together will callback to assert_register_callback.
 *
 * @return the action to take when callback returns.
 */
typedef assert_action (*assert_callback_t)( const char* cond, const char* msg, const char* file, unsigned int line, void* user_data );

/**
 * register a callback to call when an assert is triggered.
 */
void assert_register_callback( assert_callback_t callback, void* user_data );

/**
 * macro that "asserts" that a condition is true, if not it breaks into the debugger.
 * @note if ASSERT_ENABLE is not defined it expands to a noop.
 *
 * @example ASSERT( a == 1 ); // trigger if a != 1
 * @example ASSERT( a == 1, "a was not == to 1" ); // trigger if a != 1 and reports "a was not == to 1"
 * @example ASSERT( a == 1, "a == %d", a ); // trigger if a != 1 and reports "a == 1337" if a is 1337 that is!
 */
#define ASSERT(cond, ...) ((void)sizeof( cond ))

/**
 * macro that "asserts" that a condition is true, if not it breaks into the debugger.
 * @note if ASSERT_ENABLE is not defined it expands to only the condition.
 */
#define VERIFY(cond, ...) ((void)(cond))

/**
 * macro inserting a breakpoint into the code that breaks into the debugger on most platforms.
 */
#define DBG_TOOLS_BREAKPOINT

assert_action assert_call_trampoline( const char* file, unsigned int line, const char* cond );
assert_action assert_call_trampoline( const char* file, unsigned int line, const char* cond, const char* fmt, ... );



// ... private implementation ...

#undef DBG_TOOLS_BREAKPOINT

#if defined ( _MSC_VER )
#  define DBG_TOOLS_BREAKPOINT __debugbreak()
#elif defined( __GNUC__ )
#  if defined(__i386__) || defined( __x86_64__ )
     void inline __attribute__((always_inline)) _dbg_tools_gcc_break_helper() { __asm("int3"); }
#    define DBG_TOOLS_BREAKPOINT _dbg_tools_gcc_break_helper()
#  else
#    define DBG_TOOLS_BREAKPOINT __builtin_trap()
#  endif
#else
#  define DBG_TOOLS_BREAKPOINT exit(1)
#endif

#ifdef DBG_TOOLS_ASSERT_ENABLE
	#undef ASSERT
	#undef VERIFY

	#if defined( _MSC_VER )
		#define ASSERT(cond, ...) ( (void)( ( !(cond) ) && ( assert_call_trampoline( __FILE__, __LINE__, #cond, __VA_ARGS__ ) == ASSERT_ACTION_BREAK ) && ( DBG_TOOLS_BREAKPOINT, 1 ) ) )
		#define VERIFY(cond, ...) ASSERT( cond, __VA_ARGS__ )
	#elif defined( __GNUC__ )
		#define ASSERT(cond, args...) ( (void)( ( !(cond) ) && ( assert_call_trampoline( __FILE__, __LINE__, #cond, ##args ) == ASSERT_ACTION_BREAK ) && ( DBG_TOOLS_BREAKPOINT, 1 ) ) )
		#define VERIFY(cond, args...) ASSERT( cond, ##args )
	#endif
#else
	inline void assert_register_callback( assert_callback_t, void* ) {}
#endif // DBG_TOOLS_ASSERT_ENABLE

#endif // DBGTOOLS_ASSERT_INCLUDED
