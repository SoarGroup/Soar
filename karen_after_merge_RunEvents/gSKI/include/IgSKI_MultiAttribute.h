/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file igski_multiattribute.h 
*********************************************************************
* created:	   6/16/2004   14:58
*
* purpose: 
*********************************************************************/

#ifndef GSKI_IGSKI_MULTIATTRIBUTE_H
#define GSKI_IGSKI_MULTIATTRIBUTE_H

#include "IgSKI_Release.h"

namespace gSKI 
{
   struct Error;

   /**
      An interface representing agent multi-attribute settings.

      In particular the matching priority for a particular named
      multi-attribute can be accessed with this interface. To set the
      value call IAgent::SetMultiAttribute()
   */
   class IMultiAttribute : public IRelease
   {
   public:
      /** Destructor. Virtual for usual reasons */
      virtual ~IMultiAttribute() {}

      /** Returns the name of the attribute */
      virtual const char* GetAttributeName(Error *pErr = 0) const = 0;

      /** Returns the matching priority for the multi-attribute */
      virtual int GetMatchingPriority(Error *pErr = 0) const = 0;
   }; // class IMultiAttribute

} // namespace gSKI

#endif // GSKI_IGSKI_MULTIATTRIBUTE_H
