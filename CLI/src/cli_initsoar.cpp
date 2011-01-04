/////////////////////////////////////////////////////////////////
// init-soar command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "sml_Utils.h"
#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"

#include "sml_AgentSML.h"
#include "xml.h"

using namespace cli;

bool CommandLineInterface::DoInitSoar() {
    // Save the current result
    std::string oldResult = m_Result.str();

    SetTrapPrintCallbacks( false );

    bool ok = m_pAgentSML->Reinitialize() ;

    // S1 gets created during Reinitialize, clear its output from the trace buffers
    xml_invoke_callback( m_pAgentSML->GetSoarAgent() );
    m_pAgentSML->FlushPrintOutput();

    SetTrapPrintCallbacks( true );

    // restore the old result, ignoring output from init-soar
    m_Result.str(oldResult); 

    if (!ok)
        return SetError("Agent failed to reinitialize");

    if (m_RawOutput) m_Result << "Agent reinitialized.";

    return ok;
}

