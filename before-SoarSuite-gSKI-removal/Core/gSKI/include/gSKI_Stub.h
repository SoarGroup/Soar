/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file gski_stub.h 
*********************************************************************
* created:	   6/27/2002   14:46
*
* purpose: 
*********************************************************************/
#ifndef GSKI_STUB_H
#define GSKI_STUB_H
#include "gSKI_KernelFactory.h"

#include "Win32LinuxDefines.h"
/*
#ifdef WIN32
#  ifndef _DLL
#     define GSKI_API __declspec(dllimport)
#  else
#     define GSKI_API __declspec(dllexport)
#  endif // _DLL
#else
#  define GSKI_API
#endif // WIN32
*/
extern "C" 
{
   gSKI_EXPORT gSKI::KernelFactory* gSKI_CreateKernelFactory(void);
}


#endif
