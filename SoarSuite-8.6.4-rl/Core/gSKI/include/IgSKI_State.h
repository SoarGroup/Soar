/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file igski_state.h 
*********************************************************************
* created:	   6/13/2002   15:00
*
* purpose: 
*********************************************************************/
#ifndef IGSKI_STATE_H
#define IGSKI_STATE_H

#include "gSKI_Enumerations.h"
#include "IgSKI_WMObject.h"

namespace gSKI {

   class IOperator;

   /** 
    * @brief Definition of the interface for managing and querying states
    *
    * The state object encapsulates an instance of a state in a soar agent's
    *  working memory.  A state is a working memory object, and thus inherits
    *  from the IWMObject interface.  It has the additional functionality of
    *  necessary to manipulate operators and to obtain unique state information
    *  (e.g. such as impasse type).
    *
    * State objects can be easily upcasted to IWMObjects for generic object
    *  manipulation.  To downcast (from IWMObject to IState) use the ToState
    *  method of IWMObject.
    */
   class IState: public IWMObject
   {
   public:

      /** 
       * @brief Destructor for this state object
       */
      virtual ~IState() {}

      /** 
       * @brief Gets whether or not this object is the top state
       *
       * @param err Pointer to client-owned error structure.  If the pointer
       *          is not 0 this structure is filled with extended error
       *          information.  If it is 0 (the default) extended error
       *          information is not returned.
       *
       * @return true if this is the top state of an agent's working memory.
       *           false if it is a substate.
       */
      virtual bool IsTopState(Error* err = 0) const = 0;

      /** 
       * @brief Gets the type of impasse that created this state
       *
       * Substates are only generated when a problem solving impasse arises.
       * Call this method to get the type of impasse that caused this state
       *  to be created.  Top states are not created because of impasses,
       *  so their type will be gSKI_IMPASSE_NONE
       *
       * @see egSKIImpasseType
       *
       * @param err Pointer to client-owned error structure.  If the pointer
       *          is not 0 this structure is filled with extended error
       *          information.  If it is 0 (the default) extended error
       *          information is not returned.
       *
       * @return The type of impasse that caused this state to be created.  See
       *           the definition of egSKIImpasseType for more information on
       *           the meanings of the value of this return type.
       */
      virtual egSKIImpasseType     GetImpasseType(Error* err = 0) const = 0;

      /** 
       * @brief Gets a list of all operators currently being proposed for this state
       *
       * Operators are proposed before being selected.  Proposed operators are
       *  full operator objects without any WME's referencing them (except for
       *  the selected operator, which does have a WME referencing it).  Instead of
       *  WME's, proposed operators have preferences referencing them.
       *
       * You can find the preferences for (or against) a particular operator in
       *  this state by passing an operator to the IState::GetOperatorPreferences
       *  method.
       *
       * @see egSKIUnaryPreferenceSearchType
       *
       * @param prefType Filter telling the method what preferences returned operators
       *                  should have.  This can be any bitwise ORed together set of
       *                  unary preference values (see egSKIUnaryPreferenceType).
       *                  Passing a gSKI_ANY_PREF will match any unary preference type.
       * @param err Pointer to client-owned error structure.  If the pointer
       *          is not 0 this structure is filled with extended error
       *          information.  If it is 0 (the default) extended error
       *          information is not returned.
       *
       * @return A pointer to an iterator referencing all of the operators proposed for
       *           the given state that have the specified preferences. This pointer
       *           will never be 0
       */
      virtual tIWMObjectIterator*  GetProposedOperators(
                                    egSKIPreferenceType  prefType = gSKI_ANY_PREF,
                                    Error*               err      = 0) const = 0;

      /** 
       * @brief Returns a pointer to the currently selected operator
       *
       * Call this method to retrieve a pointer to the selected operator for
       *  this state.  States do not always have selected operators, so it is
       *  best to check this pointer for 0 after obtaining it.
       *
       * @param err Pointer to client-owned error structure.  If the pointer
       *          is not 0 this structure is filled with extended error
       *          information.  If it is 0 (the default) extended error
       *          information is not returned.
       *
       * @return A pointer to the currently selected operator for this state.
       *         If this state does not have a selected operator, 0 is returned.
       */
      virtual IWMObject*           GetSelectedOperator(Error* err = 0) const = 0;
      
