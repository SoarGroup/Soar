#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_GetOpt.h"
#include "cli_Constants.h"

#include "sml_StringOps.h"
#include "sml_Names.h"

#include "IgSKI_Agent.h"
#include "IgSKI_Kernel.h"
#include "IgSKI_DoNotTouch.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseWatch(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	static struct GetOpt::option longOptions[] = {
		{"aliases",					1, 0, 'a'},
		{"backtracing",				1, 0, 'b'},
		{"chunks",		            1, 0, 'c'},
		{"decisions",				1, 0, 'd'},
		{"default-productions",		1, 0, 'D'},
		{"indifferent-selection",	1, 0, 'i'},
		{"justifications",			1, 0, 'j'},
		{"learning",				1, 0, 'l'},
		{"loading",					1, 0, 'L'},
		{"none",					0, 0, 'n'},
		{"phases",					1, 0, 'p'},
		{"productions",				1, 0, 'P'},
		{"preferences",				1, 0, 'r'},
		{"user-productions",		1, 0, 'u'},
		{"wmes",					1, 0, 'w'},
		{"wme-detail",				1, 0, 'W'},
		{0, 0, 0, 0}
	};

	GetOpt::optind = 0;
	GetOpt::opterr = 0;

	int constant;
	unsigned int options = 0;	// what flag changed
	unsigned int values = 0;    // new setting

	for (;;) {
		int option = m_pGetOpt->GetOpt_Long(argv, "a:b:c:d:D:i:j:l:L:np:P:r:u:w:W:", longOptions, 0);
		if (option == -1) break;

		switch (option) {
			case 'b':
				constant = OPTION_WATCH_BACKTRACING;
				options |= OPTION_WATCH_BACKTRACING;
				break;
			case 'c':
				constant = OPTION_WATCH_CHUNKS;
				options |= OPTION_WATCH_CHUNKS;
				break;
			case 'd':
				constant = OPTION_WATCH_DECISIONS;
				options |= OPTION_WATCH_DECISIONS;
				break;
			case 'D':
				constant = OPTION_WATCH_DEFAULT_PRODUCTIONS;
				options |= OPTION_WATCH_DEFAULT_PRODUCTIONS;
				break;
			case 'i':
				constant = OPTION_WATCH_INDIFFERENT_SELECTION;
				options |= OPTION_WATCH_INDIFFERENT_SELECTION;
				break;
			case 'j':
				constant = OPTION_WATCH_JUSTIFICATIONS;
				options |= OPTION_WATCH_JUSTIFICATIONS;
				break;
			case 'l':
				constant = OPTION_WATCH_LEARNING;
				options |= OPTION_WATCH_LEARNING;
				break;
			case 'L':
				constant = OPTION_WATCH_LOADING;
				options |= OPTION_WATCH_LOADING;
				break;
			case 'n':
				constant = OPTION_WATCH_NONE;
				options |= OPTION_WATCH_NONE;
				break;
			case 'p':
				constant = OPTION_WATCH_PHASES;
				options |= OPTION_WATCH_PHASES;
				break;
			case 'P':
				constant = OPTION_WATCH_PRODUCTIONS;
				options |= OPTION_WATCH_PRODUCTIONS;
				break;
			case 'r':
				constant = OPTION_WATCH_PREFERENCES;
				options |= OPTION_WATCH_PREFERENCES;
				break;
			case 'u':
				constant = OPTION_WATCH_USER_PRODUCTIONS;
				options |= OPTION_WATCH_USER_PRODUCTIONS;
				break;
			case 'w':
				constant = OPTION_WATCH_WMES;
				options |= OPTION_WATCH_WMES;
				break;
			case 'W':
				constant = OPTION_WATCH_WME_DETAIL;
				options |= OPTION_WATCH_WME_DETAIL;
				break;
			case ':':
				return m_Error.SetError(CLIError::kMissingOptionArg);
			case '?':
				return m_Error.SetError(CLIError::kUnrecognizedOption);
			default:
				return m_Error.SetError(CLIError::kGetOptError);
		}

		// process argument
		if (!WatchArg(values, constant, GetOpt::optarg)) {
			return false;
		}
	}

	// Only one non-option argument allowed, watch level
	if ((unsigned)GetOpt::optind == argv.size() - 1) {

		if (!IsInteger(argv[GetOpt::optind])) {
			return m_Error.SetError(CLIError::kIntegerExpected);
		}
		int watchLevel = atoi(argv[GetOpt::optind].c_str());
		if ((watchLevel < 0) || (watchLevel > 5)) {
			return m_Error.SetError(CLIError::kIntegerOutOfRange);
		}

		if (watchLevel == 0) {
			// Turn everything off
			options |= OPTION_WATCH_NONE;
		} else {
		
			// Activate the options
			options |= OPTION_WATCH_DECISIONS | OPTION_WATCH_PHASES | OPTION_WATCH_DEFAULT_PRODUCTIONS | OPTION_WATCH_USER_PRODUCTIONS
				| OPTION_WATCH_CHUNKS | OPTION_WATCH_JUSTIFICATIONS | OPTION_WATCH_WMES | OPTION_WATCH_PREFERENCES;

			// Reset some settings per old soar 8.5.2 behavior
			// Don't reset wme detail or learning unless watch 0
			WatchArg(values, OPTION_WATCH_DECISIONS, 0);			// set true in watch 1
			WatchArg(values, OPTION_WATCH_PHASES, 0);				// set true in watch 2
			WatchArg(values, OPTION_WATCH_DEFAULT_PRODUCTIONS, 0);	// set true in watch 3
			WatchArg(values, OPTION_WATCH_USER_PRODUCTIONS, 0);		// set true in watch 3
			WatchArg(values, OPTION_WATCH_CHUNKS, 0);				// set true in watch 3
			WatchArg(values, OPTION_WATCH_JUSTIFICATIONS, 0);		// set true in watch 3
			WatchArg(values, OPTION_WATCH_WMES, 0);					// set true in watch 4
			WatchArg(values, OPTION_WATCH_PREFERENCES, 0);			// set true in watch 5

			// TODO: This is off by default and nothing seems to turn it on
			//pKernelHack->SetSysparam(pAgent, TRACE_OPERAND2_REMOVALS_SYSPARAM, false);

			// Switch out the level
			switch (watchLevel) {
				case 5:
					WatchArg(values, OPTION_WATCH_PREFERENCES, 1);
					// 5 includes 4

				case 4:
					WatchArg(values, OPTION_WATCH_WMES, 1);
					// 4 includes 3

				case 3:
					WatchArg(values, OPTION_WATCH_DEFAULT_PRODUCTIONS, 1);
					WatchArg(values, OPTION_WATCH_USER_PRODUCTIONS, 1);
					WatchArg(values, OPTION_WATCH_CHUNKS, 1);
					WatchArg(values, OPTION_WATCH_JUSTIFICATIONS, 1);
					// 3 includes 2

				case 2:
					WatchArg(values, OPTION_WATCH_PHASES, 1);
					// 2 includes 1

				case 1:
				default:
					WatchArg(values, OPTION_WATCH_DECISIONS, 1);
					break;
			}
		}

	} else if ((unsigned)GetOpt::optind < argv.size()) {
		return m_Error.SetError(CLIError::kTooManyArgs);
	}

	return DoWatch(pAgent, options, values);
}

