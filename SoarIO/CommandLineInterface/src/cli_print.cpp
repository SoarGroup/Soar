#include "cli_CommandLineInterface.h"

#include "cli_GetOpt.h"

#include "IgSKI_Agent.h"
#include "IgSKI_Kernel.h"
#include "IgSKI_DoNotTouch.h"

using namespace cli;

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

	GetOpt::optind = 0;
	GetOpt::opterr = 0;

	int option;
	// TODO: in 8.5.2 this is current_agent(default_wme_depth)
	int depth = 1;
	unsigned int options = 0;

	for (;;) {
		option = m_pGetOpt->GetOpt_Long(argv, ":acd:DfFijnosSu", longOptions, 0);
		if (option == -1) {
			break;
		}

		switch (option) {
			case 'a':
				options |= OPTION_PRINT_ALL;
				break;
			case 'c':
				options |= OPTION_PRINT_CHUNKS;
				break;
			case 'd':
				options |= OPTION_PRINT_DEPTH;
				if (!IsInteger(GetOpt::optarg)) {
					return HandleSyntaxError(Constants::kCLIPrint, "Depth must be an integer.");
				}
				depth = atoi(GetOpt::optarg);
				if (depth < 0) {
					return HandleSyntaxError(Constants::kCLIPrint, "Depth must be non-negative.");
				}
				break;
			case 'D':
				options |= OPTION_PRINT_DEFAULTS;
				break;
			case 'f':
				options |= OPTION_PRINT_FULL;
				break;
			case 'F':
				options |= OPTION_PRINT_FILENAME;
				break;
			case 'i':
				options |= OPTION_PRINT_INTERNAL;
				break;
			case 'j':
				options |= OPTION_PRINT_JUSTIFICATIONS;
				break;
			case 'n':
				options |= OPTION_PRINT_NAME;
				break;
			case 'o':
				options |= OPTION_PRINT_OPERATORS;
				break;
			case 's':
				options |= OPTION_PRINT_STACK;
				break;
			case 'S':
				options |= OPTION_PRINT_STATES;
				break;
			case 'u':
				options |= OPTION_PRINT_USER;
				break;
			case ':':
				return HandleSyntaxError(Constants::kCLIPrint, Constants::kCLIMissingOptionArg);
			case '?':
				return HandleSyntaxError(Constants::kCLIPrint, Constants::kCLIUnrecognizedOption);
			default:
				return HandleGetOptError((char)option);
		}
	}

	// One additional optional argument
	if ((argv.size() - GetOpt::optind) > 1) {
		return HandleSyntaxError(Constants::kCLIPrint, Constants::kCLITooManyArgs);
	} else if ((argv.size() - GetOpt::optind) == 1) {
		return DoPrint(pAgent, options, depth, &(argv[GetOpt::optind]));
	}
	return DoPrint(pAgent, options, depth);
}

bool CommandLineInterface::DoPrint(gSKI::IAgent* pAgent, const unsigned int options, int depth, std::string* pArg) {
	// TODO: structured output

	// Need agent pointer for function calls
	if (!RequireAgent(pAgent)) return false;
	if (!RequireKernel()) return false;

	// Attain the evil back door of doom, even though we aren't the TgD
	gSKI::EvilBackDoor::ITgDWorkArounds* pKernelHack = m_pKernel->getWorkaroundObject();

	// Check for stack print
	if (options & OPTION_PRINT_STACK) {
		pAgent->AddPrintListener(gSKIEVENT_PRINT, &m_ResultPrintHandler);
		pKernelHack->PrintStackTrace(pAgent, (options & OPTION_PRINT_STATES) ? true : false, (options & OPTION_PRINT_OPERATORS) ? true : false);
		pAgent->RemovePrintListener(gSKIEVENT_PRINT, &m_ResultPrintHandler);
		return true;
	}

	// Cache the flags since it makes function calls huge
	bool internal = (options & OPTION_PRINT_INTERNAL) ? true : false;
	bool filename = (options & OPTION_PRINT_FILENAME) ? true : false;
	bool full = (options & OPTION_PRINT_FULL) ? true : false;
	bool name = (options & OPTION_PRINT_NAME) ? true : false;

	// Check for the five general print options (all, chunks, defaults, justifications, user)
	if (options & OPTION_PRINT_ALL) {
		// TODO: Find out what is arg is for
		pAgent->AddPrintListener(gSKIEVENT_PRINT, &m_ResultPrintHandler);
        pKernelHack->PrintUser(pAgent, 0, internal, filename, full, DEFAULT_PRODUCTION_TYPE);
        pKernelHack->PrintUser(pAgent, 0, internal, filename, full, USER_PRODUCTION_TYPE);
        pKernelHack->PrintUser(pAgent, 0, internal, filename, full, CHUNK_PRODUCTION_TYPE);
        pKernelHack->PrintUser(pAgent, 0, internal, filename, full, JUSTIFICATION_PRODUCTION_TYPE);
		pAgent->RemovePrintListener(gSKIEVENT_PRINT, &m_ResultPrintHandler);
		return true;
	}
	if (options & OPTION_PRINT_CHUNKS) {
		pAgent->AddPrintListener(gSKIEVENT_PRINT, &m_ResultPrintHandler);
        pKernelHack->PrintUser(pAgent, 0, internal, filename, full, CHUNK_PRODUCTION_TYPE);
		pAgent->RemovePrintListener(gSKIEVENT_PRINT, &m_ResultPrintHandler);
		return true;
	}
	if (options & OPTION_PRINT_DEFAULTS) {
		pAgent->AddPrintListener(gSKIEVENT_PRINT, &m_ResultPrintHandler);
        pKernelHack->PrintUser(pAgent, 0, internal, filename, full, DEFAULT_PRODUCTION_TYPE);
		pAgent->RemovePrintListener(gSKIEVENT_PRINT, &m_ResultPrintHandler);
		return true;
	}
	if (options & OPTION_PRINT_JUSTIFICATIONS) {
		pAgent->AddPrintListener(gSKIEVENT_PRINT, &m_ResultPrintHandler);
        pKernelHack->PrintUser(pAgent, 0, internal, filename, full, JUSTIFICATION_PRODUCTION_TYPE);
		pAgent->RemovePrintListener(gSKIEVENT_PRINT, &m_ResultPrintHandler);
		return true;
	}
	if (options & OPTION_PRINT_USER) {
		pAgent->AddPrintListener(gSKIEVENT_PRINT, &m_ResultPrintHandler);
        pKernelHack->PrintUser(pAgent, 0, internal, filename, full, USER_PRODUCTION_TYPE);
		pAgent->RemovePrintListener(gSKIEVENT_PRINT, &m_ResultPrintHandler);
		return true;
	}

	// Default to symbol print
	pAgent->AddPrintListener(gSKIEVENT_PRINT, &m_ResultPrintHandler);
	pKernelHack->PrintSymbol(pAgent, const_cast<char*>(pArg->c_str()), name, filename, internal, full, depth);
	pAgent->RemovePrintListener(gSKIEVENT_PRINT, &m_ResultPrintHandler);
	return true;
}

