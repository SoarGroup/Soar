#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file gSKI_Agent.cpp
*********************************************************************
* created:	   6/27/2002   10:44
*
* purpose: 
*********************************************************************/
#include "gSKI_Agent.h"
#include "gSKI_Error.h"
#include "MegaAssert.h"
#include "gSKI_ProductionManager.h"
#include "gSKI_AgentManager.h"
#include "gSKI_InputLink.h"
#include "gSKI_OutputLink.h"
#include "gSKI_WorkingMemory.h"
#include "gSKI_EnumRemapping.h"
#include "gSKI_Kernel.h"
#include "gSKI_SymbolFactory.h"
#include "gSKI_Symbol.h"
#include "gSKI_ObjectToPtrIterator.h"
#include "IgSKI_RhsFunction.h"
#include "agent.h"
#include "init_soar.h"
#include "gski_event_system_functions.h"
#include "rhsfun.h"
#include "production.h" // for struct multi_attributes
#include "print.h"      // for symboltostring
#include "gSKI_AgentPerformanceMonitor.h"
#include "gSKI_MultiAttribute.h"

//#include "MegaUnitTest.h"
//DEF_EXPOSE(gSKI_Agent);

// Private namespace for the workaround for callbacks
namespace {

   /** 
    * @brief Creates a Soar symbol based on a gSKI::ISymbol that is passed in
    *
    * This function is used as a helper for the function below to convert values
    *   returned by the IRhsFunction object to Soar Symbols.
    *
    * @param thisAgent Pointer to the Soar agent from which to allocate the soar symbol
    * @param sym       Pointer to the ISymbol object to convert
    * @return A pointer to a soar symbol object with the same value as the given sym.
    *          This symbol will already have had its ref count incremented.
    */
   Symbol* createSoarSymbolFromISymbol(agent* thisAgent, gSKI::ISymbol* sym)
   {
      MegaAssert(sym->GetType() != gSKI_OBJECT, "Do not support returning WMOs from RHS functions.");
      MegaAssert(sym->GetType() != gSKI_VARIABLE, "Do not support returning parse variables from RHS functions.");
      switch(sym->GetType())
      {
         case gSKI_INT:
            return make_int_constant(thisAgent, sym->GetInt());
            break;
         case gSKI_DOUBLE:
            return make_float_constant(thisAgent, static_cast<float>(sym->GetDouble()));
            break;
         case gSKI_STRING:
            // Gotta create a temp buffer because make_sym_constant takes a non-const pointer
            std::vector<char> tmpBuffer(sym->GetString(), sym->GetString() + strlen(sym->GetString()) + 1);
            return make_sym_constant(thisAgent, &tmpBuffer[0]);
            break;
      }
      return NIL;
   }

   /** 
    * @brief Interfaces with Soar to map kernel rhs functions to gSKI rhs functions
    *
    *  This method gets registered with the underlying kernel and recieves callbacks
    *   for client defined RHS functions.   It uses the user-data member of the
    *   kernel RHS function structure to store the this pointer of the gSKI object
    *   then calls back the gSKI object as desired.
    */
   Symbol* rhsFunctionProxy(agent* thisAgent, list* args, void* user_data)
   {
      //typedef gSKI::ObjectToPtrIterator<gSKI::ISymbol*, std::vector<gSKI::gSymbol> > tIterator;
      typedef gSKI::Iterator<gSKI::ISymbol*, std::vector<gSKI::gSymbol*> > tIterator;

      // Since we registered this callback, we know what the user data is.
      gSKI::IRhsFunction* rhsFunction = static_cast<gSKI::IRhsFunction*>(user_data);

      // Prepare arguments

      // Event though the agent has its own symbol factory, we create a new one because
      //  we have no way to access the old one conveniently.  This should not cause
      //  any problems.
      gSKI::SymbolFactory sf(thisAgent);

      // List of symbols wrapped in gSymbols
      std::vector<gSKI::gSymbol*> symVector;
      for(; args != NIL; args = args = args->rest)
      {
         // Cannnot yet support this, so we have to check for identifier symbols
         //if(sym->sc.common_symbol_info.symbol_type != IDENTIFIER_SYMBOL_TYPE) {
	         symVector.push_back(new gSKI::gSymbol(thisAgent, static_cast<Symbol*>(args->first), 0, false));
	 //} else {
	 // Cannnot yet support this, so we have to check for identifier symbols
	 //MegaAssert(false,"Cannot yet support WMO's passed to RHS functions");
	 //}
      }

      Symbol* pSoarReturn = 0;

      // Check to make sure we have the right number of arguments.   
      if( (rhsFunction->GetNumExpectedParameters() == gSKI_PARAM_NUM_VARIABLE) ||
          ((int)symVector.size() == rhsFunction->GetNumExpectedParameters()) )
      {
         // Actually make the call.  We can do the dynamic cast because we passed in the
         //  symbol factory and thus know how the symbol was created.
         tIterator it(symVector);
         gSKI::ISymbol* pReturn = rhsFunction->Execute(&it, &sf);

         // Return the result, assuming it is not NIL
         if(rhsFunction->IsValueReturned() == true)
         {
            // There should be a return value
            MegaAssert(pReturn != 0, "Expected return value from RHS function.");
            if(pReturn != 0)
            {
               // This adds a reference count
               Symbol* s = createSoarSymbolFromISymbol(thisAgent, pReturn);

               // Return the result
               pSoarReturn = s;
            }
            else
            {
               // We have to return something to prevent a crash, so we return an error code
               pSoarReturn = make_sym_constant(thisAgent, "error_expected_rhs_function_to_return_value_but_it_did_NOT");
            }
         }
         else
         {
            MegaAssert(pReturn == 0, "Expected that the rhs function would not return a value, but it did.  Return value ignored.");
         }

         // In any case, we are done using the return value
         if(pReturn != 0)
            pReturn->Release();
      }
      else
      {
         MegaAssert(false, "Wrong number of arguments passed to rhsFunction.");

         if(rhsFunction->IsValueReturned() == true)
            // We can return anything we want to soar; we return an error message so at least the problem is obvious.
            pSoarReturn = make_sym_constant(thisAgent, "error_wrong_number_of_args_passed_to_rhs_function");
      }

      for(std::vector<gSKI::gSymbol*>::iterator it = symVector.begin(); it != symVector.end(); ++it)
      {
         (*it)->Release();
      }

      // If we allocated
      // Return this to soar if we don't have a symbol
      return pSoarReturn;
   }
}

namespace gSKI 
{

