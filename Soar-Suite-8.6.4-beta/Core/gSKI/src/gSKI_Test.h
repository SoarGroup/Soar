/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file gSKI_Test.h
*********************************************************************
* created:	   6/27/2002   10:44
*
* purpose: 
*********************************************************************/
#ifndef GSKI_TEST_H
#define GSKI_TEST_H

#include "gSKI_Enumerations.h"
#include "gdatastructs.h"

#include <string>

typedef struct agent_struct agent;

namespace gSKI
{
   class gSymbol;
   class ISymbol;
   struct Error;

   /**
    * @brief: Class used to represent a simple condition test.
    *
    * These are the simplest form of test.  These tests are all of the
    * tests exluding conjunctive tests and disjunctive tests.  Conjunctive
    * tests are made up of simple tests, and disjunctive tests.  
    * Disjunctive tests are only made of of simple test conditions.
    *
    * This test class effectively becomes the leaf nodes of each condition.
    *
    * A simple condition could be made up a three simple tests, a complicated
    * condition could be made up of dozens of simple tests.
    */
   class Test
   {
   public:

      /*
       * @brief This is the constructor for the simple test.
       *
       * @param kernelTest This is the Soar Kernel version of a test.
       */
      Test(test kernelTest, agent* a);
      
      /**
       * @brief  Destructor for test.
       */
      ~Test();

      /**
       * @brief This returns the type of the test.
       *
       * @param  pErr Pointer to client-owned error structure.  If the pointer
       *              is not NULL this structure is filled with extended error
       *              information.  If it is NULL (the default) extended error
       *              information is not returned.
       *
       * @returns the test type. 
       *
       * @see egSKITestType
       */
      egSKITestType GetType( Error* err) const ;

      /**
       * @brief Returns the value being tested against.
       *
       * @returns The symbol which holds the value we are testing
       *          against.
       *
       * @param  pErr Pointer to client-owned error structure.  If the pointer
       *              is not NULL this structure is filled with extended error
       *              information.  If it is NULL (the default) extended error
       *              information is not returned.
       */
      ISymbol*  GetValue( Error* err ) const ;

      /**
       * @brief This verifies that the test being passed in is compatible
       *        with this test class.
       *
       * It is well worth noting that the use of simple here is not
       * entirely consistent with it's usage in the Soar Kernel.  The Soar
       * Kernel considers any non equals test to be complex.  Here, we consider
       * all single tests to be simple.  This means that all tests are
       * simple except disjunction and conjunction.  Those tests are handled
       * by the TestSet class.
       *
       * @param thisTest the test we are checking for simplicity.
       *
       * @returns Whether the Soar Kernel test thisTest is compatible with
       *          this class.
       */
      static bool IsSimpleTest(test thisTest);

      /**
       * @brief Returns a text version of the simple condition.
       */
      std::string GetText();

   private:

      gSymbol*        m_symbol; /**< This symbol represents the thing we are
                                 *< testing against. */
      
		egSKITestType   m_type;   /**< The type of test. @see egSKITestType */

      agent*          m_agent;
   };
}
#endif
