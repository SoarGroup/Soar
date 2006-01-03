#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H
#include "portability.h"

/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

#include "gSKI_MultiAttribute.h"

namespace gSKI
{
   MultiAttribute::MultiAttribute(Agent* pAgent, 
                                  const char* attribute, int priority) :
      m_pAgent(pAgent), m_attribute(attribute), m_priority(priority)
   {
   }

   const char* MultiAttribute::GetAttributeName(Error *pErr /*= 0*/) const
   {
      return m_attribute.c_str();
   }

   int MultiAttribute::GetMatchingPriority(Error *pErr /*= 0*/) const
   {
      return m_priority;
   }

}
