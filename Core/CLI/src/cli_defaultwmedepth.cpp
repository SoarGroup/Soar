/////////////////////////////////////////////////////////////////
// default-wme-depth command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"
#include "sml_Names.h"
#include "agent.h"
#include "sml_AgentSML.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::DoDefaultWMEDepth(const int* pDepth) {
    agent* agnt = m_pAgentSML->GetSoarAgent();
    if (!pDepth) {
        if (m_RawOutput) {
            m_Result << agnt->default_wme_depth;
        } else {
            std::string temp;
            AppendArgTagFast(sml_Names::kParamValue, sml_Names::kTypeInt, to_string(agnt->default_wme_depth, temp));
        }
        return true;
    }

    agnt->default_wme_depth = *pDepth;
    return true;
}

