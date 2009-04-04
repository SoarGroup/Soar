#ifndef PORTABILITY_H
#define PORTABILITY_H

/* This file will contain code required on all platforms */

#if defined(_WIN32) || defined(_WINDOWS) || defined(__WIN32__)

/* This file contains code specific to the windows platforms */
#include "portability_windows.h"

#else // not ( _WIN32 || _WINDOWS )

/* This file contains code specific to the posix platforms */
#include "portability_posix.h"

#endif // not ( _WIN32 || _WINDOWS )

// Atomic operations support (for thread safety)
// BADBAD: these should go in files specific to the platform
//         we may need to introduce Visual Studio and GCC files
//         since these are compiler-specific, and not posix vs windows
//         not that I think it's a big deal to stick this stuff in those files if we want

#if defined(_MSC_VER)

#include <intrin.h>
#pragma intrinsic (_InterlockedIncrement)

static inline long atomic_inc( volatile long  *v )
{
       return _InterlockedIncrement(v);
}

static inline long atomic_dec( volatile long *v )
{
       return _InterlockedDecrement(v);
}

// requires GCC>=4.2.0
#elif __GNUC__ > 4 || \
    (__GNUC__ == 4 && (__GNUC_MINOR__ > 2 || \
                       (__GNUC_MINOR__ == 2)))

static inline long atomic_inc( volatile long  *v )
{
      return __sync_add_and_fetch(v, 1);
}

static inline long atomic_dec( volatile long *v )
{
      return __sync_sub_and_fetch(v, 1);
}

#else

// BADBAD: It would be better if the locks were here.

#define DEBUG_REFCOUNTS 1

static inline long atomic_inc( volatile long  *v )
{
       return (++(*v));
}

static inline long atomic_dec( volatile long *v )
{
       return (--(*v));
}

#endif

#endif // PORTABILITY_H
