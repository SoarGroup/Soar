#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H
#include "portability.h"

/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file gski_conditionset.cpp
*********************************************************************
* created:	   6/27/2002   10:44
*
* purpose: 
*********************************************************************/
#include "gSKI_ConditionSet.h"
#include "gSKI_Condition.h"
#include "gSKI_Test.h"
#include "gSKI_Iterator.h"
#include "gSKI_TestSet.h"
#include "gSKI_Test.h"

#include <vector>
#include <iostream>

#include "gdatastructs.h"
#include "production.h"

namespace gSKI 
{
   /*
   ==================================
  ____                _ _ _   _             ____       _
 / ___|___  _ __   __| (_) |_(_) ___  _ __ / ___|  ___| |_
| |   / _ \| '_ \ / _` | | __| |/ _ \| '_ \\___ \ / _ \ __|
| |__| (_) | | | | (_| | | |_| | (_) | | | |___) |  __/ |_
 \____\___/|_| |_|\__,_|_|\__|_|\___/|_| |_|____/ \___|\__|
   ==================================
   */
   ConditionSet::ConditionSet(agent* a) : m_agent(a)
   {

   }

   /*
   ===============================
 /\/|___                _ _ _   _             ____       _
|/\/ ___|___  _ __   __| (_) |_(_) ___  _ __ / ___|  ___| |_
  | |   / _ \| '_ \ / _` | | __| |/ _ \| '_ \\___ \ / _ \ __|
  | |__| (_) | | | | (_| | | |_| | (_) | | | |___) |  __/ |_
   \____\___/|_| |_|\__,_|_|\__|_|\___/|_| |_|____/ \___|\__|
   ===============================
   */
   ConditionSet::~ConditionSet()
   {
      std::vector<ICondition*>::iterator condIt = m_conditions.begin();
      for( ; condIt != m_conditions.end(); ++condIt)
      {
         ICondition* c = *condIt;
         delete(c);
      }
   }

   /*
   ===============================
  ____      _    ____                _ _ _   _
 / ___| ___| |_ / ___|___  _ __   __| (_) |_(_) ___  _ __  ___
| |  _ / _ \ __| |   / _ \| '_ \ / _` | | __| |/ _ \| '_ \/ __|
| |_| |  __/ |_| |__| (_) | | | | (_| | | |_| | (_) | | | \__ \
 \____|\___|\__|\____\___/|_| |_|\__,_|_|\__|_|\___/|_| |_|___/
   ===============================
   */

   tIConditionIterator* ConditionSet::GetConditions(Error *pErr) 
   {
      //return new tConditionIterator(m_conditions);
      return new Iterator<ICondition *, tConditionVec>(m_conditions);
   }

   /*
   ===============================
  ____      _   _   _                  ____                _ _ _   _
 / ___| ___| |_| \ | |_   _ _ __ ___  / ___|___  _ __   __| (_) |_(_) ___  _ __  ___
| |  _ / _ \ __|  \| | | | | '_ ` _ \| |   / _ \| '_ \ / _` | | __| |/ _ \| '_ \/ __|
| |_| |  __/ |_| |\  | |_| | | | | | | |__| (_) | | | | (_| | | |_| | (_) | | | \__ \
 \____|\___|\__|_| \_|\__,_|_| |_| |_|\____\___/|_| |_|\__,_|_|\__|_|\___/|_| |_|___/
   ===============================
   */

   unsigned int ConditionSet::GetNumConditions(Error *pErr)const
   {
      return 0;
   }

   /*
   ===============================
  ____      _    ____                _ _ _   _             ____       _
 / ___| ___| |_ / ___|___  _ __   __| (_) |_(_) ___  _ __ / ___|  ___| |_ ___
| |  _ / _ \ __| |   / _ \| '_ \ / _` | | __| |/ _ \| '_ \\___ \ / _ \ __/ __|
| |_| |  __/ |_| |__| (_) | | | | (_| | | |_| | (_) | | | |___) |  __/ |_\__ \
 \____|\___|\__|\____\___/|_| |_|\__,_|_|\__|_|\___/|_| |_|____/ \___|\__|___/
   ===============================
   */