   /*
   =============================
    _                    _
   / \   __ _  ___ _ __ | |_
  / _ \ / _` |/ _ \ '_ \| __|
 / ___ \ (_| |  __/ | | | |_
/_/   \_\__, |\___|_| |_|\__|
        |___/
   =============================
   */
   Agent::Agent(const char *agentName, Kernel *kernel): 
      m_productionManager(0), 
      m_agent(0), 
      m_active(true),
      m_kernel(kernel),
      m_ConcatRhs(),
      m_InterruptRhs(m_kernel->GetAgentManager()),
      m_pPerfMon(0),
	  m_ExecRhs(m_kernel),
	  m_CmdRhs(m_kernel)
   {
      MegaAssert(agentName != 0, "Null agent name pointer passed to agent constructor!");
      MegaAssert(kernel != 0, "Null kernel pointer passed to agent constructor!");

      // Why doesn't this call one of the initialize functions????
      initializeRuntimeState();

      m_agent = create_soar_agent(m_kernel->GetSoarKernel(), const_cast<char *>(agentName));     
      MegaAssert(m_agent != 0, "Unable to create soar agent!");

      if(m_agent)
      {
         // Temporary HACK.  This should be fixed in the kernel.
         m_agent->stop_soar = FALSE;

         // Creating the output link
         // NOTE: THE OUTPUT LINK CREATION MUST COME BEFORE THE INITIALIZE CALL
         // FOR THE OUTPUT LINK CALLBACK TO BE PROPERLY REGISTERED (see io.cpp for more 
         // details in the update_for_top_state_wme_addition function)
         m_outputlink = new OutputLink(this);

         // Initializing the soar agent
         initialize_soar_agent(m_kernel->GetSoarKernel(), m_agent);
         
         m_inputlink = new InputLink(this);
         m_workingMemory = new WorkingMemory(this);
     }

	 m_ExecRhs.SetAgent(this) ;
	 m_CmdRhs.SetAgent(this) ;

     // Adding some basic rhs functions
     this->AddClientRhsFunction(&m_InterruptRhs);
     this->AddClientRhsFunction(&m_ConcatRhs);
	 this->AddClientRhsFunction(&m_ExecRhs) ;
	 this->AddClientRhsFunction(&m_CmdRhs) ;

     m_pPerfMon = new AgentPerformanceMonitor(this);
   }

   /*
   =============================
 /\/| _                    _
|/\/ / \   __ _  ___ _ __ | |_
    / _ \ / _` |/ _ \ '_ \| __|
   / ___ \ (_| |  __/ | | | |_
  /_/   \_\__, |\___|_| |_|\__|
          |___/
   =============================
    */
   Agent::~Agent()
   {
      delete m_pPerfMon;

      // Cleaning up the input and output links and the working memory
      // object since these are wholly owned by the agent.
      delete m_inputlink;
      delete m_outputlink;
      delete m_workingMemory;
      delete m_productionManager;
   
      destroy_soar_agent(m_kernel->GetSoarKernel(), m_agent);
   }

   /*
   =============================

   =============================
   */
   void Agent::initializeRuntimeState()
   {
      m_smallestStepCount = 0;
      m_phaseCount        = 0;
      m_decisionCount     = 0;
      m_outputCount       = 0;

      // This tells run that we are starting a new cycle
      m_lastPhase         = gSKI_OUTPUT_PHASE; 
      m_nextPhase         = gSKI_INPUT_PHASE;

      // perhaps we need to tell the agent manager to stop all agents or to only stop this agent
      // BUG agent doesn't actually stop
      m_runState          = gSKI_RUNSTATE_STOPPED;

      // Clear out interrupts
      m_suspendOnInterrupt = false;
      m_interruptFlags     = 0;
   }

   /*
   =============================
 ____      _       _ _   _       _ _
|  _ \ ___(_)_ __ (_) |_(_) __ _| (_)_______
| |_) / _ \ | '_ \| | __| |/ _` | | |_  / _ \
|  _ <  __/ | | | | | |_| | (_| | | |/ /  __/
|_| \_\___|_|_| |_|_|\__|_|\__,_|_|_/___\___|
   =============================
   */
   bool Agent::Reinitialize(const char*       productionFileName, 
                            bool              learningOn,
                            egSKIOSupportMode oSupportMode,
                            Error*            err)
   {
     //MegaAssert(false, "Not implemented yet.");
      ClearError(err);

      // Notify the agent manager
      AgentManager* am = (AgentManager*)(m_kernel->GetAgentManager());
      MegaAssert(am != 0, "Agent manager is not AgentManager!");
      
      // Tell listeners about his
      am->FireBeforeAgentReinitialized(this);
         
      /// INITIALIZATION HERE
      initializeRuntimeState();

      // Reinitializing the input and output links
      m_inputlink->Reinitialize();
      m_outputlink->Reinitialize();
      m_workingMemory->Reinitialize();

      // !!!
      // perhaps we need to tell the agent manager to stop all agents or to only stop this agent
      // tcl/tgd needs to know that the agent(s?) stop
      // tcl either runs all of them, or none at all, so our only option (right now) is to stop all (?)
      // -- has to be done outside of gski, gski can't know about it from an oop perspective

      // !! remember tgd clears all interrupts each time it calls run

      // reinitialize_agent cleans out the agents memory the 
      // init_agent_memory call adds back in the top state and
      // other misc. objects and wmes.
      reinitialize_agent( m_agent );
      init_agent_memory( m_agent );

      // Tell listeners it is over
      am->FireAfterAgentReinitialized(this);

      return false;
   }

   /*
   =============================
 ____      _       _ _   _       _ _       __        ___ _   _
|  _ \ ___(_)_ __ (_) |_(_) __ _| (_)______\ \      / (_) |_| |__
| |_) / _ \ | '_ \| | __| |/ _` | | |_  / _ \ \ /\ / /| | __| '_ \
|  _ <  __/ | | | | | |_| | (_| | | |/ /  __/\ V  V / | | |_| | | |
|_|_\_\___|_|_|_|_|_|\__|_|\__,_|_|_/___\___| \_/\_/  |_|\__|_| |_|
 / _ \| | __| / ___|  ___| |_| |_(_)_ __   __ _ ___
| | | | |/ _` \___ \ / _ \ __| __| | '_ \ / _` / __|
| |_| | | (_| |___) |  __/ |_| |_| | | | | (_| \__ \
 \___/|_|\__,_|____/ \___|\__|\__|_|_| |_|\__, |___/
                                          |___/
   =============================
   */
   bool Agent::ReinitializeWithOldSettings(Error* err)
   {
      MegaAssert(false, "Not implemented yet.");
      ClearError(err);

      // Should we just call Reinitialize with parameters same
      //  as current state?
      initializeRuntimeState();

      return false;
   }

   /*
   =============================

   =============================
   */
   egSKIRunResult Agent::RunInSeparateThread(egSKIRunType        runLength, 
                                             unsigned long       count,
                                             Error*              err)
   {
      MegaAssert(false, "Not implemented yet.");
      SetError(err, gSKIERR_NOT_IMPLEMENTED);
      return gSKI_RUN_ERROR;
   }

   /*
   =============================

   =============================
   */
   egSKIRunResult Agent::RunInClientThread(egSKIRunType        runLength, 
                                           unsigned long       count,
                                           Error*              err)
   {
      // TODO: This will need to be locked at some point
      //  Just the part that checks the runstate and then
      //  sets the runstate.  Notifying the agent manager
      //  about who is running is a bit tricky.

      // Agent is already running, we cannot run
      if(m_runState != gSKI_RUNSTATE_STOPPED)
      {
         if(m_runState == gSKI_RUNSTATE_HALTED)
            SetError(err, gSKIERR_AGENT_HALTED);
         else
            SetError(err, gSKIERR_AGENT_RUNNING);

         return gSKI_RUN_ERROR;
      }

      m_runState = gSKI_RUNSTATE_RUNNING;

      // Now clear error and do the run
      ClearError(err);

      // This helper does all the work
      return run(runLength, count);
   }