//__        __    _       _        _
//\ \      / /_ _| |_ ___| |__    / \   _ __ __ _
// \ \ /\ / / _` | __/ __| '_ \  / _ \ | '__/ _` |
//  \ V  V / (_| | || (__| | | |/ ___ \| | | (_| |
//   \_/\_/ \__,_|\__\___|_| |_/_/   \_\_|  \__, |
//                                          |___/
bool CommandLineInterface::WatchArg(unsigned int& values, const unsigned int option, const char* arg) {
	if (!arg || !IsInteger(arg)) {
		return m_Error.SetError(CLIError::kIntegerExpected);
	}
	return WatchArg(values, option, atoi(arg));
}

bool CommandLineInterface::WatchArg(unsigned int& values, const unsigned int option, int argInt) {
	// If option is none, values will be ignored anyway
	if (option == OPTION_WATCH_NONE) {
		return true;
	}

	if (option <= OPTION_WATCH_WME_DETAIL) {
		// Detail arguments 
		if ((argInt < 0) || (argInt > 2)) {
			return m_Error.SetError(CLIError::kIntegerOutOfRange);
		}

		// First, shift argInt if necessary
		if (option == OPTION_WATCH_WME_DETAIL) {
			argInt <<= 2;
		}

		// Second set the bits to 1
		values |= option;

		// Third, create a value to and
		argInt |= ~option;

		// Finally, and it with the values
		values &= argInt;

	} else {
		// Switch arguments
		if ((argInt < 0) || (argInt > 1)) {
			return m_Error.SetError(CLIError::kIntegerOutOfRange);
		}

		if (argInt) {
			// Turn on option
			values |= option;
		} else {
			// Turn off option
			values &= ~option;
		}
	}

	return true;
}

