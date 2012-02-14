/////////////////////////////////////////////////////////////////
// Export file.
//
// Date  : May 2006
//
// This defines EXPORT for use on Windows, Linux, and OS X.
// This file is shared by several projects.
//
/////////////////////////////////////////////////////////////////

#ifndef EXPORT_H
#define EXPORT_H

/* Adapted from SWIG output code, because they know what they are doing */
#if (__GNUC__ >= 4) || (__GNUC__ == 3 && __GNUC_MINOR__ >= 4)
#  ifndef GCC_HASCLASSVISIBILITY
#    define GCC_HASCLASSVISIBILITY
#  endif
#endif

#ifndef EXPORT
# if defined(_MSC_VER)
#   if defined(STATIC_LINKED)
#     define EXPORT
#   else
#     pragma warning( disable : 4251 )
#     if defined(_USRDLL)
#       define EXPORT __declspec(dllexport)
#     else
#       pragma message("Warning: Only used interfaces imported from Soar.dll")
#       define EXPORT __declspec(dllimport)
#     endif
#   endif
# elif defined(__GNUC__) && defined(GCC_HASCLASSVISIBILITY)
#   define EXPORT __attribute__ ((visibility("default")))
# else
#   define EXPORT
# endif
#endif

#endif
