#include <portability.h>

/////////////////////////////////////////////////////////////////
// AgentSML class file.
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : September 2004
//
// This class is used to keep track of information needed by SML
// (Soar Markup Language) on an agent by agent basis.
//
/////////////////////////////////////////////////////////////////

#include "sml_AgentSML.h"

#include "sml_Utils.h"
#include "sml_OutputListener.h"
#include "sml_StringOps.h"
#include "sml_KernelSML.h"
#include "sml_RhsFunction.h"

#include "KernelHeaders.h"
#include "xml.h"
#include "soar_rand.h"

#ifdef _DEBUG
// Comment this in to debug init-soar and inputwme::update calls
//#define DEBUG_UPDATE
#endif

#include <assert.h>

using namespace sml ;

AgentSML::AgentSML(KernelSML* pKernelSML, agent* pAgent)
{
	m_pKernelSML = pKernelSML ;
	m_SuppressRunEndsEvent = false ;

	m_agent = pAgent ;

	m_pAgentRunCallback = new AgentRunCallback() ;
	m_pAgentRunCallback->SetAgentSML(this) ;

	m_pCaptureFile = 0;
}

void AgentSML::InitListeners()
{
	m_PrintListener.Init(m_pKernelSML, this) ;
	m_XMLListener.Init(m_pKernelSML, this) ;
	m_RunListener.Init(m_pKernelSML, this) ;
	m_ProductionListener.Init(m_pKernelSML, this) ;
	m_OutputListener.Init(m_pKernelSML, this) ;
	m_InputListener.Init(m_pKernelSML, this) ;

	// For KernelSML (us) to work correctly we need to listen for certain events, independently of what any client is interested in
	// Currently:
	// Listen for output callback events so we can send this output over to the clients
	// Listen for "before" init-soar events (we need to know when these happen so we can release all WMEs on the input link, otherwise gSKI will fail to re-init the kernel correctly.)
	// Listen for "after" init-soar events (we need to know when these happen so we can resend the output link over to the client)
	m_OutputListener.RegisterForKernelSMLEvents() ;
	m_InputListener.RegisterForKernelSMLEvents() ;

}

// Can't call this until after the Soar agent has been initialized
void AgentSML::Init()
{
	// Temporary HACK.  This should be fixed in the kernel.
	m_agent->stop_soar = FALSE;

	// Initializing the soar agent
	init_soar_agent( m_agent );

	m_pRhsInterrupt = new InterruptRhsFunction(this) ;
	m_pRhsConcat    = new ConcatRhsFunction(this) ;
	m_pRhsExec		= new ExecRhsFunction(this) ;
	m_pRhsCmd		= new CmdRhsFunction(this) ;
	RegisterRHSFunction(m_pRhsInterrupt) ;
	RegisterRHSFunction(m_pRhsConcat) ;
	RegisterRHSFunction(m_pRhsExec) ;
	RegisterRHSFunction(m_pRhsCmd) ;

	// Set counters and flags used to control runs
	InitializeRuntimeState() ;
	
	ResetCaptureReplay();

	// Register for the new INPUT_WME_GARBAGE_COLLECTED_CALLBACK
	// Base the id on the address of this object which ensures it's unique
	std::ostringstream callbackId;
	callbackId << "id_0x" << this << "_evt_" << INPUT_WME_GARBAGE_COLLECTED_CALLBACK;
	soar_add_callback ( GetSoarAgent(), INPUT_WME_GARBAGE_COLLECTED_CALLBACK, InputWmeGarbageCollectedHandler, 
		INPUT_WME_GARBAGE_COLLECTED_CALLBACK, this, 0, const_cast< char* >( callbackId.str().c_str() ) ) ;
}

AgentSML::~AgentSML()
{
	ResetCaptureReplay();

	// Register for the new INPUT_WME_GARBAGE_COLLECTED_CALLBACK
	// Base the id on the address of this object which ensures it's unique
	std::ostringstream callbackId;
	callbackId << "id_0x" << this << "_evt_" << INPUT_WME_GARBAGE_COLLECTED_CALLBACK;
	soar_remove_callback( GetSoarAgent(), INPUT_WME_GARBAGE_COLLECTED_CALLBACK, callbackId.str().c_str() ) ;

	delete m_pAgentRunCallback ;

	/* RPM 9/06 added code from reinitialize_soar to clean up stuff hanging from last run
			   need to put it here instead of in destroy_soar_agent because gSKI is
				cleaning up too much stuff and thus it will crash if called later */
	clear_goal_stack (m_agent);
	m_agent->active_level = 0; /* Signal that everything should be retracted */
	m_agent->FIRING_TYPE = IE_PRODS;
	do_preference_phase (m_agent);   /* allow all i-instantiations to retract */

	destroy_soar_agent( m_agent );
}

// Release any objects or other data we are keeping.  We do this just
// prior to deleting AgentSML, but before the underlying gSKI agent has been deleted
// 'deletingThisAgent' should only be true when we're actually in the destructor.
void AgentSML::Clear(bool deletingThisAgent)
{
#ifdef DEBUG_UPDATE
	sml::PrintDebugFormat("AgentSML::Clear start %s", deletingThisAgent ? "deleting this agent." : "not deleting this agent.") ;
#endif

	// Release any WME objects we still own.
	// (Don't flush removes in this case as we're shutting down rather than just doing an init-soar).
	ReleaseAllWmes(!deletingThisAgent) ;

	RemoveRHSFunction(m_pRhsInterrupt) ; delete m_pRhsInterrupt ; m_pRhsInterrupt = NULL ;
	RemoveRHSFunction(m_pRhsConcat)    ; delete m_pRhsConcat    ; m_pRhsConcat = NULL ;
	RemoveRHSFunction(m_pRhsExec)      ; delete m_pRhsExec      ; m_pRhsExec = NULL ;
	RemoveRHSFunction(m_pRhsCmd)       ; delete m_pRhsCmd       ; m_pRhsCmd = NULL ;

	m_ProductionListener.Clear();
	m_RunListener.Clear();
	m_PrintListener.Clear();
	m_OutputListener.Clear() ;
	m_XMLListener.Clear() ;

#ifdef DEBUG_UPDATE
	sml::PrintDebugFormat("AgentSML::Clear end %s", deletingThisAgent ? "deleting this agent." : "not deleting this agent.") ;
#endif
}

