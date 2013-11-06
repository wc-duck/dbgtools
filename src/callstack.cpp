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

#include <dbgtools/callstack.h>

#include <string.h>

typedef struct
{
	char* out_ptr;
	const char* end_ptr;
} callstack_string_buffer_t;

static const char* alloc_string( callstack_string_buffer_t* buf, const char* str, size_t str_len )
{
	char* res;

	if( (size_t)(buf->end_ptr - buf->out_ptr) < str_len + 1 )
		return "out of memory";

	res = buf->out_ptr;
	buf->out_ptr += str_len + 1;
	memcpy( res, str, str_len );
	res[str_len] = '\0';
	return res;
}

#if defined( __unix__ )
	#include <execinfo.h>
	#include <stdio.h>
	#include <stdlib.h>
	#include <unistd.h>

	#include <cxxabi.h>

#elif defined( _MSC_VER )
	#include <Windows.h>
	#include <Dbghelp.h>
#endif

int callstack_symbols( void** addresses, callstack_symbol_t* out_syms, int num_addresses, char* memory, int mem_size )
{
#if defined( _MSC_VER )
	HANDLE          process;
	DWORD64         offset;
	DWORD           line_dis;
	BOOL            res;
	IMAGEHLP_LINE64 line;
	PSYMBOL_INFO    sym_info;
	char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
#endif

	int num_translated = 0;
	callstack_string_buffer_t outbuf = { memory, memory + mem_size };

	memset( out_syms, 0x0, num_addresses * sizeof(callstack_symbol_t) );

#if defined( __unix__ )
	char** syms = backtrace_symbols( addresses, num_addresses );
	size_t tmp_buf_len = 1024 * 32;
	char*  tmp_buffer  = (char*)malloc( tmp_buf_len );

	size_t start = 0;
	start += snprintf( tmp_buffer, tmp_buf_len, "addr2line -e /proc/%u/exe", getpid() );
	for( int i = 0; i < num_addresses; ++i )
		start += snprintf( tmp_buffer + start, tmp_buf_len - start, " %p", addresses[i] );

	FILE* addr2line = popen( tmp_buffer, "r" );

	for( int i = 0; i < num_addresses; ++i )
	{
		char* symbol = syms[i];

		unsigned int offset = 0;

		// find function name and offset
		char* name_start   = strchr( symbol, '(' );
		char* offset_start = name_start   ? strchr( name_start, '+' ) : 0x0;
		char* offset_end   = offset_start ? strchr( offset_start, ')' ) : 0x0;

		if( !( name_start == 0x0 || offset_start == 0x0 || offset_end == 0x0 ) )
		{
			// zero terminate all strings
			++name_start;
			*offset_start = '\0'; ++offset_start;
			*offset_end   = '\0'; ++offset_end;

			offset = (unsigned int)strtoll( offset_start, 0x0, 16 );

			int status;
			size_t funcname_size = tmp_buf_len;
			symbol = abi::__cxa_demangle( name_start, tmp_buffer, &funcname_size, &status );

			if( status != 0 )
				symbol = name_start;
		}

		out_syms[i].function = alloc_string( &outbuf, symbol, strlen( symbol ) );
		out_syms[i].offset   = offset;
		out_syms[i].file = "failed to lookup file";
		out_syms[i].line = 0;

		if( addr2line != 0x0 )
		{
			fgets( tmp_buffer, (int)tmp_buf_len, addr2line );
			char* line_start = strchr( tmp_buffer, ':' );
			*line_start = '\0';

			if( tmp_buffer[0] != '?' && tmp_buffer[1] != '?' )
				out_syms[i].file = alloc_string( &outbuf, tmp_buffer, strlen( tmp_buffer ) );
			out_syms[i].line = (unsigned int)strtoll( line_start + 1, 0x0, 10 );
		}

		++num_translated;
	}
	free( syms );
	free( tmp_buffer );
	fclose( addr2line );

#elif defined( _MSC_VER )

	process = GetCurrentProcess();
	res     = SymInitialize( process, NULL, TRUE ); // TODO: Only initialize once!

	if( res == 0 )
		return 0;

	sym_info  = (PSYMBOL_INFO)buffer;
	sym_info->SizeOfStruct = sizeof(SYMBOL_INFO);
	sym_info->MaxNameLen   = MAX_SYM_NAME;

	line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);

	for( int i = 0; i < num_addresses; ++i )
	{
		res = SymFromAddr( process, (DWORD64)addresses[i], &offset, sym_info );
		if( res == 0 )
			out_syms[i].function = "failed to lookup symbol";
		else
			out_syms[i].function = alloc_string( &outbuf, sym_info->Name, sym_info->NameLen );

		res = SymGetLineFromAddr64( process, (DWORD64)addresses[i], &line_dis, &line );	
		if( res == 0 )
		{
			out_syms[i].offset = 0;
			out_syms[i].file   = "failed to lookup file";
			out_syms[i].line   = 0;
		}
		else
		{
			out_syms[i].offset = (unsigned int)line_dis;
			out_syms[i].file   = alloc_string( &outbuf, line.FileName, strlen( line.FileName ) );
			out_syms[i].line   = (unsigned int)line.LineNumber;
		}

		++num_translated;
	}

#endif

	return num_translated;
}
