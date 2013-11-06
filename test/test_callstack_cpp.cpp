/*
	Simple test-program of written in c++ for callstack() and callstack_symbols()from dbgtools.

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

#include <dbgtools/callstack.h>

#include <stdio.h>

void print_callstack()
{
	void* addresses[256];
	int num_addresses = callstack( 0, addresses, 256 );

	callstack_symbol_t symbols[256];
	char  symbols_buffer[2048];
	num_addresses = callstack_symbols( addresses, symbols, num_addresses, symbols_buffer, 2048 );

	int i;
	for( i = 0; i < num_addresses; ++i )
		printf( "%3d) %-50s %s(%u)\n", i, symbols[i].function, symbols[i].file, symbols[i].line );
}

typedef void (*cb)( int, int );

cb funcs[3];

template < typename T >
class my_class
{
public:
	void a_member_func( int call_me, int depth )
	{
		if( depth == 0 )
		{
			print_callstack();
			return;
		}
		funcs[call_me]( 1, depth - 1 );
	}

	static void a_static_func( int call_me, int depth )
	{
		if( depth == 0 )
		{
			print_callstack();
			return;
		}
		funcs[call_me]( 2, depth - 1 );
	}
};

template< typename T >
void a_static_func( int call_me, int depth )
{
	my_class<T> c;
	c.a_member_func( call_me, depth );
}

int main( int, const char** )
{
	funcs[0] = my_class<int>::a_static_func;
	funcs[1] = a_static_func<int>;
	funcs[2] = a_static_func<double>;

	a_static_func<char>( 0, 6 );
	return 0;
}
