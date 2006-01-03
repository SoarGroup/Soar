/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file gski_rhsaction.h 
*********************************************************************
* created:	   6/20/2002   16:57
*
* purpose: 
*********************************************************************/

#ifndef GSKI_RHS_ACTION_HEADER
#define GSKI_RHS_ACTION_HEADER

#include "IgSKI_RhsAction.h"

#include "gSKI_ActionElement.h"
#include "gSKI_ErrorIds.h"

typedef struct action_struct action;

namespace gSKI {

   /** 
    * @brief Defines the interface for an action triplet.
    *
    * An action triplet represents an action that creates a preference
    *  for a WME. It contains three or four action elements, one that
    *  creates the id for the new wme, one that creates the attribute,
    *  and one that creates the value.  For binary preferences (applying
    *  to operators only) it contains the binary preference referent (or
    *  the element for the item to compare the value to).
    *
    * An example of some actions (this is not a useful production):
    *
    * @code
    *   sp { ...
    *    ... (LHS)
    *   -->
    *    (<s> ^mynumber 5.0 -)                  # 1. reject preference
    *    (<s> ^mystring gSKI +)                 # 2. accept preference
    *    (<s> ^mynewobject <object>)            # 3. implicit accept preference
    *    (<object> ^function-call (+ 5 6 7 8))  # 4. accept with function generating value
    *    (<s> ^operator <o> !)                  # 5. operator with REQUIRE preference
    *    (<s> ^operator <o> > <o2>)             # 6. operator with binary BETTER preference
    *  }
    * @endcode
    *
    * Each of the numbered lines is a single right hand side action.
    *
    * Internally, the kernel uses action triplets to create new
    *  WMEs when a production fires.
    */
   class RhsAction: public IRhsAction
   {
   public:
      
      /** 
       * @brief Construct a RHS action from a soar action
       *
       * We also provide a default constructor.  If you use the default
       *  constructor, call the SetValues method.
       * 
       * Copying is provided for safety.
       */
      //{
      RhsAction();
      RhsAction(agent* a, action* act);
      RhsAction(const RhsAction& old) { *this = old; }
      //}

      /** 
       * @brief Set the values of the rhs action from a soar action
       *
       * Call this method to set the values of a RhsAction after it has
       *  been created with the default constructor.
       */
      void SetValues(agent* a, action* act);

      /** 
       * @brief copy operator
       */
      RhsAction& operator=(const RhsAction& rhs);

      /**
       * @brief Destructor
       *
       * The destructor cleans up all action elements.  They are owned
       *  by the action, so you should not hold pointers to action elements
       *  when the owning action has been destroyed
       */
     ~RhsAction();


      /** 
       * @brief Gets the action element for creating the id
       *
       * This method retrieves the action element corresponding to
       *  id creation for a new WME preference.
       *
       * @param  err Pointer to client-owned error structure.  If the pointer
       *               is not 0 this structure is filled with extended error
       *               information.  If it is 0 (the default) extended error
       *               information is not returned.
       *
       * @return A pointer to an action element representing the symbol 
       *          used to create the id for a wme. (Action elements for
       *          ids are NEVER functions)
       */
     IActionElement*  GetIdElement(Error* err);

      /** 
       * @brief Gets the action element for creating the attribute
       *
       * This method retrieves the action element corresponding to
       *  attribute creation for a new WME preference.
       *
       * @param  err Pointer to client-owned error structure.  If the pointer
       *               is not 0 this structure is filled with extended error
       *               information.  If it is 0 (the default) extended error
       *               information is not returned.
       *
       * @return A pointer to an action element representing the symbol or rhs
       *          function used to create the attribute for a wme.
       */
     IActionElement*  GetAttrElement(Error* err);

      /** 
       * @brief Gets the action element for creating the value
       *
       * This method retrieves the action element corresponding to
       *  value creation for a new WME preference.
       *
       * @param  err Pointer to client-owned error structure.  If the pointer
       *               is not 0 this structure is filled with extended error
       *               information.  If it is 0 (the default) extended error
       *               information is not returned.
       *
       * @return A pointer to an action element representing the symbol or rhs
       *          function used to create the value for a wme.
       */
      IActionElement*       GetValueElement(Error* err);

       /**
        * @brief Returns the element refered to by a binary preference.
        *
        * Binary preference elements are only used for operators.  In
        *  the action (<s> ^operator <o1> > <o2> the binary preference
        *  element is <o2> (also called the referent).
        *
        * Possible Errors:
        *   gSKI_PREFERENCE_NOT_BINARY if the action does not create
        *          a binary preference.
        *
        * @param  err Pointer to client-owned error structure.  If the pointer
        *               is not 0 this structure is filled with extended error
        *               information.  If it is 0 (the default) extended error
        *               information is not returned.
        *
        *
        * @return A pointer to the action element defining the binary preference
        *            referent.  This pointer will be 0 if this action does
        *            not create a binary preference.
        */
      IActionElement*       GetBinaryPreferenceElement(Error* err);

       /**
        * @brief Returns the type of preference created by this action
        *
        * RhsActions produce preferences (either unary or binary) for 
        *  WMEs.  This method returns the type of preference this action
        *  creates.
        *
        * @param  err Pointer to client-owned error structure.  If the pointer
        *               is not 0 this structure is filled with extended error
        *               information.  If it is 0 (the default) extended error
        *               information is not returned.
        *
        * @return The type of preference created by this action.  It can be
        *          one of the egSKIPreferenceTypes.
        */
      egSKIPreferenceType  GetPreferenceType(Error* err) const;

       /**
        * @brief Returns the type of support the preferences (and wmes) created
        *             created through this action get.
        *
        *  This method returns the compile time support calculation for
        *   the preference created by this action.  Most of the time, the parser
        *   can determine what support a new preference will have by looking
        *   at the lhs (if it tests an operator, it usually gets o-support,
        *   if it doesn't test an operator, it gets i-support.  There are
        *   some cases where the support cannot be determined such as
        *   with the following lhs:
        *
        *   @code
        *     sp { ....
        *        (state <s> ^<a> <b>)
        *        (<b>  ^myattribute <a>)
        *     -->
        *        ...}
        *   @endcode
        *
        *   In these cases, the o-support mode is determined at runtime based on
        *    the value for <a> (if it is 'operator', the rhs gets o-support, if it
        *    is something else, it gets i-support).  For that case, the rhs action
        *    support type is unknown.
        *
        * @note Wmes get the same support as the preferences for them have.
        *
        * @see egSKISupportType
        * 
        * @param  err Pointer to client-owned error structure.  If the pointer
        *               is not 0 this structure is filled with extended error
        *               information.  If it is 0 (the default) extended error
        *               information is not returned.
        *
        * @return The type of support the preference created by this action will have.
        *           May be gSKI_UNKNOWN_SUPPORT.
        */
      egSKISupportType     GetSupportType(Error* err) const;

   private:

      /** Flag indicating if this is a binary preference action */
      bool                  m_binary;

      /** Type of preference this creates */
      egSKIPreferenceType   m_preference;

      /** Type of compile time support it gets */
      egSKISupportType      m_support;

      /** Action elements for the id, attribute and value */
      //{
      ActionElement         m_id;
      ActionElement         m_attr;
      ActionElement         m_value;
      //}

      /** Binary preference referent (may not exist, check m_binary) */
      ActionElement         m_binaryReferent;

   };
}

#endif
