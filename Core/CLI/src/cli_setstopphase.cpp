/////////////////////////////////////////////////////////////////
// set stop phase command file.
//
// Author: Douglas Pearson, doug@threepenny.net
// Date  : 2005
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "sml_Utils.h"
#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"

#include "sml_Names.h"
#include "sml_AgentSML.h"
#include "sml_KernelSML.h"
#include "sml_Events.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::DoSetStopPhase(bool setPhase, bool before, smlPhase phase) {

    // We only set the phase if asked, but we always report the current setting.
    if (setPhase)
    {
        // The kernel only accepts stop before a phase logic.
        // The "stop after" form is just a courtesy to the user in case they prefer to think that way.
        if (!before)
        {
            phase = smlPhase(phase + 1);
            if (phase > sml_OUTPUT_PHASE)
                phase = sml_INPUT_PHASE ;
        }

        m_pKernelSML->SetStopBefore(phase) ;
    }

    std::string phaseStr ;
    smlPhase stopPhase = m_pKernelSML->GetStopBefore() ;

    if (!before)
    {
        stopPhase = smlPhase(stopPhase - 1);
        if (stopPhase < sml_INPUT_PHASE)
            stopPhase = sml_OUTPUT_PHASE ;
    }

    switch (stopPhase)
    {
    case sml_INPUT_PHASE:    phaseStr = "input phase" ; break ;
    case sml_PROPOSAL_PHASE: phaseStr = "proposal phase" ; break ;
    case sml_DECISION_PHASE: phaseStr = "decision phase" ; break ;
    case sml_APPLY_PHASE:    phaseStr = "apply phase" ; break ;
    case sml_OUTPUT_PHASE:   phaseStr = "output phase" ; break ;
    default:                  phaseStr = "unknown phase" ; break ;
    }

    if (m_RawOutput) {
        m_Result << (before ? "Stop before " : "Stop after ") << phaseStr;
    } else {
        std::ostringstream buffer;
        buffer << stopPhase;
        std::string bufferString = buffer.str() ;
        AppendArgTagFast(sml_Names::kParamPhase, sml_Names::kTypeString, bufferString);
    }

    return true;
}

