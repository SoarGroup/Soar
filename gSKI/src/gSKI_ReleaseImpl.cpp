/********************************************************************
* @file gSKI_ReleaseImpl.cpp          
*********************************************************************
* @remarks Copyright (C) 2004 Soar Technology, All rights reserved. 
* The U.S. government has non-exclusive license to this software 
* for government purposes. 
*********************************************************************
* created:	   4/21/2004
*
* purpose: 
*********************************************************************/

#include "gSKI_ReleaseImpl.h"

#include <map>
#include <iostream>
#include <fstream>
#include <typeinfo>

using namespace gSKI;

namespace
{
#ifdef GSKI_DEBUG_RELEASE_IMPL
   
   std::map<gSKI::IRelease*, unsigned int> objects;

   void dumpUnreleaseObjects(const char* fileName)
   {
      std::ofstream fs(fileName);
      if(!fs.is_open())
      {
         std::cerr << "dumpUnreleaseObjects: Failed to open file." << std::endl;
         return;
      }

      fs << objects.size() << " unreleased gSKI objects:\n";
      for(std::map<gSKI::IRelease*, unsigned int>::iterator it = objects.begin();
            it != objects.end();
            ++it)
      {
         IRelease* pR = it->first;
         unsigned int n = it->second;
         fs << pR << " [" << n << "]: " << typeid(*pR).name();
         if(IRefCountedReleaseImpl* pRC = dynamic_cast<IRefCountedReleaseImpl*>(pR))
         {
            fs << " (count = " << pRC->GetRefCount() << ")";
         }
         fs << "\n";
      }
   }

   struct AutoDump
   {
      ~AutoDump()
      {
         dumpUnreleaseObjects("gski_unreleased.txt");
      }
   } autoDump;

#endif

}

namespace gSKI
{
#ifdef GSKI_DEBUG_RELEASE_IMPL
   unsigned int allocCount = 0;

   void ReleaseImplDebugAddObject(IRelease* pRelease)
   {
      // allocCount corresponds to the allocation count in the
      // gski_unreleased.txt file that is printed when a program
      // exits. By changing the number in this if statement and 
      // setting a breakpoint you can figure out exactly where the
      // offending unreleased object is allocated.
      if(allocCount == 31)
      {
         int x = 0;
      }

      objects.insert(std::make_pair(pRelease, allocCount++));
   }

   void ReleaseImplDebugRemoveObject(IRelease* pRelease)
   {
      objects.erase(pRelease);
   }
#endif
}

