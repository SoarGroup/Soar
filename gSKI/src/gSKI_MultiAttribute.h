/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

#ifndef GSKI_MULTIATTRIBUTE_H
#define GSKI_MULTIATTRIBUTE_H

#include <string>

#include "IgSKI_MultiAttribute.h"
#include "gSKI_ReleaseImpl.h"

namespace gSKI
{
   class Agent;

   class MultiAttribute : public RefCountedReleaseImpl<IMultiAttribute>
   {
   public:
      /** 
         Constructor

         @param pAgent Owning agent
         @param attribute Attribute name
         @param priority Matching priority
      */
      MultiAttribute(Agent* pAgent, const char* attribute, int priority);

      /** Implement IMultiAttribute */
      //{
      virtual const char* GetAttributeName(Error *pErr = 0) const; 
      virtual int GetMatchingPriority(Error *pErr = 0) const;
      //}

   private:
      Agent* m_pAgent;           /// Owning agent
      std::string m_attribute;   /// Name of attribute
      int m_priority;            /// Matching priority

   protected:
      /** Private destructor so no one tries to use delete directly
	      2/23/05: changed to protected to eliminate gcc warning */
      virtual ~MultiAttribute() {}
   }; // class MultiAttribute

} // namespace gSKI

#endif // GSKI_MULTIATTRIBUTE_H
