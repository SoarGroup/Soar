#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_Constants.h"
#include "cli_GetOpt.h"
#include "sml_Names.h"

#include "IgSKI_Agent.h"
#include "IgSKI_Kernel.h"
#include "IgSKI_DoNotTouch.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseProductionFind(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	static struct GetOpt::option longOptions[] = {
		{"chunks",			0, 0, 'c'},
		{"lhs",				0, 0, 'l'},
		{"nochunks",		0, 0, 'n'},
		{"rhs",				0, 0, 'r'},
		{"show-bindings",	0, 0, 's'},
		{0, 0, 0, 0}
	};

	GetOpt::optind = 0;
	GetOpt::opterr = 0;

	unsigned int mode = 0;

	for (;;) {
		int option = m_pGetOpt->GetOpt_Long(argv, "clnrs", longOptions, 0);
		if (option == -1) break;

		switch (option) {
			case 'c':
				mode |= OPTION_PRODUCTION_FIND_INCLUDE_CHUNKS;
				break;
			case 'l':
				mode |= OPTION_PRODUCTION_FIND_INCLUDE_LHS;
				//mode &= ~OPTION_PRODUCTION_FIND_INCLUDE_RHS;
				break;
			case 'n':
				mode &= ~OPTION_PRODUCTION_FIND_INCLUDE_CHUNKS;
				break;
			case 'r':
				mode |= OPTION_PRODUCTION_FIND_INCLUDE_RHS;
				//mode &= ~OPTION_PRODUCTION_FIND_INCLUDE_LHS;
				break;
			case 's':
				mode |= OPTION_PRODUCTION_FIND_SHOWBINDINGS;
				break;
			case '?':
				return m_Error.SetError(CLIError::kUnrecognizedOption);
			default:
				return m_Error.SetError(CLIError::kGetOptError);
		}
	}

	if (argv.size() == (unsigned)GetOpt::optind) return m_Error.SetError(CLIError::kTooFewArgs);

	if (!mode) mode = OPTION_PRODUCTION_FIND_INCLUDE_LHS;

	std::string pattern;
	for (unsigned i = GetOpt::optind; i < argv.size(); ++i) {
		pattern += argv[i];
		pattern += ' ';
	}

	pattern = pattern.substr(0, pattern.length() - 1);

	return DoProductionFind(pAgent, mode, pattern);
}

bool CommandLineInterface::DoProductionFind(gSKI::IAgent* pAgent, unsigned int mode, std::string pattern) {
	// Need agent pointer for function calls
	if (!RequireAgent(pAgent)) return false;

	// Attain the evil back door of damnation, even though we aren't the TgD
	gSKI::EvilBackDoor::ITgDWorkArounds* pKernelHack = m_pKernel->getWorkaroundObject();

	AddListenerAndDisableCallbacks(pAgent);

	bool ret = pKernelHack->ProductionFind(pAgent, 
		0, 
		m_pKernel, 
		(mode & OPTION_PRODUCTION_FIND_INCLUDE_LHS) ? true : false, 
		(mode & OPTION_PRODUCTION_FIND_INCLUDE_RHS) ? true : false, 
		const_cast<char*>(pattern.c_str()), 
		(mode & OPTION_PRODUCTION_FIND_SHOWBINDINGS) ? true : false, 
		(mode & OPTION_PRODUCTION_FIND_INCLUDE_CHUNKS) ? true : false, 
		(mode & OPTION_PRODUCTION_FIND_INCLUDE_CHUNKS) ? true : false);
	
	RemoveListenerAndEnableCallbacks(pAgent);

	if (!m_RawOutput) {
		AppendArgTagFast(sml_Names::kParamMessage, sml_Names::kTypeString, m_Result.c_str());
		m_Result.clear();
	}

	return ret;
}

