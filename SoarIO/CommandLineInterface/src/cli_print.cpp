#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_GetOpt.h"
#include "cli_Constants.h"

#include "sml_Names.h"

#include "IgSKI_Agent.h"
#include "IgSKI_Kernel.h"
#include "IgSKI_DoNotTouch.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParsePrint(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	static struct GetOpt::option longOptions[] = {
		{"all",				0, 0, 'a'},
		{"chunks",			0, 0, 'c'},
		{"depth",			1, 0, 'd'},
		{"defaults",		0, 0, 'D'},
		{"full",			0, 0, 'f'},
		{"filename",		0, 0, 'F'},
		{"internal",		0, 0, 'i'},
		{"justifications",	0, 0, 'j'},
		{"name",			0, 0, 'n'},
		{"operators",		0, 0, 'o'},
		{"stack",			0, 0, 's'},
		{"states",			0, 0, 'S'},
		{"user",			0, 0, 'u'},
		{0, 0, 0, 0}
	};

	int depth = pAgent->GetDefaultWMEDepth();
	PrintBitset options(0);

	for (;;) {
		int option = m_pGetOpt->GetOpt_Long(argv, ":acd:DfFijnosSu", longOptions, 0);
		if (option == -1) break;

		switch (option) {
			case 'a':
				options.set(PRINT_ALL);
				break;
			case 'c':
				options.set(PRINT_CHUNKS);
				break;
			case 'd':
				options.set(PRINT_DEPTH);
				if (!IsInteger(m_pGetOpt->GetOptArg())) {
					return SetError(CLIError::kIntegerExpected);
				}
				depth = atoi(m_pGetOpt->GetOptArg());
				if (depth < 0) {
					return SetError(CLIError::kIntegerMustBeNonNegative);
				}
				break;
			case 'D':
				options.set(PRINT_DEFAULTS);
				break;
			case 'f':
				options.set(PRINT_FULL);
				break;
			case 'F':
				options.set(PRINT_FILENAME);
				break;
			case 'i':
				options.set(PRINT_INTERNAL);
				break;
			case 'j':
				options.set(PRINT_JUSTIFICATIONS);
				break;
			case 'n':
				options.set(PRINT_NAME);
				break;
			case 'o':
				options.set(PRINT_OPERATORS);
				break;
			case 's':
				options.set(PRINT_STACK);
				break;
			case 'S':
				options.set(PRINT_STATES);
				break;
			case 'u':
				options.set(PRINT_USER);
				break;
			case ':':
				return SetError(CLIError::kMissingOptionArg);
			case '?':
				return SetError(CLIError::kUnrecognizedOption);
			default:
				return SetError(CLIError::kGetOptError);
		}
	}

	// STATES and OPERATORS are sub-options of STACK
	if (options.test(PRINT_OPERATORS) || options.test(PRINT_STATES)) {
		if (!options.test(PRINT_STACK)) return SetError(CLIError::kPrintSubOptionsOfStack);
	}

	switch (m_pGetOpt->GetAdditionalArgCount()) {
		case 0:  // no argument
			// the i and d options require an argument
			if (options.test(PRINT_INTERNAL) || options.test(PRINT_DEPTH)) return SetError(CLIError::kTooFewArgs);
			return DoPrint(pAgent, options, depth);

		case 1: 
			// the acDjus options don't allow an argument
			if (options.test(PRINT_ALL) 
				|| options.test(PRINT_CHUNKS) 
				|| options.test(PRINT_DEFAULTS) 
				|| options.test(PRINT_JUSTIFICATIONS) 
				|| options.test(PRINT_USER) 
				|| options.test(PRINT_STACK)) 
			{
				return SetError(CLIError::kTooManyArgs);
			}
			return DoPrint(pAgent, options, depth, &(argv[m_pGetOpt->GetOptind()]));

		default: // more than 1 arg
			break;
	}

	return SetError(CLIError::kTooManyArgs);
}

