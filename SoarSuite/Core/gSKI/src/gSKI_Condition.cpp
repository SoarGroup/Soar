#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H
#include "portability.h"

/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file gski_condition.cpp
*********************************************************************
* created:	   6/27/2002   10:44
*
* purpose: 
*********************************************************************/
#include "gSKI_Condition.h"
#include "gSKI_Enumerations.h"
#include "gSKI_Test.h"
#include "gSKI_TestSet.h"
#include "gSKI_EnumRemapping.h"

#include "gdatastructs.h"
#include "production.h"
#include "print.h"
#include "MegaAssert.h"

namespace gSKI 
{


   /*
   ==================================
  ____                _ _ _   _
 / ___|___  _ __   __| (_) |_(_) ___  _ __
| |   / _ \| '_ \ / _` | | __| |/ _ \| '_ \
| |__| (_) | | | | (_| | | |_| | (_) | | | |
 \____\___/|_| |_|\__,_|_|\__|_|\___/|_| |_|
   ==================================
   */
   Condition::Condition(condition *cond, agent* a) : m_condition(cond),
                                                     m_agent(a),
                                                     m_idTest(0),
                                                     m_attrTest(0),
                                                     m_valTest(0)
   {
     Bool removed_goal_test;
     Bool removed_impasse_test;
   	
	   test id_test = copy_test_removing_goal_impasse_tests(m_agent, 
                                                           m_condition->data.tests.id_test, 
	                                                        &removed_goal_test, 
	                                                        &removed_impasse_test);
	   m_idTest   = new TestSet(id_test, m_agent);
   	
	   m_attrTest = new TestSet(m_condition->data.tests.attr_test, m_agent);
   	
	   m_valTest = new TestSet(m_condition->data.tests.value_test, m_agent);
   }

   /*
   ===============================
 /\/|___                _ _ _   _
|/\/ ___|___  _ __   __| (_) |_(_) ___  _ __
  | |   / _ \| '_ \ / _` | | __| |/ _ \| '_ \
  | |__| (_) | | | | (_| | | |_| | (_) | | | |
   \____\___/|_| |_|\__,_|_|\__|_|\___/|_| |_|
   ===============================
   */
   Condition::~Condition()
   {
	   if(m_idTest   != 0) delete m_idTest;
	   if(m_attrTest != 0) delete m_attrTest;
	   if(m_valTest  != 0) delete m_valTest;
   }

   /*
   ===============================
  ____      _  _____         _
 / ___| ___| ||_   _|____  _| |_
| |  _ / _ \ __|| |/ _ \ \/ / __|
| |_| |  __/ |_ | |  __/>  <| |_
 \____|\___|\__||_|\___/_/\_\\__|
   ===============================
   */

   const char* Condition::GetText(Error* err)
   {

      return 0;
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

   bool Condition::IsNegated(Error* err)
   {
      return m_condition->type == NEGATIVE_CONDITION;
   }

   /*
   ===============================
 ___     ____  _        _      _____        _
|_ _|___/ ___|| |_ __ _| |_ __|_   _|__ ___| |_
 | |/ __\___ \| __/ _` | __/ _ \| |/ _ Y __| __|
 | |\__ \___) | || (_| | ||  __/| |  __|__ \ |_
|___|___/____/ \__\__,_|\__\___||_|\___|___/\__|
   ===============================
   */

   bool Condition::IsImpasseCondition(Error* err)
   {
      return false;
   }

   /*
   ===============================
 ___     ___                                   _____        _
|_ _|___|_ _|_ __ ___  _ __   __ _ ___ ___  __|_   _|__ ___| |_
 | |/ __|| || '_ ` _ \| '_ \ / _` / __/ __|/ _ \| |/ _ Y __| __|
 | |\__ \| || | | | | | |_) | (_| \__ \__ \  __/| |  __|__ \ |_
|___|___/___|_| |_| |_| .__/ \__,_|___/___/\___||_|\___|___/\__|
                      |_|
   ===============================
   */

   bool Condition::IsStateCondition(Error* err)
   {
      return false;
   }

   /*
   ==================================
  ____      _   ___    _ _____        _
 / ___| ___| |_|_ _|__| |_   _|__ ___| |_
| |  _ / _ \ __|| |/ _` | | |/ _ Y __| __|
| |_| |  __/ |_ | | (_| | | |  __|__ \ |_
 \____|\___|\__|___\__,_| |_|\___|___/\__|
   ==================================
   */
   TestSet* Condition::GetIdTest(Error* err)
   {
      return m_idTest;
   }

   /*
   ==================================
  ____      _      _   _   _       _____        _
 / ___| ___| |_   / \ | |_| |_ _ _|_   _|__ ___| |_
| |  _ / _ \ __| / _ \| __| __| '__|| |/ _ Y __| __|
| |_| |  __/ |_ / ___ \ |_| |_| |   | |  __|__ \ |_
 \____|\___|\__/_/   \_\__|\__|_|   |_|\___|___/\__|
   ==================================
   */
   TestSet* Condition::GetAttrTest(Error* err)
   {
      return m_attrTest;
   }

   /*
   ==================================
  ____      _ __     __    _ _____        _
 / ___| ___| |\ \   / /_ _| |_   _|__ ___| |_
| |  _ / _ \ __\ \ / / _` | | | |/ _ Y __| __|
| |_| |  __/ |_ \ V / (_| | | | |  __|__ \ |_
 \____|\___|\__| \_/ \__,_|_| |_|\___|___/\__|
   ==================================
   */
   TestSet* Condition::GetValTest(Error* err)
   {
      return m_valTest;
   }



}
