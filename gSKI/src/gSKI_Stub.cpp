#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

/********************************************************************
* @file gski_stub.cpp 
*********************************************************************
* @remarks Copyright (C) 2002 Soar Technology, All rights reserved. 
* The U.S. government has non-exclusive license to this software 
* for government purposes. 
*********************************************************************
* created:	   6/27/2002   15:08
*
* purpose: 
*********************************************************************/
#include "gSKI_Stub.h"
#include "gSKI_KernelFactory.h"

//
// Explicit Export for this file.
//#include "MegaUnitTest.h"
//DEF_EXPOSE(gSKI_Stub);


extern "C"
{
   gSKI::IKernelFactory* gSKI_CreateKernelFactory(void)
   {
      return new gSKI::KernelFactory();
   }
}
