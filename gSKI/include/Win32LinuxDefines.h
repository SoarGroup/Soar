/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file Win32LinuxDefines.h
****************************************************************************
* created: 6/7/2002   16:27
*
* purpose: This header file contains some of the necessary cross platform
* defines for the GSKI project.
*********************************************************************/

#ifndef WIN32LINUXDEFINES_H
#define WIN32LINUXDEFINES_H

#if defined (WIN32)
#pragma warning (disable : 4060)
#endif

//
// If we are using gSKI and Windows, we want to use the _declspec
// stuff, otherwise we use nothing.  If we are using gSKI, then
// the DLL should declare functions as dllexport, and anything
// using the DLL should declare it as dllimport.
#if defined (WIN32) && !defined(_LIB) && defined(GSKI)
#  if defined(_USRDLL)
#     define gSKI_EXPORT __declspec(dllexport) 
#  else
#     define gSKI_EXPORT __declspec(dllimport)
#  endif
#else
#  define gSKI_EXPORT 
#endif

#ifdef WIN32
#  include "wtypes.h"
#  define DLL_ENTRY bool APIENTRY DllMain(HANDLE hModule,  DWORD  dwReason, LPVOID lpReserved)
#  define DLL_TRUE true
#  define DLL_FALSE false
#  define DLL_STARTUP  lpReserved = lpReserved; \
                       hModule = hModule; \
                       switch (dwReason)

#else
#  define DLL_ENTRY void _init(void)
#  define DLL_TRUE
#  define DLL_FALSE
#  define DLL_STARTUP
#endif

#endif
