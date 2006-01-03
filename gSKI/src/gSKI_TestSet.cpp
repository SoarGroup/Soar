#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H
#include "portability.h"

/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file gski_testset.cpp
*********************************************************************
* created:	   6/27/2002   10:44
*
* purpose: 
*********************************************************************/
#include "gSKI_TestSet.h"
#include "gSKI_Test.h"
#include "gSKI_Iterator.h"

#include "gdatastructs.h"
#include "mem.h"

namespace gSKI 
{


   /*
   ==================================
 _____        _   ____       _
|_   _|__ ___| |_/ ___|  ___| |_
  | |/ _ Y __| __\___ \ / _ \ __|
  | |  __|__ \ |_ ___) |  __/ |_
  |_|\___|___/\__|____/ \___|\__|
   ==================================
   */
   TestSet::TestSet(test thisTest, agent* a) : m_agent(a)
   {
      if(Test::IsSimpleTest(thisTest))
      {
         AddTest(thisTest);
      }
      else
      {
         AddTestSet(thisTest);
      }
   }

   /*
   ==================================
 /\/|____        _   ____       _
|/\/_   _|__ ___| |_/ ___|  ___| |_
     | |/ _ Y __| __\___ \ / _ \ __|
     | |  __|__ \ |_ ___) |  __/ |_
     |_|\___|___/\__|____/ \___|\__|
   ==================================
   */
   TestSet::~TestSet()
   {
      std::vector<ITest *>::iterator testIt = m_tests.begin();
      for( ; testIt != m_tests.end() ; ++testIt)
      {
         delete(*testIt);
      }
      m_tests.clear();


      std::vector<ITestSet *>::iterator djIt = m_testSets.begin();
      for( ; djIt != m_testSets.end() ; ++djIt)
      {
         delete(*djIt);
      }
      m_testSets.clear();
   }


   /*
   ==================================
  ____      _  _____        _
 / ___| ___| ||_   _|__ ___| |_ ___
| |  _ / _ \ __|| |/ _ Y __| __/ __|
| |_| |  __/ |_ | |  __|__ \ |_\__ \
 \____|\___|\__||_|\___|___/\__|___/
   ==================================
   */
   tITestIterator *TestSet::GetTests(Error * /*err*/) const
   {
      return new Iterator<ITest *, tTestVec>(m_tests);
   }

   /*
   ==================================
  ____      _  _____        _   ____       _
 / ___| ___| ||_   _|__ ___| |_/ ___|  ___| |_ ___
| |  _ / _ \ __|| |/ _ Y __| __\___ \ / _ \ __/ __|
| |_| |  __/ |_ | |  __|__ \ |_ ___) |  __/ |_\__ \
 \____|\___|\__||_|\___|___/\__|____/ \___|\__|___/
   ==================================
   */
   tITestSetIterator *TestSet::GetTestSets(Error * /*err*/) const
   {
      return new Iterator<ITestSet *, tTestSetVec>(m_testSets);
   }

   /*
   ==================================
    _       _     _ _____        _
   / \   __| | __| |_   _|__ ___| |_
  / _ \ / _` |/ _` | | |/ _ Y __| __|
 / ___ \ (_| | (_| | | |  __|__ \ |_
/_/   \_\__,_|\__,_| |_|\___|___/\__|
   ==================================
   */
   void TestSet::AddTest(const test t)
   {
      m_tests.push_back(new Test(t, m_agent));
   }

   /*
   ==================================
    _       _     _ _____        _   ____       _
   / \   __| | __| |_   _|__ ___| |_/ ___|  ___| |_
  / _ \ / _` |/ _` | | |/ _ Y __| __\___ \ / _ \ __|
 / ___ \ (_| | (_| | | |  __|__ \ |_ ___) |  __/ |_
/_/   \_\__,_|\__,_| |_|\___|___/\__|____/ \___|\__|
   ==================================
   */
   void TestSet::AddTestSet(const test ts)
   {
      //
      // We know we have either a conjunction or a disjunction at this point.
      // We will check which one, then traverse the list adding each test in
      // the conjunction and disjunction to this TestSet.
      complex_test *ct = complex_test_from_test(ts);

      list *testList;
      if(ct->type == DISJUNCTION_TEST)
         testList = ct->data.disjunction_list;
      else
         testList = ct->data.conjunct_list;

      test t;
      for(;testList != 0; testList = testList->rest)
      {
         t = (test)(testList->first);
         if(Test::IsSimpleTest(t))
         {
            m_tests.push_back(new Test(t, m_agent));
         }
         else
         {
            m_testSets.push_back(new TestSet(t, m_agent));
         }
      }
   }

   /*
   ==================================
  ____      _   _   _               _____        _
 / ___| ___| |_| \ | |_   _ _ __ __|_   _|__ ___| |_ ___
| |  _ / _ \ __|  \| | | | | '_ ` _ \| |/ _ Y __| __/ __|
| |_| |  __/ |_| |\  | |_| | | | | | | |  __|__ \ |_\__ \
 \____|\___|\__|_| \_|\__,_|_| |_| |_|_|\___|___/\__|___/
   ==================================
   */
   unsigned int TestSet::GetNumTests(Error * /*err*/) const
   {
      return static_cast<unsigned int>(m_tests.size());
   }

   /*
   ==================================
  ____      _   _   _               _____        _   ____       _
 / ___| ___| |_| \ | |_   _ _ __ __|_   _|__ ___| |_/ ___|  ___| |_ ___
| |  _ / _ \ __|  \| | | | | '_ ` _ \| |/ _ Y __| __\___ \ / _ \ __/ __|
| |_| |  __/ |_| |\  | |_| | | | | | | |  __|__ \ |_ ___) |  __/ |_\__ \
 \____|\___|\__|_| \_|\__,_|_| |_| |_|_|\___|___/\__|____/ \___|\__|___/
   ==================================
   */
   unsigned int TestSet::GetNumTestSets(Error * /*err*/) const
   {
      return static_cast<unsigned int>(m_testSets.size());
   }


}

