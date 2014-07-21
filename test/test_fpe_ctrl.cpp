#include <dbgtools/fpe_ctrl.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <string.h>

#include "greatest.h"

static float raise_fpe_invalid()   { return sqrtf( -1.0f ); }
static float raise_fpe_div_zero()  { static volatile float z = 0.0f; return 1.0f / z; }
static float raise_fpe_overflow()  { return expf(  88.8f ); }
static float raise_fpe_underflow() { return expf( -88.8f ); }
static float raise_fpe_inexact()   { static volatile float z = 3.0f; return 2.0f / z; }

int flags_set_correctly()
{
	fpe_clear( fp_exception_all );
	GREATEST_ASSERT_FALSE( fpe_test( fp_exception_div_by_zero ) );

	raise_fpe_invalid();   GREATEST_ASSERT( fpe_test( fp_exception_invalid ) );     fpe_clear( fp_exception_all );
	raise_fpe_div_zero();  GREATEST_ASSERT( fpe_test( fp_exception_div_by_zero ) ); fpe_clear( fp_exception_all );
	raise_fpe_overflow();  GREATEST_ASSERT( fpe_test( fp_exception_overflow ) );    fpe_clear( fp_exception_all );
	raise_fpe_underflow(); GREATEST_ASSERT( fpe_test( fp_exception_underflow ) );   fpe_clear( fp_exception_all );
	raise_fpe_inexact();   GREATEST_ASSERT( fpe_test( fp_exception_inexact ) );     fpe_clear( fp_exception_all );
	
	return 0;
}

int raise_set_correctly()
{
	fpe_clear( fp_exception_all );

	fpe_raise( fp_exception_invalid );     GREATEST_ASSERT( fpe_test( fp_exception_invalid ) );     fpe_clear( fp_exception_all );
	fpe_raise( fp_exception_div_by_zero ); GREATEST_ASSERT( fpe_test( fp_exception_div_by_zero ) ); fpe_clear( fp_exception_all );
	fpe_raise( fp_exception_overflow );    GREATEST_ASSERT( fpe_test( fp_exception_overflow ) );    fpe_clear( fp_exception_all );
	fpe_raise( fp_exception_underflow );   GREATEST_ASSERT( fpe_test( fp_exception_underflow ) );   fpe_clear( fp_exception_all );
	fpe_raise( fp_exception_inexact );     GREATEST_ASSERT( fpe_test( fp_exception_inexact ) );     fpe_clear( fp_exception_all );
	
	return 0;
}