   /*
   =============================

   =============================
   */
   bool Agent::Interrupt(egSKIStopLocation    stopLoc, 
                         egSKIStopType        stopType,
                         Error*               err)
   {
      // This type of stopping requires full threading
      MegaAssert(stopType != gSKI_STOP_BY_SUSPENDING, "This mode is not implemented.");
      MegaAssert(stopLoc  != gSKI_STOP_ON_CALLBACK_RETURN, "This mode is not implemented.");
      MegaAssert(stopLoc  != gSKI_STOP_AFTER_ALL_CALLBACKS_RETURN, "This mode is not implemented.");
      if((stopType == gSKI_STOP_BY_SUSPENDING) ||
         (stopLoc  == gSKI_STOP_ON_CALLBACK_RETURN) ||
         (stopLoc  == gSKI_STOP_AFTER_ALL_CALLBACKS_RETURN))
      { 
         SetError(err, gSKIERR_NOT_IMPLEMENTED);
         return false;
      }

      // We are in the stuff we can implement
      ClearError(err);

      // If the agent is not running, we can interrupt now
      if(m_runState == gSKI_RUNSTATE_STOPPED)
         m_runState = gSKI_RUNSTATE_INTERRUPTED;

      // Tell the agent where to stop
      m_interruptFlags = stopLoc;

      // If  we implement suspend, it goes in the run method, not
      //  here.
      m_suspendOnInterrupt = (stopType == gSKI_STOP_BY_SUSPENDING)? true: false;


      return true;
   }

   /*
   =============================

   =============================
   */
   void Agent::ClearInterrupts(Error* err)
   {
      ClearError(err);

      // Clear the interrupts whether running or not
      m_interruptFlags = 0;

      // Only change state of agent if it is running
      if(m_runState == gSKI_RUNSTATE_INTERRUPTED)
      {

         // Check about suspension
         if(m_suspendOnInterrupt)
         {
            // TODO: When we implement suspend capabilities, waking
            //  would go here...
            m_suspendOnInterrupt = false;

            // We were in the middle of running
            m_runState = gSKI_RUNSTATE_RUNNING;
         }
         else
         {
            // We returned, and thus are stopped
            m_runState = gSKI_RUNSTATE_STOPPED;
         }
      }
   }

   /*
   =============================

   =============================
   */
   void Agent::Halt(Error* err)
   {
      ClearError(err);

      // Tell soar we halted (may have to lock here)
      m_agent->system_halted = TRUE;

      // If we are not running, set the run state to halted
      // If we are running, the run method will set the
      //   state to halted.
      if(m_runState != gSKI_RUNSTATE_RUNNING)
         m_runState = gSKI_RUNSTATE_HALTED;
   }

   /*
   =============================

   =============================
   */
   egSKIRunState Agent::GetRunState(Error* err)
   {
      ClearError(err);
      return m_runState;
   }

      /*
   =========================
    _       _     _ ____  _         _____                 _   _
   / \   __| | __| |  _ \| |__  ___|  ___|   _ _ __   ___| |_(_) ___  _ __
  / _ \ / _` |/ _` | |_) | '_ \/ __| |_ | | | | '_ \ / __| __| |/ _ \| '_ \
 / ___ \ (_| | (_| |  _ <| | | \__ \  _|| |_| | | | | (__| |_| | (_) | | | |
/_/   \_\__,_|\__,_|_| \_\_| |_|___/_|   \__,_|_| |_|\___|\__|_|\___/|_| |_|
   =========================
   */
   bool Agent::AddClientRhsFunction(IRhsFunction* rhsFunction, 
                                    Error*        err)
   {
      ClearError(err);

      MegaAssert(rhsFunction != 0, "0 pointer passed in as a RHS function");
      if(rhsFunction == 0)
      {
         SetError(err, gSKIERR_INVALID_PTR);
         return false;
      }

      // The name has a null pointer?
      MegaAssert(rhsFunction->GetName() != 0, "The name of the rhs function being added is NULL.");
      if(rhsFunction->GetName() == 0)
      {
         SetError(err, gSKIERR_INVALID_PTR);
         return false;
      }

      // Add it to the list of RHS functions (if it doesn't already exist)
      tRhsFunctionMapIt it = m_rhsFunctions.find(rhsFunction->GetName());
      if(it != m_rhsFunctions.end())
      {
         SetError(err, gSKIERR_RHS_FUNCTION_ALREADY_EXISTS);
         return false;
      }

      // This is our storage part,...
      m_rhsFunctions[rhsFunction->GetName()] = rhsFunction;

      // Tell Soar about it
      std::vector<char> tmpBuffer(rhsFunction->GetName(), rhsFunction->GetName() + strlen(rhsFunction->GetName()) + 1);
      add_rhs_function (m_agent, 
                        make_sym_constant(m_agent, &tmpBuffer[0]),
                        rhsFunctionProxy,
                        rhsFunction->GetNumExpectedParameters(),
                        rhsFunction->IsValueReturned(),
                        true,
                        static_cast<void*>(rhsFunction));

      return false;
   }

   /*
   =========================
 ____                               ____  _         _____                 _   _
|  _ \ ___ _ __ ___   _____   _____|  _ \| |__  ___|  ___|   _ _ __   ___| |_(_) ___  _ __
| |_) / _ \ '_ ` _ \ / _ \ \ / / _ \ |_) | '_ \/ __| |_ | | | | '_ \ / __| __| |/ _ \| '_ \
|  _ <  __/ | | | | | (_) \ V /  __/  _ <| | | \__ \  _|| |_| | | | | (__| |_| | (_) | | | |
|_| \_\___|_| |_| |_|\___/ \_/ \___|_| \_\_| |_|___/_|   \__,_|_| |_|\___|\__|_|\___/|_| |_|
   =========================
   */
   bool Agent::RemoveClientRhsFunction(const char* szName, Error* err)
   {
      ClearError(err);
   
      MegaAssert(szName != 0, "0 pointer to a name passed in to RemoveRhsFunction");
      if(szName == 0)
      {
         SetError(err, gSKIERR_INVALID_PTR);
         return false;
      }

      tRhsFunctionMapIt it = m_rhsFunctions.find(szName);
      if(it == m_rhsFunctions.end())
      {
         SetError(err, gSKIERROR_NO_SUCH_RHS_FUNCTION);
         return false;
      }

      // Tell the kernel we are done listening.
      std::vector<char> tmpBuffer(szName, szName + strlen(szName) + 1);
      remove_rhs_function(m_agent, make_sym_constant(m_agent, &tmpBuffer[0]));

      // Do our stuff
      m_rhsFunctions.erase(it);

      return false;
   }

   /*
   =========================

   =========================
   */
   void Agent::RemoveAllClientRhsFunctions(Error* err)
   {
      ClearError(err);

      // For each we unregister from soar then clear the whole list
      for(tRhsFunctionMapIt it = m_rhsFunctions.begin(); it != m_rhsFunctions.end(); ++it)
      {
         const char* name = ((*it).first).c_str();

         // Do soar removal
         std::vector<char> tmpBuffer(name, name + strlen(name) + 1);
         remove_rhs_function(m_agent, make_sym_constant(m_agent, &tmpBuffer[0]));
      }

      // Clear our list
      m_rhsFunctions.clear();
   }

   /*
   =============================
  ____      _   _   _
 / ___| ___| |_| \ | | __ _ _ __ ___   ___
| |  _ / _ \ __|  \| |/ _` | '_ ` _ \ / _ \
| |_| |  __/ |_| |\  | (_| | | | | | |  __/
 \____|\___|\__|_| \_|\__,_|_| |_| |_|\___|
   =============================
   */
   const char* Agent::GetName(Error* err)
   {
      ClearError(err);

      return m_agent->name;
   }



