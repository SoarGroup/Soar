#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H
#include "portability.h"

/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file gski_actionelement.cpp
*********************************************************************
* created:	   6/20/2002   16:57
*
* purpose: 
*********************************************************************/

#include "gSKI_ActionElement.h"
#include "rhsfun.h"

#include "gSKI_Error.h"
#include "MegaAssert.h"

#include "gSKI_Symbol.h"
#include "gSKI_RhsFunctionAction.h"

namespace gSKI
{

   /*
   ===============================
   ===============================
   */
   ActionElement::ActionElement(agent* a, rhs_value rhsval)
   {
      // This ensures we don't try to cleanup the pointer before allocating anything.
      m_val.sym = 0;
      SetValues(a, rhsval);
   }


   /*
   ===============================
   ===============================
   */
   ActionElement::ActionElement()
   {
      // No default type to set
      m_val.sym = 0;
   }

   /*
   ===============================
   ===============================
   */
   ActionElement& ActionElement::operator=(const ActionElement& rhs)
   {
      if(this != &rhs)
      {
         m_type   = rhs.m_type;
         
         // Make sure we don't leak memory
         cleanup();

         if(m_type == gSKI_ACTION_SYMBOL)
            m_val.sym  = new gSymbol(*rhs.m_val.sym);
         else if(m_type == gSKI_ACTION_FUNCTION)
            m_val.func = new RhsFunctionAction(*m_val.func);
      }
      return *this;
   }

   /** 
      * @brief Sets the value of this action element from a soar rhs_value
      *
      * Call this to set the action element's values if you used the default
      *   constructor.
      */
   void ActionElement::SetValues(agent* a, rhs_value rhsval)
   {
      MegaAssert(rhsval != 0, "Cannot create an action from a 0 rhs value pointer.");
      MegaAssert(!rhs_value_is_reteloc(rhsval), "Rhs value is unexpectedly a rete location! Expected symbol or function.");
      MegaAssert(!rhs_value_is_unboundvar(rhsval), "Rhs value is unexpectedly an unbound variable index! Expected symbol or function.");

      // Make sure that we are not leaking memory
      cleanup();

      if(rhs_value_is_symbol(rhsval))
      {
         m_type    = gSKI_ACTION_SYMBOL;
         m_val.sym = new gSymbol(a, rhs_value_to_symbol(rhsval), 0, false);
      }
      else if(rhs_value_is_funcall(rhsval))
      {
         m_type    = gSKI_ACTION_FUNCTION;
         m_val.func = new RhsFunctionAction(a, rhs_value_to_funcall_list(rhsval));
      }
      else
      {
         MegaAssert(false, "Unknown action element type.  Expected symbol or function.");
         m_val.sym = 0;
      }
   }

   /*
   ===============================
   ===============================
   */
   ActionElement::~ActionElement() 
   {
      cleanup();
   }

   /*
   ===============================
   ===============================
   */
   void ActionElement::cleanup()
   {
      // Choose the right type so we get the right destructor
      if(m_val.sym != 0)
      {
         if(m_type == gSKI_ACTION_SYMBOL)
         {
            m_val.sym->Release();
         }
         else if(m_type == gSKI_ACTION_FUNCTION)
         {
            delete m_val.func;
         }
      }

      // Set the pointers to 0 (both pointers share space)
      m_val.sym = 0;
    }


   /*
   ===============================

   ===============================
   */
   egSKIActionElementType      ActionElement::GetType(Error* err) const
   {
      ClearError(err);
      return m_type;
   }

   /*
   ===============================

   ===============================
   */
   const RhsFunctionAction*   ActionElement::GetFunction(Error* err) const
   {
      MegaAssert(m_type == gSKI_ACTION_FUNCTION, "Cannot return a symbol as a function.");

      ClearError(err);

      if(m_type == gSKI_ACTION_FUNCTION)
      {
         return m_val.func;
      }
      else
      {
         SetError(err, gSKIERR_ELEMENT_NOT_FUNCTION);
         return 0;
      }
   }

   /*
   ===============================

   ===============================
   */
   const ISymbol*              ActionElement::GetSymbol(Error* err) const
   {
      MegaAssert(m_type == gSKI_ACTION_SYMBOL, "Cannot return a symbol as a function.");

      ClearError(err);

      if(m_type == gSKI_ACTION_SYMBOL)
      {
         return m_val.sym;
      }
      else
      {
         SetError(err, gSKIERR_ELEMENT_NOT_SYMBOL);
         return 0;
      }
   }

}

