#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H
#include "portability.h"

/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file gski_rhsaction.cpp
*********************************************************************
* created:	   6/20/2002   16:57
*
* purpose: 
*********************************************************************/

#include "gSKI_RhsAction.h"

#include "gSKI_EnumRemapping.h"
#include "gSKI_Error.h"
#include "MegaAssert.h"

#include "rhsfun.h"

namespace gSKI 
{
   /*
   ===============================
   ===============================
   */
   RhsAction::RhsAction(): m_binary(false), m_preference(gSKI_ANY_PREF), m_support(gSKI_UNKNOWN_SUPPORT)
   {
      // Cannnot set other values.  Creator needs to call SetValues
   }

   /*
   ===============================
   ===============================
   */
   RhsAction::RhsAction(agent* a, action* act) 
   {
      SetValues(a, act);
   }

   /*
   ===============================
   ===============================
   */
   RhsAction& RhsAction::operator=(const RhsAction& rhs)
   {
      if(this != &rhs)
      {
         m_support         = rhs.m_support;
         m_preference      = rhs.m_preference;
         m_id              = rhs.m_id;
         m_attr            = rhs.m_attr;
         m_value           = rhs.m_value;
         m_binary          = rhs.m_binary;
         m_binaryReferent  = rhs.m_binaryReferent;
      }
      return *this;
   }

   /*
   ===============================
   ===============================
   */
   void RhsAction::SetValues(agent* a, action* act)
   {
      MegaAssert(a != 0, "Cannot initialize a rhs action with a 0 agent pointer!");
      MegaAssert(act != 0, "Cannot initialize a rhs action with a 0 pointer!");
      MegaAssert(act->type == MAKE_ACTION, "Cannot initiale a rhs action from a function!");
      MegaAssert(act->id != 0, "Id of action is 0");
      MegaAssert(act->attr != 0, "Attribute of action is 0");
      MegaAssert(act->value != 0, "Value of action is 0");

      // Support and preference type,...
      m_support    = EnumRemappings::ReMapSupportType(act->support);
      m_preference = EnumRemappings::ReMapPreferenceType(act->preference_type);

      // Set up the id, attribute and action
      m_id.SetValues(a, act->id);
      m_attr.SetValues(a, act->attr);
      m_value.SetValues(a, act->value);

      // Set up the binary part
      m_binary     = (preference_is_binary (act->preference_type))? true: false;
      if(m_binary)
         m_binaryReferent.SetValues(a, act->referent);
   }

   /*
   ===============================
   ===============================
   */
   RhsAction::~RhsAction() 
   {
   }

   /*
   ===============================
   ===============================
   */
   ActionElement*  RhsAction::GetIdElement(Error* err)
   {
      ClearError(err);
      return &m_id;
   }

   /*
   ===============================
   ===============================
   */

   ActionElement*  RhsAction::GetAttrElement(Error* err)
   {
      ClearError(err);
      return &m_attr;
   }

   /*
   ===============================
   ===============================
   */

   ActionElement*       RhsAction::GetValueElement(Error* err)
   {
      ClearError(err);
      return &m_value;
   }

   /*
   ===============================
   ===============================
   */
   ActionElement*       RhsAction::GetBinaryPreferenceElement(Error* err)
   {
      if (m_binary)
      {
         ClearError(err);
         return &m_binaryReferent;
      }
      else
      {
         SetError(err, gSKIERR_PREFERENCE_NOT_BINARY);
         return 0;
      }
   }

   /*
   ===============================

   ===============================
   */
   egSKIPreferenceType  RhsAction::GetPreferenceType(Error* err) const
   {
      return m_preference;
   }

   /*
   ===============================

   ===============================
   */
   egSKISupportType     RhsAction::GetSupportType(Error* err) const
   {
      return m_support;
   }

}
