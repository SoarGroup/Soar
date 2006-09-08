/////////////////////////////////////////////////////////////////
// production-find command file.
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

#include "gSKI_Kernel.h"
#include "gSKI_DoNotTouch.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseProductionFind(gSKI::Agent* pAgent, std::vector<std::string>& argv) {
	Options optionsData[] = {
		{'c', "chunks",			0},
		{'l', "lhs",				0},
		{'n', "nochunks",		0},
		{'r', "rhs",				0},
		{'s', "show-bindings",	0},
		{0, 0, 0}
	};

	ProductionFindBitset options(0);

	for (;;) {
		if (!ProcessOptions(argv, optionsData)) return false;
		if (m_Option == -1) break;

		switch (m_Option) {
			case 'c':
				options.set(PRODUCTION_FIND_ONLY_CHUNKS);
				options.reset(PRODUCTION_FIND_NO_CHUNKS);
				break;
			case 'l':
				options.set(PRODUCTION_FIND_INCLUDE_LHS);
				break;
			case 'n':
				options.set(PRODUCTION_FIND_NO_CHUNKS);
				options.reset(PRODUCTION_FIND_ONLY_CHUNKS);
				break;
			case 'r':
				options.set(PRODUCTION_FIND_INCLUDE_RHS);
				break;
			case 's':
				options.set(PRODUCTION_FIND_SHOWBINDINGS);
				break;
			default:
				return SetError(CLIError::kGetOptError);
		}
	}

	if (!m_NonOptionArguments) {
		SetErrorDetail("Pattern required.");
		return SetError(CLIError::kTooFewArgs);
	}

	if (options.none()) options.set(PRODUCTION_FIND_INCLUDE_LHS);

	std::string pattern;
	for (unsigned i = m_Argument - m_NonOptionArguments; i < argv.size(); ++i) {
		pattern += argv[i];
		pattern += ' ';
	}
	pattern = pattern.substr(0, pattern.length() - 1);

	return DoProductionFind(pAgent, options, pattern);
}

bool CommandLineInterface::DoProductionFind(gSKI::Agent* pAgent, const ProductionFindBitset& options, const std::string& pattern) {
	// Need agent pointer for function calls
	if (!RequireAgent(pAgent)) return false;

	// Attain the evil back door of damnation, even though we aren't the TgD
	gSKI::EvilBackDoor::TgDWorkArounds* pKernelHack = m_pKernel->getWorkaroundObject();

	AddListenerAndDisableCallbacks(pAgent);

	bool ret = pKernelHack->ProductionFind(pAgent, 
		0, 
		m_pKernel, 
		options.test(PRODUCTION_FIND_INCLUDE_LHS), 
		options.test(PRODUCTION_FIND_INCLUDE_RHS), 
		const_cast<char*>(pattern.c_str()), 
		options.test(PRODUCTION_FIND_SHOWBINDINGS), 
		options.test(PRODUCTION_FIND_ONLY_CHUNKS),
		options.test(PRODUCTION_FIND_NO_CHUNKS));
	
	RemoveListenerAndEnableCallbacks(pAgent);

	// put the result into a message(string) arg tag
	if (!m_RawOutput) ResultToArgTag();
	return ret;
}

