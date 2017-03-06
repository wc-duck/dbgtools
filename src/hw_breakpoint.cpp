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

/**
 * Based on:
 * linux:   https://gist.github.com/jld/5d292c2c48eb07980562
 * windows: http://www.morearty.com/code/breakpoint
 */

#include <dbgtools/hw_breakpoint.h>

/**
 * Currently the linux implementation depend on having linux-headers installed.
 * If you are on a compiler that do not support __has_include() but still have
 * linux-headers installed this can be manually defined.
 *
 * In the future I might add a fallback that just defines the used symbols from these
 * headers and "hope" for the best.
 */
#if defined(__linux)
#  if !defined(HW_BREAKPOINT_HAS_LINUX_HEADERS)
#    if defined(__has_include)
#      if __has_include(linux/hw_breakpoint.h) &&  __has_include(linux/perf_event.h)
#        define HW_BREAKPOINT_HAS_LINUX_HEADERS
#      endif
#    endif
#  endif
#endif

#if defined(HW_BREAKPOINT_HAS_LINUX_HEADERS)

#include <fcntl.h>
#include <string.h>
#include <linux/hw_breakpoint.h>
#include <linux/perf_event.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#include <sys/syscall.h>
#include <unistd.h>

extern "C" int hw_breakpoint_set( void* address, unsigned int size, enum hw_breakpoint_type type )
{
	struct perf_event_attr attr;
	struct f_owner_ex owner;
	int fd;

	memset(&attr, 0, sizeof(attr));
	attr.size = sizeof(attr);
	attr.type = PERF_TYPE_BREAKPOINT;

	switch( type )
	{
		case HW_BREAKPOINT_READ:      attr.bp_type = HW_BREAKPOINT_R;  break;
		case HW_BREAKPOINT_WRITE:     attr.bp_type = HW_BREAKPOINT_W;  break;
		case HW_BREAKPOINT_READWRITE: attr.bp_type = HW_BREAKPOINT_RW; break;
	}

	attr.bp_addr = (uint64_t)address;

	switch( size )
	{
		case 1:
		case 2:
		case 4:
		case 8:
			attr.bp_len = size;
			break;
		default:
			return HW_BREAKPOINT_ERROR_INVALID_ARG;
	}

	attr.sample_period = 1;
	attr.precise_ip = 2; // request synchronous delivery
	attr.wakeup_events = 1;

	fd = (int)syscall(__NR_perf_event_open, &attr, 0, -1, -1,
		#if defined(PERF_FLAG_FD_CLOEXEC)
			PERF_FLAG_FD_CLOEXEC
		#else
			0
		#endif
			);

	if( fd < 0 )
	{
		if( errno == ENOSPC )
			return HW_BREAKPOINT_ERROR_OUT_OF_SLOTS;
		perror("perf_event_open");
		return HW_BREAKPOINT_ERROR_UNKNOWN;
	}

	if (fcntl(fd, F_SETSIG, SIGSEGV) < 0)
	{
		perror("fcntl F_SETSIG");
		close(fd);
		return HW_BREAKPOINT_ERROR_UNKNOWN;
	}

	owner.type = F_OWNER_TID;
	owner.pid = (pid_t)syscall(__NR_gettid);

	if (fcntl(fd, F_SETOWN_EX, &owner) < 0)
	{
		perror("fcntl F_SETOWN_EX");
		close(fd);
		return HW_BREAKPOINT_ERROR_UNKNOWN;
	}

	if (fcntl(fd, F_SETFL, O_ASYNC) < 0)
	{
		perror("fcntl F_SETFL");
		close(fd);
		return HW_BREAKPOINT_ERROR_UNKNOWN;
	}

	return fd;
}

extern "C" void hw_breakpoint_clear( int hwbp )
{
	close(hwbp);
}

#elif defined(_MSC_VER)

#include <windows.h>

inline DWORD64 hw_breakpoint_setbits(DWORD64 dw, int lowBit, int bits, int newValue)
{
	int mask = (1 << bits) - 1;
	return (dw & ~(mask << lowBit)) | (newValue << lowBit);
}

extern "C" int hw_breakpoint_set( void* address, unsigned int size, enum hw_breakpoint_type type )
{
	switch( size )
	{
		case 1: size = 0; break; // 00
		case 2: size = 1; break; // 01
		case 4: size = 3; break; // 11
		case 8: size = 2; break; // 10
			break;
		default:
			return HW_BREAKPOINT_ERROR_INVALID_ARG;
	}

	CONTEXT cxt;
	HANDLE thread = GetCurrentThread();
	cxt.ContextFlags = CONTEXT_DEBUG_REGISTERS;

		// Read the register values
	if( !GetThreadContext(thread, &cxt) )
		return HW_BREAKPOINT_ERROR_UNKNOWN;

	int reg_index;
	for( reg_index = 0; reg_index < 4; ++reg_index )
	{
		if( ( cxt.Dr7 & ( 1 << (reg_index * 2) ) ) == 0 )
			break;
	}

	switch( reg_index )
	{
		case 0: cxt.Dr0 = (DWORD) address; break;
		case 1: cxt.Dr1 = (DWORD) address; break;
		case 2: cxt.Dr2 = (DWORD) address; break;
		case 3: cxt.Dr3 = (DWORD) address; break;
		default:
			return HW_BREAKPOINT_ERROR_OUT_OF_SLOTS;
	}

	int when;
	switch( type )
	{
		case HW_BREAKPOINT_WRITE:     when = 1; break; // 01
		case HW_BREAKPOINT_READ:      when = 2; break; // 10
		case HW_BREAKPOINT_READWRITE: when = 3; break; // 11
	}

	cxt.Dr7 = hw_breakpoint_setbits(cxt.Dr7, 16 + (reg_index * 4), 2, when);
	cxt.Dr7 = hw_breakpoint_setbits(cxt.Dr7, 18 + (reg_index * 4), 2, size);
	cxt.Dr7 = hw_breakpoint_setbits(cxt.Dr7,       reg_index * 2,  1, 1);

	if( !SetThreadContext(thread, &cxt) )
		return HW_BREAKPOINT_ERROR_UNKNOWN;
	return reg_index;
}

extern "C" void hw_breakpoint_clear( int hwbp )
{
	CONTEXT cxt;
	HANDLE thread = GetCurrentThread();
	cxt.ContextFlags = CONTEXT_DEBUG_REGISTERS;

	GetThreadContext(thread, &cxt);
	cxt.Dr7 = hw_breakpoint_setbits(cxt.Dr7, hwbp * 2, 1, 0);
	SetThreadContext(thread, &cxt);
}

#elif defined(__APPLE__)

extern "C" int hw_breakpoint_set( void* address, unsigned int size, enum hw_breakpoint_type type )
{
	// I would love to implement this but I have no access to OSX to test it out, pull-requests are welcome!
	// on x86 it should be implementable by something like this:
	// http://stackoverflow.com/questions/2604439/how-do-i-write-x86-debug-registers-from-user-space-on-osx
	return HW_BREAKPOINT_ERROR_NOT_SUPPORTED;
}

extern "C" void hw_breakpoint_clear( int hwbp )
{
}


#else

extern "C" int hw_breakpoint_set( void*, unsigned int, enum hw_breakpoint_type )
{
	return HW_BREAKPOINT_ERROR_NOT_SUPPORTED;
}

extern "C" void hw_breakpoint_clear( int )
{
}

#endif