   /*
   =============================
  ____      _   ____                _            _   _
 / ___| ___| |_|  _ \ _ __ ___   __| |_   _  ___| |_(_) ___  _ __
| |  _ / _ \ __| |_) | '__/ _ \ / _` | | | |/ __| __| |/ _ \| '_ \
| |_| |  __/ |_|  __/| | | (_) | (_| | |_| | (__| |_| | (_) | | | |
 \____|\___|\__|_|   |_|  \___/ \__,_|\__,_|\___|\__|_|\___/|_| |_|
|  \/  | __ _ _ __   __ _  __ _  ___ _ __
| |\/| |/ _` | '_ \ / _` |/ _` |/ _ \ '__|
| |  | | (_| | | | | (_| | (_| |  __/ |
|_|  |_|\__,_|_| |_|\__,_|\__, |\___|_|
                          |___/
   =============================
   */
   IProductionManager* Agent::GetProductionManager(Error* err)
   {
      ClearError(err);

      if(m_productionManager == 0) {
         m_productionManager = new ProductionManager(this);
      }

      return m_productionManager;
   }

   /*
   =============================
  ____      _   ___                   _   _     _       _
 / ___| ___| |_|_ _|_ __  _ __  _   _| |_| |   (_)_ __ | | __
| |  _ / _ \ __|| || '_ \| '_ \| | | | __| |   | | '_ \| |/ /
| |_| |  __/ |_ | || | | | |_) | |_| | |_| |___| | | | |   <
 \____|\___|\__|___|_| |_| .__/ \__,_|\__|_____|_|_| |_|_|\_\
                         |_|
   =============================
   */
   IInputLink* Agent::GetInputLink(Error* err)
   {
      ClearError(err);
      return m_inputlink;
   }

   /*
   =============================
  ____      _    ___        _               _   _     _       _
 / ___| ___| |_ / _ \ _   _| |_ _ __  _   _| |_| |   (_)_ __ | | __
| |  _ / _ \ __| | | | | | | __| '_ \| | | | __| |   | | '_ \| |/ /
| |_| |  __/ |_| |_| | |_| | |_| |_) | |_| | |_| |___| | | | |   <
 \____|\___|\__|\___/ \__,_|\__| .__/ \__,_|\__|_____|_|_| |_|_|\_\
                               |_|
   =============================
   */
   IOutputLink* Agent::GetOutputLink(Error* err)
   {
      ClearError(err);
      return m_outputlink;
   }

   /*
   =============================
  ____      _ __        __         _    _
 / ___| ___| |\ \      / /__  _ __| | _(_)_ __   __ _
| |  _ / _ \ __\ \ /\ / / _ \| '__| |/ / | '_ \ / _` |
| |_| |  __/ |_ \ V  V / (_) | |  |   <| | | | | (_| |
 \____|\___|\__| \_/\_/ \___/|_|  |_|\_\_|_| |_|\__, |
|  \/  | ___ _ __ ___   ___  _ __ _   _         |___/
| |\/| |/ _ \ '_ ` _ \ / _ \| '__| | | |
| |  | |  __/ | | | | | (_) | |  | |_| |
|_|  |_|\___|_| |_| |_|\___/|_|   \__, |
                                  |___/
   =============================
   */
   IWorkingMemory* Agent::GetWorkingMemory(Error* err)
   {
      ClearError(err);
      return m_workingMemory;
   }

   /*
   =============================
 ___     _                          _              ___
|_ _|___| |    ___  __ _ _ __ _ __ (_)_ __   __ _ / _ \ _ __
 | |/ __| |   / _ \/ _` | '__| '_ \| | '_ \ / _` | | | | '_ \
 | |\__ \ |__|  __/ (_| | |  | | | | | | | | (_| | |_| | | | |
|___|___/_____\___|\__,_|_|  |_| |_|_|_| |_|\__, |\___/|_| |_|
                                            |___/
   =============================
   */
   bool Agent::IsLearningOn(Error* err)
   {
      ClearError(err);
      return m_agent->sysparams[LEARNING_ON_SYSPARAM] ? true : false;
   }

   /*
   =============================
 ____       _   _                          _
/ ___|  ___| |_| |    ___  __ _ _ __ _ __ (_)_ __   __ _
\___ \ / _ \ __| |   / _ \/ _` | '__| '_ \| | '_ \ / _` |
 ___) |  __/ |_| |__|  __/ (_| | |  | | | | | | | | (_| |
|____/ \___|\__|_____\___|\__,_|_|  |_| |_|_|_| |_|\__, |
                                                   |___/
   =============================
   */
   void Agent::SetLearning(bool on, Error* err)
   {
      ClearError(err);
      m_agent->sysparams[LEARNING_ON_SYSPARAM] = on;
   }
 
   int Agent::GetMaxChunks(Error* err /*= 0*/)
   {
      ClearError(err);
      return m_agent->sysparams[MAX_CHUNKS_SYSPARAM];
   }
   void Agent::SetMaxChunks(int maxChunks, Error* err /*= 0*/)
   {
      ClearError(err);
      m_agent->sysparams[MAX_CHUNKS_SYSPARAM] = maxChunks;
   }

   int Agent::GetMaxElaborations(Error* err /*= 0*/)
   {
      ClearError(err);
      return m_agent->sysparams[MAX_ELABORATIONS_SYSPARAM];
   }
   void Agent::SetMaxElaborations(int maxElabs, Error* err /*= 0*/)
   {
      ClearError(err);
      m_agent->sysparams[MAX_ELABORATIONS_SYSPARAM] = maxElabs;
   }

   int Agent::GetMaxNilOutputCycles(Error* err /*= 0*/)
   {
      ClearError(err);
      return m_agent->sysparams[MAX_NIL_OUTPUT_CYCLES_SYSPARAM];
   }
   void Agent::SetMaxNilOutputCycles(int maxNils, Error* err /*= 0*/)
   {
      ClearError(err);
      m_agent->sysparams[MAX_NIL_OUTPUT_CYCLES_SYSPARAM] = maxNils;
   }

   bool Agent::IsWaitingOnStateNoChange(Error* err)
   {
      ClearError(err);
      return m_agent->waitsnc != 0;
   }

   void Agent::SetWaitOnStateNoChange(bool on, Error* err)
   {
      ClearError(err);
      m_agent->waitsnc = on;
   }

   /*
   =============================
  ____      _    ___  ____                               _
 / ___| ___| |_ / _ \/ ___| _   _ _ __  _ __   ___  _ __| |_
| |  _ / _ \ __| | | \___ \| | | | '_ \| '_ \ / _ \| '__| __|
| |_| |  __/ |_| |_| |___) | |_| | |_) | |_) | (_) | |  | |_
 \____|\___|\__|\___/|____/ \__,_| .__/| .__/ \___/|_|   \__|
|  \/  | ___   __| | ___         |_|   |_|
| |\/| |/ _ \ / _` |/ _ \
| |  | | (_) | (_| |  __/
|_|  |_|\___/ \__,_|\___|
   =============================
   */
   egSKIOSupportMode Agent::GetOSupportMode(Error* err)
   {
      ClearError(err);

      egSKIOSupportMode m = gSKI_O_SUPPORT_MODE_4;
      switch(m_agent->o_support_calculation_type)
      {
      case 0: m = gSKI_O_SUPPORT_MODE_0; break;
      case 2: m = gSKI_O_SUPPORT_MODE_2; break;
      case 3: m = gSKI_O_SUPPORT_MODE_3; break;
      case 4: m = gSKI_O_SUPPORT_MODE_4; break;
      default:
         MegaAssert(false, "Invalid o-support-mode setting");
      }
      return m;
   }

