/////////////////////////////////////////////////////////////////
// print command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"

#include "sml_Names.h"

#include "IgSKI_Agent.h"
#include "IgSKI_Kernel.h"
#include "IgSKI_DoNotTouch.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParsePrint(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	Options optionsData[] = {
		{'a', "all",			0},
		{'c', "chunks",			0},
		{'d', "depth",			1},
		{'D', "defaults",		0},
		{'f', "full",			0},
		{'F', "filename",		0},
		{'i', "internal",		0},
		{'j', "justifications",	0},
		{'n', "name",			0},
		{'o', "operators",		0},
		{'s', "stack",			0},
		{'S', "states",			0},
		{'u', "user",			0},
		{'T', "template",       0},			// NUMERIC_INDIFFERENCE
		{'R', "RL" , 			0},			// NUMERIC_INDIFFERENCE
		{'v', "varprint",		0},
		{0, 0, 0}
	};

	int depth = pAgent->GetDefaultWMEDepth();
	PrintBitset options(0);

	for (;;) {
		if (!ProcessOptions(argv, optionsData)) return false;
		if (m_Option == -1) break;

		switch (m_Option) {
			case 'a':
				options.set(PRINT_ALL);
				break;
			case 'c':
				options.set(PRINT_CHUNKS);
				break;
			case 'd':
				options.set(PRINT_DEPTH);
				if (!IsInteger(m_OptionArgument)) {
					return SetError(CLIError::kIntegerExpected);
				}
				depth = atoi(m_OptionArgument.c_str());
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
			case 'T':								// NUMERIC_INDIFFERENCE
				options.set(PRINT_TEMPLATE);
				break;
			case 'R':								// NUMERIC_INDIFFERENCE
				options.set(PRINT_RL);
				break;
			case 'v':
				options.set(PRINT_VARPRINT);
				break;
			default:
				return SetError(CLIError::kGetOptError);
		}
	}

	// STATES and OPERATORS are sub-options of STACK
	if (options.test(PRINT_OPERATORS) || options.test(PRINT_STATES)) {
		if (!options.test(PRINT_STACK)) return SetError(CLIError::kPrintSubOptionsOfStack);
	}

	switch (m_NonOptionArguments) {
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
				|| options.test(PRINT_TEMPLATE)
				|| options.test(PRINT_RL)
				|| options.test(PRINT_STACK)) 
			{
				SetErrorDetail("No argument allowed when printing all/chunks/defaults/justifications/user/stack.");
				return SetError(CLIError::kTooManyArgs);
			}
			return DoPrint(pAgent, options, depth, &(argv[m_Argument - m_NonOptionArguments]));

		default: // more than 1 arg
			break;
	}

	return SetError(CLIError::kTooManyArgs);
}

bool CommandLineInterface::DoPrint(gSKI::IAgent* pAgent, PrintBitset options, int depth, const std::string* pArg) {
	// Need agent pointer for function calls
	if (!RequireAgent(pAgent)) return false;

	// Strip any surrounding "{"
	/*
	std::string local = *pArg ;
	if (local.length() > 2)
	{
		if (local[0] == '{') local = local.substr(1) ;
		if (local[local.length()-1] == '}') local = local.substr(0, local.length()-1) ;
	}
	*/

	// Attain the evil back door of doom, even though we aren't the TgD
	gSKI::EvilBackDoor::ITgDWorkArounds* pKernelHack = m_pKernel->getWorkaroundObject();

	// Check for stack print
	if (options.test(PRINT_STACK)) {

		// if neither states option nor operators option are set, set them both
		if (!options.test(PRINT_STATES) && !options.test(PRINT_OPERATORS)) {
			options.set(PRINT_STATES);
			options.set(PRINT_OPERATORS);
		}

		// Structured output through structured output callback
		if (m_RawOutput) AddListenerAndDisableCallbacks(pAgent);
		pKernelHack->PrintStackTrace(pAgent, (options.test(PRINT_STATES)) ? true : false, (options.test(PRINT_OPERATORS)) ? true : false);
		if (m_RawOutput) RemoveListenerAndEnableCallbacks(pAgent);
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
	pKernelHack->PrintUser(pAgent, 0, internal, filename, full, TEMPLATE_PRODUCTION_TYPE);
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
	if (options.test(PRINT_TEMPLATE)) {
		AddListenerAndDisableCallbacks(pAgent);
        pKernelHack->PrintUser(pAgent, 0, internal, filename, full, TEMPLATE_PRODUCTION_TYPE);
		RemoveListenerAndEnableCallbacks(pAgent);
		return true;
	}	
	if (options.test(PRINT_RL)) {
		AddListenerAndDisableCallbacks(pAgent);
		pKernelHack->PrintRL(pAgent, 0, internal, filename, full);
		RemoveListenerAndEnableCallbacks(pAgent);
		return true;
	}

	// Default to symbol print if there is an arg, otherwise print all
	AddListenerAndDisableCallbacks(pAgent);
	if (options.test(PRINT_VARPRINT)) {
		m_VarPrint = true;
	}
	if (pArg) {
		pKernelHack->PrintSymbol(pAgent, const_cast<char*>(pArg->c_str()), name, filename, internal, full, depth);
	} else {
        pKernelHack->PrintUser(pAgent, 0, internal, filename, full, DEFAULT_PRODUCTION_TYPE);
        pKernelHack->PrintUser(pAgent, 0, internal, filename, full, USER_PRODUCTION_TYPE);
        pKernelHack->PrintUser(pAgent, 0, internal, filename, full, CHUNK_PRODUCTION_TYPE);
        pKernelHack->PrintUser(pAgent, 0, internal, filename, full, JUSTIFICATION_PRODUCTION_TYPE);
        pKernelHack->PrintUser(pAgent, 0, internal, filename, full, TEMPLATE_PRODUCTION_TYPE);
	}
	m_VarPrint = false;
	RemoveListenerAndEnableCallbacks(pAgent);

	// put the result into a message(string) arg tag
	if (!m_RawOutput) ResultToArgTag();
	return true;
}