   tIConditionSetIterator *ConditionSet::GetConditionSets(Error *pErr) const
   {
      //return new Iterator<tConditionSet::V *, tConditionSet::t>(m_conditionSets);
      return new tConditionSetIter(m_conditionSets);
   }

   /*
   ===============================
  ____      _   _   _                  ____                _ _ _   _
 / ___| ___| |_| \ | |_   _ _ __ ___  / ___|___  _ __   __| (_) |_(_) ___  _ __
| |  _ / _ \ __|  \| | | | | '_ ` _ \| |   / _ \| '_ \ / _` | | __| |/ _ \| '_ \
| |_| |  __/ |_| |\  | |_| | | | | | | |__| (_) | | | | (_| | | |_| | (_) | | | |
 \____|\___|\__|_| \_|\__,_|_| |_| |_|\____\___/|_| |_|\__,_|_|\__|_|\___/|_| |_|
/ ___|  ___| |_ ___
\___ \ / _ \ __/ __|
 ___) |  __/ |_\__ \
|____/ \___|\__|___/
   ===============================
   */

   unsigned int ConditionSet::GetNumConditionSets(Error *pErr) const
   {
      return static_cast<unsigned int>(m_conditionSets.size());
   }

   /*
   ===============================
 ___     _   _                  _           _
|_ _|___| \ | | ___  __ _  __ _| |_ ___  __| |
 | |/ __|  \| |/ _ \/ _` |/ _` | __/ _ \/ _` |
 | |\__ \ |\  |  __/ (_| | (_| | ||  __/ (_| |
|___|___/_| \_|\___|\__, |\__,_|\__\___|\__,_|
                    |___/
   ===============================
   */

   bool ConditionSet::IsNegated(Error *pErr) const
   {
      return true;
   }

   /*
   ==================================
    _       _     _  ____                _ _ _   _
   / \   __| | __| |/ ___|___  _ __   __| (_) |_(_) ___  _ __
  / _ \ / _` |/ _` | |   / _ \| '_ \ / _` | | __| |/ _ \| '_ \
 / ___ \ (_| | (_| | |__| (_) | | | | (_| | | |_| | (_) | | | |
/_/   \_\__,_|\__,_|\____\___/|_| |_|\__,_|_|\__|_|\___/|_| |_|
   ==================================
   */
   void ConditionSet::AddCondition(condition *cond)
   {
      m_conditions.push_back(new Condition(cond, m_agent));
   }

   /*
   ==================================
    _       _     _  ____                _ _ _   _             ____       _
   / \   __| | __| |/ ___|___  _ __   __| (_) |_(_) ___  _ __ / ___|  ___| |_
  / _ \ / _` |/ _` | |   / _ \| '_ \ / _` | | __| |/ _ \| '_ \\___ \ / _ \ __|
 / ___ \ (_| | (_| | |__| (_) | | | | (_| | | |_| | (_) | | | |___) |  __/ |_
/_/   \_\__,_|\__,_|\____\___/|_| |_|\__,_|_|\__|_|\___/|_| |_|____/ \___|\__|
   ==================================
   */
   ConditionSet* ConditionSet::AddConditionSet()
   {
      ConditionSet* newConditionSet = new ConditionSet(m_agent);
      m_conditionSets.push_back(newConditionSet);
      return newConditionSet;
   }


}

//////////////////////////////////////////////////////////////////////////////
////                            Unit Tests                                ////
//////////////////////////////////////////////////////////////////////////////
#ifndef NO_MEGA_UNIT_TESTS
//#include "MegaUnitTest.h"

#include "gSKI_Agent.h"
#include "gSKI_AgentManager.h"
#include "gSKI_Kernel.h"
#include "gSKI_KernelFactory.h"
#include "gSKI_ProductionManager.h"
#include "gSKI_Stub.h"
#include "IgSKI_Production.h"

//DEF_EXPOSE(ConditionSet);

using namespace gSKI;


