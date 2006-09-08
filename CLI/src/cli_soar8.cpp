/////////////////////////////////////////////////////////////////
// soar8 command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H
#include <portability.h>

#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"
#include "sml_Names.h"

#include "gSKI_Agent.h"
#include "gSKI_ProductionManager.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseSoar8(gSKI::Agent* pAgent, std::vector<std::string>& argv) {
	Options optionsData[] = {
		{'e', "on",			0},
		{'e', "enable",		0},
		{'d', "off",		0},
		{'d', "disable",	0},
		{0, 0, 0}
	};

	bool query = true;
	bool soar8 = true;

	for (;;) {
		if (!ProcessOptions(argv, optionsData)) return false;
		if (m_Option == -1) break;

		switch (m_Option) {
			case 'd':
				query = false;
				soar8 = false;
				break;
			case 'e':
				query = false;
				soar8 = true;
				break;
			default:
				return SetError(CLIError::kGetOptError);
		}
	}

	// No non-option arguments
	if (m_NonOptionArguments) return SetError(CLIError::kTooManyArgs);

	return DoSoar8(pAgent, query ? 0 : &soar8);
}

bool CommandLineInterface::DoSoar8(gSKI::Agent* pAgent, bool* pSoar8) {

	if (!RequireAgent(pAgent)) return false;

	if (!pSoar8) {
		// query
		if (m_RawOutput) {
			m_Result << "Soar 8 mode is " << (pAgent->GetOperand2Mode() ? "on." : "off.");
		} else {
			AppendArgTagFast(sml_Names::kParamValue, sml_Names::kTypeBoolean, pAgent->GetOperand2Mode() ? sml_Names::kTrue : sml_Names::kFalse);
		}
		return true;
	}

	// FIXME: Check for empty system?
	// voigtjr/kcoulter - can't do this check because of gSKI's pre-existing wmes
	// maybe check for only those wmes to exist?  sounds problematic :)

	// {int i;

	// /* --- check for empty system --- */
	// if (current_agent(all_wmes_in_rete)) {
	//   sprintf(interp->result,
	//       "Can't change modes unless working memory is empty.");
	//   return TCL_ERROR;
	// }

	// Check that production memory is empty
	gSKI::ProductionManager* pProductionManager = pAgent->GetProductionManager(&m_gSKIError);
	if (gSKI::isError(m_gSKIError)) {
		SetErrorDetail("Unable to get production manager.");
		return SetError(CLIError::kgSKIError);
	}
	gSKI::tIProductionIterator* pIter = pProductionManager->GetAllProductions(false, &m_gSKIError);
	if (gSKI::isError(m_gSKIError) || !pIter) {
		SetErrorDetail("Unable to get all productions.");
		return SetError(CLIError::kgSKIError);
	}
	unsigned long numProductions = pIter->GetNumElements(&m_gSKIError);
	if (gSKI::isError(m_gSKIError) || !pIter) {
		SetErrorDetail("Unable to get number of elements (productions).");
		return SetError(CLIError::kgSKIError);
	}
	if (numProductions) {
		return SetError(CLIError::kProductionMemoryNotEmpty);
	}

	if (*pSoar8) {
		// Turn Soar8 mode ON

		pAgent->SetOperand2Mode(true);

		// o-support-mode 4
		if (!DoOSupportMode(pAgent, 4)) return false;
		
		// init-soar
		if (!DoInitSoar(pAgent)) return false;
	} else {
		// Turn Soar8 mode OFF

		pAgent->SetOperand2Mode(false);

		// o-support-mode 0
		if (!DoOSupportMode(pAgent, 0)) return false;

		// init-soar
		if (!DoInitSoar(pAgent)) return false;
	}
	return true;
}