bool CommandLineInterface::DoWatch(gSKI::IAgent* pAgent, const unsigned int options, unsigned int values) {
	// Need agent pointer for function calls
	if (!RequireAgent(pAgent)) return false;
	if (!RequireKernel()) return false;

	// Attain the evil back door of doom, even though we aren't the TgD, because we'll probably need it
	gSKI::EvilBackDoor::ITgDWorkArounds* pKernelHack = m_pKernel->getWorkaroundObject();

	if (!options) {
		// Print watch settings.
		const long* pSysparams = pKernelHack->GetSysparams(pAgent);
		char buf[kMinBufferSize];
		char buf2[kMinBufferSize];

		if (m_RawOutput) {
			AppendToResult("Current watch settings:\n  Decisions:  ");
			AppendToResult(pSysparams[TRACE_CONTEXT_DECISIONS_SYSPARAM] ? "on" : "off");
			AppendToResult("\n  Phases:  ");
			AppendToResult(pSysparams[TRACE_PHASES_SYSPARAM] ? "on" : "off");
			AppendToResult("\n  Production firings/retractions\n    default productions:  ");
			AppendToResult(pSysparams[TRACE_FIRINGS_OF_DEFAULT_PRODS_SYSPARAM] ? "on" : "off");
			AppendToResult("\n    user productions:  ");
			AppendToResult(pSysparams[TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM] ? "on" : "off");
			AppendToResult("\n    chunks:  ");
			AppendToResult(pSysparams[TRACE_FIRINGS_OF_CHUNKS_SYSPARAM] ? "on" : "off");
			AppendToResult("\n    justifications:  ");
			AppendToResult(pSysparams[TRACE_FIRINGS_OF_JUSTIFICATIONS_SYSPARAM] ? "on" : "off");
			AppendToResult("\n    WME detail level:  ");
			AppendToResult(Int2String(pSysparams[TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM], buf, kMinBufferSize));
			AppendToResult("\n  Working memory changes:  ");
			AppendToResult(pSysparams[TRACE_WM_CHANGES_SYSPARAM] ? "on" : "off");
			AppendToResult("\n  Preferences generated by firings/retractions:  ");
			AppendToResult(pSysparams[TRACE_FIRINGS_PREFERENCES_SYSPARAM] ? "on" : "off");
			AppendToResult("\n  Learning:  ");
			AppendToResult(Int2String(pSysparams[TRACE_CHUNKS_SYSPARAM], buf, kMinBufferSize));
			AppendToResult("\n  Backtracing:  ");
			AppendToResult(pSysparams[TRACE_BACKTRACING_SYSPARAM] ? "on" : "off");
			AppendToResult("\n  Loading:  ");
			AppendToResult(pSysparams[TRACE_LOADING_SYSPARAM] ? "on" : "off");
			AppendToResult("\n");
		} else {
			AppendArgTag(Int2String(TRACE_CONTEXT_DECISIONS_SYSPARAM, buf, kMinBufferSize), sml_Names::kTypeBoolean, 
				pSysparams[TRACE_CONTEXT_DECISIONS_SYSPARAM] ? sml_Names::kTrue : sml_Names::kFalse);

			AppendArgTag(Int2String(TRACE_PHASES_SYSPARAM, buf, kMinBufferSize), sml_Names::kTypeBoolean, 
				pSysparams[TRACE_PHASES_SYSPARAM] ? sml_Names::kTrue : sml_Names::kFalse);

			AppendArgTag(Int2String(TRACE_FIRINGS_OF_DEFAULT_PRODS_SYSPARAM, buf, kMinBufferSize), sml_Names::kTypeBoolean, 
				pSysparams[TRACE_FIRINGS_OF_DEFAULT_PRODS_SYSPARAM] ? sml_Names::kTrue : sml_Names::kFalse);

			AppendArgTag(Int2String(TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM, buf, kMinBufferSize), sml_Names::kTypeBoolean, 
				pSysparams[TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM] ? sml_Names::kTrue : sml_Names::kFalse);

			AppendArgTag(Int2String(TRACE_FIRINGS_OF_CHUNKS_SYSPARAM, buf, kMinBufferSize), sml_Names::kTypeBoolean, 
				pSysparams[TRACE_FIRINGS_OF_CHUNKS_SYSPARAM] ? sml_Names::kTrue : sml_Names::kFalse);

			AppendArgTag(Int2String(TRACE_FIRINGS_OF_JUSTIFICATIONS_SYSPARAM, buf, kMinBufferSize), sml_Names::kTypeBoolean, 
				pSysparams[TRACE_FIRINGS_OF_JUSTIFICATIONS_SYSPARAM] ? sml_Names::kTrue : sml_Names::kFalse);

			AppendArgTag(Int2String(TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM, buf, kMinBufferSize), sml_Names::kTypeInt, 
				Int2String(pSysparams[TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM], buf2, kMinBufferSize));

			AppendArgTag(Int2String(TRACE_WM_CHANGES_SYSPARAM, buf, kMinBufferSize), sml_Names::kTypeBoolean, 
				pSysparams[TRACE_WM_CHANGES_SYSPARAM] ? sml_Names::kTrue : sml_Names::kFalse);

			AppendArgTag(Int2String(TRACE_FIRINGS_PREFERENCES_SYSPARAM, buf, kMinBufferSize), sml_Names::kTypeBoolean, 
				pSysparams[TRACE_FIRINGS_PREFERENCES_SYSPARAM] ? sml_Names::kTrue : sml_Names::kFalse);

			AppendArgTag(Int2String(TRACE_CHUNKS_SYSPARAM, buf, kMinBufferSize), sml_Names::kTypeInt, 
				Int2String(pSysparams[TRACE_CHUNKS_SYSPARAM], buf2, kMinBufferSize));

			AppendArgTag(Int2String(TRACE_BACKTRACING_SYSPARAM, buf, kMinBufferSize), sml_Names::kTypeBoolean, 
				pSysparams[TRACE_BACKTRACING_SYSPARAM] ? sml_Names::kTrue : sml_Names::kFalse);

			AppendArgTag(Int2String(TRACE_LOADING_SYSPARAM, buf, kMinBufferSize), sml_Names::kTypeBoolean, 
				pSysparams[TRACE_LOADING_SYSPARAM] ? sml_Names::kTrue : sml_Names::kFalse);
		}

		return true;
	}

	// If option is watch none, set values all off
	if (options == OPTION_WATCH_NONE) {
		values = 0;
	}

	// Next, do we have a watch level? (none flag will set this to zero)
	// No watch level and no none flags, that means we have to do the rest
	if (options & OPTION_WATCH_BACKTRACING) {
		pKernelHack->SetSysparam(pAgent, TRACE_BACKTRACING_SYSPARAM, values & OPTION_WATCH_BACKTRACING);
	}

	if (options & OPTION_WATCH_CHUNKS) {
		pKernelHack->SetSysparam(pAgent, TRACE_FIRINGS_OF_CHUNKS_SYSPARAM, values & OPTION_WATCH_CHUNKS);
	}

	if (options & OPTION_WATCH_DECISIONS) {
		pKernelHack->SetSysparam(pAgent, TRACE_CONTEXT_DECISIONS_SYSPARAM, values & OPTION_WATCH_DECISIONS);
	}

	if (options & OPTION_WATCH_DEFAULT_PRODUCTIONS) {
		pKernelHack->SetSysparam(pAgent, TRACE_FIRINGS_OF_DEFAULT_PRODS_SYSPARAM, values & OPTION_WATCH_DEFAULT_PRODUCTIONS);
	}

	if (options & OPTION_WATCH_INDIFFERENT_SELECTION) {
		pKernelHack->SetSysparam(pAgent, TRACE_INDIFFERENT_SYSPARAM, values & OPTION_WATCH_INDIFFERENT_SELECTION);
	}

	if (options & OPTION_WATCH_JUSTIFICATIONS) {
		pKernelHack->SetSysparam(pAgent, TRACE_FIRINGS_OF_JUSTIFICATIONS_SYSPARAM, values & OPTION_WATCH_JUSTIFICATIONS);
	}

	if (options & OPTION_WATCH_LOADING) {
		pKernelHack->SetSysparam(pAgent, TRACE_LOADING_SYSPARAM, values & OPTION_WATCH_LOADING);
	}

	if (options & OPTION_WATCH_PHASES) {
		pKernelHack->SetSysparam(pAgent, TRACE_PHASES_SYSPARAM, values & OPTION_WATCH_PHASES);
	}

	if (options & OPTION_WATCH_PRODUCTIONS) {
		pKernelHack->SetSysparam(pAgent, TRACE_FIRINGS_OF_DEFAULT_PRODS_SYSPARAM, values & OPTION_WATCH_PRODUCTIONS);
		pKernelHack->SetSysparam(pAgent, TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM, values & OPTION_WATCH_PRODUCTIONS);
		pKernelHack->SetSysparam(pAgent, TRACE_FIRINGS_OF_CHUNKS_SYSPARAM, values & OPTION_WATCH_PRODUCTIONS);
		pKernelHack->SetSysparam(pAgent, TRACE_FIRINGS_OF_JUSTIFICATIONS_SYSPARAM, values & OPTION_WATCH_PRODUCTIONS);
	}

	if (options & OPTION_WATCH_PREFERENCES) {
		pKernelHack->SetSysparam(pAgent, TRACE_FIRINGS_PREFERENCES_SYSPARAM, values & OPTION_WATCH_PREFERENCES);
	}

	if (options & OPTION_WATCH_USER_PRODUCTIONS) {
		pKernelHack->SetSysparam(pAgent, TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM, values & OPTION_WATCH_USER_PRODUCTIONS);
	}

	if (options & OPTION_WATCH_WMES) {
		pKernelHack->SetSysparam(pAgent, TRACE_WM_CHANGES_SYSPARAM, values & OPTION_WATCH_WMES);
	}

	if (options & OPTION_WATCH_LEARNING) {
		switch (values & OPTION_WATCH_LEARNING) {
			case 0:
			default:
				pKernelHack->SetSysparam(pAgent, TRACE_CHUNK_NAMES_SYSPARAM, false);
				pKernelHack->SetSysparam(pAgent, TRACE_CHUNKS_SYSPARAM, false);
				pKernelHack->SetSysparam(pAgent, TRACE_JUSTIFICATION_NAMES_SYSPARAM, false);
				pKernelHack->SetSysparam(pAgent, TRACE_JUSTIFICATIONS_SYSPARAM, false);
				break;
			case 1:
				pKernelHack->SetSysparam(pAgent, TRACE_CHUNK_NAMES_SYSPARAM, true);
				pKernelHack->SetSysparam(pAgent, TRACE_CHUNKS_SYSPARAM, false);
				pKernelHack->SetSysparam(pAgent, TRACE_JUSTIFICATION_NAMES_SYSPARAM, true);
				pKernelHack->SetSysparam(pAgent, TRACE_JUSTIFICATIONS_SYSPARAM, false);
				break;
			case 2:
				pKernelHack->SetSysparam(pAgent, TRACE_CHUNK_NAMES_SYSPARAM, true);
				pKernelHack->SetSysparam(pAgent, TRACE_CHUNKS_SYSPARAM, true);
				pKernelHack->SetSysparam(pAgent, TRACE_JUSTIFICATION_NAMES_SYSPARAM, true);
				pKernelHack->SetSysparam(pAgent, TRACE_JUSTIFICATIONS_SYSPARAM, true);
				break;
		}
	}

	if (options & OPTION_WATCH_WME_DETAIL) {
		switch ((values & OPTION_WATCH_WME_DETAIL) >> 2) {
			case 0:
			default:
				pKernelHack->SetSysparam(pAgent, TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM, NONE_WME_TRACE);
				break;
			case 1:
				pKernelHack->SetSysparam(pAgent, TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM, TIMETAG_WME_TRACE);
				break;
			case 2:
				pKernelHack->SetSysparam(pAgent, TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM, FULL_WME_TRACE);
				break;
		}
	}

	return true;
}