// Release all of the WMEs that we currently have references to
// It's a little less severe than clear() which releases everything we own, not just wmes.
// If flushPendingRemoves is true, make sure gSKI removes all wmes from Soar's working memory
// that have been marked for removal but are still waiting for the next input phase to actually
// be removed (this should generally be correct so we'll default to true for it).
void AgentSML::ReleaseAllWmes(bool flushPendingRemoves)
{
#ifdef DEBUG_UPDATE
	sml::PrintDebugFormat("****************************************************") ;
	sml::PrintDebugFormat("%s AgentSML::ReleaseAllWmes start %s", this->GetgSKIAgent()->GetName(), flushPendingRemoves ? "flush pending removes." : "do not flush pending removes.") ;
	sml::PrintDebugFormat("****************************************************") ;
#endif

	if (flushPendingRemoves)
	{
		// TODO:
		//bool forceAdds = false ;	// It doesn't matter if we do these or not as we're about to release everything.  Seems best to not start things up.
		//bool forceRemoves = true ;	// SML may have deleted a wme but gSKI has yet to act on this.  As SML has removed its object we have no way to free the gSKI object w/o doing this update.
	}

	//PrintDebugFormat("About to release kernel wmes") ;
	
   // BADBAD: we don't create or maintain this map anymore -- does this loop need to be replaced with something?
	//for (KernelTimeTagMapIter mapIter = m_KernelTimeTagMap.begin() ; mapIter != m_KernelTimeTagMap.end() ; mapIter++) {
	//	wme* wme = mapIter->second ;
	//	KernelSML::PrintDebugWme("Releasing ", wme, true) ;
	//	release_io_symbol(this->GetSoarAgent(), wme->id) ;
	//	release_io_symbol(this->GetSoarAgent(), wme->attr) ;
	//	release_io_symbol(this->GetSoarAgent(), wme->value) ;
	//}

	for (PendingInputListIter iter = m_PendingInput.begin() ; iter != m_PendingInput.end() ; iter++)
	{
		soarxml::ElementXML* pMsg = *iter ;
		delete pMsg ;
	}

	m_PendingInput.clear() ;
	m_ToClientIdentifierMap.clear() ;
	m_IdentifierMap.clear() ;
	m_IdentifierRefMap.clear() ;

#ifdef DEBUG_UPDATE
	sml::PrintDebugFormat("****************************************************") ;
	sml::PrintDebugFormat("%s AgentSML::ReleaseAllWmes end %s", this->GetgSKIAgent()->GetName(), flushPendingRemoves ? "flush pending removes." : "do not flush pending removes.") ;
	sml::PrintDebugFormat("****************************************************") ;
#endif
}

char const* AgentSML::GetName()
{
	return m_agent->name ;
}

void AgentSML::InitializeRuntimeState()
{
	m_WasOnRunList = false;
	m_ScheduledToRun = false ;
	m_OnStepList = false;
	m_ResultOfLastRun = sml_RUN_COMPLETED ;
	m_InitialRunCount = 0 ;
	m_CompletedOutputPhase = false ;
	m_GeneratedOutput = false ;
	m_OutputCounter = 0 ;
    m_localRunCount = 0 ;
    m_localStepCount= 0 ;
    m_runState = sml_RUNSTATE_STOPPED;
    m_interruptFlags = 0 ;
}

void AgentSML::ResetCaptureReplay()
{
	if (m_pCaptureFile) 
	{
		StopCaptureInput();
	}

	m_ReplayTimetagMap.clear();

	while (!m_CapturedActions.empty())
	{
		m_CapturedActions.pop();
	}
	
	m_ReplayInput = false;
}

bool AgentSML::Reinitialize()
{
	m_pKernelSML->FireAgentEvent(this, smlEVENT_BEFORE_AGENT_REINITIALIZED) ;

    bool ok = reinitialize_soar( m_agent );
    init_agent_memory( m_agent );

	InitializeRuntimeState() ;

	ResetCaptureReplay();
	m_pKernelSML->FireAgentEvent(this, smlEVENT_AFTER_AGENT_REINITIALIZED) ;
	return ok ;
}

void AgentSML::RemoveAllListeners(Connection* pConnection)
{
	m_ProductionListener.RemoveAllListeners(pConnection);
	m_RunListener.RemoveAllListeners(pConnection);
	m_PrintListener.RemoveAllListeners(pConnection);
	m_OutputListener.RemoveAllListeners(pConnection) ; 
	m_XMLListener.RemoveAllListeners(pConnection) ;
}

/*************************************************************
* @brief	Add an input message to the pending input list
*			-- it will be processed on the next input phase callback from the kernel.
*************************************************************/
void AgentSML::AddToPendingInputList(ElementXML_Handle hInputMsgHandle)
{
	// Create a new wrapper object for the message and store that with
	// an increased ref count so the caller can do whatever they want with the original message
	soarxml::ElementXML* pMsg = new soarxml::ElementXML(hInputMsgHandle) ;
	pMsg->AddRefOnHandle() ;

	m_PendingInput.push_back(pMsg) ;
}

//=============================
//Number of complete decision cycles (through to end of output)
//=============================
unsigned long AgentSML::GetNumDecisionCyclesExecuted()
{
  return m_agent->d_cycle_count;
}

//=============================
//Number of decisions (operator selections/impasses) instead of full D_cycles
//=============================
unsigned long AgentSML::GetNumDecisionsExecuted()
{
  return m_agent->decision_phases_count;
}

//=============================
//Number of phases since last init-soar
//=============================
unsigned long AgentSML::GetNumPhasesExecuted()
{
	return m_agent->run_phase_count;
}

//=============================
// Number of elaborations (or completed phases w/o rules firing) since last init-soar
// This definitions of elaborations matches with "run 1 -e".
// If you want to know how many sets of rules have fired you need a different counter.
//=============================
unsigned long AgentSML::GetNumElaborationsExecuted()
{
	return m_agent->run_elaboration_count ;
}

//=============================
//Number of outputs generated by this agent (or times reaching "max-nil-outputs" limit)
//=============================
unsigned long AgentSML::GetNumOutputsGenerated()
{
	return m_agent->run_generated_output_count ;
}

//=============================
//Number of output phases since this agent last generated output
//=============================
unsigned long AgentSML::GetLastOutputCount()
{
	return m_agent->run_last_output_count ;
}

