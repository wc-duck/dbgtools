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

#include <dbgtools/debugger.h>

#if defined( __linux )

	#include <stdio.h>

	int debugger_present()
	{
		int i;
		FILE* f = fopen( "/proc/self/status", "r" );
		if( f == 0x0 )
			return 0;

		// ... skip the first 5 lines ...
		char buffer[1024];
		for( i = 0; i < 5; ++i )
		{
			if( fgets( buffer, 1024, f ) == 0x0 )
				return 0;
		}

		int trace_pid;
		if( fscanf( f, "TracerPid: %d", &trace_pid ) != 1 )
			trace_pid = 0;
		fclose( f );

		return trace_pid != 0;
	}

#elif defined( _MSC_VER )

	#include <Windows.h>

	int debugger_present()
	{
		return IsDebuggerPresent();
	}

#elif defined( __APPLE__ )

#include <sys/sysctl.h>
#include <unistd.h>

	int debugger_present()
	{
		int mib[4];
		struct kinfo_proc info;
		size_t size;

		info.kp_proc.p_flag = 0;
		mib[0] = CTL_KERN;
		mib[1] = KERN_PROC;
		mib[2] = KERN_PROC_PID;
		mib[3] = getpid();

		size = sizeof(info);
		sysctl( mib, sizeof(mib) / sizeof(*mib), &info, &size, NULL, 0) ;

		return (info.kp_proc.p_flag & P_TRACED) != 0;
	}
#else

	int debugger_present()
	{
		return 0;
	}

#endif
