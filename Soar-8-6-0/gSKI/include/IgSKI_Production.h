/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file igski_production.h 
*********************************************************************
* created:	   6/13/2002   12:18
*
* purpose: 
*********************************************************************/
#ifndef IGSKI_PRODUCTION_H
#define IGSKI_PRODUCTION_H

#include "IgSKI_Iterator.h"
#include "IgSKI_Release.h"

namespace gSKI {

   class IAction;     /**< @see GetActions    */
   class ICondition;  /**< @see GetConditions */
   class IConditionSet;
   class IMatch;      /**< @see GetMatches    */
   struct Error;      /**< @see Error         */

   /** 
    * @brief This is the interface definition for Productions.
    *
    */
   class IProduction : public IRelease
   {
   public:

      /**
       * @brief Destructor for Productions
       *
       * This function insures that the destructor in the most derived
       * class is called when it is destroyed.  This will always be 
       * neccesary because this is a pure virtual base class.
       */
      virtual ~IProduction() {}

      /**
       * @brief This gets the firing count.
       *
       * This returns the number of times this productions has fired.
       *
       * @param  pErr Pointer to client-owned error structure.  If the pointer
       *              is not NULL this structure is filled with extended error
       *              information.  If it is NULL (the default) extended error
       *              information is not returned.
       *
       * @returns Firing count.
       */
      virtual unsigned long GetFiringCount(Error *pErr = 0) const = 0;

      /**
       * @brief Returns the name of the Productions
       *
       * @param  pErr Pointer to client-owned error structure.  If the pointer
       *              is not NULL this structure is filled with extended error
       *              information.  If it is NULL (the default) extended error
       *              information is not returned.
       *
       * @returns The Production name.
       */
      virtual const char* GetName(Error *pErr = 0) const = 0;

      /**
       * @brief Return the type of the Production
       *
       * @li User
       * @li Justification
       * @li Chunk
       * @li Default
       *
       * @param  pErr Pointer to client-owned error structure.  If the pointer
       *              is not NULL this structure is filled with extended error
       *              information.  If it is NULL (the default) extended error
       *              information is not returned.
       */
      virtual egSKIProdType GetType(Error *pErr = 0) const = 0;

      /**
       * @brief Returns the Documentation for the Productions
       *
       * @param  pErr Pointer to client-owned error structure.  If the pointer
       *              is not NULL this structure is filled with extended error
       *              information.  If it is NULL (the default) extended error
       *              information is not returned.
       *
       * @returns The Production Documentation.
       */
      virtual const char* GetDocs(Error *pErr = 0) const = 0;

      /**
       * @brief Returns the Productions Source
       *
       * @param  pErr Pointer to client-owned error structure.  If the pointer
       *              is not NULL this structure is filled with extended error
       *              information.  If it is NULL (the default) extended error
       *              information is not returned.
       *
       * @returns The name of the file it came from, or the alternate
       *          source name given at the time the production was 
       *          added.
       */
      virtual const char* GetSource(Error *pErr = 0) const = 0;

      /**
       * @brief The (rebuilt) source for the production.
       *
       * @param  pErr Pointer to client-owned error structure.  If the pointer
       *              is not NULL this structure is filled with extended error
       *              information.  If it is NULL (the default) extended error
       *              information is not returned.
       *
       * @returns A textural representation of the production from the
       *          RETE in a Soar Production compliant format.
       */
      virtual const char* GetText(Error *pErr = 0) const = 0;

      /**
       * @brief Get the conditions
       *
       * @param  pErr Pointer to client-owned error structure.  If the pointer
       *              is not NULL this structure is filled with extended error
       *              information.  If it is NULL (the default) extended error
       *              information is not returned.
       *
       * @returns The conditions that make up the LHS of the
       *          production.
       */
      virtual IConditionSet* GetConditions(Error *pErr = 0) const = 0; 

      /**
       * @brief Get the actions for this production that make wme preferences.
       *
       * Productions can have two types of actions on their rhs, actions that
       *  make wme preferences and actions that are stand alone functions.
       *  This method only retrieves an iterator to the actions that create
       *  wme preferences.  Call IProduction::GetStandAloneFunctions to get
       *  an iterator to all of the actions that are stand alone functions.
       *
       * @see IRhsAction
       *
       * @param  pErr Pointer to client-owned error structure.  If the pointer
       *              is not NULL this structure is filled with extended error
       *              information.  If it is NULL (the default) extended error
       *              information is not returned.
       *
       * @returns An Iterator to the list of actions that that make wme preferences
       *            for this production.
       */
      virtual tIRhsActionIterator* GetActions(Error *pErr = 0) = 0;

      /** 
       * @brief Returns the stand alone rhs functions on the rhs of this production
       *
       * Stand alone right hand side functions are executed for their side effect
       *  on the rhs of matching productions.  Examples are (halt) and (write).
       *  These functions do not create wme preferences or return symbols.
       *
       * @param  pErr Pointer to client-owned error structure.  If the pointer
       *              is not NULL this structure is filled with extended error
       *              information.  If it is NULL (the default) extended error
       *              information is not returned.
       *
       * @brief returns an iterator to the list of rhs function actions.  This pointer
       *            will never be 0.
       */
      virtual tIRhsFunctionActionIterator* GetStandAloneFunctions(Error* err = 0) = 0;

      /**
       * @brief returns the matches for this production.  @see MatchSet
       *
       * @param  pErr Pointer to client-owned error structure.  If the pointer
       *              is not NULL this structure is filled with extended error
       *              information.  If it is NULL (the default) extended error
       *              information is not returned.
       *
       * @brief returns an iterator to the list of matches.
       */
      virtual tIMatchIterator* GetMatches(Error *pErr = 0) = 0;

      /**
       * @brief Removes this production from the kernel and then deletes this
       *        object
       *
       * @param  pErr Pointer to client-owned error structure.  If the pointer
       *              is not NULL this structure is filled with extended error
       *              information.  If it is NULL (the default) extended error
       *              information is not returned.
       */
      virtual void Excise(Error *pErr = 0) = 0;

      /**
         @brief Returns a count of the number of tokens currently in use for the given
               production.  
               
         The count does not include:
            - tokens in the p_node (i.e., tokens representing complete matches)
            - local join result tokens on (real) tokens in negative/NCC nodes

         @param pErr Receives error info
         @returns Rete token count
      */
      virtual unsigned long CountReteTokens(Error * pErr = 0) = 0;

	  virtual bool IsWatched() = 0;
   };
}
#endif