//=============================
// Reset the count of how long since the agent last generated output
//=============================
void AgentSML::ResetLastOutputCount()
{
	m_agent->run_last_output_count = 0 ;
}

//=============================
// Returns the current phase (which generally means the phase that is next going to execute
// if you inspect this in between runs)
//=============================
smlPhase AgentSML::GetCurrentPhase()
{
	switch ( (unsigned short)m_agent->current_phase )
	{
	case INPUT_PHASE:		return sml_INPUT_PHASE;
	case PROPOSE_PHASE:		return sml_PROPOSAL_PHASE;
	case DECISION_PHASE:	return sml_DECISION_PHASE;
	case APPLY_PHASE:		return sml_APPLY_PHASE;
	case OUTPUT_PHASE:		return sml_OUTPUT_PHASE;
	case PREFERENCE_PHASE:	return sml_PREFERENCE_PHASE;
	case WM_PHASE:			return sml_WM_PHASE;

	default:
		assert(false);
		return sml_INPUT_PHASE;
	}
}

unsigned long AgentSML::GetRunCounter(smlRunStepSize runStepSize)
{
	switch(runStepSize)
	{
	case sml_PHASE:
		{
			unsigned long phases = GetNumPhasesExecuted() ;
			return phases ;
		}
	case sml_ELABORATION:
		{
			unsigned long elabs = GetNumElaborationsExecuted() ;
			return elabs ;
		}
	case sml_DECISION:
		{
			unsigned long decs = GetNumDecisionCyclesExecuted() ;
			return decs ;
		}
	case sml_UNTIL_OUTPUT:
		{
			unsigned long outputs = GetNumOutputsGenerated() ;
			return outputs ;
		}
	default:
		return 0;
	}
}

//=============================
// Request that the agent stop soon.
//=============================
void AgentSML::Interrupt(smlStopLocationFlags stopLoc)
{
  // Tell the agent where to stop
  m_interruptFlags = stopLoc;

  // If the request for interrupt is sml_STOP_AFTER_DECISION_CYCLE, then it 
  // will be caught in the RunScheduler::CompletedRunType() and/or IsAgentFinished().
  // We don't want to interrupt agents until the appropriate time if the request is  
  // sml_STOP_AFTER_DECISION_CYCLE.
  
  // These are immediate requests for interrupt, such as from RHS or application
  if ((sml_STOP_AFTER_SMALLEST_STEP == stopLoc) || (sml_STOP_AFTER_PHASE == stopLoc)) {
	  m_agent->stop_soar = TRUE;
	  // If the agent is not running, we should set the runState flag now so agent won't run
	  if (m_runState == sml_RUNSTATE_STOPPED)
	  {
		  m_runState = sml_RUNSTATE_INTERRUPTED;
	  }
	  // Running agents must test stopLoc & stop_soar in Step method to see if interrupted.
	  // Because we set m_agent->stop_soar == TRUE above, any running agents should return to
	  // gSKI at the end of the current phase, even if interleaving by larger steps.  KJC
  }
}

//=============================
// Clear any existing interrupt requests
// (Generally done before starting a run)
//=============================
void AgentSML::ClearInterrupts()
{
  // Clear the interrupts whether running or not
  m_interruptFlags = 0;

  // Only change state of agent if it is running
  if(m_runState == sml_RUNSTATE_INTERRUPTED)
  {
    // We returned, and thus are stopped
    m_runState = sml_RUNSTATE_STOPPED;
  }
}

smlRunResult AgentSML::StepInClientThread(smlRunStepSize  stepSize)
{
  // Agent is already running, we cannot run
  if(m_runState != sml_RUNSTATE_STOPPED)
  {
     return sml_RUN_ERROR;
  }

  m_runState = sml_RUNSTATE_RUNNING;

  // This method does all the work
  return Step(stepSize);
}

void AgentSML::FireRunEvent(smlRunEventId eventId) {
	if ( eventId == smlEVENT_AFTER_RUN_ENDS )
	{
		// Send the trace to the clients
		xml_invoke_callback( GetSoarAgent() ); // invokes XML_GENERATION_CALLBACK, clears XML state
	}

	// Trigger a callback from the kernel to propagate the event out to listeners.
	// This allows us to use a single uniform model for all run events (even when some are really originating here in SML).
	int callbackEvent = KernelCallback::GetCallbackFromEventID(eventId) ;
	soar_invoke_callbacks(m_agent, SOAR_CALLBACK_TYPE(callbackEvent),reinterpret_cast<soar_call_data>(m_agent->current_phase));
}

void AgentSML::FireSimpleXML(char const* pMsg)
{
	// Trigger a callback from the kernel to propagate the event out to listeners.
	// This allows us to use a single uniform model for all run events (even when some are really originating here in SML).
	soar_invoke_first_callback(m_agent, PRINT_CALLBACK, /*(ClientData)*/ static_cast<void*>( const_cast<char*>( pMsg ) ) );
	xml_generate_message( m_agent, pMsg );
}

static bool maxStepsReached(unsigned long steps, unsigned long maxSteps)
{
	return (steps >= maxSteps);
}

