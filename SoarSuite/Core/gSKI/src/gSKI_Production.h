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
#ifndef GSKI_PRODUCTION_H
#define GSKI_PRODUCTION_H

#include "IgSKI_Iterator.h"
#include "IgSKI_Production.h"
#include "gSKI_ReleaseImpl.h"

#include "gSKI_RhsAction.h"
#include "gSKI_RhsFunctionAction.h"

//#include "MegaUnitTest.h"

#include <vector>

//
// Soar kernel forward declarations.
typedef struct production_struct production;
typedef struct condition_struct condition;
typedef struct agent_struct agent;

namespace gSKI {

   class IAction;     /**< @see GetActions    */
   class ICondition;  /**< @see GetConditions */
   class IMatch;      /**< @see GetMatches    */
   struct Error;      /**< @see Error         */
   class ConditionSet;/**< @see ConditionSet  */

   /** 
    * @brief This is the interface definition for Productions.
    *
    */
  class Production : public RefCountedReleaseImpl<IProduction, true>
   {
   public:
      /**
       * @brief: Constructor
       */
      //{
      Production(production *prod, bool includeConditions, agent* agent);
      Production(const Production& old): 
         RefCountedReleaseImpl<IProduction, true>(old),
         m_soarProduction(0), m_agent(0), m_conditionSet(0)
      {
         *this = old;
      }
      //}

     /** 
      * @brief Copy operator ensuring a correct deep copy.
      */
     Production&  operator=(const Production& rhs);

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
     unsigned long GetFiringCount(Error *pErr = 0) const;

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
     const char* GetName(Error *pErr = 0) const;

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
      egSKIProdType GetType(Error *pErr = 0) const;

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
     const char* GetDocs(Error *pErr = 0) const;

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
     const char* GetSource(Error *pErr = 0) const;

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
      const char* GetText(Error *pErr = 0) const;

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
      IConditionSet* GetConditions(Error *pErr = 0) const; 

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
       *            for this production.  This pointer will never be 0.
       */
      tIRhsActionIterator* GetActions(Error* err = 0);

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
     tIRhsFunctionActionIterator* GetStandAloneFunctions(Error* err = 0);

      /**
       * @brief returns the matches for this production.  @see MatchSet
       *
       * @param  pErr Pointer to client-owned error structure.  If the pointer
       *              is not NULL this structure is filled with extended error
       *              information.  If it is NULL (the default) extended error
       *              information is not returned.
       *
       * @brief returns an iterator to the list of matches.  This pointer will
       *            never be 0.
       */
     tIMatchIterator* GetMatches(Error *pErr = 0);

      /**
       * @brief Removes this production from the kernel and then deletes this
       *        object
       *
       * @param  pErr Pointer to client-owned error structure.  If the pointer
       *              is not NULL this structure is filled with extended error
       *              information.  If it is NULL (the default) extended error
       *              information is not returned.
       */
      void Excise(Error *pErr = 0);

      virtual unsigned long CountReteTokens(Error * pErr = 0);

	  bool IsWatched();

     private:
  
        /** 
         * Container to store rhs actions ("make" actions in soar lingo)
         */
        //{
        typedef std::vector<RhsAction*>         tRhsActionVec;
        typedef tRhsActionVec::iterator         tRhsActionVecIt;
        typedef tRhsActionVec::const_iterator   tRhsActionVecCIt;
        //}

        /** 
         * Container to store stand alone rhs functions 
         */
        //{
        typedef std::vector<RhsFunctionAction*>  tRhsFunctionVec;
        typedef tRhsFunctionVec::iterator        tRhsFunctionVecIt;
        typedef tRhsFunctionVec::const_iterator  tRhsFunctionVecCIt;
        //}

        /**
         * @brief: Takes the current production and loads up all of it's 
         * information from the soar kernel in to this object.
         */
        void loadProductionFromKernel(void) const;

        /**
         * @brief: Reads in the conditions from the production.
         */
        void loadConditions(condition *c, ConditionSet &cs) const ;

        /** 
         * @brief Reads in the actions from the rhs of the production.
         *
         * The first parameter is a Soar kernel action.
         * The last two parameters are out parmaeters that get the list of preference
         *  creation actions and stand alone function actions respectively.
         */
        void loadActions(action* actions, tRhsActionVec& actionVec, tRhsFunctionVec& funcVec);

        /** 
         * @brief Cleanup all memory allocated by the production
         */
        void cleanup();

        production*     m_soarProduction;
        agent*          m_agent;

        ConditionSet*   m_conditionSet;


        /** Stores the actions that make wme preferences */
        tRhsActionVec     m_actions;

        /** Stores the stand alone function actions (if any) */
        tRhsFunctionVec   m_functions;

//        DECL_TEST_INCLASS(test1);

	protected:  // 2/23/05: changed to protected to eliminate gcc warning
		/**
         * @brief Destructor for Productions
         *
         * This function insures that the destructor in the most derived
         * class is called when it is destroyed.  This will always be 
         * neccesary because this is a purebase class.
         */
        ~Production();
   };
}
#endif