   egSKIUserSelectType Agent::GetIndifferentSelection(Error* err /*= 0*/)
   {
      int us = m_agent->sysparams[USER_SELECT_MODE_SYSPARAM];

      switch(us)
      {
      case USER_SELECT_FIRST  : return gSKI_USER_SELECT_FIRST;
      case USER_SELECT_ASK    : return gSKI_USER_SELECT_ASK;
      case USER_SELECT_RANDOM : return gSKI_USER_SELECT_RANDOM;
      case USER_SELECT_LAST   : return gSKI_USER_SELECT_LAST;
      default:
         MegaAssert(false, "Invalid indifferent selection setting");
         return gSKI_USER_SELECT_FIRST;
      }
   }

   void Agent::SetIndifferentSelection(egSKIUserSelectType t, Error* err /*= 0*/)
   {
      int us = -1;
      switch(t)
      {
      case gSKI_USER_SELECT_FIRST  : us = USER_SELECT_FIRST;   break;
      case gSKI_USER_SELECT_ASK    : us = USER_SELECT_ASK;     break;
      case gSKI_USER_SELECT_RANDOM : us = USER_SELECT_RANDOM;  break;
      case gSKI_USER_SELECT_LAST   : us = USER_SELECT_LAST;    break;
      default:
         MegaAssert(false, "Invalid indifferent selection setting");
      }
      if(us != -1)
      {
         m_agent->sysparams[USER_SELECT_MODE_SYSPARAM] = us;
      }
   }

   /*
   =============================
  ____      _    ____                          _   ____            _     _
 / ___| ___| |_ / ___|   _ _ __ _ __ ___ _ __ | |_|  _ \  ___  ___(_)___(_) ___  _ __
| |  _ / _ \ __| |  | | | | '__| '__/ _ \ '_ \| __| | | |/ _ \/ __| / __| |/ _ \| '_ \
| |_| |  __/ |_| |__| |_| | |  | | |  __/ | | | |_| |_| |  __/ (__| \__ \ | (_) | | | |
 \____|\___|\__|\____\__,_|_|  |_|  \___|_| |_|\__|____/ \___|\___|_|___/_|\___/|_| |_|
|  _ \| |__   __ _ ___  ___
| |_) | '_ \ / _` / __|/ _ \
|  __/| | | | (_| \__ \  __/
|_|   |_| |_|\__,_|___/\___|
   =============================
   */
   egSKIPhaseType Agent::GetCurrentPhase(Error* err)
   {
      ClearError(err);
      return m_nextPhase;
   }

   /*
   =============================

   =============================
   */
   unsigned long Agent::GetNumSmallestStepsExecuted(Error* err)
   {
      ClearError(err);
      return m_smallestStepCount;
   }

   
   /*
   =============================

   =============================
   */
   unsigned long Agent::GetNumPhasesExecuted(Error* err)
   {
      ClearError(err);
      return m_phaseCount;
   }

   /*
   =============================

   =============================
   */
   unsigned long Agent::GetNumDecisionCyclesExecuted(Error* err)
   {
      ClearError(err);
      return m_decisionCount;
   }

   /*
   =============================

   =============================
   */
   unsigned long Agent::GetNumOutputsExecuted(Error* err)
   {
      ClearError(err);
      return m_outputCount;
   }

      /*
   =========================
    _       _     _ ____  _         _____                 _   _
   / \   __| | __| |  _ \| |__  ___|  ___|   _ _ __   ___| |_(_) ___  _ __
  / _ \ / _` |/ _` | |_) | '_ \/ __| |_ | | | | '_ \ / __| __| |/ _ \| '_ \
 / ___ \ (_| | (_| |  _ <| | | \__ \  _|| |_| | | | | (__| |_| | (_) | | | |
/_/___\_\__,_|\__,_|_| \_\_| |_|___/_|   \__,_|_| |_|\___|\__|_|\___/|_| |_|
 / ___| |__   __ _ _ __   __ _  ___| |   (_)___| |_ ___ _ __   ___ _ __
| |   | '_ \ / _` | '_ \ / _` |/ _ \ |   | / __| __/ _ \ '_ \ / _ \ '__|
| |___| | | | (_| | | | | (_| |  __/ |___| \__ \ ||  __/ | | |  __/ |
 \____|_| |_|\__,_|_| |_|\__, |\___|_____|_|___/\__\___|_| |_|\___|_|
                         |___/
      =========================
      */
      void Agent::AddRhsFunctionChangeListener(egSKIEventId                 nEventId, 
                                        IRhsFunctionChangeListener*  pListener, 
                                        bool                         bAllowAsynch,
                                        Error*                       err)
      {
         ClearError(err);
      
      }


   /*
   =========================
 ____                               ____  _         _____                 _   _
|  _ \ ___ _ __ ___   _____   _____|  _ \| |__  ___|  ___|   _ _ __   ___| |_(_) ___  _ __
| |_) / _ \ '_ ` _ \ / _ \ \ / / _ \ |_) | '_ \/ __| |_ | | | | '_ \ / __| __| |/ _ \| '_ \
|  _ <  __/ | | | | | (_) \ V /  __/  _ <| | | \__ \  _|| |_| | | | | (__| |_| | (_) | | | |
|_|_\_\___|_| |_| |_|\___/ \_/ \___|_| \_\_| |_|___/_|   \__,_|_| |_|\___|\__|_|\___/|_| |_|
 / ___| |__   __ _ _ __   __ _  ___| |   (_)___| |_ ___ _ __   ___ _ __
| |   | '_ \ / _` | '_ \ / _` |/ _ \ |   | / __| __/ _ \ '_ \ / _ \ '__|
| |___| | | | (_| | | | | (_| |  __/ |___| \__ \ ||  __/ | | |  __/ |
 \____|_| |_|\__,_|_| |_|\__, |\___|_____|_|___/\__\___|_| |_|\___|_|
                         |___/
      =========================
      */
      void Agent::RemoveRhsFunctionChangeListener(egSKIEventId                 nEventId,
                                           IRhsFunctionChangeListener*  pListener,
                                           Error*                       err)
      {
         ClearError(err);
      
      }


   /*
   =========================
    _       _     _ ____  _         _____                 _   _
   / \   __| | __| |  _ \| |__  ___|  ___|   _ _ __   ___| |_(_) ___  _ __
  / _ \ / _` |/ _` | |_) | '_ \/ __| |_ | | | | '_ \ / __| __| |/ _ \| '_ \
 / ___ \ (_| | (_| |  _ <| | | \__ \  _|| |_| | | | | (__| |_| | (_) | | | |
/_/   \_\__,_|\__,_|_| \_\_| |_|___/_|   \__,_|_| |_|\___|\__|_|\___/|_| |_|
| |   (_)___| |_ ___ _ __   ___ _ __
| |   | / __| __/ _ \ '_ \ / _ \ '__|
| |___| \__ \ ||  __/ | | |  __/ |
|_____|_|___/\__\___|_| |_|\___|_|
      =========================
      */
      void Agent::AddRhsFunctionListener(egSKIEventId          nEventId, 
                                  IRhsFunctionListener* pListener, 
                                  bool                  bAllowAsynch,
                                  Error*                err)
      {
         ClearError(err);
      
      }


