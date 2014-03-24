#include <dbgtools/static_assert.h>

#include "test_static_assert1.h"
#include "test_static_assert2.h"

enum {
	test_var1 = 1337,
	test_var2 = 7331,

	test_var1_cmp = 1337,
	test_var2_cmp = 7331,
};

/* ... globally ... */
STATIC_ASSERT( test_var1 == test_var1_cmp, "static assert fail" );
STATIC_ASSERT( test_var2 == test_var2_cmp, "static assert fail" );

int func()
{
	/* ... in func ... */
	STATIC_ASSERT( test_var1 == test_var1_cmp, "static assert fail" );
	STATIC_ASSERT( test_var2 == test_var2_cmp, "static assert fail" );
	return 1;
}
