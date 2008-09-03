/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file igski_match.h 
*********************************************************************
* created:	   6/20/2002   9:24
*
* purpose: 
*********************************************************************/
#ifndef IGSKI_MATCH_H
#define IGSKI_MATCH_H

#include "IgSKI_ConditionSet.h"

namespace gSKI{

   class ICondition;
   class IWme;
   struct Error;

   /**
    * @brief Holds information about a specific condition match.
    *
    * The reason we use this IMatch class, instead of just an IWme,
    * is that we need to access, not only the Wme, but the condition
    * that it is matching.  This is just another layer allowing
    * condition access.
    */
   class IMatch /*: public ICondition */{
   public:
      /**
      * @brief
      *
      */
      virtual ~IMatch(){}

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
      virtual ICondition *GetCondition(Error *e) const = 0;


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
      virtual IWme *GetWme(Error *e) const = 0;
   };
}
#endif