   /*
   =========================
    _       _     _ ____  _         _____                 _   _
   / \   __| | __| |  _ \| |__  ___|  ___|   _ _ __   ___| |_(_) ___  _ __
  / _ \ / _` |/ _` | |_) | '_ \/ __| |_ | | | | '_ \ / __| __| |/ _ \| '_ \
 / ___ \ (_| | (_| |  _ <| | | \__ \  _|| |_| | | | | (__| |_| | (_) | | | |
/_/   \_\__,_|\__,_|_| \_\_| |_|___/_|   \__,_|_| |_|\___|\__|_|\___/|_|_|_|
| |   (_)___| |_ ___ _ __   ___ _ __| \ | | __ _ _ __ ___   ___|  ___(_) | |_ ___ _ __
| |   | / __| __/ _ \ '_ \ / _ \ '__|  \| |/ _` | '_ ` _ \ / _ \ |_  | | | __/ _ \ '__|
| |___| \__ \ ||  __/ | | |  __/ |  | |\  | (_| | | | | | |  __/  _| | | | ||  __/ |
|_____|_|___/\__\___|_| |_|\___|_|  |_| \_|\__,_|_| |_| |_|\___|_|   |_|_|\__\___|_|
      =========================
      */
      void Agent::AddRhsFunctionListenerNameFilter(egSKIEventId          nEventId,
                                            IRhsFunctionListener* pListener,
                                            const char*           szRhsFuncNamePattern,
                                            bool                  bNegate,
                                            Error*                err)
      {
         ClearError(err);
      
      }

   /*
   =========================
 ____                               ____  _         _____                 _   _
|  _ \ ___ _ __ ___   _____   _____|  _ \| |__  ___|  ___|   _ _ __   ___| |_(_) ___  _ __
| |_) / _ \ '_ ` _ \ / _ \ \ / / _ \ |_) | '_ \/ __| |_ | | | | '_ \ / __| __| |/ _ \| '_ \
|  _ <  __/ | | | | | (_) \ V /  __/  _ <| | | \__ \  _|| |_| | | | | (__| |_| | (_) | | | |
|_| \_\___|_|_|_| |_|\___/ \_/ \___|_| \_\_| |_|___/_|   \__,_|_| |_|\___|\__|_|\___/|_| |_|
| |   (_)___| |_ ___ _ __   ___ _ __
| |   | / __| __/ _ \ '_ \ / _ \ '__|
| |___| \__ \ ||  __/ | | |  __/ |
|_____|_|___/\__\___|_| |_|\___|_|
      =========================
      */
      void Agent::RemoveRhsFunctionListener(egSKIEventId           nEventId,
                                             IRhsFunctionListener*  pListener,
                                             Error*                 err)
      {
         ClearError(err);

      }


   /*
   =========================
 ____                               ____  _         _____                 _   _
|  _ \ ___ _ __ ___   _____   _____|  _ \| |__  ___|  ___|   _ _ __   ___| |_(_) ___  _ __
| |_) / _ \ '_ ` _ \ / _ \ \ / / _ \ |_) | '_ \/ __| |_ | | | | '_ \ / __| __| |/ _ \| '_ \
|  _ <  __/ | | | | | (_) \ V /  __/  _ <| | | \__ \  _|| |_| | | | | (__| |_| | (_) | | | |
|_| \_\___|_|_|_| |_|\___/ \_/ \___|_|_\_\_| |_|___/_|   \__,_|_| |_|\___|\__|_|\___/|_| |_|
| |   (_)___| |_ ___ _ __   ___ _ __|  ___(_) | |_ ___ _ __ ___
| |   | / __| __/ _ \ '_ \ / _ \ '__| |_  | | | __/ _ \ '__/ __|
| |___| \__ \ ||  __/ | | |  __/ |  |  _| | | | ||  __/ |  \__ \
|_____|_|___/\__\___|_| |_|\___|_|  |_|   |_|_|\__\___|_|  |___/
      =========================
      */
      void Agent::RemoveRhsFunctionListenerFilters(egSKIEventId          nEventId,
                                                    IRhsFunctionListener* pListener,
                                                    Error*                err)
      {
      
      }

   /*
   ==========================
    _       _     _ ____       _       _   _     _     _
   / \   __| | __| |  _ \ _ __(_)_ __ | |_| |   (_)___| |_ ___ _ __   ___ _ __
  / _ \ / _` |/ _` | |_) | '__| | '_ \| __| |   | / __| __/ _ \ '_ \ / _ \ '__|
 / ___ \ (_| | (_| |  __/| |  | | | | | |_| |___| \__ \ ||  __/ | | |  __/ |
/_/   \_\__,_|\__,_|_|   |_|  |_|_| |_|\__|_____|_|___/\__\___|_| |_|\___|_|
   ==========================
   */
   void Agent::AddPrintListener(egSKIEventId             eventId, 
                                 IPrintListener*          listener, 
                                 bool                     allowAsynch, 
                                 Error*                   err)
   {
      MegaAssert(listener, "Cannot add a 0 listener pointer.");
      if(!listener)
      {
         SetError(err, gSKIERR_INVALID_PTR);
         return;
      }

      ClearError(err);
      m_printListeners.AddListener(eventId, listener);

      // If we have added our first listener, we tell the kernel
      //  we want to recieve these events.
      if(m_printListeners.GetNumListeners(eventId) == 1)
      {
         // This is a kernel call (not part of gSKI)
         gSKI_SetAgentCallback(GetSoarAgent(), 
                               gSKI_K_EVENT_PRINT_CALLBACK,
                               static_cast<void*>(this),
                               HandleKernelPrintCallback);
      }
   }

   /*
   ==========================
 ____                               ____       _       _   _     _     _
|  _ \ ___ _ __ ___   _____   _____|  _ \ _ __(_)_ __ | |_| |   (_)___| |_ ___ _ __   ___ _ __
| |_) / _ \ '_ ` _ \ / _ \ \ / / _ \ |_) | '__| | '_ \| __| |   | / __| __/ _ \ '_ \ / _ \ '__|
|  _ <  __/ | | | | | (_) \ V /  __/  __/| |  | | | | | |_| |___| \__ \ ||  __/ | | |  __/ |
|_| \_\___|_| |_| |_|\___/ \_/ \___|_|   |_|  |_|_| |_|\__|_____|_|___/\__\___|_| |_|\___|_|
   ==========================
   */
   void Agent::RemovePrintListener(egSKIEventId          eventId,
                                    IPrintListener*       listener,
                                    Error*                err)
   {
      MegaAssert(listener, "Cannot remove a 0 listener pointer.");
      if(!listener)
      {
         SetError(err, gSKIERR_INVALID_PTR);
         return;
      }

      ClearError(err);
      m_printListeners.RemoveListener(eventId, listener);

      // If we have no more listeners, stop asking kernel to
      //  notify us
      if(m_printListeners.GetNumListeners(eventId) == 0)
      {
         // This is a kernel call (not part of gSKI)
         // Setting the callback to 0 causes the kernel
         //   not to fire the event
         gSKI_SetAgentCallback(GetSoarAgent(), 
                                 gSKI_K_EVENT_PRINT_CALLBACK,
                                 0, 0);
      }
   }

