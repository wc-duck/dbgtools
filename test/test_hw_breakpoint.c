/*
   Simpl test-program of hw_breakpoint_set() from dbgtools.

   version 1.0, october, 2013

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

#include <dbgtools/hw_breakpoint.h>

#include <stdio.h>
#include <stdlib.h>

#if defined( __unix__ ) || defined(unix) || defined(__unix) || ( defined(__APPLE__) && defined(__MACH__) )
	#include <unistd.h>
#elif defined( _MSC_VER )
	#include <Windows.h>
	#define sleep( x ) Sleep( x * 1000 )
#endif

int main( int argc, const char** argv )
{
	int hwbp;
	int break_size = 4;
	int break_index = 16;
	int break_offset = -1;
	char memory[256];

	if( argc > 1 )
		break_size = atoi( argv[1] );
	if( argc > 2 )
		break_offset = atoi( argv[2] );

	if( break_offset < 0 )
		break_offset = break_size - 1;

	printf("installing breakpoint of size %d on index %d\n", break_size, break_index);
	hwbp = hw_breakpoint_set( &memory[break_index], (unsigned int)break_size, HW_BREAKPOINT_WRITE );
	if( hwbp < 0 )
	{
		printf("failed to install hwbp!\n");
	}
	else
	{
		printf( "write before breakpoint at index %d...\n", break_index-1 );
		memory[break_index-1]++;

		printf( "write after breakpoint at index %d...\n", break_index+break_size );
		memory[break_index+break_size]++;

		printf( "write at end of breakpoint at index %d...\n", break_index+break_offset );
		memory[break_index+break_offset]++;

		hw_breakpoint_clear(hwbp);
	}

	printf("exit normaly!?!\n");

	return 0;
}
