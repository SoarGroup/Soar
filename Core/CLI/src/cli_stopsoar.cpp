/////////////////////////////////////////////////////////////////
// stop-soar command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "sml_Utils.h"
#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"

#include "sml_KernelSML.h"
#include "sml_AgentSML.h"
#include "sml_Events.h"

using namespace cli;

bool CommandLineInterface::DoStopSoar(bool self, const std::string* /*reasonForStopping*/) {

    if (self) {
        m_pAgentSML->Interrupt(sml::sml_STOP_AFTER_DECISION_CYCLE);
    } else {
        // Make sure the system stop event will be fired at the end of the run.
        // We used to call FireSystemStop() in this function, but that's no good because
        // it comes before the agent has stopped because interrupt only stops at the next
        // phase or similar boundary (so could be a long time off).
        // So instead we set a flag and allow system stop to fire at the end of the run.
        m_pKernelSML->RequireSystemStop(true) ;
        m_pKernelSML->InterruptAllAgents(sml::sml_STOP_AFTER_DECISION_CYCLE);
    }
    return true;
}

