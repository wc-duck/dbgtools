#include <dbgtools/fpe_ctrl.h>

#if defined( __cplusplus )
extern "C" {
#endif

#if defined( __APPLE__ )
// These implementations of fegetexcept, feenableexcept and fedisableexcept is lifted from:
// http://www-personal.umich.edu/~williams/archive/computation/fe-handling-example.c

#define DEFINED_PPC      (defined(__ppc__) || defined(__ppc64__))
#define DEFINED_INTEL    (defined(__i386__) || defined(__x86_64__))

#  if DEFINED_PPC

#  define FE_EXCEPT_SHIFT 22  // shift flags right to get masks
#  define FM_ALL_EXCEPT   FE_ALL_EXCEPT >> FE_EXCEPT_SHIFT

	static int fegetexcept()
	{
	  fenv_t fenv;
	  return ( fegetenv (&fenv) ? -1 : ( ( fenv & (FM_ALL_EXCEPT) ) << FE_EXCEPT_SHIFT ) );
	}

	static int feenableexcept( unsigned int excepts )
	{
	  fenv_t fenv;
	  unsigned int new_excepts = (excepts & FE_ALL_EXCEPT) >> FE_EXCEPT_SHIFT,
				   old_excepts;  // all previous masks

	  if ( fegetenv (&fenv) ) return -1;
	  old_excepts = (fenv & FM_ALL_EXCEPT) << FE_EXCEPT_SHIFT;

	  fenv = (fenv & ~new_excepts) | new_excepts;
	  return ( fesetenv (&fenv) ? -1 : old_excepts );
	}

	static int fedisableexcept( unsigned int excepts )
	{
	  fenv_t fenv;
	  unsigned int still_on = ~( (excepts & FE_ALL_EXCEPT) >> FE_EXCEPT_SHIFT ),
				   old_excepts;  // previous masks

	  if ( fegetenv (&fenv) ) return -1;
	  old_excepts = (fenv & FM_ALL_EXCEPT) << FE_EXCEPT_SHIFT;

	  fenv &= still_on;
	  return ( fesetenv (&fenv) ? -1 : old_excepts );
	}

#  elif DEFINED_INTEL
	static int fegetexcept()
	{
	  fenv_t fenv;
	  return fegetenv (&fenv) ? -1 : (fenv.__control & FE_ALL_EXCEPT);
	}

	static int feenableexcept( unsigned int excepts )
	{
	  fenv_t fenv;
	  unsigned int new_excepts = excepts & FE_ALL_EXCEPT,
				   old_excepts;  // previous masks

	  if ( fegetenv (&fenv) ) return -1;
	  old_excepts = fenv.__control & FE_ALL_EXCEPT;

	  // unmask
	  fenv.__control &= ~new_excepts;
	  fenv.__mxcsr   &= ~(new_excepts << 7);

	  return ( fesetenv (&fenv) ? -1 : old_excepts );
	}

	static int fedisableexcept( unsigned int excepts )
	{
	  fenv_t fenv;
	  unsigned int new_excepts = excepts & FE_ALL_EXCEPT,
				   old_excepts;  // all previous masks

	  if ( fegetenv (&fenv) ) return -1;
	  old_excepts = fenv.__control & FE_ALL_EXCEPT;

	  // mask
	  fenv.__control |= new_excepts;
	  fenv.__mxcsr   |= new_excepts << 7;

	  return ( fesetenv (&fenv) ? -1 : old_excepts );
	}
#  endif  // PPC or INTEL enabling
#endif

#if defined( _GNU_SOURCE ) || defined( __APPLE__ )

	#include <fenv.h>

	// how to handle that this is an unknown pragma?
	// #pragma STDC FENV_ACCESS on

	int fpe_clear( unsigned int except )
	{
		return feclearexcept( except );
	}

	int fpe_raise( unsigned int except )
	{
		return feraiseexcept( except );
	}

	unsigned int fpe_test( unsigned int except )
	{
		return fetestexcept( except );
	}

	int fpe_enable_trap( unsigned int except )
	{
		if( feclearexcept( except ) < 0 )
			return -1;

		return feenableexcept( except );
	}

	int fpe_disable_trap( unsigned int except )
	{
		return fedisableexcept( except );
	}

	int fpe_set_trapped( unsigned int except )
	{
		if( fpe_disable_trap( ~except ) < 0 )
			return -1;
		return fpe_enable_trap( except );
	}

	unsigned int fpe_get_trapped()
	{
		return (unsigned int)fegetexcept();
	}

#elif defined( _MSC_VER )

	#include <float.h>
	#pragma fenv_access(on)

	int fpe_clear( unsigned int except )
	{
		_clearfp();
		return 0;
	}

	int fpe_raise( unsigned int except )
	{
		return -1;
	}

	unsigned int fpe_test( unsigned int except )
	{
		return ( _statusfp() & except ) > 0;
	}

	int fpe_enable_trap( unsigned int except )
	{
		unsigned int curr;
		errno_t err = _controlfp_s( &curr, ~except, _MCW_EM );
		return err == 0 ? 0 : -1;
	}

	int fpe_disable_trap( unsigned int except )
	{
		unsigned int curr;
		errno_t err = _controlfp_s( &curr, except, _MCW_EM );
		return err == 0 ? 0 : -1;
	}

	int fpe_set_trapped( unsigned int except )
	{
		if( fpe_disable_trap( ~except ) < 0 )
			return -1;
		return fpe_enable_trap( except );
	}
	
	unsigned int fpe_get_trapped()
	{
		unsigned int current_word = 0;
		_controlfp_s( &current_word, 0, 0 );
		return ( ~current_word ) & fp_exception_all;
	}

#else

	int fpe_clear( unsigned int /*except*/ ) { return -1; }
	int fpe_raise( unsigned int /*except*/ ) { return -1; }
	unsigned int fpe_test( unsigned int /*except*/ ) { return 0; }
	int fpe_enable_trap  ( unsigned int /*except*/ ) { return -1; }
	int fpe_disable_trap ( unsigned int /*except*/ ) { return -1; }
	int fpe_get_trapped  ( unsigned int /*except*/ ) { return -1; }

#endif

#if defined( __cplusplus )
}
#endif