/////////////////////////////////////////////////////////////////
// save-backtraces command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "sml_Utils.h"
#include "sml_AgentSML.h"
#include "cli_CommandLineInterface.h"

#include "sml_Names.h"

#include "sml_KernelSML.h"
#include "gsysparam.h"
#include "agent.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::DoSaveBacktraces(bool* pSetting) {
    agent* agnt = m_pAgentSML->GetSoarAgent();
    if (!pSetting) {
        if (m_RawOutput) {
            m_Result << "Save bactraces is " << (agnt->sysparams[EXPLAIN_SYSPARAM] ? "enabled." : "disabled.");
        } else {
            AppendArgTagFast(sml_Names::kParamValue, sml_Names::kTypeBoolean, agnt->sysparams[EXPLAIN_SYSPARAM] ? sml_Names::kTrue : sml_Names::kFalse);
        }
        return true;
    }

    set_sysparam(agnt, EXPLAIN_SYSPARAM, *pSetting);
    return true;
}

