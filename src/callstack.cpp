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

#include <dbgtools/callstack.h>

#if defined( __unix__ ) || defined(unix) || defined(__unix) || ( defined(__APPLE__) && defined(__MACH__) )
#  define DBG_TOOLS_CALLSTACK_UNIX
#endif

#include <string.h>

#if defined( DBG_TOOLS_CALLSTACK_UNIX ) || defined(_MSC_VER)
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
#endif

#if defined( DBG_TOOLS_CALLSTACK_UNIX )
	#include <execinfo.h>
	#include <stdio.h>
	#include <stdlib.h>
	#include <unistd.h>
	#include <cxxabi.h>

	int callstack( int skip_frames, void** addresses, int num_addresses )
	{
		++skip_frames;
		void* trace[256];
		int fetched = backtrace( trace, num_addresses + skip_frames ) - skip_frames;
		memcpy( addresses, trace + skip_frames, (size_t)fetched * sizeof(void*) );
		return fetched;
	}

	static FILE* run_addr2line( void** addresses, int num_addresses, char* tmp_buffer, size_t tmp_buf_len )
	{
	#if defined(__linux)
		const char addr2line_run_string[] = "addr2line -e /proc/%u/exe";
	#elif defined(__APPLE__) && defined(__MACH__)
		const char addr2line_run_string[] = "xcrun atos -p %u -l";
	#else
	#   error "Unhandled platform"
	#endif

		size_t start = (size_t)snprintf( tmp_buffer, tmp_buf_len, addr2line_run_string, getpid() );
		for( int i = 0; i < num_addresses; ++i )
			start += (size_t)snprintf( tmp_buffer + start, tmp_buf_len - start, " %p", addresses[i] );

		return popen( tmp_buffer, "r" );
	}

	static char* demangle_symbol( char* symbol, char* buffer, size_t buffer_size )
	{
		int status;
		char* demangled_symbol = abi::__cxa_demangle( symbol, buffer, &buffer_size, &status );
		return status != 0 ? symbol : demangled_symbol;
	}

	int callstack_symbols( void** addresses, callstack_symbol_t* out_syms, int num_addresses, char* memory, int mem_size )
	{
		int num_translated = 0;
		callstack_string_buffer_t outbuf = { memory, memory + mem_size };
		memset( out_syms, 0x0, (size_t)num_addresses * sizeof(callstack_symbol_t) );

		char** syms = backtrace_symbols( addresses, num_addresses );
		size_t tmp_buf_len = 1024 * 32;
		char*  tmp_buffer  = (char*)malloc( tmp_buf_len );

		FILE* addr2line = run_addr2line( addresses, num_addresses, tmp_buffer, tmp_buf_len );

		for( int i = 0; i < num_addresses; ++i )
		{
			char* symbol = syms[i];
			unsigned int offset = 0;

			// find function name and offset
		#if defined(__linux)
			char* name_start   = strchr( symbol, '(' );
			char* offset_start = name_start ? strchr( name_start, '+' ) : 0x0;

			if( name_start && offset_start )
			{
				// zero terminate all strings
				++name_start;
				*offset_start = '\0'; ++offset_start;
			}
		#elif defined(__APPLE__) && defined(__MACH__)
			char* name_start   = 0x0;
			char* offset_start = strrchr( symbol, '+' );
			if( offset_start )
			{
				offset_start[-1] = '\0'; ++offset_start;
				name_start = strrchr( symbol, ' ' );
				if( name_start )
					++name_start;
			}
		#else
		#  error "Unhandled platform"
		#endif

			if( name_start && offset_start )
			{
				offset = (unsigned int)strtoll( offset_start, 0x0, 16 );
				symbol = demangle_symbol( name_start, tmp_buffer, tmp_buf_len );
			}

			out_syms[i].function = alloc_string( &outbuf, symbol, strlen( symbol ) );
			out_syms[i].offset   = offset;
			out_syms[i].file = "failed to lookup file";
			out_syms[i].line = 0;

			if( addr2line != 0x0 )
			{
				if( fgets( tmp_buffer, (int)tmp_buf_len, addr2line ) != 0x0 )
				{
				#if defined(__linux)
					char* line_start = strchr( tmp_buffer, ':' );
					*line_start = '\0';

					if( tmp_buffer[0] != '?' && tmp_buffer[1] != '?' )
						out_syms[i].file = alloc_string( &outbuf, tmp_buffer, strlen( tmp_buffer ) );
					out_syms[i].line = (unsigned int)strtoll( line_start + 1, 0x0, 10 );
				#elif defined(__APPLE__) && defined(__MACH__)
					char* file_start = strrchr( tmp_buffer, '(');
					if( file_start )
					{
						++file_start;
						char* line_start = strchr( file_start, ':' );
						if( line_start )
						{
							*line_start = '\0';
							++line_start;

							out_syms[i].file = alloc_string( &outbuf, file_start, strlen( file_start ) );
							out_syms[i].line = (unsigned int)strtoll( line_start, 0x0, 10 );
						}
					}
				#else
				#  error "Unhandled platform"
				#endif
				}
			}

			++num_translated;
		}
		free( syms );
		free( tmp_buffer );
		fclose( addr2line );
		return num_translated;
	}

#elif defined(_MSC_VER)
	#define WIN32_LEAN_AND_MEAN
	#define NOMINMAX
	#include <Windows.h>
	#include <Dbghelp.h>

	int callstack( int skip_frames, void** addresses, int num_addresses )
	{
		return RtlCaptureStackBackTrace( skip_frames + 1, num_addresses, addresses, 0 );
	}

	int callstack_symbols( void** addresses, callstack_symbol_t* out_syms, int num_addresses, char* memory, int mem_size )
	{
		HANDLE          process;
		DWORD64         offset;
		DWORD           line_dis;
		BOOL            res;
		IMAGEHLP_LINE64 line;
		PSYMBOL_INFO    sym_info;
		char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];

		int num_translated = 0;
		callstack_string_buffer_t outbuf = { memory, memory + mem_size };

		memset( out_syms, 0x0, (size_t)num_addresses * sizeof(callstack_symbol_t) );

		process = GetCurrentProcess();
		res     = SymInitialize( process, NULL, TRUE ); // TODO: Only initialize once!

		if( res == 0 )
		{
			DWORD err = GetLastError();
			// ERROR_INVALID_PARAMETER seems to be returned IF symbols for a specific module could not be loaded.
			// However the lookup will still work for all other symbols so let us ignore this error!
			if (err != ERROR_INVALID_PARAMETER)
				return 0;
		}

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
		return num_translated;
	}

#else

	int callstack( int skip_frames, void** addresses, int num_addresses )
	{
		(void)skip_frames; (void)addresses; (void)num_addresses;
		return 0;
	}

	int callstack_symbols( void** addresses, callstack_symbol_t* out_syms, int num_addresses, char* memory, int mem_size )
	{
		(void)addresses; (void)out_syms; (void)num_addresses; (void)memory; (void)mem_size;
		return 0;
	}

#endif

#if defined( DBG_TOOLS_CALLSTACK_UNIX )
#  undef DBG_TOOLS_CALLSTACK_UNIX
#endif
