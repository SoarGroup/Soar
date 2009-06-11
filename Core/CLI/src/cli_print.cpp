/////////////////////////////////////////////////////////////////
// print command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "sml_Utils.h"
#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"
#include "cli_CLIError.h"

#include "sml_Names.h"

#include "agent.h"

#include "sml_KernelHelpers.h"
#include "sml_KernelSML.h"
#include "sml_AgentSML.h"
#include "gsysparam.h"
#include "xml.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParsePrint(std::vector<std::string>& argv) {
	Options optionsData[] = {
		{'a', "all",			OPTARG_NONE},
		{'c', "chunks",			OPTARG_NONE},
		{'d', "depth",			OPTARG_REQUIRED},
		{'D', "defaults",		OPTARG_NONE},
		{'e', "exact",			OPTARG_NONE},
		{'f', "full",			OPTARG_NONE},
		{'F', "filename",		OPTARG_NONE},
		{'i', "internal",		OPTARG_NONE},
		{'j', "justifications",	OPTARG_NONE},
		{'n', "name",			OPTARG_NONE},
		{'o', "operators",		OPTARG_NONE},
		{'r', "rl",				OPTARG_NONE},
		{'s', "stack",			OPTARG_NONE},
		{'S', "states",			OPTARG_NONE},
		{'t', "tree",			OPTARG_NONE},
		{'T', "template",		OPTARG_NONE},
		{'u', "user",			OPTARG_NONE},
		{'v', "varprint",		OPTARG_NONE},
		{0, 0, OPTARG_NONE}
	};

	int depth = m_pAgentSoar->default_wme_depth;
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
				if ( !from_string( depth, m_OptionArgument ) ) return SetError(CLIError::kIntegerExpected);
				if (depth < 0) return SetError(CLIError::kIntegerMustBeNonNegative);
				break;
			case 'D':
				options.set(PRINT_DEFAULTS);
				break;
			case 'e':
				options.set(PRINT_EXACT);
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
			case 'r':
				options.set(PRINT_RL);
				break;
			case 's':
				options.set(PRINT_STACK);
				break;
			case 'S':
				options.set(PRINT_STATES);
				break;
			case 't':
				options.set(PRINT_TREE);
				break;
			case 'T':
				options.set(PRINT_TEMPLATE);
				break;
			case 'u':
				options.set(PRINT_USER);
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
			if (options.test(PRINT_INTERNAL) || options.test(PRINT_TREE) || options.test(PRINT_DEPTH)) return SetError(CLIError::kTooFewArgs);
			return DoPrint(options, depth);

		case 1: 
			// the acDjus options don't allow an argument
			if (options.test(PRINT_ALL) 
				|| options.test(PRINT_CHUNKS) 
				|| options.test(PRINT_DEFAULTS) 
				|| options.test(PRINT_JUSTIFICATIONS)
				|| options.test(PRINT_RL)
				|| options.test(PRINT_TEMPLATE)
				|| options.test(PRINT_USER) 
				|| options.test(PRINT_STACK)) 
			{
				SetErrorDetail("No argument allowed when printing all/chunks/defaults/justifications/rl/template/user/stack.");
				return SetError(CLIError::kTooManyArgs);
			}
			if (options.test(PRINT_EXACT) && (options.test(PRINT_DEPTH) || options.test(PRINT_TREE))) 
			{
				SetErrorDetail("No depth/tree flags allowed when printing exact.");
				return SetError(CLIError::kTooManyArgs);
			}
			return DoPrint(options, depth, &(argv[m_Argument - m_NonOptionArguments]));

		default: // more than 1 arg
			break;
	}

	return SetError(CLIError::kTooManyArgs);
}

