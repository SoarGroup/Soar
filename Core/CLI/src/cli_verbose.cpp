/////////////////////////////////////////////////////////////////
// verbose command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "sml_Utils.h"
#include "sml_AgentSML.h"
#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"

#include "sml_Names.h"

#include "sml_KernelSML.h"
#include "agent.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::DoVerbose(bool* pSetting) {
    agent* agnt = m_pAgentSML->GetSoarAgent();
    if (!pSetting) {
        if (m_RawOutput) {
            m_Result << "Verbose is " << (agnt->soar_verbose_flag ? "on." : "off.");
        } else {
            AppendArgTagFast(sml_Names::kParamValue, sml_Names::kTypeBoolean, agnt->soar_verbose_flag ? sml_Names::kTrue : sml_Names::kFalse);
        }
        return true;
    }

    agnt->soar_verbose_flag = *pSetting;
    return true;
}

