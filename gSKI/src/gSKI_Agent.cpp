#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H
#include "portability.h"

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
#include "gSKI_RhsFunction.h"
#include "agent.h"
#include "init_soar.h"
#include "gski_event_system_functions.h"
#include "rhsfun.h"
#include "production.h" // for struct multi_attributes
#include "print.h"      // for symboltostring
#include "gSKI_AgentPerformanceMonitor.h"
#include "gSKI_MultiAttribute.h"
#include "xmlTraceNames.h" // for constants for XML function types, tags and attributes
#include "decide.h"
#include "recmem.h"


//#include "MegaUnitTest.h"
//DEF_EXPOSE(gSKI_Agent);

// Private namespace for the workaround for callbacks
namespace {

   /** 
    * @brief Creates a Soar symbol based on a gSKI::ISymbol that is passed in
    *
    * This function is used as a helper for the function below to convert values
    *   returned by the RhsFunction object to Soar Symbols.
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
      case gSKI_DOUBLE:
	return make_float_constant(thisAgent, static_cast<float>(sym->GetDouble()));
      case gSKI_STRING:
	{
	  // Gotta create a temp buffer because make_sym_constant takes a non-const pointer
	  std::vector<char> tmpBuffer(sym->GetString(), sym->GetString() + strlen(sym->GetString()) + 1);
	  return make_sym_constant(thisAgent, &tmpBuffer[0]);
	}
      default:
	MegaAssert(sym->GetType() != gSKI_INT && sym->GetType() != gSKI_DOUBLE && sym->GetType() != gSKI_STRING, "Unsupported type returned from RHS function.") ;
	break ;
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
      gSKI::RhsFunction* rhsFunction = static_cast<gSKI::RhsFunction*>(user_data);

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
	  m_ExecRhs(m_kernel),
	  m_CmdRhs(m_kernel),
	  m_pPerfMon(0)
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

	 
	 // These need to be registered in gSKI RunListener because 
	 // the gSKI event handler has to increment counters on these events.
	 this->AddRunListener(gSKIEVENT_AFTER_ELABORATION_CYCLE, this) ;
	 this->AddRunListener(gSKIEVENT_AFTER_INPUT_PHASE, this) ;
	 this->AddRunListener(gSKIEVENT_AFTER_PROPOSE_PHASE, this) ;
	 this->AddRunListener(gSKIEVENT_AFTER_DECISION_PHASE, this) ;
	 this->AddRunListener(gSKIEVENT_AFTER_APPLY_PHASE, this) ;
	 this->AddRunListener(gSKIEVENT_AFTER_OUTPUT_PHASE, this) ;
	 this->AddRunListener(gSKIEVENT_AFTER_PREFERENCE_PHASE, this) ;  // Soar-7 mode only
	 this->AddRunListener(gSKIEVENT_AFTER_WM_PHASE, this) ;          // Soar-7 mode only     
	 
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
	   /* RPM 9/06 added code from reinitialize_soar to clean up stuff hanging from last run
	               need to put it here instead of in destroy_soar_agent because gSKI is
				    cleaning up too much stuff and thus it will crash if called later */
	   clear_goal_stack (m_agent);
	   m_agent->active_level = 0; /* Signal that everything should be retracted */
	   m_agent->FIRING_TYPE = IE_PRODS;
	   do_preference_phase (m_agent);   /* allow all i-instantiations to retract */

	   //remove the RhsFunctions we created (RPM 9/06)
	   this->RemoveClientRhsFunction(m_InterruptRhs.GetName());
	   this->RemoveClientRhsFunction(m_ConcatRhs.GetName());
	   this->RemoveClientRhsFunction(m_ExecRhs.GetName());
	   this->RemoveClientRhsFunction(m_CmdRhs.GetName());

	   //remove the RunListeners we created for the counters
	   this->RemoveRunListener(gSKIEVENT_AFTER_ELABORATION_CYCLE, this) ;
	   this->RemoveRunListener(gSKIEVENT_AFTER_INPUT_PHASE, this) ;
	   this->RemoveRunListener(gSKIEVENT_AFTER_PROPOSE_PHASE, this) ;
	   this->RemoveRunListener(gSKIEVENT_AFTER_DECISION_PHASE, this) ;
	   this->RemoveRunListener(gSKIEVENT_AFTER_APPLY_PHASE, this) ;
	   this->RemoveRunListener(gSKIEVENT_AFTER_OUTPUT_PHASE, this) ;
	   this->RemoveRunListener(gSKIEVENT_AFTER_PREFERENCE_PHASE, this) ;  // Soar-7 mode only
	   this->RemoveRunListener(gSKIEVENT_AFTER_WM_PHASE, this) ;          // Soar-7 mode only

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
      m_elaborationCount  = 0;
      m_decisionCount     = 0;  // should be m_agent->d_cycle_count.  Can we delete this var?
      m_outputCount       = 0;
	  m_nilOutputCycles   = 0;

      // This tells run that we are starting a new cycle
      m_lastPhase         = gSKI_OUTPUT_PHASE; /* okay eventhough not correct for Soar 7 */
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

      // reinitialize_soar cleans out the agents memory the 
      // init_agent_memory call adds back in the top state and
      // other misc. objects and wmes.
      bool ok = reinitialize_soar( m_agent );
      init_agent_memory( m_agent );
	  // reset_statistics is called inside reintialize_soar, but it needs to be called here
	  // again to ensure that timers and other counters are reset after init_agent_memory 
	  reset_statistics ( m_agent );   

      // Tell listeners it is over
      am->FireAfterAgentReinitialized(this);

      return ok;
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