bool CommandLineInterface::DoPrint(gSKI::IAgent* pAgent, PrintBitset options, int depth, const std::string* pArg) {
	// Need agent pointer for function calls
	if (!RequireAgent(pAgent)) return false;

	// Attain the evil back door of doom, even though we aren't the TgD
	gSKI::EvilBackDoor::ITgDWorkArounds* pKernelHack = m_pKernel->getWorkaroundObject();

	// Check for stack print
	if (options.test(PRINT_STACK)) {

		// if neither states option nor operators option are set, set them both
		if (!options.test(PRINT_STATES) && !options.test(PRINT_OPERATORS)) {
			options.set(PRINT_STATES);
			options.set(PRINT_OPERATORS);
		}

		AddListenerAndDisableCallbacks(pAgent);
		pKernelHack->PrintStackTrace(pAgent, (options.test(PRINT_STATES)) ? true : false, (options.test(PRINT_OPERATORS)) ? true : false);
		RemoveListenerAndEnableCallbacks(pAgent);
		return true;
	}

	// Cache the flags since it makes function calls huge
	bool internal = options.test(PRINT_INTERNAL);
	bool filename = options.test(PRINT_FILENAME);
	bool full = options.test(PRINT_FULL);
	bool name = options.test(PRINT_NAME);

	// Check for the five general print options (all, chunks, defaults, justifications, user)
	if (options.test(PRINT_ALL)) {
		AddListenerAndDisableCallbacks(pAgent);
        pKernelHack->PrintUser(pAgent, 0, internal, filename, full, DEFAULT_PRODUCTION_TYPE);
        pKernelHack->PrintUser(pAgent, 0, internal, filename, full, USER_PRODUCTION_TYPE);
        pKernelHack->PrintUser(pAgent, 0, internal, filename, full, CHUNK_PRODUCTION_TYPE);
        pKernelHack->PrintUser(pAgent, 0, internal, filename, full, JUSTIFICATION_PRODUCTION_TYPE);
		RemoveListenerAndEnableCallbacks(pAgent);
		return true;
	}
	if (options.test(PRINT_CHUNKS)) {
		AddListenerAndDisableCallbacks(pAgent);
        pKernelHack->PrintUser(pAgent, 0, internal, filename, full, CHUNK_PRODUCTION_TYPE);
		RemoveListenerAndEnableCallbacks(pAgent);
		return true;
	}
	if (options.test(PRINT_DEFAULTS)) {
		AddListenerAndDisableCallbacks(pAgent);
        pKernelHack->PrintUser(pAgent, 0, internal, filename, full, DEFAULT_PRODUCTION_TYPE);
		RemoveListenerAndEnableCallbacks(pAgent);
		return true;
	}
	if (options.test(PRINT_JUSTIFICATIONS)) {
		AddListenerAndDisableCallbacks(pAgent);
        pKernelHack->PrintUser(pAgent, 0, internal, filename, full, JUSTIFICATION_PRODUCTION_TYPE);
		RemoveListenerAndEnableCallbacks(pAgent);
		return true;
	}
	if (options.test(PRINT_USER)) {
		AddListenerAndDisableCallbacks(pAgent);
        pKernelHack->PrintUser(pAgent, 0, internal, filename, full, USER_PRODUCTION_TYPE);
		RemoveListenerAndEnableCallbacks(pAgent);
		return true;
	}

	// Default to symbol print if there is an arg, otherwise print all
	AddListenerAndDisableCallbacks(pAgent);
	if (pArg) {
		pKernelHack->PrintSymbol(pAgent, const_cast<char*>(pArg->c_str()), name, filename, internal, full, depth);
	} else {
        pKernelHack->PrintUser(pAgent, 0, internal, filename, full, DEFAULT_PRODUCTION_TYPE);
        pKernelHack->PrintUser(pAgent, 0, internal, filename, full, USER_PRODUCTION_TYPE);
        pKernelHack->PrintUser(pAgent, 0, internal, filename, full, CHUNK_PRODUCTION_TYPE);
        pKernelHack->PrintUser(pAgent, 0, internal, filename, full, JUSTIFICATION_PRODUCTION_TYPE);
	}
	RemoveListenerAndEnableCallbacks(pAgent);

	// put the result into a message(string) arg tag
	if (!m_RawOutput) ResultToArgTag();
	return true;
}

