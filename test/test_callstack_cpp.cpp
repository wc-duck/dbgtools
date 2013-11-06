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