   egSKIRunResult Agent::StepInClientThread(egSKIInterleaveType  stepSize, 
                                                  unsigned long  stepCount,
                                                         Error*  err)
   {
      //KJC copied from RunInClientThread above.

      // Agent is already running, we cannot run
      if(m_runState != gSKI_RUNSTATE_STOPPED)
      {
         if(m_runState == gSKI_RUNSTATE_HALTED)
            SetError(err, gSKIERR_AGENT_HALTED);  // nothing ever tests for this...
         else
            SetError(err, gSKIERR_AGENT_RUNNING);

         return gSKI_RUN_ERROR;
      }

      m_runState = gSKI_RUNSTATE_RUNNING;

      // Now clear error and do the run
      ClearError(err);

      // This method does all the work
      return step(stepSize, stepCount);
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

      // Tell the agent where to stop
      m_interruptFlags = stopLoc;

	  // If the request for interrupt is gSKI_STOP_AFTER_DECISION_CYCLE, then it 
	  // will be caught in the RunScheduler::CompletedRunType() and/or IsAgentFinished().
	  // We don't want to interrupt agents until the appropriate time if the request is  
	  // gSKI_STOP_AFTER_DECISION_CYCLE.
	  
      // These are immediate requests for interrupt, such as from RHS or application
	  if ((gSKI_STOP_AFTER_SMALLEST_STEP == stopLoc) || (gSKI_STOP_AFTER_PHASE == stopLoc)) {
		  m_agent->stop_soar = TRUE;
		  // If the agent is not running, we should set the runState flag now so agent won't run
		  if (m_runState == gSKI_RUNSTATE_STOPPED)
		  {
			  m_runState = gSKI_RUNSTATE_INTERRUPTED;
		  }
		  // Running agents must test stopLoc & stop_soar in Step method to see if interrupted.
		  // Because we set m_agent->stop_soar == TRUE above, any running agents should return to
		  // gSKI at the end of the current phase, even if interleaving by larger steps.  KJC
	  }
 
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

   unsigned long Agent::GetInterruptFlags(Error* err)
   {
      ClearError(err);
      return m_interruptFlags;
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
      // If we are running, the step method will set the
      //   state to halted.
	  if(m_runState != gSKI_RUNSTATE_RUNNING) 
	  {
		  m_runState = gSKI_RUNSTATE_HALTED;

		   RunNotifier nfAfterHalt(this, m_lastPhase);
           m_runListeners.Notify(gSKIEVENT_AFTER_HALTED, nfAfterHalt);

		  // fix for BUG 514  01-12-06
		  PrintNotifier nfHalted(this, "This Agent halted.");
		  m_printListeners.Notify(gSKIEVENT_PRINT, nfHalted);
		  XMLNotifier xn1(this, kFunctionBeginTag, kTagMessage, 0) ;
		  m_XMLListeners.Notify(gSKIEVENT_XML_TRACE_OUTPUT, xn1);
		  XMLNotifier xn2(this, kFunctionAddAttribute, kTypeString, "This Agent halted.") ;
		  m_XMLListeners.Notify(gSKIEVENT_XML_TRACE_OUTPUT, xn2);
		  XMLNotifier xn3(this, kFunctionEndTag, kTagMessage, 0) ;
		  m_XMLListeners.Notify(gSKIEVENT_XML_TRACE_OUTPUT, xn3);
	  }
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
   	  
   void Agent::SetRunState(egSKIRunState state,Error* err)
   { 
      ClearError(err);
	  m_runState = state; 
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
   bool Agent::AddClientRhsFunction(RhsFunction* rhsFunction, 
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

	  //RPM 9/06: removed symbol ref so symbol is released properly
	  Symbol* tmp = make_sym_constant(m_agent, &tmpBuffer[0]);
      remove_rhs_function(m_agent, tmp);
	  symbol_remove_ref (m_agent, tmp);

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
   ProductionManager* Agent::GetProductionManager(Error* err)
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

   void Agent::SetOSupportMode(egSKIOSupportMode mode, Error* err) 
   {
      ClearError(err);
	  switch (mode) 
	  {
		  case gSKI_O_SUPPORT_MODE_0:
			  m_agent->o_support_calculation_type = 0;
			  break;
		  case gSKI_O_SUPPORT_MODE_2:
			  m_agent->o_support_calculation_type = 2;
			  break;
		  case gSKI_O_SUPPORT_MODE_3:
			  m_agent->o_support_calculation_type = 3;
			  break;
		  case gSKI_O_SUPPORT_MODE_4:
			  m_agent->o_support_calculation_type = 4;
			  break;
		  default:
			  MegaAssert(false, "Invalid o-support-mode");
	  }
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

   int Agent::GetDefaultWMEDepth(Error* err)
   {
	   return m_agent->default_wme_depth;
   }

   void Agent::SetDefaultWMEDepth(int d, Error* err)
   {
	   m_agent->default_wme_depth = d;
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
      // return m_nextPhase;
	  // KJC:  shouldn't this really be 
	  return EnumRemappings::ReMapPhaseType(m_agent->current_phase,0);
	  // should we also set m_lastPhase??
   }

   /*
   =============================
   This counter deprecated as of v8.6.2
   =============================
   */
   unsigned long Agent::GetNumSmallestStepsExecuted(Error* err)
   {
      ClearError(err);
      return m_smallestStepCount;
   }
   
   /*
   =============================
   This is a local gSKI counter, not from Kernel
   =============================
   */
   unsigned long Agent::GetNumPhasesExecuted(Error* err)
   {
      ClearError(err);
      return m_phaseCount;
   }
   void Agent::ResetNumPhasesExecuted(Error* err)
   {
      ClearError(err);
      m_phaseCount = 0;
   }
   void Agent::ResetNilOutputCounter(Error* err)
   {
	  ClearError(err);
	  m_nilOutputCycles = 0;
   }

   /*
   =============================
   This is a local gSKI counter, not from Kernel.  m_elaborationCount
   gets incremented for each e_cycle or phase, but not both in same
   step.  m_agent->e_cycle_count is true e_cycles only, from kernel.
   =============================
   */
   unsigned long Agent::GetNumElaborationsExecuted(Error* err)
   {
      ClearError(err);
      return m_elaborationCount;  
   }

   /*
   =============================

   =============================
   */
   unsigned long Agent::GetNumDecisionCyclesExecuted(Error* err)
   {
      ClearError(err);
	  return m_agent->d_cycle_count;
   }

   /*
   =============================
   if desire number of decisions instead of full D_cycles
   =============================
   */
   unsigned long Agent::GetNumDecisionsExecuted(Error* err)
   {
      ClearError(err);
	  return m_agent->decision_phases_count;
   }


   /*
   =============================
   This is a local gSKI counter, not from Kernel
   =============================
   */
   unsigned long Agent::GetNumOutputsExecuted(Error* err)
   {
	   // This variable is the number of output phases that 
	   // actually had a modified output-link.
      ClearError(err);
      return m_outputCount;
   }
   void Agent::ResetNumOutputsExecuted(Error* err)
   {
      ClearError(err);
      m_outputCount = 0;
	  m_nilOutputCycles = 0;
   }

/********************************************************************
* @brief	Agents maintain a number of counters (for how many phase,
*			decisions etc.) they have ever executed.
*			We use these counters to determine when a run should stop.
*********************************************************************/
void Agent::IncrementgSKIStepCounter(egSKIInterleaveType interleaveStepSize)
{
	switch(interleaveStepSize)
	{
	case gSKI_INTERLEAVE_SMALLEST_STEP:
		 m_smallestStepCount++;
		return;
	case gSKI_INTERLEAVE_PHASE:
		 m_phaseCount++;
		return;
	case gSKI_INTERLEAVE_ELABORATION_PHASE:
		 m_elaborationCount++;
		return;
	case gSKI_INTERLEAVE_DECISION_CYCLE:
		// do nothing.  gSKI gets d_cycles from SoarKernel
		return;
	case gSKI_INTERLEAVE_OUTPUT:
		 m_outputCount++;				
		 m_nilOutputCycles = 0;

	default:
		return;
	}
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
      void Agent::AddRhsFunctionChangeListener(egSKISystemEventId    nEventId, 
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
      void Agent::RemoveRhsFunctionChangeListener(egSKISystemEventId    nEventId,
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
      void Agent::AddRhsFunctionListener(egSKISystemEventId  nEventId, 
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
      void Agent::AddRhsFunctionListenerNameFilter(egSKISystemEventId nEventId,
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
      void Agent::RemoveRhsFunctionListener(egSKISystemEventId      nEventId,
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
      void Agent::RemoveRhsFunctionListenerFilters(egSKISystemEventId     nEventId,
                                                    IRhsFunctionListener* pListener,
                                                    Error*                err)
      {
      
      }

	  /*
   =========================
	AddXMLListener
   =========================
	  */
	  void Agent::AddXMLListener(egSKIXMLEventId	 eventId, 
							IXMLListener*            listener, 
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
		  bool added = m_XMLListeners.AddListener(eventId, listener);


		  // If we have added our first listener, we tell the kernel
		  //  we want to recieve these events.
		  if(added && m_XMLListeners.GetNumListeners(eventId) == 1)
		  {
			  // This is a kernel call (not part of gSKI)
			  // Must convert gSKI event to Kernel event
			  gSKI_SetAgentCallback(GetSoarAgent(), 
				  EnumRemappings::RemapXMLEventType(eventId),
				  static_cast<void*>(this),
				  HandleKernelXMLCallback);
		  }
	  }

	  /*
   =========================
	RemoveXMLListener
   =========================
	  */
	  void Agent::RemoveXMLListener(egSKIXMLEventId  eventId,
                               IXMLListener*         listener,
                               Error*                err)
   {
      MegaAssert(listener, "Cannot remove a 0 listener pointer.");
      if(!listener)
      {
         SetError(err, gSKIERR_INVALID_PTR);
         return;
      }

      ClearError(err);
      bool removed = m_XMLListeners.RemoveListener(eventId, listener);

      // If we have no more listeners, stop asking kernel to
      //  notify us
      if(removed && m_XMLListeners.GetNumListeners(eventId) == 0)
      {
         // This is a kernel call (not part of gSKI)
		 // Must convert gSKI event to Kernel event
         // Setting the callback to 0 causes the kernel
         //   not to fire the event
         gSKI_SetAgentCallback(GetSoarAgent(), 
			 				     EnumRemappings::RemapXMLEventType(eventId),
                                 static_cast<void*>(this),
								 HandleKernelXMLCallback);
      }
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
   void Agent::AddPrintListener(egSKIPrintEventId         eventId, 
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
      bool added = m_printListeners.AddListener(eventId, listener);


      // If we have added our first listener, we tell the kernel
      //  we want to recieve these events.
      if(added && m_printListeners.GetNumListeners(eventId) == 1)
      {
         // This is a kernel call (not part of gSKI)
		 // Must convert gSKI event to Kernel event
         gSKI_SetAgentCallback(GetSoarAgent(), 
							   EnumRemappings::RemapPrintEventType(eventId),
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
   void Agent::RemovePrintListener(egSKIPrintEventId      eventId,
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
      bool removed = m_printListeners.RemoveListener(eventId, listener);

      // If we have no more listeners, stop asking kernel to
      //  notify us
      if(removed && m_printListeners.GetNumListeners(eventId) == 0)
      {
         // This is a kernel call (not part of gSKI)
		 // Must convert gSKI event to Kernel event
         // Setting the callback to 0 causes the kernel
         //   not to fire the event
         gSKI_SetAgentCallback(GetSoarAgent(), 
			 				     EnumRemappings::RemapPrintEventType(eventId),
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
      a->m_printListeners.Notify(EnumRemappings::Map_Kernel_to_gSKI_PrintEventId(eventId), pn);
   }

   /*
   =========================
	HandleKernelXMLCallback
   =========================
   */
   void Agent::HandleKernelXMLCallback(unsigned long			  eventId, 
                                            unsigned char         eventOccured,
                                            void*                 object, 
                                            agent*                soarAgent, 
                                            void*                 data)
   {
	  Agent* a = static_cast<Agent*>(object);
      gSKI_K_XMLCallbackData* xml_data = static_cast<gSKI_K_XMLCallbackData*>(data);

      // We have to change the the event id from a kernel id to a gSKI id

	  XMLNotifier xn(a, xml_data->funcType, xml_data->attOrTag, xml_data->value);
      a->m_XMLListeners.Notify(EnumRemappings::Map_Kernel_to_gSKI_XMLEventId(eventId), xn);
   }


   /* 
   ==========================
	AddRunListener
   ==========================
   */   
   void Agent::AddRunListener(egSKIRunEventId  eventId, 
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
      bool added = m_runListeners.AddListener(eventId, listener);
 
      // If we have added our first listener, we tell the kernel
      //  we want to recieve these events.
      if (added && m_runListeners.GetNumListeners(eventId) == 1)
      {
		  switch (eventId)   // temporary, til all are moved to kernel. KJC
		  {
		  case gSKIEVENT_BEFORE_PHASE_EXECUTED:
		  case gSKIEVENT_AFTER_PHASE_EXECUTED:      		
			  // events above are generated by each phase, so don't add again.
			  break;
		  case gSKIEVENT_BEFORE_ELABORATION_CYCLE:
		  case gSKIEVENT_AFTER_ELABORATION_CYCLE:  
			  // This is a kernel call (not part of gSKI)	
              // Must convert gSKI event to Kernel event     			
			  gSKI_SetAgentCallback(GetSoarAgent(), 
							   EnumRemappings::RemapRunEventType(eventId),
                               static_cast<void*>(this),
                               HandleRunEventCallback);
			 
			  break;
    	  case gSKIEVENT_BEFORE_DECISION_CYCLE:  
		  case gSKIEVENT_AFTER_DECISION_CYCLE:	
		  case gSKIEVENT_MAX_MEMORY_USAGE_EXCEEDED:
			  { RunEventCallbackData * eventInfo = new RunEventCallbackData();
			  m_RunEvents[eventId] = eventInfo;  // RPM 9/06 track this eventInfo so we can release it later
			    eventInfo->a = this;
		        eventInfo->eventId = eventId;
			  soar_add_callback (GetSoarAgent(),static_cast<void*>(GetSoarAgent()),
							     static_cast<SOAR_CALLBACK_TYPE>(EnumRemappings::KernelRunEventType(eventId)), 
								 HandleKernelRunEventCallback,
								 static_cast <void*> (eventInfo), 0, 
								 soar_callback_enum_to_name(static_cast<SOAR_CALLBACK_TYPE>(EnumRemappings::KernelRunEventType(eventId)), 1));
			  }
			  break; 
		  default:
			  ; // do nothing
		  }

		  if (IsPhaseEventID(eventId)) 
		  {		 
		      RunEventCallbackData * eventInfo = new RunEventCallbackData();
			  m_RunEvents[eventId] = eventInfo;  // RPM 9/06 track this eventInfo so we can release it later
			  eventInfo->a = this;
		      eventInfo->eventId = eventId;
			  soar_add_callback (GetSoarAgent(),static_cast<void*>(GetSoarAgent()),
							     static_cast<SOAR_CALLBACK_TYPE>(EnumRemappings::KernelRunEventType(eventId)), 
								 HandleKernelRunEventCallback,
								 static_cast <void*> (eventInfo), 0, 
								 soar_callback_enum_to_name(static_cast<SOAR_CALLBACK_TYPE>(EnumRemappings::KernelRunEventType(eventId)), 1));
		  }

      }
   }

   /* 
   ==========================
	RemoveRunListener
   ==========================
   */   
   void Agent::RemoveRunListener(egSKIRunEventId   eventId,
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
      bool removed = m_runListeners.RemoveListener(eventId, listener);

	  // If we have no more listeners, stop asking kernel to
      //  notify us
      if(removed && m_runListeners.GetNumListeners(eventId) == 0)
      {		 
		  switch (eventId)   // temporary, til all are moved to kernel. KJC
		  {
		  case gSKIEVENT_BEFORE_PHASE_EXECUTED:
		  case gSKIEVENT_AFTER_PHASE_EXECUTED:     
			  // These are no longer added, since phases generate above events.
			  break;
		  case gSKIEVENT_BEFORE_ELABORATION_CYCLE:
		  case gSKIEVENT_AFTER_ELABORATION_CYCLE:  
			  // This is a kernel call (not part of gSKI)	
              // Must convert gSKI event to Kernel event     			
			  // Setting the callback to 0 causes the kernel        
			  //   not to fire the event
			  gSKI_SetAgentCallback(GetSoarAgent(), 
							   EnumRemappings::RemapRunEventType(eventId),
                               0,0);
			  break;
		  case gSKIEVENT_BEFORE_DECISION_CYCLE:  
		  case gSKIEVENT_AFTER_DECISION_CYCLE:
		  case gSKIEVENT_MAX_MEMORY_USAGE_EXCEEDED:
			  soar_remove_callback (GetSoarAgent(),static_cast<void*>(GetSoarAgent()),
							     static_cast<SOAR_CALLBACK_TYPE>(EnumRemappings::KernelRunEventType(eventId)),
								 soar_callback_enum_to_name(static_cast<SOAR_CALLBACK_TYPE>(EnumRemappings::KernelRunEventType(eventId)), 1));
			  // RPM 9/06 release the eventInfo and cleanup the map
			  delete m_RunEvents.find(eventId)->second;
			  m_RunEvents.erase(eventId);

			  break; 
		  default:
			  ; // do nothing
		  }

		  if (IsPhaseEventID(eventId)) 
		  {		 
			  soar_remove_callback (GetSoarAgent(),static_cast<void*>(GetSoarAgent()),
							     static_cast<SOAR_CALLBACK_TYPE>(EnumRemappings::KernelRunEventType(eventId)),
								 soar_callback_enum_to_name(static_cast<SOAR_CALLBACK_TYPE>(EnumRemappings::KernelRunEventType(eventId)), 1));
			  // RPM 9/06 release the eventInfo and cleanup the map
			  delete m_RunEvents.find(eventId)->second;
			  m_RunEvents.erase(eventId);
		  }

		  //  We should probably do some checking to make sure the callbacks were indeed
		  //  removed and THEN we should call m_runListeners.RemoveListener(eventId, listener);

	  }
   }

	// Called when a "RunEvent" occurs in the kernel
   void Agent::HandleEvent(egSKIRunEventId eventId, gSKI::Agent* agentPtr, egSKIPhaseType phase)
   {
   }

   /*
   =========================
	HandleRunEventCallback
   =========================
   */
   void Agent::HandleKernelRunEventCallback( soar_callback_agent agent,
					                         soar_callback_data callbackdata,
                                             soar_call_data calldata )
   {
	   // Kernel Decision cycle events have NULL calldata, so do phases...at least for now
	   // callbackdata holds the agent object and eventId
	RunEventCallbackData * e = static_cast<RunEventCallbackData*>(callbackdata);
	Agent*            a = e->a;
	egSKIRunEventId eventId = e->eventId;

	//increment phase counts, output counts, and elaboration counts
	/* */
	if (IsAFTERPhaseEventID(eventId))
	{
		a->IncrementgSKIStepCounter(gSKI_INTERLEAVE_PHASE);
		if ((gSKIEVENT_AFTER_INPUT_PHASE  == eventId) ||
            (gSKIEVENT_AFTER_OUTPUT_PHASE == eventId) ||
			(gSKIEVENT_AFTER_DECISION_PHASE == eventId))
		{	
			a->IncrementgSKIStepCounter(gSKI_INTERLEAVE_ELABORATION_PHASE);
		}
		if (gSKIEVENT_AFTER_OUTPUT_PHASE == eventId) {
			// increments m_outputCount for output-generation so
			// can step by phases or decisions as well as output.
			if ( (a->m_agent->output_link_changed) ||
			     ( (++(a->m_nilOutputCycles)) >= (unsigned long) a->GetMaxNilOutputCycles()) )
			{
				a->IncrementgSKIStepCounter(gSKI_INTERLEAVE_OUTPUT);
				// m_nilOutputCycles gets reset whenever StepCtr is incremented for output
			}
		}
	}
    if (gSKIEVENT_AFTER_ELABORATION_CYCLE == eventId) 
	{
		a->IncrementgSKIStepCounter(gSKI_INTERLEAVE_ELABORATION_PHASE);
	} /* */

	RunNotifier rn(a, EnumRemappings::ReMapPhaseType(a->m_agent->current_phase,0));
    a->m_runListeners.Notify(eventId, rn);

	if ((a->m_runListeners.GetNumListeners(gSKIEVENT_BEFORE_PHASE_EXECUTED) > 0) &&
		(IsBEFOREPhaseEventID(eventId)))
	{
		a->m_runListeners.Notify(gSKIEVENT_BEFORE_PHASE_EXECUTED , rn);
	}
	else if ((a->m_runListeners.GetNumListeners(gSKIEVENT_AFTER_PHASE_EXECUTED) > 0) &&
		(IsAFTERPhaseEventID(eventId)))
	{
		a->m_runListeners.Notify(gSKIEVENT_AFTER_PHASE_EXECUTED , rn);
	} 
	
   }

   void Agent::DeleteRunEventCallbackData (soar_callback_data data)
   {
	   delete static_cast <Agent::RunEventCallbackData*>  (data); 
   }

   void Agent::HandleRunEventCallback(unsigned long         eventId, 
                                      unsigned char         eventOccured,
                                      void*                 object, 
                                      agent*                soarAgent, 
                                      void*                 data)
   {
      Agent*        a = static_cast<Agent*>(object);
      gSKI_K_PhaseCallbackData* phase_data = static_cast<gSKI_K_PhaseCallbackData*>(data);

	  // have to map to a gSKI event enum
 	// Only elaboration events should be going thru here.  Everything else is a
    // KernelEvent now.
	if (gSKIEVENT_AFTER_ELABORATION_CYCLE == EnumRemappings::Map_Kernel_to_gSKI_RunEventId(eventId,eventOccured)) 
	{
		a->IncrementgSKIStepCounter(gSKI_INTERLEAVE_ELABORATION_PHASE);
	} /* */

     // We have to change the the event id from a kernel id to a gSKI id
	  RunNotifier rn(a, EnumRemappings::ReMapPhaseType(phase_data->phase_type,0));
      a->m_runListeners.Notify(EnumRemappings::Map_Kernel_to_gSKI_RunEventId(eventId,eventOccured), rn);
   }

   // Listener to propagate the gSKI BEFORE_PHASE and AFTER_PHASE events 
   void Agent::HandleEventStatic(egSKIRunEventId eventId, Agent* a, egSKIPhaseType phase)
   {
	RunNotifier rn(a, EnumRemappings::ReMapPhaseType(a->m_agent->current_phase,0));
 
	// KJC 12/1/05:  everything should be going thru HandleKernelRunEventCallback
	//  or HandleRunEventCallback (for Elaborations only).
	if ((a->m_runListeners.GetNumListeners(gSKIEVENT_BEFORE_PHASE_EXECUTED) > 0) &&
		(IsBEFOREPhaseEventID(eventId)))
	{
		a->m_runListeners.Notify(gSKIEVENT_BEFORE_PHASE_EXECUTED , rn);
	}
	else if ((a->m_runListeners.GetNumListeners(gSKIEVENT_AFTER_PHASE_EXECUTED) > 0) &&
		(IsAFTERPhaseEventID(eventId)))
	{
		a->m_runListeners.Notify(gSKIEVENT_AFTER_PHASE_EXECUTED , rn);
	} 
}


   /*
   =========================
	
   =========================
   */
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

   int Agent::GetAttributePreferencesMode(Error* err) 
   {
	   return GetSoarAgent()->attribute_preferences_mode;
   }
	
   void Agent::SetAttributePreferencesMode(int mode, Error* err) 
   {
	   MegaAssert((mode >= 0) && (mode <= 2), "Attribute preferences mode must be 0, 1, or 2");
	   GetSoarAgent()->attribute_preferences_mode = mode;
   }
      
   int Agent::GetInputPeriod(Error* err) 
   {
	   return GetSoarAgent()->input_period;
   }
	
   void Agent::SetInputPeriod(int period, Error* err) 
   {
	   MegaAssert(period >= 0, "Input period must be non-negative");
	   GetSoarAgent()->input_period = period;
   }
      
   //////////////////////////////////////////////////// PRIVATES /////////////
   /*
   =============================
       step
   =============================
   */
   egSKIRunResult Agent::step(egSKIInterleaveType stepSize, unsigned long count)
   {    
	   // This method runs a single agent.  count typically will equal 1.
    
	   unsigned long*  startCount        = getReleventCounter(stepSize);
	   const unsigned long  END_COUNT    = (startCount)? ((*startCount) + count): 0;
  
   	   bool interrupted  = (m_runState == gSKI_RUNSTATE_INTERRUPTED)? true: false;

	   MegaAssert((count >= 0), "Cannot step for fewer than one count.");

	   if (! interrupted) {
		   MegaAssert(!m_agent->system_halted, "System should not be halted here!");
		   // Notify that agent is about to execute. (NOT the start of a run, just a step)
		   RunNotifier nfBeforeRunning(this,EnumRemappings::ReMapPhaseType(m_agent->current_phase,0));
		   m_runListeners.Notify(gSKIEVENT_BEFORE_RUNNING, nfBeforeRunning);
    
		   switch (stepSize) 
		   {
		   case  gSKI_INTERLEAVE_SMALLEST_STEP:     run_for_n_elaboration_cycles(m_agent, count); break;
		   case  gSKI_INTERLEAVE_ELABORATION_PHASE: run_for_n_elaboration_cycles(m_agent, count); break;
		   case  gSKI_INTERLEAVE_PHASE:             run_for_n_phases(m_agent, count);             break;
		   case  gSKI_INTERLEAVE_DECISION_CYCLE:    run_for_n_decision_cycles(m_agent, count);    break;
		   case  gSKI_INTERLEAVE_OUTPUT:            run_for_n_modifications_of_output(m_agent, count); 
//			                                        if (m_agent->stop_soar) 
//														this->IncrementgSKIStepCounter(gSKI_INTERLEAVE_OUTPUT);
													break;
		   }
	   }

       if ((m_interruptFlags & gSKI_STOP_AFTER_SMALLEST_STEP) || 
		   (m_interruptFlags & gSKI_STOP_AFTER_PHASE))
	   {
		   interrupted = true;
	   }
  
	   // KJC: If a gSKI_STOP_AFTER_DECISION_CYCLE has been requested, need to
	   // check that agent phase is at the proper stopping point before interrupting.
	   // If not at the right phase, but interrupt was requested, then the SML scheduler
	   // method IsAgentFinished will return true and MoveTo_StopBeforePhase will
	   // step the agent by phases until this test is satisfied.
       if ((m_interruptFlags & gSKI_STOP_AFTER_DECISION_CYCLE) && 
		   (m_agent->current_phase == m_kernel->GetStopPoint()))
	   {
		   interrupted = true;
	   }

	   		   
	   if (interrupted) 
	   {
	       // Notify of the interrupt
           RunNotifier nfAfterInt(this, m_lastPhase);
           m_runListeners.Notify(gSKIEVENT_AFTER_INTERRUPT, nfAfterInt);
 
		   /* This is probably redundant with the event above, which clients can listen for... */
		   PrintNotifier nfIntr(this, "Interrupt received.");
		   m_printListeners.Notify(gSKIEVENT_PRINT, nfIntr);
		   XMLNotifier xn1(this, kFunctionBeginTag, kTagMessage, 0) ;
		   m_XMLListeners.Notify(gSKIEVENT_XML_TRACE_OUTPUT, xn1);
		   XMLNotifier xn2(this, kFunctionAddAttribute, kTypeString, "Interrupt received.") ;
		   m_XMLListeners.Notify(gSKIEVENT_XML_TRACE_OUTPUT, xn2);
		   XMLNotifier xn3(this, kFunctionEndTag, kTagMessage, 0) ;
		   m_XMLListeners.Notify(gSKIEVENT_XML_TRACE_OUTPUT, xn3);
		   /* */
	   }

     
	   egSKIRunResult retVal;
  
	   // We've exited the run loop so we see what we should return
	   if(m_agent->system_halted)
	   {
		   // if the agent halted because it is in an infinite loop of no-change impasses
		   // interrupt the agents and allow the user to try to recover.
		   if ((long)m_agent->bottom_goal->id.level >=  m_agent->sysparams[MAX_GOAL_DEPTH])
		   {// the agent halted because it seems to be in an infinite loop, so throw interrupt
               AgentManager* am = (AgentManager*)(m_kernel->GetAgentManager());
			   am->InterruptAll(gSKI_STOP_AFTER_PHASE);
			   m_agent->system_halted = FALSE; // hack! otherwise won't run again.  
			   m_runState = gSKI_RUNSTATE_INTERRUPTED;
			   retVal     = gSKI_RUN_INTERRUPTED;
			   // Notify of the interrupt
    
			   RunNotifier nfAfterInt(this, m_lastPhase);
			   m_runListeners.Notify(gSKIEVENT_AFTER_INTERRUPT, nfAfterInt);

			   /* This is probably redundant with the event above, which clients can listen for... */
			   PrintNotifier nfIntr(this, "Interrupt received.");
			   m_printListeners.Notify(gSKIEVENT_PRINT, nfIntr);
			   XMLNotifier xn1(this, kFunctionBeginTag, kTagMessage, 0) ;
			   m_XMLListeners.Notify(gSKIEVENT_XML_TRACE_OUTPUT, xn1);
			   XMLNotifier xn2(this, kFunctionAddAttribute, kTypeString, "Interrupt received.") ;
			   m_XMLListeners.Notify(gSKIEVENT_XML_TRACE_OUTPUT, xn2);
			   XMLNotifier xn3(this, kFunctionEndTag, kTagMessage, 0) ;
			   m_XMLListeners.Notify(gSKIEVENT_XML_TRACE_OUTPUT, xn3);
		   }
		   else {
		   // If we halted, we completed and our state is halted
		   m_runState    = gSKI_RUNSTATE_HALTED;
		   retVal        = gSKI_RUN_COMPLETED;

		   RunNotifier nfAfterHalt(this, m_lastPhase);
           m_runListeners.Notify(gSKIEVENT_AFTER_HALTED, nfAfterHalt);

		   // fix for BUG 514  01-12-06
		   PrintNotifier nfHalted(this, "This Agent halted.");
		   m_printListeners.Notify(gSKIEVENT_PRINT, nfHalted);
		   XMLNotifier xn1(this, kFunctionBeginTag, kTagMessage, 0) ;
		   m_XMLListeners.Notify(gSKIEVENT_XML_TRACE_OUTPUT, xn1);
		   XMLNotifier xn2(this, kFunctionAddAttribute, kTypeString, "This Agent halted.") ;
		   m_XMLListeners.Notify(gSKIEVENT_XML_TRACE_OUTPUT, xn2);
		   XMLNotifier xn3(this, kFunctionEndTag, kTagMessage, 0) ;
		   m_XMLListeners.Notify(gSKIEVENT_XML_TRACE_OUTPUT, xn3);
		   }
	   }
	   else if(maxStepsReached(startCount, END_COUNT)) 
	   {
		   if(interrupted)
		   {
			   m_runState = gSKI_RUNSTATE_INTERRUPTED;
			   retVal     = gSKI_RUN_COMPLETED_AND_INTERRUPTED; 
			   //retVal     = gSKI_RUN_INTERRUPTED;
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

	   // Notify that agent stopped. (NOT the end of a run, just a step)
	   // Use AFTER_RUN_ENDS if you want to trap the end of the complete run.
	   RunNotifier nfAfterStop(this, m_lastPhase);
	   m_runListeners.Notify(gSKIEVENT_AFTER_RUNNING, nfAfterStop);

	   return retVal;
   }
 
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

		 /*  Agent RunEvents moved back to Kernel; preStep does nothing now

          * // Do pre-step notifications to listeners
		  * if (m_agent->operand2_mode) {
          *    preStepNotifications();
		  * } else {
		  *	 preStepNotificationsSoar7();
		  * }
		  */

         ///////////////////////////////////////////////////////////////////
         // Execute the next step
 
		 if (runType == gSKI_RUN_ELABORATION_CYCLE) {
			 run_for_n_elaboration_cycles(m_agent, 1);
		 } else {
             run_for_n_phases(m_agent, 1);
		 }

         // Remember the phase we just did
         m_lastPhase = m_nextPhase;

		 // the current_phase in SoarKernel is the phase to be run next.
         m_nextPhase = EnumRemappings::ReMapPhaseType((unsigned short)m_agent->current_phase, 
                                                      (m_agent->applyPhase)? true: false);
		 // boolean above is no longer used to determine Propose or Apply, should remove

		 ///////////////////////////////////////////////////////////////////

         // Do post step notifications -- only checks interrupts as of 8.6.2
		 //  all events are generated from SoarKernel
		 if (m_agent->operand2_mode) {
			 interrupted = postStepNotifications();
		 } else {
			 interrupted = postStepNotificationsSoar7();
		 }

         // If we are doing a suspend type interrupt, here is where we suspend!
         // If we don't suspend, we exit this loop.
         if(interrupted && m_suspendOnInterrupt)
         {
           // Notify of the interrupt
           RunNotifier nfAfterInt(this, m_lastPhase);
           m_runListeners.Notify(gSKIEVENT_AFTER_INTERRUPT, nfAfterInt);		
		   /* This is probably redundant with the event above, which clients can listen for... */
		   PrintNotifier nfIntr(this, "Interrupt received.");
		   m_printListeners.Notify(gSKIEVENT_PRINT, nfIntr);
		   XMLNotifier xn1(this, kFunctionBeginTag, kTagMessage, 0) ;
		   m_XMLListeners.Notify(gSKIEVENT_XML_TRACE_OUTPUT, xn1);
		   XMLNotifier xn2(this, kFunctionAddAttribute, kTypeString, "Interrupt received.") ;
		   m_XMLListeners.Notify(gSKIEVENT_XML_TRACE_OUTPUT, xn2);
		   XMLNotifier xn3(this, kFunctionEndTag, kTagMessage, 0) ;
		   m_XMLListeners.Notify(gSKIEVENT_XML_TRACE_OUTPUT, xn3);
		   /* */

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
	  // This is "AFTER_RUNNING_A_PHASE" (or other chunk) not after the end of the complete run.
	  // Use AFTER_RUN_ENDS if you want to trap the end of the complete run.
      RunNotifier nfAfterStop(this, m_lastPhase);
      m_runListeners.Notify(gSKIEVENT_AFTER_RUNNING, nfAfterStop);

      return retVal;
   }

	/** Fire the gSKIEVENT_BEFORE_RUN_STARTS event.
	    This is fired once before any running occurs **/
   void Agent::FireRunStartsEvent()
	{
       RunNotifier nfBeforeStart(this, m_lastPhase);
       m_runListeners.Notify(gSKIEVENT_BEFORE_RUN_STARTS, nfBeforeStart);
	}

	/** Fire the gSKIEVENT_AFTER_RUN_ENDS event.
	    This is fired once at the end of a complete run. **/
	void Agent::FireRunEndsEvent()
	{
       RunNotifier nfAfterEnd(this, m_lastPhase);
       m_runListeners.Notify(gSKIEVENT_AFTER_RUN_ENDS, nfAfterEnd);
	}

   /*
   =============================
   sometimes we want the relevant counter for the RunType and
   sometimes we want it for the interleaveType
   =============================
   */
   unsigned long* Agent::getReleventCounter(egSKIRunType runType)
   {
      // Returns a pointer to the counter that is relevant
      //  for the given run type.  This is used by the
      //  run method to track how many of a particular
      //  run step it has executed.
      switch(runType)
      {
      case gSKI_RUN_SMALLEST_STEP:
         return &m_smallestStepCount;
      case gSKI_RUN_PHASE:
         return &m_phaseCount;
      case gSKI_RUN_ELABORATION_CYCLE:
		 return &m_elaborationCount; 
		 /***********
		 if (m_agent->operand2_mode) {
 			//      count only true Soar Elaboration cycles
			//      this makes most sense for Soar 8 mode.
			//      or can choose to use m_elaborationCounts...
			return &m_agent->e_cycle_count;
		 } else {
			// m_elaborationCount increments for an elaboration,
			// or a phase, but not both, in the same agent::run.
			// this is consistent with SoarKernel in Soar7 mode
			return &m_elaborationCount; 
		 }  ***********/
      case gSKI_RUN_DECISION_CYCLE:
		 return &m_agent->d_cycle_count;
      case gSKI_RUN_UNTIL_OUTPUT:
         return &m_outputCount;
		 // KJC Nov 05.  Added to kernel agent, d_cycle_last_output
      default:
         return 0;
      }
   }

      
   unsigned long* Agent::getReleventCounter(egSKIInterleaveType stepType)
   {
      // Returns a pointer to the counter that is relevant
      //  for the given run type.  This is used by the
      //  run method to track how many of a particular
      //  run step it has executed.
      switch(stepType)
      {
      case gSKI_INTERLEAVE_PHASE:
         return &m_phaseCount;
      case gSKI_INTERLEAVE_ELABORATION_PHASE:
		 return &m_elaborationCount; 
      case gSKI_INTERLEAVE_DECISION_CYCLE:
		 return &m_agent->d_cycle_count;
      case  gSKI_INTERLEAVE_OUTPUT:
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
       // KJC:  These can all go away if we use the gSKI events in SoarKernel
	   // where they are all in the proper part of respective phases.
	   
	 /*  // KJC removing agent RunEvents from schedulers

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
	

      // See if this is an elaboration cycle   <<-- won't work for Soar 7 mode
      if((m_nextPhase == gSKI_PROPOSAL_PHASE) || (m_nextPhase == gSKI_APPLY_PHASE))
      {
         RunNotifier nfBeforeElab(this, m_nextPhase);
         m_runListeners.Notify(gSKIEVENT_BEFORE_ELABORATION_CYCLE, nfBeforeElab);
      }
	  */

	 /* deprecated as of 8.6.2
      // Tell listeners about smallest step   <<-- won't work for elaborations
      RunNotifier nfBeforeStep(this, m_nextPhase);
      m_runListeners.Notify(gSKIEVENT_BEFORE_SMALLEST_STEP, nfBeforeStep);
	  */
   }

   void Agent::preStepNotificationsSoar7()
   {
       // KJC:  These can all go away if we use the gSKI events in SoarKernel
	   // where they are all in the proper part of respective phases.

	  /*  // KJC removing agent RunEvents from schedulers
	   
      // Input phase is the beginning of an elaboration phase
      if(m_nextPhase == gSKI_INPUT_PHASE)
      {
         RunNotifier nfBeforeElab(this, m_nextPhase);
         m_runListeners.Notify(gSKIEVENT_BEFORE_ELABORATION_CYCLE, nfBeforeElab);
         // Input phase is the beginning of the decision cycle if e_cycles_this_d_cycle == 0
         if (!m_agent->e_cycles_this_d_cycle)
		 {
 			 RunNotifier nfBeforeDC(this, m_nextPhase);
 			 m_runListeners.Notify(gSKIEVENT_BEFORE_DECISION_CYCLE, nfBeforeDC);
		 }
      } 

      // Soar 7 is always starting a new phase
      RunNotifier nfBeforePhase(this, m_nextPhase);
      m_runListeners.Notify(gSKIEVENT_BEFORE_PHASE_EXECUTED, nfBeforePhase);
 
	  */

	  //  KJC: not sure this is needed for Soar 7 (or Soar 8) because always new phase
	  // Tell listeners about smallest step   <<-- won't work for elaborations
      //RunNotifier nfBeforeStep(this, m_nextPhase);
      //m_runListeners.Notify(gSKIEVENT_BEFORE_SMALLEST_STEP, nfBeforeStep);
   }

   /*
   =============================
!!!!  As of 8.6.2, all RunEvents are generated from SoarKernel and
      all counters are updated on the eventHandlers.  This extricates
	  gSKI from the Soar scheduling loop.  The only thing now done
	  in the postStepNotifications routines is checking Interrupts,
	  and not sure if these are propagated as intended given the
	  not-quite-logical bitmapping tests...
   =============================
   */
   bool Agent::postStepNotifications()
   {
      bool interrupted = false;

      // Notify listeners about the end of the phase
	  /* // KJC removing agent RunEvents from schedulers
         //   RunNotifier nfAfterStep(this, m_lastPhase);
         //   m_runListeners.Notify(gSKIEVENT_AFTER_SMALLEST_STEP, nfAfterStep); 
       */
      // Increment smallest step
      //  deprecated as of 8.6.2   ++m_smallestStepCount;
 	  // see GetRelevantCounter if only want to count actual Soar e_cycles.
      // moved to HandleKernelRunEvent   ++m_elaborationCount;   
 
      // Check an interrupt
      if(m_interruptFlags & gSKI_STOP_AFTER_SMALLEST_STEP)
         interrupted = true;

	  /* // KJC removing agent RunEvents from schedulers
         // See if this was an elaboration cycle 
         if((m_lastPhase == gSKI_PROPOSAL_PHASE) || (m_lastPhase == gSKI_APPLY_PHASE))
         {
            RunNotifier nfAfterElab(this, m_lastPhase);
            m_runListeners.Notify(gSKIEVENT_AFTER_ELABORATION_CYCLE, nfAfterElab);
         }
	  */

      // Check to see if we've moved to another phase or not
      if(m_lastPhase != m_nextPhase)
      {
         // Increment the number of outputs that have occured
		 // (Do this before notify listeners so they can see the updated counter)
         // moved to HandleKernelRunEvent    
		  /*if((m_lastPhase == gSKI_OUTPUT_PHASE) && m_agent->output_link_changed)
            ++m_outputCount; */

	  /*   // KJC removing agent RunEvents from schedulers
         RunNotifier nfAfterPhase(this, m_lastPhase);
         m_runListeners.Notify(gSKIEVENT_AFTER_PHASE_EXECUTED, nfAfterPhase);
	  */
         // Increment the phase count
      // moved to HandleKernelRunEvent   ++m_phaseCount;

         // Check an interrupt
         if(m_interruptFlags & gSKI_STOP_AFTER_PHASE)
            interrupted = true;
      }


      // If the next phase is the input phase, we just ended a DC
      if(m_nextPhase == gSKI_INPUT_PHASE)
      {
	  /*   // KJC removing agent RunEvents from schedulers

         RunNotifier nfAfterDC(this, m_lastPhase);
         m_runListeners.Notify(gSKIEVENT_AFTER_DECISION_CYCLE, nfAfterDC);
	  */

         // Check an interrupt
         if(m_interruptFlags & gSKI_STOP_AFTER_DECISION_CYCLE)
            interrupted = true;
      }

      return interrupted;
   }

   bool Agent::postStepNotificationsSoar7()
   {
      bool interrupted = false;

	  //  We took these out of the Soar7 preStep...
      // Notify listeners about the end of the phase
 	  /* // KJC removing agent RunEvents from schedulers
         //RunNotifier nfAfterStep(this, m_lastPhase);
         //m_runListeners.Notify(gSKIEVENT_AFTER_SMALLEST_STEP, nfAfterStep);
	   */

      // Increment smallest step
      //  deprecated as of 8.6.2   ++m_smallestStepCount;   
 	  // Increment the elaboration count  
	  // see GetRelevantCounter if only want to count actual Soar e_cycles.
	  // ++m_elaborationCount;    // works for Soar 7, since gSKI now calls
	                           // run_for_n_elaboration_cycles
 

      // Check an interrupt
      if(m_interruptFlags & gSKI_STOP_AFTER_SMALLEST_STEP)
         interrupted = true;

	     /*   // KJC removing agent RunEvents from schedulers
         // Soar 7 is always finishing a phase
         RunNotifier nfAfterPhase(this, m_lastPhase);
         m_runListeners.Notify(gSKIEVENT_AFTER_PHASE_EXECUTED, nfAfterPhase);
		  */

	     /*   // KJC these were all moved to HandleKernelRunEvent
         // Increment the phase count
         ++m_phaseCount;  

         // Increment the number of outputs that have occured
         if((m_lastPhase == gSKI_OUTPUT_PHASE) && m_agent->output_link_changed)
            ++m_outputCount;
		  */

         // Check an interrupt
         if(m_interruptFlags & gSKI_STOP_AFTER_PHASE)
            interrupted = true;
     
	  /* // KJC removing agent RunEvents from schedulers
	  // See if this was an elaboration cycle 
      if(m_lastPhase == gSKI_OUTPUT_PHASE)
      {
         RunNotifier nfAfterElab(this, m_lastPhase);
         m_runListeners.Notify(gSKIEVENT_AFTER_ELABORATION_CYCLE, nfAfterElab);
      }
	  */

      // If the last phase was DECISION, we just ended a DC
      if(m_lastPhase == gSKI_DECISION_PHASE)
      {
		  	 /*  // KJC removing agent RunEvents from schedulers
         RunNotifier nfAfterDC(this, m_lastPhase);
         m_runListeners.Notify(gSKIEVENT_AFTER_DECISION_CYCLE, nfAfterDC);
			  */
         // Check an interrupt
         if(m_interruptFlags & gSKI_STOP_AFTER_DECISION_CYCLE)
            interrupted = true;
      }

      return interrupted;
   }

   bool Agent::GetOperand2Mode() {
		return m_agent->operand2_mode ? true : false;
   }

   void Agent::SetOperand2Mode(bool mode) {
		m_agent->operand2_mode = mode ? TRUE : FALSE;
   }

   /*
   ==================================
   //
   //
   // NOTE!  THIS IS A WORKAROUND HACK!!!  THIS SHOULD NOT STAY HERE!!!
   //        IT IS NEEDED FOR THE TSI UNTIL IT CAN GET REWRITTEN!
   //
   //  KJC, June 05:  it's used all over in gSKI...
   ==================================
   */
   extern agent* GetSoarAgentPtr(Agent* agent)
   {
      return ((Agent*)agent)->GetSoarAgent();
   }


}
