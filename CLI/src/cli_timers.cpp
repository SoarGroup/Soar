/////////////////////////////////////////////////////////////////
// timers command file.
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
#include "gsysparam.h"
#include "agent.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::DoTimers(bool* pSetting) {
    agent* agnt = m_pAgentSML->GetSoarAgent();
	if (pSetting) {
		// set, don't print
		set_sysparam(agnt, TIMERS_ENABLED, *pSetting);

	} else {
		// print current setting
		if (m_RawOutput) {
#ifdef NO_TIMING_STUFF
            m_Result << "Timers are disabled (compiled out).";
#else // NO_TIMING_STUFF
#ifdef USE_PERFORMANCE_FOR_BOTH
			m_Result << "High-resolution timers are ";
#else // USE_PERFORMANCE_FOR_BOTH
			m_Result << "Timers are ";
#endif // USE_PERFORMANCE_FOR_BOTH
			m_Result << (agnt->sysparams[TIMERS_ENABLED] ? "enabled" : "disabled");
#ifdef DETAILED_TIMING_STATS
			m_Result << ", detailed stats are on";
#endif // DETAILED_TIMING_STATS
#endif // NO_TIMING_STUFF
            m_Result << ".";
		} else {
			// adds <arg name="timers">true</arg> (or false) if the timers are
			// enabled (or disabled)
			AppendArgTagFast(sml_Names::kParamTimers, sml_Names::kTypeBoolean, agnt->sysparams[TIMERS_ENABLED] ? sml_Names::kTrue : sml_Names::kFalse);
		}
	}
	return true;
}

