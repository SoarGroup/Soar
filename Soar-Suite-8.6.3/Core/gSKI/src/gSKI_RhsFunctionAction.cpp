#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H
#include "portability.h"

/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file gski_rhsfunctionaction.cpp 
*********************************************************************
* created:	   6/20/2002   16:57
*
* purpose: 
*********************************************************************/

#include "rhsfun.h"
#include "agent.h"
#include "symtab.h"

#include "gSKI_Error.h"
#include "gSKI_Iterator.h"
#include "gSKI_Iterator.h"

#include "gSKI_RhsFunctionAction.h"

namespace gSKI
{
   /*
   ===============================
   ===============================
   */
   RhsFunctionAction::RhsFunctionAction(): 
      m_name(""), m_numParams(0), m_standAlone(true) {}

   /*
   ===============================
   ===============================
   */
   RhsFunctionAction::RhsFunctionAction(agent* a, list* funcall_list)
   {
      SetValues(a, funcall_list);
   }

   /*
   ===============================
   ===============================
   */
   RhsFunctionAction& RhsFunctionAction::operator=(const RhsFunctionAction& rhs)
   {
      if(this != &rhs)
      {
         // Make sure old parameters are cleaned
         cleanup();

         m_name         = rhs.m_name;
         m_numParams    = rhs.m_numParams;
         m_standAlone   = rhs.m_standAlone;

         // Do a deep copy of the parameters
         for(tParamVecCIt it = rhs.m_parameters.begin(); 
             it != rhs.m_parameters.end(); ++it)
         {
            m_parameters.push_back(new ActionElement(*(*it)));
         }
      }
      return *this;
   }

   /*
   ===============================
   ===============================
   */
   void RhsFunctionAction::SetValues(agent* a, list* funcall_list)
   {
      MegaAssert(a != 0, "Cannnot set rhs function actions with a 0 agent pointer.");
      MegaAssert(funcall_list != 0, "Setting values from a 0 function call");

      list*     elem;

      if(funcall_list != 0)
      {
         // cleanup previous parameters
         cleanup();

         // This will be the function name info structure
         rhs_function_struct* fn = static_cast<rhs_function_struct*>(funcall_list->first);
         
         m_name       = fn->name->sc.name;
         m_numParams  = fn->num_args_expected;
         m_standAlone = (fn->can_be_stand_alone_action)? true: false;

         // The rest of the elements are the arguments.  They are all action elements (which
         //  can recursively contain rhs functions)
         for(elem = funcall_list->rest; elem != 0; elem = elem->rest)
            m_parameters.push_back(new ActionElement(a, static_cast<rhs_value>(elem->first)));
      }
   }

   /*
   ===============================
   ===============================
   */
   void RhsFunctionAction::cleanup()
   {
      for(tParamVecIt it = m_parameters.begin(); it != m_parameters.end(); ++it)
         delete (*it);

      m_parameters.clear();
   }


   /*
   ===============================
   ===============================
   */
   RhsFunctionAction::~RhsFunctionAction() 
   {
      cleanup();
   }

   /*
   ===============================
   ===============================
   */
   const char*             RhsFunctionAction::GetName(Error* err)
   {
      ClearError(err);
      return m_name.c_str();
   }

   /*
   ===============================
   ===============================
   */
   int                     RhsFunctionAction::GetNumParameters(Error* err) const
   {
      ClearError(err);
      return m_numParams;
   }

   /*
   ===============================
   ===============================
   */
   tIActionElementIterator* RhsFunctionAction::GetParameterList(Error* err) 
   {
      // Create iterator and return it
      return new Iterator<ActionElement*, tParamVec>(m_parameters);
   }

   /*
   ===============================
   ===============================
   */

   bool                    RhsFunctionAction::IsStandAlone(Error* err) const
   {
      ClearError(err);
      return m_standAlone;
   }

}
