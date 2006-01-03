#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H
#include "portability.h"

/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file gski.cpp 
****************************************************************************
* created:	   5/31/2002   9:54
*
* purpose: 
*********************************************************************/

//#include "IteratorHelpers.h"
#include "Win32LinuxDefines.h"
#include "gSKI.h"
#include <iostream>



/**
 *   @namespace gSKI
 *   @brief This is the primary namespace for the gSKI project.
 */
namespace gSKI {

// Required DLL entry point for windows
   DLL_ENTRY
   {
      DLL_STARTUP
      {

      }
      return DLL_TRUE;
   }
 
   gSKI_EXPORT int outfunc(int joh){
      int x = 5;
      int y = joh;

      return y + x;
   }
}
