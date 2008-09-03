#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file gski_production.cpp
*********************************************************************
* created:	   6/27/2002   10:44
*
* purpose: 
*********************************************************************/
#include "gSKI_Production.h"
#include "gSKI_Error.h"
#include "gSKI_Iterator.h"
#include "gSKI_ConditionSet.h"
#include "gSKI_Condition.h"
#include "gSKI_Iterator.h"
#include "gSKI_EnumRemapping.h"

//#include "MegaUnitTest.h"

#include <string>

//
// Soar includes.
#include "production.h"
#include "symtab.h"
#include "rete.h"
#include "print.h"
#include "gdatastructs.h"
#include "rhsfun.h"

//DEF_EXPOSE(gSKI_Production);

namespace gSKI
{
   /*
   ==================================
 ____                _            _   _
|  _ \ _ __ ___   __| |_   _  ___| |_(_) ___  _ __
| |_) | '__/ _ \ / _` | | | |/ __| __| |/ _ \| '_ \
|  __/| | | (_) | (_| | |_| | (__| |_| | (_) | | | |
|_|   |_|  \___/ \__,_|\__,_|\___|\__|_|\___/|_| |_|
   ==================================
   */
   Production::Production(production *prod, agent* agent): m_soarProduction(prod),
                                                           m_agent(agent),
                                                           m_conditionSet(0)  
   {
      condition* top;
      condition* bottom;
      action*    rhs;
      m_conditionSet = new ConditionSet(m_agent);
      
      // This will extract the structures for the lhs and the rhs
      p_node_to_conditions_and_nots (m_agent, m_soarProduction->p_node, 0, 0, &top, &bottom, 0,&rhs);

      // We tranlate the structures to the gSKI structures
      loadConditions(top, *m_conditionSet);
      loadActions(rhs, m_actions, m_functions);
   }

   /** 
    * @brief Copy operator ensuring a correct deep copy.
    */
   Production&  Production::operator=(const Production& rhs)
   {
      if(this != &rhs)
      {
         // Cleanup all old memory
         cleanup();

         m_soarProduction = rhs.m_soarProduction;
         m_agent          = rhs.m_agent;
         m_conditionSet   = rhs.m_conditionSet;
        
         // Copy the actions and rhs functions
         for(tRhsActionVecCIt ita = rhs.m_actions.begin(); 
             ita != rhs.m_actions.end(); ++ita)
         {
            m_actions.push_back(new RhsAction(*(*ita)));
         }

         for(tRhsFunctionVecCIt itf = rhs.m_functions.begin(); 
             itf != rhs.m_functions.end(); ++itf)
         {
            m_functions.push_back(new RhsFunctionAction(*(*itf)));
         }
      }
      return *this;
   }

   /*
   ===============================
 /\/|____                _            _   _
|/\/|  _ \ _ __ ___   __| |_   _  ___| |_(_) ___  _ __
    | |_) | '__/ _ \ / _` | | | |/ __| __| |/ _ \| '_ \
    |  __/| | | (_) | (_| | |_| | (__| |_| | (_) | | | |
    |_|   |_|  \___/ \__,_|\__,_|\___|\__|_|\___/|_| |_|
   ===============================
   */
   Production::~Production() 
   {
      cleanup();
   }

   /*
   ===============================

   ===============================
   */
   void Production::cleanup()
   {
      if(m_conditionSet)
      {
         delete m_conditionSet;
         m_conditionSet = 0;
      }

      // Delete all rhs actions
      for(tRhsActionVecIt ita = m_actions.begin(); ita != m_actions.end(); ++ita)
         delete (*ita);

      // Delete all rhs functions (standalone)
      for(tRhsFunctionVecIt itf = m_functions.begin(); itf != m_functions.end(); ++itf)
         delete (*itf);

      // Clear out the vectors
      m_actions.clear();
      m_functions.clear();
   }

   /*
   ===============================
  ____      _   _____ _      _              ____                  _
 / ___| ___| |_|  ___(_)_ __(_)_ __   __ _ / ___|___  _   _ _ __ | |_
| |  _ / _ \ __| |_  | | '__| | '_ \ / _` | |   / _ \| | | | '_ \| __|
| |_| |  __/ |_|  _| | | |  | | | | | (_| | |__| (_) | |_| | | | | |_
 \____|\___|\__|_|   |_|_|  |_|_| |_|\__, |\____\___/ \__,_|_| |_|\__|
                                     |___/
   ===============================
   */

   unsigned long Production::GetFiringCount(Error *err) const
   {
      return m_soarProduction->firing_count;
   }

