/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file igski_productionmatch.h 
*********************************************************************
* created:	   6/21/2002   16:02
*
* purpose: 
*********************************************************************/
#ifndef IGSKI_PRODUCTIONMATCH_H
#define IGSKI_PRODUCTIONMATCH_H
namespace gSKI {
   class IProductionMatch {
   public:
     /**
      * @brief Destructor for the IMatchSet
      *
      * This function insures that the destructor in the most derived
      * class is called when it is destroyed.  This will always be 
      * neccesary because this is a pure virtual base class.
      */
      virtual ~IProductionMatch() {};

      /**
      * @brief
      *
      * @param  err Pointer to client-owned error structure.  If the pointer
      *              is not NULL this structure is filled with extended error
      *              information.  If it is NULL (the default) extended error
      *              information is not returned.
      *
      * @returns The MatchSet for this ProductionMatch.
      */
      virtual IMatchSet* GetMatchSet(Error* err = 0) = 0;


      /**
      * @brief Get the production associated with this match.
      *
      * @param  err Pointer to client-owned error structure.  If the pointer
      *              is not NULL this structure is filled with extended error
      *              information.  If it is NULL (the default) extended error
      *              information is not returned.
      *
      * @returns The production associated with this match.
      */
      virtual IProduction* GetProduction(Error* err = 0) = 0;

   };
}
#endif
