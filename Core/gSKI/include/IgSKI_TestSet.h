/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file IgSKI_TestSet.h
*********************************************************************
* created:	   6/27/2002   10:44
*
* purpose: 
*********************************************************************/
#ifndef IGSKI_TESTSET_H
#define IGSKI_TESTSET_H

#include "IgSKI_Iterator.h"

namespace gSKI
{
   struct Error;

   /**
   * @brief: This is used to represent a set of Tests
   */
   class ITestSet
   {
   public:
      
      /**
       * @brief
       */
      virtual ~ITestSet(){}
      
       /**
        * @brief 
        * @param  err Pointer to client-owned error structure.  If the pointer
        *              is not NULL this structure is filled with extended error
        *              information.  If it is NULL (the default) extended error
        *              information is not returned.
        *
        * @returns the list of tests held by this test-set.
        */
       virtual tITestIterator*    GetTests(Error *err = 0) const = 0;

       /**
        * @brief 
        * @param  err Pointer to client-owned error structure.  If the pointer
        *              is not NULL this structure is filled with extended error
        *              information.  If it is NULL (the default) extended error
        *              information is not returned.
        *
        * @returns The list of test sets held by this test set.
        *
        */
       virtual tITestSetIterator* GetTestSets(Error *err = 0) const = 0;

       /**
        * @brief returns the number of simple tests under this test.
        * @param  err Pointer to client-owned error structure.  If the pointer
        *              is not NULL this structure is filled with extended error
        *              information.  If it is NULL (the default) extended error
        *              information is not returned.
        *
        * @returns returns the number of simple tests under this test.
        */ 
       virtual unsigned int GetNumTests(Error *err = 0) const = 0;

       /**
        * @brief Get the number of test sets under this test set.
        * @param  err Pointer to client-owned error structure.  If the pointer
        *              is not NULL this structure is filled with extended error
        *              information.  If it is NULL (the default) extended error
        *              information is not returned.
        *
        * @returns Get the number of test sets under this test set.
        */
       virtual unsigned int GetNumTestSets(Error *err = 0) const = 0;

   };
}
#endif
