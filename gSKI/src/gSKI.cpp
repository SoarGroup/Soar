/********************************************************************
* @file gski.cpp 
****************************************************************************
* @remarks Copyright (C) 2002 Soar Technology, All rights reserved. 
* The U.S. government has non-exclusive license to this software 
* for government purposes. 
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
