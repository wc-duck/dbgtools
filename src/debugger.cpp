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
			fgets( buffer, 1024, f );

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

		return info.kp_proc.p_flag & P_TRACED) != 0;
	}
#else

	int debugger_present()
	{
		return 0;
	}

#endif