   /*
   ===============================
  ____      _   _   _
 / ___| ___| |_| \ | | __ _ _ __ ___   ___
| |  _ / _ \ __|  \| |/ _` | '_ ` _ \ / _ \
| |_| |  __/ |_| |\  | (_| | | | | | |  __/
 \____|\___|\__|_| \_|\__,_|_| |_| |_|\___|
   ===============================
   */

   const char* Production::GetName(Error *err) const
   {
      MegaAssert(m_soarProduction != 0, "Uninitialized PRoduction.");

      static std::string s("");

      if(s.length() == 0)
      {
         MegaAssert(m_soarProduction->name->sc.name != 0, "Empty Production Name");
         return m_soarProduction->name->sc.name;
      }
      return 0;
   }

   /*
   ==================================
   
   ==================================
   */
   egSKIProdType Production::GetType(Error *pErr) const
   {
      MegaAssert(m_soarProduction != 0, "Uninitialized PRoduction.");

      return EnumRemappings::ReMapProductionType(m_soarProduction->type);
   }


   /*
   ===============================
  ____      _   ____
 / ___| ___| |_|  _ \  ___   ___ ___
| |  _ / _ \ __| | | |/ _ \ / __/ __|
| |_| |  __/ |_| |_| | (_) | (__\__ \
 \____|\___|\__|____/ \___/ \___|___/
   ===============================
   */

   const char* Production::GetDocs(Error *err) const
   {

      return 0;
   }

   /*
   ===============================
  ____      _   ____
 / ___| ___| |_/ ___|  ___  _   _ _ __ ___ ___
| |  _ / _ \ __\___ \ / _ \| | | | '__/ __/ _ \
| |_| |  __/ |_ ___) | (_) | |_| | | | (_|  __/
 \____|\___|\__|____/ \___/ \__,_|_|  \___\___|
   ===============================
   */

   const char* Production::GetSource(Error *err) const
   {

      return 0;
   }

