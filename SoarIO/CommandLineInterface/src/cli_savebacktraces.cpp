/////////////////////////////////////////////////////////////////
// save-backtraces command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "sml_Names.h"
#include "sml_StringOps.h"

#include "IgSKI_Agent.h"
#include "IgSKI_Kernel.h"
#include "IgSKI_DoNotTouch.h"
#include "IgSKI_ProductionManager.h"
#include "IgSKI_Production.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseSaveBacktraces(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	Options optionsData[] = {
		{'d', "disable",	0},
		{'e', "enable",		0},
		{'d', "off",		0},
		{'e', "on",			0},
		{0, 0, 0}
	};

	bool setting = true;
	bool query = true;

	for (;;) {
		if (!ProcessOptions(argv, optionsData)) return false;
		if (m_Option == -1) break;

		switch (m_Option) {
			case 'd':
				setting = false;
				query = false;
				break;
			case 'e':
				setting = true;
				query = false;
				break;
			default:
				return SetError(CLIError::kGetOptError);
		}
	}
	if (m_NonOptionArguments) return SetError(CLIError::kTooManyArgs);
	return DoSaveBacktraces(pAgent, query ? 0 : &setting);
}

bool CommandLineInterface::DoSaveBacktraces(gSKI::IAgent* pAgent, bool* pSetting) {

	if (!RequireAgent(pAgent)) return false;

	// Attain the evil back door of doom, even though we aren't the TgD
	gSKI::EvilBackDoor::ITgDWorkArounds* pKernelHack = m_pKernel->getWorkaroundObject();

	if (!pSetting) {
		if (m_RawOutput) {
			m_Result << "Save bactraces is " << (pKernelHack->GetSysparam(pAgent, EXPLAIN_SYSPARAM) ? "enabled." : "disabled.");
		} else {
			AppendArgTagFast(sml_Names::kParamValue, sml_Names::kTypeBoolean, pKernelHack->GetSysparam(pAgent, EXPLAIN_SYSPARAM) ? sml_Names::kTrue : sml_Names::kFalse);
		}
		return true;
	}

	pKernelHack->SetSysparam(pAgent, EXPLAIN_SYSPARAM, *pSetting);
	return true;
}