smlRunResult AgentSML::Step(smlRunStepSize stepSize)
{
   // This method runs a single agent
   unsigned long count = 1 ;
   unsigned long startCount        = GetRunCounter(stepSize) ; // getReleventCounter(stepSize);
   const unsigned long  END_COUNT  = startCount + count ;

   bool interrupted  = (m_runState == sml_RUNSTATE_INTERRUPTED)? true: false;

   if (! interrupted) {
	   assert(!m_agent->system_halted) ; // , "System should not be halted here!");
	   // Notify that agent is about to execute. (NOT the start of a run, just a step)
	   FireRunEvent(smlEVENT_BEFORE_RUNNING) ;

	   switch (stepSize) 
	   {
	   case  sml_ELABORATION:	run_for_n_elaboration_cycles(m_agent, count); break;
	   case  sml_PHASE:         run_for_n_phases(m_agent, count);             break;
	   case  sml_DECISION:		run_for_n_decision_cycles(m_agent, count);    break;
	   case  sml_UNTIL_OUTPUT:  run_for_n_modifications_of_output(m_agent, count); break;
	   }
   }

   if ((m_interruptFlags & sml_STOP_AFTER_SMALLEST_STEP) || 
	   (m_interruptFlags & sml_STOP_AFTER_PHASE))
   {
	   interrupted = true;
   }

   // KJC: If a gSKI_STOP_AFTER_DECISION_CYCLE has been requested, need to
   // check that agent phase is at the proper stopping point before interrupting.
   // If not at the right phase, but interrupt was requested, then the SML scheduler
   // method IsAgentFinished will return true and MoveTo_StopBeforePhase will
   // step the agent by phases until this test is satisfied.
   if ( m_interruptFlags & sml_STOP_AFTER_DECISION_CYCLE )
   {
	   // JRV: Bug 782: I changed the second half of the interrupt test to be true if:
	   //  * The agent is in the correct phase for stopping (this was only what was here before)
	   //  * OR stepSize == run-til-output because this current_phase will always be input here if running until output
	   //       and we do want to stop in this case, even though the stop-before phase isn't technically honored.
	   // I've noted that the stop-before phase isn't honored in this case in the bug report.
	   bool inCorrectStopBeforePhase = m_agent->current_phase == m_pKernelSML->ConvertSMLToSoarPhase( m_pKernelSML->GetStopBefore() );
	   bool runningUntilOutput = stepSize == sml_UNTIL_OUTPUT;
	   if ( inCorrectStopBeforePhase || runningUntilOutput )
	   {
		   interrupted = true;
	   }
   }
   		   
   if (interrupted) 
   {
       // Notify of the interrupt
	   FireRunEvent(smlEVENT_AFTER_INTERRUPT) ;

	   /* This is probably redundant with the event above, which clients can listen for... */
	   FireSimpleXML("Interrupt received.") ;
   }
 
   smlRunResult retVal;

   // We've exited the run loop so we see what we should return
   if(m_agent->system_halted)
   {
	   // if the agent halted because it is in an infinite loop of no-change impasses
	   // interrupt the agents and allow the user to try to recover.
	   if (m_agent->bottom_goal->id.level >=  m_agent->sysparams[MAX_GOAL_DEPTH])
	   {// the agent halted because it seems to be in an infinite loop, so throw interrupt
		   m_pKernelSML->InterruptAllAgents(sml_STOP_AFTER_PHASE) ;
		   m_agent->system_halted = FALSE; // hack! otherwise won't run again.  
		   m_runState = sml_RUNSTATE_INTERRUPTED;
		   retVal     = sml_RUN_INTERRUPTED;
		   // Notify of the interrupt

		   FireRunEvent(smlEVENT_AFTER_INTERRUPT) ;

		   /* This is probably redundant with the event above, which clients can listen for... */
		   FireSimpleXML("Interrupt received.") ;
	   }
	   else 
	   {
		   // If we halted, we completed and our state is halted
		   m_runState    = sml_RUNSTATE_HALTED;
		   retVal        = sml_RUN_COMPLETED;

		   FireRunEvent(smlEVENT_AFTER_HALTED) ;

		   // fix for BUG 514  01-12-06
		   FireSimpleXML("This Agent halted.") ;
	   }
   }
   else if(maxStepsReached(GetRunCounter(stepSize), END_COUNT)) 
   {
	   if(interrupted)
	   {
		   m_runState = sml_RUNSTATE_INTERRUPTED;
		   retVal     = sml_RUN_COMPLETED_AND_INTERRUPTED; 
		   //retVal     = sml_RUN_INTERRUPTED;
	   }
	   else
	   {
		   m_runState = sml_RUNSTATE_STOPPED;
		   retVal     = sml_RUN_COMPLETED;
	   }
   }
   else 
   {  
	   // We were interrupted before we could complete
	   assert(interrupted) ; //, "Should never get here if we aren't interrupted");

	   m_runState    = sml_RUNSTATE_INTERRUPTED;
	   retVal        = sml_RUN_INTERRUPTED;
   }    

   // Notify that agent stopped. (NOT the end of a run, just a step)
   // Use AFTER_RUN_ENDS if you want to trap the end of the complete run.
   FireRunEvent(smlEVENT_AFTER_RUNNING) ;

   return retVal;
}

void AgentSML::DeleteSelf()
{
	this->Clear(true) ;

	// Remove the listeners that KernelSML uses for this agent.
	// This is important.  Otherwise if we create a new agent using the same kernel object
	// the listener will still exist inside gSKI and will crash when an agent event is next generated.
	this->GetOutputListener()->UnRegisterForKernelSMLEvents() ;

	this->GetInputListener()->UnRegisterForKernelSMLEvents() ;

	// Unregister ourselves (this is important for the same reasons as listed above)
	//m_pKernelSML->GetKernel()->GetAgentManager()->RemoveAgentListener(gSKIEVENT_BEFORE_AGENT_DESTROYED, this) ;

	// Then delete our matching agent sml information
	m_pKernelSML->DeleteAgentSML( this->GetName() ) ;

	// Do self clean-up of this object as it's just called
	// prior to deleting the AgentSML structure.
	delete this ;
}

//void AgentSML::RegisterForBeforeAgentDestroyedEvent()
//{
//	// We should do this immediately before we delete the agent.
//	// We shouldn't do it earlier or we can't be sure it'll be last on the list of listeners which is where we
//	// need it to be (so that we clear our information about the gSKI agent *after* we've notified any of our listeners
//	// about this event).
//	m_pBeforeDestroyedListener = new AgentBeforeDestroyedListener() ;
//	m_pKernelSML->GetKernel()->GetAgentManager()->AddAgentListener(gSKIEVENT_BEFORE_AGENT_DESTROYED, m_pBeforeDestroyedListener) ;
//}
//
void AgentSML::ScheduleAgentToRun(bool state) 
{ 
	if (this->GetRunState() != sml_RUNSTATE_HALTED) 
	{
		m_ScheduledToRun = state ; 
		m_WasOnRunList = state; 
	}
}

/*************************************************************
* @brief	Converts an id from a client side value to a kernel side value.
*			We need to be able to do this because the client is adding a collection
*			of wmes at once, so it makes up the ids for those objects.
*			But the kernel will assign them a different value when the
*			wme is actually added in the kernel.
*************************************************************/
bool AgentSML::ConvertID(char const* pClientID, std::string* pKernelID)
{
	if (pClientID == NULL)
		return false ;

	IdentifierMapIter iter = m_IdentifierMap.find(pClientID) ;

	if (iter == m_IdentifierMap.end())
	{
		// If the client id is not in the map, then we may have been
		// passed a kernel id (this will happen at times).
		// So return the value we were passed
		*pKernelID = pClientID ;
		return false ;
	}
	else
	{
		// If we found a mapping, return the mapped value
		*pKernelID = iter->second ;
		return true ;
	}
}