bool CommandLineInterface::DoPrint(PrintBitset options, int depth, const std::string* pArg) {
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
	sml::KernelHelpers* pKernelHack = m_pKernelSML->GetKernelHelpers() ;

	// Check for stack print
	if (options.test(PRINT_STACK)) {

		// if neither states option nor operators option are set, set them both
		if (!options.test(PRINT_STATES) && !options.test(PRINT_OPERATORS)) {
			options.set(PRINT_STATES);
			options.set(PRINT_OPERATORS);
		}

		// Structured output through structured output callback
		pKernelHack->PrintStackTrace(m_pAgentSML, (options.test(PRINT_STATES)) ? true : false, (options.test(PRINT_OPERATORS)) ? true : false);
		return true;
	}

	// Cache the flags since it makes function calls huge
	bool internal = options.test(PRINT_INTERNAL);
	bool tree = options.test(PRINT_TREE);
	bool filename = options.test(PRINT_FILENAME);
	bool full = options.test(PRINT_FULL);
	bool name = options.test(PRINT_NAME);
	bool exact = options.test(PRINT_EXACT);

	// Check for the five general print options (all, chunks, defaults, justifications, user)
	if (options.test(PRINT_ALL)) {
        pKernelHack->PrintUser(m_pAgentSML, 0, internal, filename, full, DEFAULT_PRODUCTION_TYPE);
        pKernelHack->PrintUser(m_pAgentSML, 0, internal, filename, full, USER_PRODUCTION_TYPE);
        pKernelHack->PrintUser(m_pAgentSML, 0, internal, filename, full, CHUNK_PRODUCTION_TYPE);
        pKernelHack->PrintUser(m_pAgentSML, 0, internal, filename, full, JUSTIFICATION_PRODUCTION_TYPE);
        pKernelHack->PrintUser(m_pAgentSML, 0, internal, filename, full, TEMPLATE_PRODUCTION_TYPE);
		return true;
	}
	if (options.test(PRINT_CHUNKS)) {
        pKernelHack->PrintUser(m_pAgentSML, 0, internal, filename, full, CHUNK_PRODUCTION_TYPE);
		return true;
	}
	if (options.test(PRINT_DEFAULTS)) {
        pKernelHack->PrintUser(m_pAgentSML, 0, internal, filename, full, DEFAULT_PRODUCTION_TYPE);
		return true;
	}
	if (options.test(PRINT_JUSTIFICATIONS)) {
        pKernelHack->PrintUser(m_pAgentSML, 0, internal, filename, full, JUSTIFICATION_PRODUCTION_TYPE);
		return true;
	}
	if (options.test(PRINT_USER)) {
        pKernelHack->PrintUser( m_pAgentSML, 0, internal, filename, full, USER_PRODUCTION_TYPE);
		return true;
	}
	if (options.test(PRINT_RL)) {
		pKernelHack->print_rl_rules( m_pAgentSoar, 0, internal, filename, full );
		return true;
	}
	if (options.test(PRINT_TEMPLATE)) {
        pKernelHack->PrintUser( m_pAgentSML, 0, internal, filename, full, TEMPLATE_PRODUCTION_TYPE );
		return true;
	}

	// Default to symbol print if there is an arg, otherwise print all
	if (options.test(PRINT_VARPRINT)) {
		m_VarPrint = true;
	}
	if (pArg) {
		pKernelHack->PrintSymbol(m_pAgentSML, const_cast<char*>(pArg->c_str()), name, filename, internal, tree, full, depth, exact);
	} else {
        pKernelHack->PrintUser(m_pAgentSML, 0, internal, filename, full, DEFAULT_PRODUCTION_TYPE);
        pKernelHack->PrintUser(m_pAgentSML, 0, internal, filename, full, USER_PRODUCTION_TYPE);
        pKernelHack->PrintUser(m_pAgentSML, 0, internal, filename, full, CHUNK_PRODUCTION_TYPE);
        pKernelHack->PrintUser(m_pAgentSML, 0, internal, filename, full, JUSTIFICATION_PRODUCTION_TYPE);
        pKernelHack->PrintUser(m_pAgentSML, 0, internal, filename, full, TEMPLATE_PRODUCTION_TYPE);
	}
	m_VarPrint = false;

	return true;
}

