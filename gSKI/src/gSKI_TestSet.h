/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file gSKI_TestSet.h
*********************************************************************
* created:	   6/27/2002   10:44
*
* purpose: 
*********************************************************************/
#ifndef GSKI_TESTSET_H
#define GSKI_TESTSET_H

#include "IgSKI_TestSet.h"

typedef char * test;

typedef struct agent_struct agent;

namespace gSKI 
{
   struct Error;
   class  Test;

   /**
   * @brief: This is used to represent a set of Tests
   */
   class TestSet : public ITestSet
   {
   public:

      /**
       * @brief 
       */
      //{
      //TestSet();
      TestSet(const test t, agent* a);
      //}

      /**
       * @brief
       */
      virtual ~TestSet();
      
       /**
        * @brief 
        * @param  err Pointer to client-owned error structure.  If the pointer
        *              is not NULL this structure is filled with extended error
        *              information.  If it is NULL (the default) extended error
        *              information is not returned.
        *
        * @returns the list of tests held by this test-set.
        */
       tITestIterator *GetTests(Error *err = 0) const ;

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
       tITestSetIterator *GetTestSets(Error *err = 0) const;

       /**
        * @brief Adds a test to this test set.
        */
       void AddTest(const test t);


       /**
        * @brief returns the number of simple tests under this test.
        * @param  err Pointer to client-owned error structure.  If the pointer
        *              is not NULL this structure is filled with extended error
        *              information.  If it is NULL (the default) extended error
        *              information is not returned.
        *
        * @returns returns the number of simple tests under this test.
        */ 
       unsigned int GetNumTests(Error *err = 0) const;

       /**
        * @brief Get the number of test sets under this test set.
        * @param  err Pointer to client-owned error structure.  If the pointer
        *              is not NULL this structure is filled with extended error
        *              information.  If it is NULL (the default) extended error
        *              information is not returned.
        *
        * @returns Get the number of test sets under this test set.
        */
       unsigned int GetNumTestSets(Error *err = 0) const;

       /**
        * @brief Adds a Test Set to this test set.
        */
       void AddTestSet(const test t);

   private:
      typedef std::vector<ITest *> tTestVec;
      typedef std::vector<ITestSet *> tTestSetVec;

      tTestVec     m_tests;
      tTestSetVec  m_testSets;

      agent *m_agent;
   };
}
#endif