void AgentSML::RecordIDMapping(char const* pClientID, char const* pKernelID)
{
	// Do we already have a mapping?
	IdentifierMapIter iter = m_IdentifierMap.find(pClientID);

	if (iter == m_IdentifierMap.end())
	{
		// We don't, create a mapping, this indicates a reference count of 1
		m_IdentifierMap[pClientID] = pKernelID ;

		// Record in both directions, so we can clean up (at which time we only know the kernel side ID).
		m_ToClientIdentifierMap[pKernelID] = pClientID ;

		// Note that we leave the entry out of m_IdentifierRefMap, we only use
		// that for counts of two or greater
	}
	else
	{
		// The mapping already exists, so we need to check and see if we have a reference
		// count for it yet
		IdentifierRefMapIter iter = m_IdentifierRefMap.find(pClientID);
		if (iter == m_IdentifierRefMap.end())
		{
			// there is no reference count and this is the second reference, so set it to two
			m_IdentifierRefMap[pClientID] = 2 ;
		}
		else 
		{
			// there is a reference count, increment it
			iter->second += 1;
		}
	}
}

void AgentSML::RemoveID(char const* pKernelID)
{
	// first, find the identifer
	IdentifierMapIter iter = m_ToClientIdentifierMap.find(pKernelID) ;

	// This identifier should have been in the table
	// Note: sometimes this is called by gSKI when it is removing wmes. gSKI doesn't know if
	// we're a direct connection and therefore aren't using this map, so in that case we need to not
	// update this. Therefore, we can't assert here.
	//assert (iter != m_ToClientIdentifierMap.end()) ;
	if (iter == m_ToClientIdentifierMap.end())
		return ;

	// cache the identifer value
	std::string& clientID = iter->second ;

	// decrement the reference count and remove the identifier from the maps if it is there
	IdentifierRefMapIter refIter = m_IdentifierRefMap.find(clientID);
	if (refIter == m_IdentifierRefMap.end())
	{
		// when we have an entry in the m_IdentifierMap but not m_IdentifierRefMap, this 
		// means our ref count is one, so we're decrementing to zero, so we remove it
		m_IdentifierMap.erase(clientID) ;
		m_ToClientIdentifierMap.erase(pKernelID) ;
		return;
	}
	else 
	{
		// if we have an entry, decrement it
		refIter->second -= 1;

		// if the count falls to 1, remove it from this map since presence in the map requires 
		// at least a ref count of two
		if (refIter->second < 2) {
			m_IdentifierRefMap.erase(refIter);
		}
	}
}

unsigned long AgentSML::ConvertTime(long clientTimeTag)
{
   CKTimeMapIter iter = m_CKTimeMap.find(clientTimeTag) ;

	if (iter == m_CKTimeMap.end())
	{
		return 0 ;
	}
	else
	{
		// If we found a mapping, return the mapped value
		return iter->second ;
	}
}

unsigned long AgentSML::ConvertTime(char const* pTimeTag)
{
	if (pTimeTag == NULL)
		return 0 ;

   return ConvertTime(atoi(pTimeTag));
}

void AgentSML::RecordTime(long clientTimeTag, long kernelTimeTag)
{
	assert(m_CKTimeMap.find(clientTimeTag) == m_CKTimeMap.end());
	m_CKTimeMap[clientTimeTag] = kernelTimeTag ;
	assert(m_KCTimeMap.find(kernelTimeTag) == m_KCTimeMap.end());
	m_KCTimeMap[kernelTimeTag] = clientTimeTag ;
}

void AgentSML::RemoveKernelTime( unsigned long kernelTimeTag )
{
	KCTimeMapIter kcIter = m_KCTimeMap.find( kernelTimeTag );
	if ( kcIter == m_KCTimeMap.end() )
	{
		// Can't assert false here because this gets called for architecturally
		// created wmes such as (S1 ^io I1), etc.
		//assert( false );
		return;
	}

	m_CKTimeMap.erase( kcIter->second );
	m_KCTimeMap.erase( kcIter );
}

long AgentSML::GetClientTimetag( unsigned long kernelTimeTag )
{
	KCTimeMapIter kcIter = m_KCTimeMap.find( kernelTimeTag );
	if ( kcIter == m_KCTimeMap.end() )
	{
		return 0;
	}
	return kcIter->second;
}

void AgentSML::RegisterRHSFunction(RhsFunction* rhsFunction)
{
	// Tell Soar about it
	add_rhs_function (m_agent, 
					make_sym_constant(m_agent, rhsFunction->GetName()),
					RhsFunction::RhsFunctionCallback,
					rhsFunction->GetNumExpectedParameters(),
					rhsFunction->IsValueReturned(),
					true,
					static_cast<void*>(rhsFunction));
}

void AgentSML::RemoveRHSFunction(RhsFunction* rhsFunction)
{
	if (rhsFunction == NULL)
		return ;

	char const* szName = rhsFunction->GetName() ;

	// Tell the kernel we are done listening.
	//RPM 9/06: removed symbol ref so symbol is released properly
	Symbol* tmp = make_sym_constant(m_agent, szName);
	remove_rhs_function(m_agent, tmp);
	symbol_remove_ref (m_agent, tmp);
}

char const* AgentSML::GetValueType(int type)
{
	switch (type)
	{
	case VARIABLE_SYMBOL_TYPE: return sml_Names::kTypeVariable ;
	case FLOAT_CONSTANT_SYMBOL_TYPE: return sml_Names::kTypeDouble ;
	case INT_CONSTANT_SYMBOL_TYPE:	  return sml_Names::kTypeInt ;
	case SYM_CONSTANT_SYMBOL_TYPE: return sml_Names::kTypeString ;
	case IDENTIFIER_SYMBOL_TYPE: return sml_Names::kTypeID ;
	default: return NULL ;
	}
}

