/////////////////////////////////////////////////////////////////
// run command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "sml_Utils.h"
#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"
#include "sml_Names.h"
#include "sml_KernelSML.h"
#include "sml_AgentSML.h"
#include "sml_Events.h"
#include "sml_RunScheduler.h"

#include "misc.h"
#include "agent.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::DoRun(const RunBitset& options, int count, eRunInterleaveMode interleaveIn) {
    // Default run type is sml_DECISION
    smlRunStepSize runType = sml_DECISION;
    //// ... unless there is a count, then the default is a decision cycle:
    //if (count >= 0) runType = sml_DECISION;

    bool forever = false;

    // Override run type with option flag:
    if (options.test(RUN_ELABORATION)) {
        runType = sml_ELABORATION;

    } else if (options.test(RUN_PHASE)) {
        runType = sml_PHASE;

    } else if (options.test(RUN_DECISION)) {
        runType = sml_DECISION;

    } else if (options.test(RUN_OUTPUT)) {
        runType = sml_UNTIL_OUTPUT;
    } else {
        // if there is no step size given and no count, we're going forever
        forever = (count < 0);
    }

    if (count == -1)
    {
        count = 1 ;
    }

    smlRunResult runResult ;

    // NOTE: We use a scheduler implemented in kernelSML rather than
    // the gSKI scheduler implemented by AgentManager.  This gives us
    // more flexibility to adjust the behavior of the SML scheduler without
    // impacting SoarTech systems that may rely on the gSKI scheduler.
    RunScheduler* pScheduler = m_pKernelSML->GetRunScheduler() ;
    smlRunFlags runFlags = sml_NONE ;

    if (options.test(RUN_UPDATE))
        runFlags = sml_UPDATE_WORLD ;
    else if (options.test(RUN_NO_UPDATE))
        runFlags = sml_DONT_UPDATE_WORLD ;

    if (options.test(RUN_SELF))
    {
        runFlags = smlRunFlags(runFlags | sml_RUN_SELF) ;

        // Schedule just this one agent to run
        pScheduler->ScheduleAllAgentsToRun(false) ;
        pScheduler->ScheduleAgentToRun(m_pAgentSML, true) ;
    }
    else
    {
        runFlags = smlRunFlags(runFlags | sml_RUN_ALL) ;

        // Ask all agents to run
        pScheduler->ScheduleAllAgentsToRun(true) ;
    }

    smlRunStepSize interleave;

    switch(interleaveIn) {
        case RUN_INTERLEAVE_DEFAULT:
        default:
            interleave  = pScheduler->DefaultInterleaveStepSize(forever, runType) ;
            break;
        case RUN_INTERLEAVE_ELABORATION:
            interleave = sml_ELABORATION;
            break;
        case RUN_INTERLEAVE_DECISION:
            interleave = sml_DECISION;
            break;
        case RUN_INTERLEAVE_PHASE:
            interleave = sml_PHASE;
            break;
        case RUN_INTERLEAVE_OUTPUT:
            interleave = sml_UNTIL_OUTPUT;
            break;
    }

    if (!pScheduler->VerifyStepSizeForRunType(forever, runType, interleave) ) {
        return SetError("Run type and interleave setting incompatible.");
    }

    // If we're running by decision cycle synchronize up the agents to the same phase before we start
    bool synchronizeAtStart = (runType == sml_DECISION) ; 

    SetTrapPrintCallbacks( false );

    agent* agnt = m_pAgentSML->GetSoarAgent();
    if (options.test(RUN_GOAL))
    {
        agnt->substate_break_level = agnt->bottom_goal->id.level;
    }

    // Do the run
    runResult = pScheduler->RunScheduledAgents(forever, runType, count, runFlags, interleave, synchronizeAtStart) ;

    // Reset goal retraction stop flag after any run
    agnt->substate_break_level = 0;

    SetTrapPrintCallbacks( true );

    switch (runResult) {
        case sml_RUN_ERROR_ALREADY_RUNNING:
            return SetError("Soar is already running");

        case sml_RUN_ERROR:
            return SetError("Run failed.");

        case sml_RUN_EXECUTING:
            if (m_RawOutput) {
                // NOTE: I don't think this is currently possible
                m_Result << "\nRun stopped (still executing).";
            } else {
                std::string temp;
                AppendArgTagFast(sml_Names::kParamRunResult, sml_Names::kTypeInt, to_string( runResult, temp ) );
            }
            break;

        case sml_RUN_COMPLETED_AND_INTERRUPTED:                    // an interrupt was requested, but the run completed first
            // falls through
        case sml_RUN_INTERRUPTED:
            if (m_RawOutput) {
                m_Result << "\nRun stopped (interrupted).";
            } else {
                std::string temp;
                AppendArgTagFast(sml_Names::kParamRunResult, sml_Names::kTypeInt, to_string( runResult, temp ));
            }
            if (pScheduler->AnAgentHaltedDuringRun())
            {
                if (m_RawOutput) {
                    m_Result << "\nAn agent halted during the run.";
                } else {                    
                    AppendArgTagFast(sml_Names::kParamMessage, sml_Names::kTypeString, "\nAn agent halted during the run.");        
                }
            }
            break;

        case sml_RUN_COMPLETED:
            // Do not print anything
            // might be helpful if we checked agents to see if any halted...
            // retval is sml_RUN_COMPLETED, but agent m_RunState == gSKI_RUNSTATE_HALTED
            // should only check the agents m_pAgentSML->WasOnRunList()
            if (pScheduler->AnAgentHaltedDuringRun())
            {
                if (m_RawOutput) {
                    m_Result << "\nAn agent halted during the run.";
                } else {                    
                    AppendArgTagFast(sml_Names::kParamMessage, sml_Names::kTypeString, "\nAn agent halted during the run.");        
                }
            }
            break;

        default:
            assert(false);
            return SetError("Run failed.");
    }
    return true;
}