int test_get_trapped()
{
	fpe_enable_trap( fp_exception_all );
	GREATEST_ASSERT_EQ( fpe_get_trapped() & fp_exception_invalid,     fp_exception_invalid );
	GREATEST_ASSERT_EQ( fpe_get_trapped() & fp_exception_div_by_zero, fp_exception_div_by_zero );
	GREATEST_ASSERT_EQ( fpe_get_trapped() & fp_exception_overflow,    fp_exception_overflow );
	GREATEST_ASSERT_EQ( fpe_get_trapped() & fp_exception_underflow,   fp_exception_underflow );
	GREATEST_ASSERT_EQ( fpe_get_trapped() & fp_exception_inexact,     fp_exception_inexact );
	GREATEST_ASSERT_EQ( fpe_get_trapped() & ~fp_exception_all,        0 );
	fpe_disable_trap( fp_exception_all );

	fpe_enable_trap( fp_exception_invalid );
	GREATEST_ASSERT_EQ( fpe_get_trapped() & fp_exception_invalid,     fp_exception_invalid );
	GREATEST_ASSERT_EQ( fpe_get_trapped() & fp_exception_div_by_zero, 0 );
	GREATEST_ASSERT_EQ( fpe_get_trapped() & fp_exception_overflow,    0 );
	GREATEST_ASSERT_EQ( fpe_get_trapped() & fp_exception_underflow,   0 );
	GREATEST_ASSERT_EQ( fpe_get_trapped() & fp_exception_inexact,     0 );
	fpe_disable_trap( fp_exception_all );
	
	fpe_enable_trap( fp_exception_div_by_zero );
	GREATEST_ASSERT_EQ( fpe_get_trapped() & fp_exception_invalid,     0 );
	GREATEST_ASSERT_EQ( fpe_get_trapped() & fp_exception_div_by_zero, fp_exception_div_by_zero );
	GREATEST_ASSERT_EQ( fpe_get_trapped() & fp_exception_overflow,    0 );
	GREATEST_ASSERT_EQ( fpe_get_trapped() & fp_exception_underflow,   0 );
	GREATEST_ASSERT_EQ( fpe_get_trapped() & fp_exception_inexact,     0 );
	fpe_disable_trap( fp_exception_all );
	
	fpe_enable_trap( fp_exception_overflow );
	GREATEST_ASSERT_EQ( fpe_get_trapped() & fp_exception_invalid,     0 );
	GREATEST_ASSERT_EQ( fpe_get_trapped() & fp_exception_div_by_zero, 0 );
	GREATEST_ASSERT_EQ( fpe_get_trapped() & fp_exception_overflow,    fp_exception_overflow );
	GREATEST_ASSERT_EQ( fpe_get_trapped() & fp_exception_underflow,   0 );
	GREATEST_ASSERT_EQ( fpe_get_trapped() & fp_exception_inexact,     0 );
	fpe_disable_trap( fp_exception_all );
	
	fpe_enable_trap( fp_exception_underflow );
	GREATEST_ASSERT_EQ( fpe_get_trapped() & fp_exception_invalid,     0 );
	GREATEST_ASSERT_EQ( fpe_get_trapped() & fp_exception_div_by_zero, 0 );
	GREATEST_ASSERT_EQ( fpe_get_trapped() & fp_exception_overflow,    0 );
	GREATEST_ASSERT_EQ( fpe_get_trapped() & fp_exception_underflow,   fp_exception_underflow );
	GREATEST_ASSERT_EQ( fpe_get_trapped() & fp_exception_inexact,     0 );
	fpe_disable_trap( fp_exception_all );
	
	fpe_enable_trap( fp_exception_inexact );
	GREATEST_ASSERT_EQ( fpe_get_trapped() & fp_exception_invalid,     0 );
	GREATEST_ASSERT_EQ( fpe_get_trapped() & fp_exception_div_by_zero, 0 );
	GREATEST_ASSERT_EQ( fpe_get_trapped() & fp_exception_overflow,    0 );
	GREATEST_ASSERT_EQ( fpe_get_trapped() & fp_exception_underflow,   0 );
	GREATEST_ASSERT_EQ( fpe_get_trapped() & fp_exception_inexact,     fp_exception_inexact );
	fpe_disable_trap( fp_exception_all );
	
	GREATEST_ASSERT_EQ( fpe_get_trapped() & fp_exception_invalid,     0 );
	GREATEST_ASSERT_EQ( fpe_get_trapped() & fp_exception_div_by_zero, 0 );
	GREATEST_ASSERT_EQ( fpe_get_trapped() & fp_exception_overflow,    0 );
	GREATEST_ASSERT_EQ( fpe_get_trapped() & fp_exception_underflow,   0 );
	GREATEST_ASSERT_EQ( fpe_get_trapped() & fp_exception_inexact,     0 );
	
	return 0;
}

#include <signal.h>

#if defined( _MSC_VER )
// TODO: generalize this!
void sigfpe_handler( int sig, int fpe )
{
	if( sig != SIGFPE )
		printf( "no SIGFPE!?!\n" );
	else
	{
		switch( fpe )
		{
			case FPE_INVALID:    printf("raised invalid!\n"); break;
			case FPE_ZERODIVIDE: printf("raised div by zero!\n"); break;
			case FPE_OVERFLOW:   printf("raised overflow!\n"); break;
			case FPE_UNDERFLOW:  printf("raised underflow!\n"); break;
			case FPE_INEXACT:    printf("raised inexact!\n"); break;
		}
	}
	exit( 0 );
}
#else
void sigfpe_handler( int /*sig*/ )
{
	printf("raised!\n");
	exit( 0 );
}
#endif

GREATEST_SUITE(fpe_ctrl)
{
	RUN_TEST( flags_set_correctly );
	RUN_TEST( raise_set_correctly );
	RUN_TEST( test_get_trapped );
}

extern SUITE(fpe_ctrl);

GREATEST_MAIN_DEFS();

int main( int argc, char** argv )
{
    GREATEST_MAIN_BEGIN();
    RUN_SUITE( fpe_ctrl );
    GREATEST_MAIN_END();

	/*
	if( argc > 1 )
	{
		signal( SIGFPE, (void(*)( int ))sigfpe_handler );

		fpe_enable_trap( fp_exception_all );

		if( strcmp( argv[1], "invalid"   ) == 0 ) raise_fpe_invalid();
		if( strcmp( argv[1], "divzero"   ) == 0 ) raise_fpe_div_zero();
		if( strcmp( argv[1], "overflow"  ) == 0 ) raise_fpe_overflow();
		if( strcmp( argv[1], "inexact"   ) == 0 ) raise_fpe_inexact();
		if( strcmp( argv[1], "underflow" ) == 0 ) raise_fpe_underflow();

		printf("fail!!!\n");
		return 1;
	}

	printf("success!\n");*/
	return 0;
}