   /* 
   ==========================
 _   _                 _ _      _  __                    _
| | | | __ _ _ __   __| | | ___| |/ /___ _ __ _ __   ___| |
| |_| |/ _` | '_ \ / _` | |/ _ \ ' // _ \ '__| '_ \ / _ \ |
|  _  | (_| | | | | (_| | |  __/ . \  __/ |  | | | |  __/ |
|_|_|_|\__,_|_| |_|\__,_|_|\___|_|\_\___|_|  |_| |_|\___|_|
|  _ \ _ __(_)_ __ | |_ / ___|__ _| | | |__   __ _  ___| | __
| |_) | '__| | '_ \| __| |   / _` | | | '_ \ / _` |/ __| |/ /
|  __/| |  | | | | | |_| |__| (_| | | | |_) | (_| | (__|   <
|_|   |_|  |_|_| |_|\__|\____\__,_|_|_|_.__/ \__,_|\___|_|\_\
   ==========================
   */
   void Agent::HandleKernelPrintCallback(unsigned long         eventId, 
                                         unsigned char         eventOccured,
                                         void*                 object, 
                                         agent*                soarAgent, 
                                         void*                 data)
   {
      Agent*        a = static_cast<Agent*>(object);
      const char* str = static_cast<char*>(data);

      // We have to change the the event id from a kernel id to a gSKI id
      PrintNotifier pn(a, str);
      a->m_printListeners.Notify(static_cast<egSKIEventId>(gSKIEVENT_PRINT), pn);
   }

   /* 
   ==========================

   ==========================
   */   
   void Agent::AddRunListener(egSKIEventId     eventId, 
                              IRunListener*    listener, 
                              bool             allowAsynch,
                              Error*           err)
   {
      MegaAssert(listener, "Cannot add a 0 listener pointer.");
      if(!listener)
      {
         SetError(err, gSKIERR_INVALID_PTR);
         return;
      }

      ClearError(err);
      m_runListeners.AddListener(eventId, listener);
   }

   /* 
   ==========================

   ==========================
   */   
   void Agent::RemoveRunListener(egSKIEventId      eventId,
                                 IRunListener*     listener,
                                 Error*            err)
   {
      MegaAssert(listener, "Cannot remove a 0 listener pointer.");
      if(!listener)
      {
         SetError(err, gSKIERR_INVALID_PTR);
         return;
      }

      ClearError(err);
      m_runListeners.RemoveListener(eventId, listener);
   }


   // TODO: Flesh out this function (dummy body allows compilation)
   IState* Agent::GetTopState(Error* err)
   {
      ClearError(err);

      return 0;
   }

   // TODO: Flesh out this function (dummy body allows compilation)
   IState* Agent::GetBottomState(Error* err)
   {
      ClearError(err);
      
      return 0;
   }

   tIMultiAttributeIterator* Agent::GetMultiAttributes(Error* pErr /*= 0*/)
   {
      typedef FwdContainerType< std::vector<IMultiAttribute * > >  tVec;
      typedef IteratorWithRelease<tVec::V, tVec::t>  tIter;
      
      tVec::t attrs;
      multi_attribute* m = m_agent->multi_attributes;
      while(m)
      {
         char tmp[1024];
         attrs.push_back(new MultiAttribute(this, 
                                            symbol_to_string(m_agent, m->symbol, TRUE, tmp, 1024), 
                                            m->value));
         m = m->next;
      }
      return new tIter(attrs);
   }

   IMultiAttribute* Agent::GetMultiAttribute(const char* attribute, Error* pErr /*= 0*/)
   {
      multi_attribute* m = m_agent->multi_attributes;
      std::vector<char> tmp(attribute, attribute + strlen(attribute) + 1);
      Symbol* s = make_sym_constant(m_agent, &tmp[0]);

      while (m) 
      {
         if (m->symbol == s) 
         {
            symbol_remove_ref(m_agent, s);
            return new MultiAttribute(this, attribute, m->value);
         }
         m = m->next;
      }
      symbol_remove_ref(m_agent, s);
      return 0;
   }

   void Agent::SetMultiAttribute(const char* attribute, 
                                    int priority,
                                    Error* pErr /*= 0*/)
   {
      multi_attribute* m = m_agent->multi_attributes;
      std::vector<char> tmp(attribute, attribute + strlen(attribute) + 1);
      Symbol* s = make_sym_constant(m_agent, &tmp[0]);

      while (m) 
      {
         if (m->symbol == s) 
         {
            m->value = priority;
            symbol_remove_ref(m_agent, s);
            return;
         }
         m = m->next;
      }
      /* sym wasn't in the table if we get here, so add it */
      m = (multi_attribute *) allocate_memory(m_agent, sizeof(multi_attribute), MISCELLANEOUS_MEM_USAGE);
      m->value = priority;
      m->symbol = s;
      m->next = m_agent->multi_attributes;
      m_agent->multi_attributes = m;
   }


   egSKINumericIndifferentMode Agent::GetNumericIndifferentMode(Error* pErr/* = 0*/) {

      egSKINumericIndifferentMode m = gSKI_NUMERIC_INDIFFERENT_MODE_AVG;

      switch(GetSoarAgent()->numeric_indifferent_mode) {
         case NUMERIC_INDIFFERENT_MODE_AVG:
            m = gSKI_NUMERIC_INDIFFERENT_MODE_AVG;
            break;
         case NUMERIC_INDIFFERENT_MODE_SUM:
            m =  gSKI_NUMERIC_INDIFFERENT_MODE_SUM;
            break;
         default:
            MegaAssert(false, "Invalid numeric indifferent mode");
            break;
      }

      return m;
   }

   void Agent::SetNumericIndifferentMode(egSKINumericIndifferentMode m, Error* pErr/* = 0*/) {
      switch(m) {
         case gSKI_NUMERIC_INDIFFERENT_MODE_AVG:
            GetSoarAgent()->numeric_indifferent_mode = NUMERIC_INDIFFERENT_MODE_AVG;
            return;
         case gSKI_NUMERIC_INDIFFERENT_MODE_SUM:
            GetSoarAgent()->numeric_indifferent_mode = NUMERIC_INDIFFERENT_MODE_SUM;
            return;
         default:
            MegaAssert(false, "Invalid numeric indifferent mode");
      }
   }

      
   //////////////////////////////////////////////////// PRIVATES /////////////

