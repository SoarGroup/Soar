#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H
#include "portability.h"

/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file gski_stub.cpp 
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
