/////////////////////////////////////////////////////////////////
// Export file.
//
// Date  : May 2006
//
// This defines EXPORT for use on Windows, Linux, and OS X.
// This file is shared by several projects.
//
/////////////////////////////////////////////////////////////////

#ifndef EXPORT
# if defined(_WIN32) || defined(__WIN32__) || defined(__CYGWIN__)
#   if defined(_USRDLL)
#     define EXPORT __declspec(dllexport)
#   elif defined(STATIC_LINKED)
#     define EXPORT
#   else
#     define EXPORT __declspec(dllimport)
#   endif
# else
#   if defined(__GNUC__) && defined(GCC_HASCLASSVISIBILITY)
#     define EXPORT __attribute__ ((visibility("default")))
#   else
#     define EXPORT
#   endif
# endif
#endif
