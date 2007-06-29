#ifndef CHECK_H
#define CHECK_H

#include <assert.h>

#define CHECK(x)			{ assert(x) ; if (!(x)) return ; }
#define CHECK_RET_FALSE(x)	{ assert(x) ; if (!(x)) return false ; }
#define CHECK_RET_ZERO(x)	{ assert(x) ; if (!(x)) return 0 ; }
#define CHECK_RET_NULL(x)	{ assert(x) ; if (!(x)) return NULL ; }

#endif // CHECK_H