std::string AgentSML::ExecuteCommandLine(std::string const& commandLine)
{
	KernelSML* pKernel = m_pKernelSML ;

	// We'll pretend this came from the local (embedded) connection.
	Connection* pConnection = pKernel->GetEmbeddedConnection() ;

	// Build up a message to execute the command line
	bool rawOutput = true ;
	soarxml::ElementXML* pMsg = pConnection->CreateSMLCommand(sml_Names::kCommand_CommandLine, rawOutput) ;
	pConnection->AddParameterToSMLCommand(pMsg, sml_Names::kParamAgent, this->GetName());
	pConnection->AddParameterToSMLCommand(pMsg, sml_Names::kParamLine, commandLine.c_str()) ;

	AnalyzeXML incoming ;
	incoming.Analyze(pMsg) ;

	// Create a response object which the command line can fill in
	soarxml::ElementXML* pResponse = pConnection->CreateSMLResponse(pMsg) ;

	// Execute the command line
	bool ok = pKernel->ProcessCommand(sml_Names::kCommand_CommandLine, pConnection, &incoming, pResponse) ;

	std::string result ;

	if (ok)
	{
		// Take the result from executing the command line and fill it in to our "pReturnValue" array.
		AnalyzeXML response ;
		response.Analyze(pResponse) ;

		char const* pRes = response.GetResultString() ;

		if (pRes)
			result = pRes ;
	}
	else
	{
		result = std::string("Error executing command ") + commandLine ;
	}

	// Clean up
	delete pMsg ;
	delete pResponse ;

	return result ;
}

void AgentSML::InputPhaseCallback( agent* /*agent*/,
								   soar_callback_event_id /*eventid*/,
								   soar_callback_data /*callbackdata*/,
								   soar_call_data /*calldata*/ )
{
	KernelSML::GetKernelSML()->ReceiveAllMessages();	
}

bool AgentSML::AddInputWME(char const* pID, char const* pAttribute, Symbol* pValueSymbol, long clientTimeTag)
{
	std::string idKernel ;
	ConvertID(pID, &idKernel) ;

	char idLetter = idKernel[0] ;
	int idNumber = atoi( &(idKernel.c_str()[1]) ) ;

	// Now create the wme
	Symbol* pIDSymbol   = get_io_identifier( m_agent, idLetter, idNumber) ;
	Symbol* pAttrSymbol = get_io_sym_constant( m_agent, pAttribute ) ;

	CHECK_RET_FALSE( pIDSymbol ) ;
	CHECK_RET_FALSE( pAttrSymbol ) ;

	wme* pNewInputWme = add_input_wme( m_agent, pIDSymbol, pAttrSymbol, pValueSymbol ) ;

	CHECK_RET_FALSE( pNewInputWme ) ;

	AddWmeToWmeMap( clientTimeTag, pNewInputWme ) ;

	//long timeTag = pNewInputWme->timetag ;

	//if (kDebugInput)
	//	KernelSML::PrintDebugWme( "Adding wme ", pNewInputWme, true ) ;

	// we just created these so lets release our ownership of them (they belong to the wme now)
	/*unsigned long refCount1 = */release_io_symbol( m_agent, pNewInputWme->id ) ;
	/*unsigned long refCount2 = */release_io_symbol( m_agent, pNewInputWme->attr ) ;
	/*unsigned long refCount3 = */release_io_symbol( m_agent, pNewInputWme->value ) ;

	//if (kDebugInput)
	//	KernelSML::PrintDebugWme("Adding wme ", pNewInputWme, true) ;

	return true ;
}

bool AgentSML::AddStringInputWME(char const* pID, char const* pAttribute, char const* pValue, long clientTimeTag)
{
	// Creating a wme with a string constant value
	Symbol* pValueSymbol = get_io_sym_constant(m_agent, pValue) ;

	if (CaptureQuery())
	{
		// capture input enabled
		CapturedAction ca;
		ca.dc = m_agent->d_cycle_count;
		ca.timetag = clientTimeTag;
		ca.CreateAdd();
		ca.Add()->id = pID;
		ca.Add()->attr = pAttribute;
		ca.Add()->value = pValue;
		ca.Add()->type = sml_Names::kTypeString;
		CaptureInputWME(ca);
	}

	return AddInputWME(pID, pAttribute, pValueSymbol, clientTimeTag);
}

bool AgentSML::AddIntInputWME(char const* pID, char const* pAttribute, int value, long clientTimeTag)
{
	// Creating a wme with an int constant value
	Symbol* pValueSymbol = get_io_int_constant(m_agent, value) ;

	if (CaptureQuery())
	{
		// capture input enabled
		CapturedAction ca;
		ca.dc = m_agent->d_cycle_count;
		ca.timetag = clientTimeTag;
		ca.CreateAdd();
		ca.Add()->id = pID;
		ca.Add()->attr = pAttribute;
		std::stringstream valueString;
		valueString << value;
		ca.Add()->value = valueString.str();
		ca.Add()->type = sml_Names::kTypeInt;
		CaptureInputWME(ca);
	}

	return AddInputWME(pID, pAttribute, pValueSymbol, clientTimeTag);
}

bool AgentSML::AddDoubleInputWME(char const* pID, char const* pAttribute, double value, long clientTimeTag)
{
	// Creating a wme with an int constant value
	Symbol* pValueSymbol = get_io_float_constant(m_agent, static_cast< float >( value ) ) ; // regarding warning on this line: nate possibly changed this on the RL branch (double -> float conversion)

	if (CaptureQuery())
	{
		// capture input enabled
		CapturedAction ca;
		ca.dc = m_agent->d_cycle_count;
		ca.timetag = clientTimeTag;
		ca.CreateAdd();
		ca.Add()->id = pID;
		ca.Add()->attr = pAttribute;
		std::stringstream valueString;
		valueString << value;
		ca.Add()->value = valueString.str();
		ca.Add()->type = sml_Names::kTypeDouble;
		CaptureInputWME(ca);
	}

	return AddInputWME(pID, pAttribute, pValueSymbol, clientTimeTag);
}

