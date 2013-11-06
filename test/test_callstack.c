#include <dbgtools/callstack.h>

#include <stdio.h>

void print_callstack()
{
	void* addresses[256];
	int i;
	int num_addresses = callstack( 0, addresses, 256 );

	callstack_symbol_t symbols[256];
	char  symbols_buffer[1024];
	num_addresses = callstack_symbols( addresses, symbols, num_addresses, symbols_buffer, 1024 );

	for( i = 0; i < num_addresses; ++i )
		printf( "%3d) %-50s %s(%u)\n", i, symbols[i].function, symbols[i].file, symbols[i].line );
}

typedef void (*cb)( int, int );

void func1( int, int );
void func2( int, int );
void func3( int, int );

cb funcs[] = { func1, func2, func3 };

void func1( int call_me, int depth )
{
	if( depth == 0 )
	{
		print_callstack();
		return;
	}
	funcs[call_me]( 2, depth - 1 );
}

void func2( int call_me, int depth )
{
	if( depth == 0 )
	{
		print_callstack();
		return;
	}
	funcs[call_me]( 0, depth - 1 );
}

void func3( int call_me, int depth )
{
	if( depth == 0 )
	{
		print_callstack();
		return;
	}
	funcs[call_me]( 1, depth - 1 );
}

int main( int argc, const char** argv )
{
	(void)argc; (void) argv;
	func1( 1, 5 );
	return 0;
}
