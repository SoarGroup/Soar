/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file gski_match.h 
*********************************************************************
* created:	   6/20/2002   9:24
*
* purpose: 
*********************************************************************/
#ifndef GSKI_MATCH_H
#define GSKI_MATCH_H

#include "IgSKI_Match.h"

//typedef struct agent_struct agent;
typedef struct condition_struct condition;
struct agent;

namespace gSKI {

   class ConditionSet;
   typedef ConditionSet MatchSet;

   struct Error;
   class ICondition;
   class IWme;
   /**
    * @brief Holds information about a specific condition match.
    *
    * The reason we use this IMatch class, instead of just an IWme,
    * is that we need to access, not only the Wme, but the condition
    * that it is matching.  This is just another layer allowing
    * condition access.
    */
   class Match : public IMatch{
   public:

      /**
       * @brief Constructor for a Match
       *
       * @param a    The agent we are getting the matches for.
       * @param cond The condition we are testing for a match.
       */
      Match(agent *a, condition *cond);
      

      /**
      * @brief
      *
      */
     ~Match();

      /**
      * @brief Returns the Condition that this match is associated with.
      *
      * @param  err Pointer to client-owned error structure.  If the pointer
      *               is not 0 this structure is filled with extended error
      *               information.  If it is 0 (the default) extended error
      *               information is not returned.
      *
      *
      * @returns The Condition, or 0 on error.
      */
     ICondition *GetCondition(Error *e) const;


      /**
      * @brief returns the Wme that matches the condition.
      *
      * @param  err Pointer to client-owned error structure.  If the pointer
      *               is not 0 this structure is filled with extended error
      *               information.  If it is 0 (the default) extended error
      *               information is not returned.
      *
      * @returns The Wme, or 0 on error.
      */
     IWme *GetWme(Error *e) const;
   };
}
#endif