   void dumpConditions(ConditionSet* cs)
   {
      tIConditionSetIterator* CondSets = cs->GetConditionSets();
      while(CondSets->IsValid());
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

   const char* Production::GetText(Error *err) const 
   {
//      condition *top, *bottom;
//      action* rhs;
//      bool internal=true;  // The "internal" parameter, if TRUE,
//                           // indicates that the LHS and RHS should 
//                           // be printed in internal format.
//    
//      p_node_to_conditions_and_nots (m_agent, m_soarProduction->p_node, 0, 0, &top, &bottom, 0,&rhs);
//      print_string ("   ");
//        
//      print_condition_list (top, 3, internal);
//
//      loadConditions(top, *m_conditionSet);
//
//      deallocate_condition_list (top);  
//        
//      print_string ("\n    -->\n  "); 
//      print_string ("  "); 
//      print_action_list (rhs, 4, internal);
//      print_string ("\n}\n");
//         
      return 0;
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

   IConditionSet* Production::GetConditions(Error *err) const
   {
      return m_conditionSet;
   }

   /*
   ===============================
  ____      _      _        _   _
 / ___| ___| |_   / \   ___| |_(_) ___  _ __  ___
| |  _ / _ \ __| / _ \ / __| __| |/ _ \| '_ \/ __|
| |_| |  __/ |_ / ___ \ (__| |_| | (_) | | | \__ \
 \____|\___|\__/_/   \_\___|\__|_|\___/|_| |_|___/
   ===============================
   */

   tIRhsActionIterator* Production::GetActions(Error *err)
   {
      ClearError(err);
      return new Iterator<IRhsAction*, tRhsActionVec>(m_actions);
   }

   /*
   ===============================
  ____      _   ____  _                  _    _    _
 / ___| ___| |_/ ___|| |_ __ _ _ __   __| |  / \  | | ___  _ __   ___
| |  _ / _ \ __\___ \| __/ _` | '_ \ / _` | / _ \ | |/ _ \| '_ \ / _ \
| |_| |  __/ |_ ___) | || (_| | | | | (_| |/ ___ \| | (_) | | | |  __/
 \____|\___|\__|____/ \__\__,_|_| |_|\__,_/_/   \_\_|\___/|_| |_|\___|
|  ___|   _ _ __   ___| |_(_) ___  _ __  ___
| |_ | | | | '_ \ / __| __| |/ _ \| '_ \/ __|
|  _|| |_| | | | | (__| |_| | (_) | | | \__ \
|_|   \__,_|_| |_|\___|\__|_|\___/|_| |_|___/
   ===============================
   */
   tIRhsFunctionActionIterator* Production::GetStandAloneFunctions(Error* err)
   {
      ClearError(err);
      return new Iterator<IRhsFunctionAction*, tRhsFunctionVec>(m_functions);
   }

   /*
   ===============================
  ____      _   __  __       _       _
 / ___| ___| |_|  \/  | __ _| |_ ___| |__   ___ ___
| |  _ / _ \ __| |\/| |/ _` | __/ __| '_ \ / _ Y __|
| |_| |  __/ |_| |  | | (_| | || (__| | | |  __|__ \
 \____|\___|\__|_|  |_|\__,_|\__\___|_| |_|\___|___/
   ===============================
   */

   tIMatchIterator* Production::GetMatches(Error *err)
   {
      ClearError(err);
      return 0;
   }


   /*
   ==================================

   ==================================
   */
   void Production::Excise(Error *pErr )
   {
      excise_production(m_agent, m_soarProduction, true);
   }

   unsigned long Production::CountReteTokens(Error * pErr /*= 0*/)
   {
      ClearError(pErr);
      
      return count_rete_tokens_for_production(m_agent, m_soarProduction);
   }

   /*
   ==================================
 _                 _ ____                _            _   _
| | ___   __ _  __| |  _ \ _ __ ___   __| |_   _  ___| |_(_) ___  _ __
| |/ _ \ / _` |/ _` | |_) | '__/ _ \ / _` | | | |/ __| __| |/ _ \| '_ \
| | (_) | (_| | (_| |  __/| | | (_) | (_| | |_| | (__| |_| | (_) | | | |
|_|\___/ \__,_|\__,_|_|   |_|__\___/ \__,_|\__,_|\___|\__|_|\___/|_| |_|
|  ___| __ ___  _ __ ___ | |/ /___ _ __ _ __   ___| |
| |_ | '__/ _ \| '_ ` _ \| ' // _ \ '__| '_ \ / _ \ |
|  _|| | | (_) | | | | | | . \  __/ |  | | | |  __/ |
|_|  |_|  \___/|_| |_| |_|_|\_\___|_|  |_| |_|\___|_|
   ==================================
   */
   void Production::loadProductionFromKernel(void) const
   {
      
   }

   /*
   ==================================
 _                 _  ____                _ _ _   _
| | ___   __ _  __| |/ ___|___  _ __   __| (_) |_(_) ___  _ __  ___
| |/ _ \ / _` |/ _` | |   / _ \| '_ \ / _` | | __| |/ _ \| '_ \/ __|
| | (_) | (_| | (_| | |__| (_) | | | | (_| | | |_| | (_) | | | \__ \
|_|\___/ \__,_|\__,_|\____\___/|_| |_|\__,_|_|\__|_|\___/|_| |_|___/
   ==================================
   */
   void Production::loadConditions(condition *conds, ConditionSet &condSet) const
   {
      if(conds == 0) 
         return;
      
      for(condition *c = conds; c != 0; c = c->next)
      {
         //
         // If we have a negated set.
         if (c->type == CONJUNCTIVE_NEGATION_CONDITION)
         {
            loadConditions((c)->data.ncc.top, *(condSet.AddConditionSet()));
         }
         else
         {
            condSet.AddCondition(c);
         }
      }
   }

   /*
   ==================================
 _                 _    _        _   _
| | ___   __ _  __| |  / \   ___| |_(_) ___  _ __  ___
| |/ _ \ / _` |/ _` | / _ \ / __| __| |/ _ \| '_ \/ __|
| | (_) | (_| | (_| |/ ___ \ (__| |_| | (_) | | | \__ \
|_|\___/ \__,_|\__,_/_/   \_\___|\__|_|\___/|_| |_|___/
   ==================================
   */
   void Production::loadActions(action* actions, tRhsActionVec& actionVec, tRhsFunctionVec& funcVec)
   {
      if(actions == 0)
         return;

      for(action* a = actions; a != 0; a = a->next)
      {
         if(a->type == MAKE_ACTION)
         {
            actionVec.push_back(new RhsAction(m_agent, a));
         }
         else if(a->type == FUNCALL_ACTION)
         {
            MegaAssert(rhs_value_is_funcall(a->value), "Rhs function is supposed to be a standalone function.");
            if(rhs_value_is_funcall(a->value))
               funcVec.push_back(new RhsFunctionAction(m_agent, rhs_value_to_funcall_list(a->value)));
         }
         else
         {
            MegaAssert(false, "Unknown type of action for a production.");
         }
      }
   }

}