      /** 
       * @brief Returns the list of operators better than the given operator
       *
       * Call this method to retrieve the list of operators currently ranked
       *  as better than the given operator through thsi state's binary 
       *  preferences.  This will not return operators with less preferable
       *  unary preferences.
       *
       * Possible Errors:
       *   gSKIERR_INVALID_PTR if operatorObject is 0 or is detectably invalid
       *
       * @param operatorObject Pointer to an operator for which to retrieve
       *           the other operators ranked better than it.
       * @param err Pointer to client-owned error structure.  If the pointer
       *          is not 0 this structure is filled with extended error
       *          information.  If it is 0 (the default) extended error
       *          information is not returned.
       *
       * @return A pointer to an iterator referencing the list of operators
       *          currently ranked better than operatorObject.  This pointer
       *          will never be 0.
       */
      virtual tIWMObjectIterator*  GetOperatorsBetterThan(const IWMObject* operatorObject,
                                                          Error* err = 0) const = 0;

      /** 
       * @brief Returns the list of operators worse than the given operator
       *
       * Call this method to retrieve the list of operators currently ranked
       *  as worse than the given operator through binary preferences.  This
       *  will not return operators with less preferable unary preferences.
       *
       * Possible Errors:
       *   gSKIERR_INVALID_PTR if operatorObject is 0 or is detectably invalid
       *
       * @param operatorObject Pointer to an operator for which to retrieve
       *           the other operators ranked worse than it.
       * @param err Pointer to client-owned error structure.  If the pointer
       *          is not 0 this structure is filled with extended error
       *          information.  If it is 0 (the default) extended error
       *          information is not returned.
       *
       * @return A pointer to an iterator referencing the list of operators
       *          this operator is currently ranked worse than.  This pointer
       *          will never be 0.
       */
      virtual tIWMObjectIterator*  GetOperatorsWorseThan(const IWMObject* operatorObject,
                                                         Error* err = 0) const = 0;

      /** 
       * @brief Returns the list of operators indifferent to the given operator
       *
       * Call this method to retrieve the list of operators currently ranked
       *  as indifferent to the given operator through binary preferences.  This
       *  will not return operators with indifferent unary preferences.  
       *  An indifferent binary preference means that two operators are 
       *  equally acceptable.
       *
       * Possible Errors:
       *   gSKIERR_INVALID_PTR if operatorObject is 0 or is detectably invalid
       *
       * @param operatorObject Pointer to an operator for which to retrieve
       *           the other operators indifferent to it.
       * @param err Pointer to client-owned error structure.  If the pointer
       *          is not 0 this structure is filled with extended error
       *          information.  If it is 0 (the default) extended error
       *          information is not returned.
       *
       * @return A pointer to an iterator referencing the list of operators
       *          this operator is indifferent to.  This pointer
       *          will never be 0.
       */
      virtual tIWMObjectIterator*  GetOperatorsIndifferentTo(const IWMObject* operatorObject,
                                                             Error* err = 0) const = 0;

      /** 
       *  This may be dangerous, because productions should be
       *   i-supported, though it might be nice to have the ability to
       *   add an operator for debug purposes.
       */
      /* TODO: Find out if we should have a "Propose Operator method*/
      // virtual IWMObject* ProposeOperator()
      
      /** 
       * @brief Retrieves the goal dependency stack for this state
       *
       * The goal dependency stack is a list of WMEs in higher states that
       *  support the o-supported objects in this state.  If any WME in
       *  the goal dependency stack is removed, this state and all its
       *  substates are removed.
       *
       * The goal dependency stack is read only, you cannnot modify its 
       *  contents.
       *
       * @param err Pointer to client-owned error structure.  If the pointer
       *          is not 0 this structure is filled with extended error
       *          information.  If it is 0 (the default) extended error
       *          information is not returned.
       *
       * @return An iterator into the list of WMEs in the goal dependency stack
       *          for this state.
       */
      virtual tIWmeIterator* GetGoalDependencyStack(Error* err = 0) const = 0;
   };

}

#endif
