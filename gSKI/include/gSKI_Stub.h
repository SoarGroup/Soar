/********************************************************************
* @file gski_stub.h 
*********************************************************************
* @remarks Copyright (C) 2002 Soar Technology, All rights reserved. 
* The U.S. government has non-exclusive license to this software 
* for government purposes. 
*********************************************************************
* created:	   6/27/2002   14:46
*
* purpose: 
*********************************************************************/
#ifndef GSKI_STUB_H
#define GSKI_STUB_H
#include "IgSKI_KernelFactory.h"

#ifdef WIN32
#  ifndef _DLL
#     define GSKI_API __declspec(dllimport)
#  else
#     define GSKI_API __declspec(dllexport)
#  endif // _DLL
#else
#  define GSKI_API
#endif // WIN32

extern "C" 
{
   GSKI_API gSKI::IKernelFactory* gSKI_CreateKernelFactory(void);
}


#endif