bool AgentSML::AddIdInputWME(char const* pID, char const* pAttribute, char const* pValue, long clientTimeTag)
{
	Symbol* pValueSymbol = 0 ;

	// We will always receive a client-side identifier
	// If that identifier is found when we try to convert it, it already exists in the kernel, we make a shared id.
	// If that identifier is not found when we try to convert it, we make a new identifier.

	std::string idValue ;
	int idValueNumber = 0 ;
	char idValueLetter = 0;
	bool didntFindId = true;

	if ( ConvertID( pValue, &idValue ) )
	{
		// we found a kernel side mapping, shared id
		didntFindId = false;	// for sanity check below
		idValueLetter = idValue[0];
		idValueNumber = atoi( &(idValue.c_str()[1]) ) ;
	}
	else 
	{
		// no kernel side mapping, new id

		// new id based on first character of attribute
		if ( isalpha( pAttribute[0] ) )
		{
			idValueLetter = pAttribute[0];				// take the first letter of the attribute
			idValueLetter = static_cast<char>(toupper( idValueLetter ));	// make it upper case
		} 
		else 
		{
			idValueLetter = 'I';	// attribute is not alpha, use default 'I'
		}
		// Number is ignored in new ID case
	}
	// Find/create the identifier
	pValueSymbol = get_io_identifier( m_agent, idValueLetter, idValueNumber ) ;	
	
	// If pValueSymbol is a new id, then RecordIDMapping will create a map between the client and kernel id names. 
	// Otherwise, RecordIDMapping will add a ref count to client id name.
	std::ostringstream buffer;
	buffer << pValueSymbol->id.name_letter ;
	buffer << pValueSymbol->id.name_number ;
	this->RecordIDMapping(pValue, buffer.str().c_str()) ;
	//if (kDebugInput)
	//{
	//	PrintDebugFormat("Recording id mapping of %s to %s", pValue, newid.c_str()) ;
	//}	

	if (CaptureQuery())
	{
		// capture input enabled
		CapturedAction ca;
		ca.dc = m_agent->d_cycle_count;
		ca.timetag = clientTimeTag;
		ca.CreateAdd();
		ca.Add()->id = pID;
		ca.Add()->attr = pAttribute;
		ca.Add()->value = pValue;
		ca.Add()->type = sml_Names::kTypeID;
		CaptureInputWME(ca);
	}

	return AddInputWME(pID, pAttribute, pValueSymbol, clientTimeTag);
}

bool AgentSML::AddInputWME(char const* pID, char const* pAttribute, char const* pValue, char const* pType, char const* pClientTimeTag)
{
	// TODO: 
	// If input performance continues to be an issue, maybe some of these checks are redundant and can be removed.

	// Begin sanity check
	// This function requires client side identifiers and timetags
	CHECK_RET_FALSE( pID );		// must have id
	// FIXME: enable, I2 seems to be a special case
	//CHECK_RET_FALSE( islower( pID[0] ) );	// id must be lower case

	CHECK_RET_FALSE( pAttribute );		// must have attribute

	CHECK_RET_FALSE( pValue );
	if ( pType == sml_Names::kTypeID ) // if we're making a shared identifier
	{
		// the shared ID must be client side
		CHECK_RET_FALSE( isalpha( pValue[0] ) );	// id must be alpha
		CHECK_RET_FALSE( islower( pValue[0] ) );	// id must be lower case
	}

	// must have client side timetag
	CHECK_RET_FALSE( pClientTimeTag );
	CHECK_RET_FALSE( atoi( pClientTimeTag ) < 0 ) ;
	// End sanity check

	// Convert ID to kernel side.
	CHECK_RET_FALSE(strlen(pID) >= 2) ;

	long clientTimeTag = atoi(pClientTimeTag);
	if (IsStringEqual(sml_Names::kTypeString, pType)) 
	{
		// Creating a wme with a string constant value
		return AddStringInputWME(pID, pAttribute, pValue, clientTimeTag);

	} 
	else if (IsStringEqual(sml_Names::kTypeInt, pType)) 
	{
		// Creating a WME with an int value
		int value = atoi(pValue) ;
		return AddIntInputWME(pID, pAttribute, value, clientTimeTag);

	} 
	else if (IsStringEqual(sml_Names::kTypeDouble, pType)) 
	{
		// Creating a WME with a float value
		return AddDoubleInputWME(pID, pAttribute, atof(pValue), clientTimeTag);

	} 
	else if (IsStringEqual(sml_Names::kTypeID, pType)) 
	{
		return AddIdInputWME(pID, pAttribute, pValue, clientTimeTag);
	}
	else
	{
		assert( false ); // bad type
		return false;
	}
}

bool AgentSML::RemoveInputWME(long clientTimeTag)
{
	// Get the wme that matches this time tag
	long kernelTimeTag = this->ConvertTime(clientTimeTag) ;

	//if (kDebugInput)
	//{
	//	PrintDebugFormat("Before removing wme") ;
	//	this->PrintKernelTimeTags() ;
	//}

	wme *pWME = 0;

	pWME = FindWmeFromKernelTimetag( kernelTimeTag ) ;

	//if (kDebugInput)
	//{
	//    std::string printInput1 = this->ExecuteCommandLine("print --internal --depth 2 I2") ;
	//	PrintDebugFormat("%s\nLooking for %ld", printInput1.c_str(), timetag) ;
	//}

	// The wme is already gone so no work to do
	if (!pWME)
	{
		return false ;
	}

	//if (kDebugInput)
	//	KernelSML::PrintDebugWme("Removing input wme ", pWME, true) ;

	CHECK_RET_FALSE(pWME) ;  //BADBAD: above check means this will never be triggered; one of the checks should go, but not sure which (can this function be legitimately called with a timetag for a wme that's already been removed?)

	if ( pWME->value->common.symbol_type == IDENTIFIER_SYMBOL_TYPE ) {
		this->RemoveID( symbol_to_string( GetSoarAgent(), pWME->value, true, 0, 0 ) ) ;
	}

	RemoveWmeFromWmeMap( pWME );
	Bool ok = remove_input_wme(m_agent, pWME) ;

	CHECK_RET_FALSE(ok) ;

	if (CaptureQuery())
	{
		// capture input enabled
		CapturedAction ca;
		ca.dc = m_agent->d_cycle_count;
		ca.timetag = clientTimeTag;
		CaptureInputWME(ca);
	}

	return (ok != 0) ;  // BADBAD: redundant with previous line?
}

bool AgentSML::RemoveInputWME(char const* pTimeTag)
{
   return RemoveInputWME(atoi(pTimeTag));
}

void AgentSML::AddWmeToWmeMap( long clientTimeTag, wme* w ) 
{
	unsigned long timetag = w->timetag ;
	m_KernelTimeTagToWmeMap[timetag] = w ;

	// Keep track of which client timetags correspond to which kernel timetags
	this->RecordTime( clientTimeTag, timetag ) ;
}