/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
char testProduction1[] =                                       \
"testProduction1"                                              \
"   (state <s> ^test <t> ^test2 <t2>)"                         \
"   (<t>       ^<< a b 5.0 1 >> { <> a >= b < c <bind1> })"    \
"   (<t2>      ^{ a <bind1> <bind2> } << x y z 3.14 20 >> )"   \
"-->"                                                          \
"   (<s> ^dosomething a)" ;
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
char testProduction1Result[] =                                 \
" <s>   test2   <t2>\n"                                        \
" <s>   test   <t>\n"                                          \
" <t2>  ( <> a >= b < c <bind1> <bind2> a)   <a*2><< ( x y z 3.14 20) >>\n"\
" <t>   <a*1><< ( a b 5 1) >>   <bind1>\n";
 /*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/


/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
char testProduction2[] =                                       \
"testProduction2"                                              \
"   (state <s> ^test <a> ^test <b>)"                           \
"  -{"                                                         \
"      (<a> ^is ok)"                                           \
"     -(<b> ^p  <q>)"                                          \
"     -{"                                                      \
"        -(<q> ^attachedto <b>)"                               \
"         (<b> ^combined-with <c>)"                            \
"        -{"                                                   \
"           -(<c> ^final-nest 4.5)"                            \
"           -(<b> ^whatever 24)"                               \
"         }"                                                   \
"      }"                                                      \
"   }"                                                         \
"-->"                                                          \
"   (<s> ^finally-done *yes*)";
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
char testProduction2Result[] =                                 \
" <s>   test   <b>\n"                                          \
" <s>   test   <a>\n"                                          \
"-{\n"                                                         \
"   -( <b>   p   <q>)\n"                                       \
"    <a>   is   ok\n"                                          \
"   -{\n"                                                      \
"      -( <q>   attachedto   <b>)\n"                           \
"       <b>   combined-with   <c>\n"                           \
"      -{\n"                                                   \
"         -( <b>   whatever   24)\n"                           \
"         -( <c>   final-nest   4.5)\n"                        \
"\n"                                                           \
"      }\n"                                                    \
"   }\n"                                                       \
"}";
/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/


/*
==================================
     _                     _____        _
  __| |_   _ _ __ ___  _ _|_   _|__ ___| |_ ___
 / _` | | | | '_ ` _ \| '_ \| |/ _ Y __| __/ __|
| (_| | |_| | | | | | | |_) | |  __|__ \ |_\__ \
 \__,_|\__,_|_| |_| |_| .__/|_|\___|___/\__|___/
                      |_|
==================================
*/
std::string dumpTests(ITestSet* ts)
{
   std::string out;
   //
   //
   if(ts->GetNumTests() > 1)
      out += "(";
   tITestIterator *titer = ts->GetTests();
   for( ; titer->IsValid() ; titer->Next() )
   {
      ITest* t = titer->GetVal();
      std::string s = dynamic_cast<Test*>(t)->GetText();
      out += " ";
      out += s;
   }
   if(ts->GetNumTests() > 1)
      out += ")";

   //
   //
   tITestSetIterator *tsiter = ts->GetTestSets();
   for( ; tsiter->IsValid() ; tsiter->Next() )
   {
      out += "<< " + dumpTests(tsiter->GetVal()) + " >>";
  }
   return out;
}

std::string printtablevel(int tablevel)
{
   std::string out;
   for(int i = 0 ; i < tablevel ; ++i)
      out += "   ";
   return out;
}

