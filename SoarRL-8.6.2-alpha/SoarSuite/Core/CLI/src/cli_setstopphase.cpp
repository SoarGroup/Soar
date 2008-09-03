/////////////////////////////////////////////////////////////////
// set stop phase command file.
//
// Author: Douglas Pearson, doug@threepenny.net
// Date  : 2005
//
/////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"

#include "sml_Names.h"
#include "sml_KernelSML.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseSetStopPhase(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	unused(pAgent);

	Options optionsData[] = {
		{'B', "before",		0},	// optional (defaults to before)
		{'A', "after",		0},	// optional
		{'i', "input",	    0},	// requires one of these
		{'p', "proposal",	0},
		{'d', "decision",	0},
		{'a', "apply",		0},
		{'o', "output",		0},
		{0, 0, 0}
	};

	egSKIPhaseType phase = gSKI_INPUT_PHASE ;
	int countPhaseArgs = 0 ;
	bool before = true ;

	for (;;) {
		if (!ProcessOptions(argv, optionsData)) return false;
		if (m_Option == -1) break;

		switch (m_Option) {
			case 'B':
				before = true ;
				break ;
			case 'A':
				before = false ;
				break ;
			case 'i':
				phase = gSKI_INPUT_PHASE ;
				countPhaseArgs++ ;
				break;
			case 'p':
				phase = gSKI_PROPOSAL_PHASE ;
				countPhaseArgs++ ;
				break;
			case 'd':
				phase = gSKI_DECISION_PHASE ;
				countPhaseArgs++ ;
				break;
			case 'a':
				phase = gSKI_APPLY_PHASE ;
				countPhaseArgs++ ;
				break;
			case 'o':
				phase = gSKI_OUTPUT_PHASE ;
				countPhaseArgs++ ;
				break;
			default:
				return SetError(CLIError::kGetOptError);
		}
	}

	if (m_NonOptionArguments || countPhaseArgs > 1)
	{
		SetErrorDetail("Format is 'set-stop-phase [--Before | --After] <phase>' where <phase> is --input | --proposal | --decision | --apply | --output\ne.g. set-stop-phase --before --input") ;
		return SetError(CLIError::kGetOptError) ;
	}

	return DoSetStopPhase(countPhaseArgs == 1, before, phase);
}

bool CommandLineInterface::DoSetStopPhase(bool setPhase, bool before, egSKIPhaseType phase) {

	// We only set the phase if asked, but we always report the current setting.
	if (setPhase)
	{
		// The kernel only accepts stop before a phase logic.
		// The "stop after" form is just a courtesy to the user in case they prefer to think that way.
		if (!before)
		{
			phase = (egSKIPhaseType)(((int)phase)+1) ;
			if (phase > gSKI_OUTPUT_PHASE)
				phase = gSKI_INPUT_PHASE ;
		}

		m_pKernelSML->SetStopBefore(phase) ;
	}

	std::string phaseStr ;
	egSKIPhaseType stopPhase = m_pKernelSML->GetStopBefore() ;

	if (!before)
	{
		stopPhase = (egSKIPhaseType)(((int)stopPhase)-1) ;
		if (stopPhase < gSKI_INPUT_PHASE)
			stopPhase = gSKI_OUTPUT_PHASE ;
	}

	switch (stopPhase)
	{
	case gSKI_INPUT_PHASE:    phaseStr = "input phase" ; break ;
	case gSKI_PROPOSAL_PHASE: phaseStr = "proposal phase" ; break ;
	case gSKI_DECISION_PHASE: phaseStr = "decision phase" ; break ;
	case gSKI_APPLY_PHASE:    phaseStr = "apply phase" ; break ;
	case gSKI_OUTPUT_PHASE:   phaseStr = "output phase" ; break ;
	default:                  phaseStr = "unknown phase" ; break ;
	}

	if (m_RawOutput) {
		m_Result << (before ? "Stop before " : "Stop after ") << phaseStr;
	} else {
		std::ostringstream buffer;
		buffer << stopPhase;
		std::string bufferString = buffer.str() ;
		AppendArgTagFast(sml_Names::kParamPhase, sml_Names::kTypeString, bufferString.c_str());
	}

	return true;
}