      /*
   =============================

   =============================
   */
   egSKIRunResult Agent::run(egSKIRunType runType, unsigned long maxSteps)
   {
      MegaAssert((maxSteps >= 0) || (runType == gSKI_RUN_FOREVER), "Cannot run for fewer than one steps.");

      // Getting the relavent counter returns a pointer to the smallest step
      //  phase, decision or output counter depending on the type of run
      //  that was requrested.  We use that counter to control the
      //  run loop.  The 'steps' variable can be 0 if the runType is
      //  run forever
      unsigned long*       steps           = getReleventCounter(runType);
      const unsigned long  MAX_STEPS       = (steps)? ((*steps) + maxSteps): 0;
      
      // Tells the run loop if an interrupt has occured
      bool interrupted  = (m_runState == gSKI_RUNSTATE_INTERRUPTED)? true: false;

      if(!interrupted)
      {
         // Notify that we stopped
         RunNotifier nfBeforeRunning(this, m_nextPhase);
         m_runListeners.Notify(gSKIEVENT_BEFORE_RUNNING, nfBeforeRunning);
      }

      // Does the running
      while(!interrupted && !maxStepsReached(steps, MAX_STEPS) && !(m_agent->system_halted))
      {
         MegaAssert(!m_agent->system_halted, "System should not be halted here!");

         // Do pre-step notifications to listeners
         preStepNotifications();

         ///////////////////////////////////////////////////////////////////
         // Execute the next smallest step
         // do_one_top_level_phase(m_agent);
		 run_for_n_phases(m_agent, 1);

         // Execute soar's determine level phase.  It does not map
         //  to any high-level phases
		 while((m_agent->current_phase == DETERMINE_LEVEL_PHASE) && !(m_agent->system_halted)) {
           // do_one_top_level_phase(m_agent);
			 run_for_n_phases(m_agent, 1); }
         // Remember our current phase
         m_lastPhase = m_nextPhase;
         m_nextPhase = EnumRemappings::ReMapPhaseType((unsigned short)m_agent->current_phase, 
                                                      (m_agent->applyPhase)? true: false);
         ///////////////////////////////////////////////////////////////////

         // Do post step notifications to listeners
         interrupted = postStepNotifications();

         // If we are doing a suspend type interrupt, here is where we suspend!
         // If we don't suspend, we exit this loop.
         if(interrupted && m_suspendOnInterrupt)
         {
           // Notify of the interrupt
           RunNotifier nfAfterInt(this, m_lastPhase);
           m_runListeners.Notify(gSKIEVENT_AFTER_INTERRUPT, nfAfterInt);

#ifdef _WIN32
#pragma warning (disable : 4390)
#endif
           if(m_suspendOnInterrupt)
               ; // suspend me!
         }
#ifdef _WIN32
#pragma warning (default : 4390)
#endif
      }

      egSKIRunResult retVal;

      // We've exited the run loop so we see what we should return
      if(m_agent->system_halted)
      {
         // If we halted, we completed and our state is halted
         m_runState    = gSKI_RUNSTATE_HALTED;
         retVal        = gSKI_RUN_COMPLETED;
      }
      else if(maxStepsReached(steps, MAX_STEPS)) 
      {
         // If we ran the required steps, we completed, but we
         //  may still be interrupted.
         if(interrupted)
         {
            m_runState = gSKI_RUNSTATE_INTERRUPTED;
            retVal     = gSKI_RUN_COMPLETED_AND_INTERRUPTED;
         }
         else
         {
            m_runState = gSKI_RUNSTATE_STOPPED;
            retVal     = gSKI_RUN_COMPLETED;
         }
      }
      else
      {
         // We were interrupted before we could complete
         MegaAssert(interrupted, "Should never get here if we aren't interrupted");

         m_runState    = gSKI_RUNSTATE_INTERRUPTED;
         retVal        = gSKI_RUN_INTERRUPTED;
      }    

      // Notify that we stopped
      RunNotifier nfAfterStop(this, m_lastPhase);
      m_runListeners.Notify(gSKIEVENT_AFTER_RUNNING, nfAfterStop);

      return retVal;
   }

   /*
   =============================

   =============================
   */
   unsigned long* Agent::getReleventCounter(egSKIRunType runType)
   {
      // Returns a pointer to the counter that is relavent
      //  for the given run type.  This is used by the
      //  run method to track how many of a particular
      //  run step it has executed.
      switch(runType)
      {
      case gSKI_RUN_SMALLEST_STEP:
         return &m_smallestStepCount;
      case gSKI_RUN_PHASE:
         return &m_phaseCount;
      case gSKI_RUN_DECISION_CYCLE:
         return &m_decisionCount;
      case gSKI_RUN_UNTIL_OUTPUT:
         return &m_outputCount;
      default:
         return 0;
      }
   }

   /*
   =============================

   =============================
   */
   void Agent::preStepNotifications()
   {
      // Input phase is the beginning of the decision cycle
      if(m_nextPhase == gSKI_INPUT_PHASE)
      {
         RunNotifier nfBeforeDC(this, m_nextPhase);
         m_runListeners.Notify(gSKIEVENT_BEFORE_DECISION_CYCLE, nfBeforeDC);
      }

      // See if we are starting a new phase
      if(m_lastPhase != m_nextPhase)
      {
         RunNotifier nfBeforePhase(this, m_nextPhase);
         m_runListeners.Notify(gSKIEVENT_BEFORE_PHASE_EXECUTED, nfBeforePhase);
      }

      // See if this is an elaboration cycle 
      if((m_nextPhase == gSKI_PROPOSAL_PHASE) || (m_nextPhase == gSKI_APPLY_PHASE))
      {
         RunNotifier nfBeforeElab(this, m_nextPhase);
         m_runListeners.Notify(gSKIEVENT_BEFORE_ELABORATION_CYCLE, nfBeforeElab);
      }

      // Tell listeners about smallest step
      RunNotifier nfBeforeStep(this, m_nextPhase);
      m_runListeners.Notify(gSKIEVENT_BEFORE_SMALLEST_STEP, nfBeforeStep);
   }

   /*
   =============================

   =============================
   */
   bool Agent::postStepNotifications()
   {
      bool interrupted = false;

      // Notify listeners about the end of the phase
      RunNotifier nfAfterStep(this, m_lastPhase);
      m_runListeners.Notify(gSKIEVENT_AFTER_SMALLEST_STEP, nfAfterStep);

      // Increment smallest step
      ++m_smallestStepCount;

      // Check an interrupt
      if(m_interruptFlags & gSKI_STOP_AFTER_SMALLEST_STEP)
         interrupted = true;

      // See if this was an elaboration cycle 
      if((m_lastPhase == gSKI_PROPOSAL_PHASE) || (m_lastPhase == gSKI_APPLY_PHASE))
      {
         RunNotifier nfAfterElab(this, m_lastPhase);
         m_runListeners.Notify(gSKIEVENT_AFTER_ELABORATION_CYCLE, nfAfterElab);
      }

      // Check to see if we've moved to another phase or not
      if(m_lastPhase != m_nextPhase)
      {
         RunNotifier nfAfterPhase(this, m_lastPhase);
         m_runListeners.Notify(gSKIEVENT_AFTER_PHASE_EXECUTED, nfAfterPhase);
      
         // Increment the phase count
         ++m_phaseCount;

         // Increment the number of outputs that have occured
         if((m_lastPhase == gSKI_OUTPUT_PHASE) && m_agent->output_link_changed)
            ++m_outputCount;

         // Check an interrupt
         if(m_interruptFlags & gSKI_STOP_AFTER_PHASE)
            interrupted = true;
      }

      // If the next phase is the input phase, we just ended a DC
      if(m_nextPhase == gSKI_INPUT_PHASE)
      {
         RunNotifier nfAfterDC(this, m_lastPhase);
         m_runListeners.Notify(gSKIEVENT_AFTER_DECISION_CYCLE, nfAfterDC);

         // Increment the decision count
         ++m_decisionCount;

         // Check an interrupt
         if(m_interruptFlags & gSKI_STOP_AFTER_DECISION_CYCLE)
            interrupted = true;
      }

      return interrupted;
   }

   /*
   ==================================
   //
   //
   // NOTE!  THIS IS A WORKAROUND HACK!!!  THIS SHOULD NOT STAY HERE!!!
   //        IT IS NEEDED FOR THE TSI UNTIL IT CAN GET REWRITTEN!
   //
   //
   ==================================
   */
   extern agent* GetSoarAgentPtr(IAgent* agent)
   {
      return ((Agent*)agent)->GetSoarAgent();
   }


}