/*
==================================
     _                        ____                _ _ _   _
  __| |_   _ _ __ ___  _ __  / ___|___  _ __   __| (_) |_(_) ___  _ __  ___
 / _` | | | | '_ ` _ \| '_ \| |   / _ \| '_ \ / _` | | __| |/ _ \| '_ \/ __|
| (_| | |_| | | | | | | |_) | |__| (_) | | | | (_| | | |_| | (_) | | | \__ \
 \__,_|\__,_|_| |_| |_| .__/ \____\___/|_| |_|\__,_|_|\__|_|\___/|_| |_|___/
                      |_|
==================================
*/
std::string dumpConditions(IConditionSet* cset, int &tablevel)
{
   std::string out;
   //
   //
   tIConditionIterator *iters = cset->GetConditions();
   for( ; iters->IsValid() ; iters->Next())
   {
      ICondition* set = iters->GetVal();
      out += printtablevel(tablevel);
      if(set->IsNegated())
         out += "-(";
      out += dumpTests(set->GetIdTest());
      out += "  " + dumpTests(set->GetAttrTest());
      out += "  " + dumpTests(set->GetValTest());
      if(set->IsNegated())
         out += ")";
      out += "\n";
   }

   //
   //
   tIConditionSetIterator *cSetIter = cset->GetConditionSets();
   for( ; cSetIter->IsValid() ; cSetIter->Next())
   {
     IConditionSet* nextCSet = cSetIter->GetVal();
     out += printtablevel(tablevel);
      if(nextCSet->IsNegated()) 
         out +=   "-{\n";
      tablevel++;
      out += dumpConditions(nextCSet, tablevel);
      tablevel--;
      if(nextCSet->IsNegated()) 
         out += "\n" + printtablevel(tablevel)+ "}";
   }
   return out;
}


/*
==================================
                                  ____                _            _   _
 _ __  _ __ ___   ___ ___ ___ ___|  _ \ _ __ ___   __| |_   _  ___| |_(_) ___  _ __
| '_ \| '__/ _ \ / __/ _ Y __/ __| |_) | '__/ _ \ / _` | | | |/ __| __| |/ _ \| '_ \
| |_) | | | (_) | (_|  __|__ \__ \  __/| | | (_) | (_| | |_| | (__| |_| | (_) | | | |
| .__/|_|  \___/ \___\___|___/___/_|   |_|  \___/ \__,_|\__,_|\___|\__|_|\___/|_| |_|
|_|
==================================
*/
void processProduction(const char* prodName, 
                       const char* production,
                       const char* expectedResult, 
                       IProductionManager*  IPM)
{
   IPM->AddProduction(const_cast<char *>(production));

   tIProductionIterator* prodIter = IPM->GetProduction(prodName);
   
   MegaAssert(prodIter->IsValid(), "Could not find production.");
   IProduction* ip = prodIter->GetVal();

   IConditionSet* cSet = ip->GetConditions();

   int junk=0;
   std::string result = dumpConditions(cSet, junk);

   std::cout << result << std::endl;

   //tIConditionIterator *iters = cSet->GetConditions();

   int is_same = result.compare(expectedResult);
   std::cout << "=====================================\n";
   std::cout << expectedResult;
   VALIDATE( is_same == 0);

   ip->Release();
   prodIter->Release();
}

/*
==================================
  ____                _ _ _   _           _____        _   _
 / ___|___  _ __   __| (_) |_(_) ___  _ _|_   _|__ ___| |_(_)_ __   __ _
| |   / _ \| '_ \ / _` | | __| |/ _ \| '_ \| |/ _ Y __| __| | '_ \ / _` |
| |__| (_) | | | | (_| | | |_| | (_) | | | | |  __|__ \ |_| | | | | (_| |
 \____\___/|_| |_|\__,_|_|\__|_|\___/|_| |_|_|\___|___/\__|_|_| |_|\__, |
                                                                   |___/
==================================
*/
DEF_TEST_INSUITE(ConditionTesting, Start)
{
   IKernelFactory*      kF    = gSKI_CreateKernelFactory();
   VALIDATE(kF != 0);

   IKernel*             k     = kF->Create();
   VALIDATE(k != 0);

   IAgentManager*       IAM   = k->GetAgentManager();
   VALIDATE(IAM != 0);

   IAgent*              agent = IAM->AddAgent("ProductionTestAgent");
   VALIDATE(agent != 0);

   IProductionManager*  IPM   = agent->GetProductionManager();
   VALIDATE(IPM != 0);

   std::cout << "***********************************************************" << std::endl;
   processProduction("testProduction1", testProduction1, testProduction1Result, IPM);
   std::cout << "***********************************************************" << std::endl;
   processProduction("testProduction2", testProduction2, testProduction2Result, IPM);

}
#endif