void AgentSML::RemoveWmeFromWmeMap( wme* w ) 
{
	unsigned long timetag = w->timetag ;
	m_KernelTimeTagToWmeMap.erase(timetag) ;

	// Keep track of which client timetags correspond to which kernel timetags
	this->RemoveKernelTime( timetag ) ;
}

wme* AgentSML::FindWmeFromKernelTimetag( unsigned long timetag ) 
{
   WmeMapIter wi = m_KernelTimeTagToWmeMap.find( timetag );
   if( wi == m_KernelTimeTagToWmeMap.end() )
   {
      return NULL;
   }
   return wi->second;
}

void AgentSML::InputWmeGarbageCollectedHandler( agent* /*pSoarAgent*/, int eventID, void* pData, void* pCallData )
{
	(void)eventID; // silences warning in release mode
	assert( eventID == static_cast< int >( INPUT_WME_GARBAGE_COLLECTED_CALLBACK ) );

	wme* pWME = reinterpret_cast< wme* >( pCallData );
	AgentSML* pAgent = reinterpret_cast< AgentSML* >( pData );

	pAgent->RemoveWmeFromWmeMap( pWME );
}

bool AgentSML::StartCaptureInput(const std::string& pathname, bool autoflush, unsigned long seed)
{
	if (CaptureQuery()) return false;
	if (ReplayQuery()) return false;

	m_CaptureAutoflush = autoflush;

	m_pCaptureFile = new std::fstream( pathname.c_str(), std::fstream::out | std::fstream::trunc );
	if (m_pCaptureFile && m_pCaptureFile->good())
	{
		SoarSeedRNG(seed);
		*m_pCaptureFile << seed << std::endl;
		return true;
	}

	delete m_pCaptureFile;
	m_pCaptureFile = 0;
	return false;
}

bool AgentSML::StopCaptureInput()
{
	if (!CaptureQuery()) return false;
	if (ReplayQuery()) return false;

	bool good = true;
	if (!m_CaptureAutoflush)
	{
		m_CaptureAutoflush = true;
		while (good && !m_CapturedActions.empty())
		{
			CaptureInputWME(m_CapturedActions.front());
			m_CapturedActions.pop();
		}
	}

	delete m_pCaptureFile;
	m_pCaptureFile = 0;
	return good;
}

bool AgentSML::StartReplayInput(const std::string& pathname)
{
	if (ReplayQuery()) return false;
	if (CaptureQuery()) return false;

	std::fstream replayFile(pathname.c_str(), std::fstream::in);
	if (replayFile.bad())
	{
		return false;
	} 

	unsigned long seed = 0;
	replayFile >> seed;
	if (replayFile.bad())
	{
		return false;
	}
	SoarSeedRNG(seed);

	// load replay file
	while (replayFile.good())
	{
		CapturedAction ca;
		std::string actionType;
		
		replayFile >> ca.dc >> ca.timetag >> actionType;
		if (replayFile.bad()) return false;
		
		if (actionType == "add-wme")
		{
			ca.CreateAdd();
			std::string type;
			replayFile >> ca.Add()->id >> ca.Add()->attr >> ca.Add()->value >> type;
	
			if (type == sml_Names::kTypeID)
			{
				ca.Add()->type = sml_Names::kTypeID;
			}
			else if (type == sml_Names::kTypeInt)
			{
				ca.Add()->type = sml_Names::kTypeInt;
			}
			else if (type == sml_Names::kTypeDouble)
			{
				ca.Add()->type = sml_Names::kTypeDouble;
			}
			else if (type == sml_Names::kTypeString)
			{
				ca.Add()->type = sml_Names::kTypeString;
			}
			else
			{
				assert(false);
				return false;
			}

			if (replayFile.bad()) {
				return false;
			}
		}
		m_CapturedActions.push(ca);
	}

	m_ReplayInput = true;
	replayFile.close();
	return true;
}

bool AgentSML::StopReplayInput()
{
	if (!ReplayQuery()) return false;
	if (CaptureQuery()) return false;

	while (!m_CapturedActions.empty())
	{
		m_CapturedActions.pop();
	}
	return true;
}

const std::string AgentSML::CAPTURE_SEPERATOR = " ";

bool AgentSML::CaptureInputWME(const CapturedAction& ca) 
{
	if (!m_CaptureAutoflush)
	{
		m_CapturedActions.push(ca);
		return true;
	}

	if (!m_pCaptureFile) return false;
	if (m_pCaptureFile->bad()) return false;

	*m_pCaptureFile << ca.dc << CAPTURE_SEPERATOR << ca.timetag << CAPTURE_SEPERATOR;
	if (ca.Add())
	{
		*m_pCaptureFile << "add-wme" << CAPTURE_SEPERATOR << ca.Add()->id << CAPTURE_SEPERATOR 
			<< ca.Add()->attr << CAPTURE_SEPERATOR << ca.Add()->value << CAPTURE_SEPERATOR << ca.Add()->type << std::endl;
	}
	else
	{
		*m_pCaptureFile << "remove-wme" << std::endl;
	}
	return m_pCaptureFile->good();
}

void AgentSML::ReplayInputWMEs()
{
	if (m_CapturedActions.empty())
	{
		print(m_agent, "\n\nWarning: end of replay has been reached.\n");
		return;
	}

	while (!m_CapturedActions.empty())
	{
		CapturedAction ca = m_CapturedActions.front();
		assert (ca.dc >= m_agent->d_cycle_count);

		if (ca.dc != m_agent->d_cycle_count)
		{
			break;
		}

		m_CapturedActions.pop();

		if (ca.Add())
		{
			// add-wme
			char timetagString[25];
			SNPRINTF(timetagString, 25, "%ld", ca.timetag);

			if (!AddInputWME(ca.Add()->id.c_str(), ca.Add()->attr.c_str(), ca.Add()->value.c_str(), ca.Add()->type, timetagString))
			{
				print(m_agent, "\n\nWarning: replay add-wme failed.\n");
			}
		} 
		else
		{
			// remove-wme
			if (!RemoveInputWME(ca.timetag))
			{
				print(m_agent, "\n\nWarning: replay remove-wme failed.\n");
			}
		}
	}
}
