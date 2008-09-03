/////////////////////////////////////////////////////////////////
// echo command file.
//
// Author: Douglas Pearson, doug@threepenny.net
// Date  : 2005
//
/////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_Constants.h"

#include "sml_Names.h"
#include "sml_KernelSML.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseStopBefore(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	unused(pAgent);

	Options optionsData[] = {
		{'i', "input",	0},
		{'p', "proposal",	0},
		{'d', "decision",	0},
		{'a', "apply",		0},
		{'o', "output",		0},
		{0, 0, 0}
	};

	egSKIPhaseType phase = gSKI_INPUT_PHASE ;
	int countOptions = 0 ;

	for (;;) {
		if (!ProcessOptions(argv, optionsData)) return false;
		if (m_Option == -1) break;

		countOptions++ ;

		switch (m_Option) {
			case 'i':
				phase = gSKI_INPUT_PHASE ;
				break;
			case 'p':
				phase = gSKI_PROPOSAL_PHASE ;
				break;
			case 'd':
				phase = gSKI_DECISION_PHASE ;
				break;
			case 'a':
				phase = gSKI_APPLY_PHASE ;
				break;
			case 'o':
				phase = gSKI_OUTPUT_PHASE ;
				break;
			default:
				SetErrorDetail("Format is stop-before <phase> e.g. stop-before --input") ;
				return SetError(CLIError::kGetOptError);
		}
	}

	if (m_NonOptionArguments || countOptions > 1)
	{
		SetErrorDetail("Format is stop-before <phase> e.g. stop-before --input") ;
		return SetError(CLIError::kGetOptError) ;
	}

	return DoStopBefore(countOptions == 1, phase);
}

bool CommandLineInterface::DoStopBefore(bool setPhase, egSKIPhaseType phase) {

	// We only set the phase if asked, but we always report the current setting.
	if (setPhase)
		m_pKernelSML->SetStopBefore(phase) ;


	std::string phaseStr ;
	switch (m_pKernelSML->GetStopBefore())
	{
	case gSKI_INPUT_PHASE:    phaseStr = "input phase" ; break ;
	case gSKI_PROPOSAL_PHASE: phaseStr = "proposal phase" ; break ;
	case gSKI_DECISION_PHASE: phaseStr = "decision phase" ; break ;
	case gSKI_APPLY_PHASE:    phaseStr = "apply phase" ; break ;
	case gSKI_OUTPUT_PHASE:   phaseStr = "output phase" ; break ;
	default:                  phaseStr = "unknown phase" ; break ;
	}

	if (m_RawOutput) {
		m_Result << "Stop before " << phaseStr;
	} else {
		AppendArgTagFast(sml_Names::kParamPhase, sml_Names::kTypeString, phaseStr.c_str());
	}

	return true;
}

