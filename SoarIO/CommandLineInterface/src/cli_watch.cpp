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
		{"backtracing",				2, 0, 'b'},
		{"chunks",		            2, 0, 'c'},
		{"decisions",				2, 0, 'd'},
		{"default-productions",		2, 0, 'D'},
		{"fullwmes",				0, 0, 'f'},
		{"indifferent-selection",	2, 0, 'i'},
		{"justifications",			2, 0, 'j'},
		{"learning",				1, 0, 'L'},
		{"level",					1, 0, 'l'},
		{"none",					0, 0, 'N'},
		{"nowmes",					0, 0, 'n'},
		{"phases",					2, 0, 'p'},
		{"productions",				2, 0, 'P'},
		{"preferences",				2, 0, 'r'},
		{"timetags",				0, 0, 't'},
		{"user-productions",		2, 0, 'u'},
		{"wmes",					2, 0, 'w'},
		{0, 0, 0, 0}
	};

	GetOpt::optind = 0;
	GetOpt::opterr = 0;

	int options = 0;
	int settings = 0;
	int learnSetting = 0;
	int wmeSetting = 0;

	for (;;) {
		int option = m_pGetOpt->GetOpt_Long(argv, ":b::c::d::D::fi::j::L:l:Nnp::P::r::tu::w::", longOptions, 0);
		if (option == -1) break;

		switch (option) {
			case 'b'://backtracing
				options |= OPTION_WATCH_BACKTRACING;
				if (GetOpt::optarg) {
					if (!CheckOptargRemoveOrZero()) return false; //error, code set in CheckOptargRemoveOrZero
					settings &= ~OPTION_WATCH_BACKTRACING;
				} else {
					settings |= OPTION_WATCH_BACKTRACING;
				}
				break;

			case 'c'://chunks
				options |= OPTION_WATCH_CHUNKS;
				if (GetOpt::optarg) {
					if (!CheckOptargRemoveOrZero()) return false; //error, code set in CheckOptargRemoveOrZero
					settings &= ~OPTION_WATCH_CHUNKS;
				} else {
					settings |= OPTION_WATCH_CHUNKS;
				}
				break;

			case 'd'://decisions
				options |= OPTION_WATCH_DECISIONS;
				if (GetOpt::optarg) {
					if (!CheckOptargRemoveOrZero()) return false; //error, code set in CheckOptargRemoveOrZero
					settings &= ~OPTION_WATCH_DECISIONS;
				} else {
					settings |= OPTION_WATCH_DECISIONS;
				}
				break;

			case 'D'://default-productions
				options |= OPTION_WATCH_DEFAULT;
				if (GetOpt::optarg) {
					if (!CheckOptargRemoveOrZero()) return false; //error, code set in CheckOptargRemoveOrZero
					settings &= ~OPTION_WATCH_DEFAULT;
				} else {
					settings |= OPTION_WATCH_DEFAULT;
				}
				break;

			case 'f'://fullwmes
				options |= OPTION_WATCH_WME_DETAIL;
				wmeSetting = 2;
				break;

			case 'i'://indifferent-selection
				options |= OPTION_WATCH_INDIFFERENT;
				if (GetOpt::optarg) {
					if (!CheckOptargRemoveOrZero()) return false; //error, code set in CheckOptargRemoveOrZero
					settings &= ~OPTION_WATCH_INDIFFERENT;
				} else {
					settings |= OPTION_WATCH_INDIFFERENT;
				}
				break;

			case 'j'://justifications
				options |= OPTION_WATCH_JUSTIFICATIONS;
				if (GetOpt::optarg) {
					if (!CheckOptargRemoveOrZero()) return false; //error, code set in CheckOptargRemoveOrZero
					settings &= ~OPTION_WATCH_JUSTIFICATIONS;
				} else {
					settings |= OPTION_WATCH_JUSTIFICATIONS;
				}
				break;

			case 'L'://learning
				options |= OPTION_WATCH_LEARNING;
				learnSetting = ParseLearningOptarg();
				if (learnSetting == -1) return false; //error, code set in ParseLearningOptarg
				break;

			case 'l'://level
				{
					// All of these are going to change
					options = OPTION_WATCH_PREFERENCES | OPTION_WATCH_WMES | OPTION_WATCH_DEFAULT 
						| OPTION_WATCH_USER | OPTION_WATCH_CHUNKS | OPTION_WATCH_JUSTIFICATIONS
						| OPTION_WATCH_PHASES | OPTION_WATCH_DECISIONS;

					// Start with all off, turn on as appropriate
					settings &= ~(OPTION_WATCH_PREFERENCES | OPTION_WATCH_WMES | OPTION_WATCH_DEFAULT 
						| OPTION_WATCH_USER | OPTION_WATCH_CHUNKS | OPTION_WATCH_JUSTIFICATIONS
						| OPTION_WATCH_PHASES | OPTION_WATCH_DECISIONS);

					switch (ParseLevelOptarg()) {
						case 0:// none
							options = OPTION_WATCH_ALL;
							settings = 0;
							learnSetting = 0;
							wmeSetting = 0;
							break;
							
						case 5:// preferences
							settings |= OPTION_WATCH_PREFERENCES;
							// falls through
						case 4:// wmes
							settings |= OPTION_WATCH_WMES;
							// falls through
						case 3:// productions (default, user, chunks, justifications)
							settings |= OPTION_WATCH_DEFAULT;
							settings |= OPTION_WATCH_USER;
							settings |= OPTION_WATCH_CHUNKS;
							settings |= OPTION_WATCH_JUSTIFICATIONS;
							// falls through
						case 2:// phases
							settings |= OPTION_WATCH_PHASES;
							// falls through
						case 1:// decisions
							settings |= OPTION_WATCH_DECISIONS;
							break;

						default:
							return false; //error, code set in ParseLevelOptarg

					}
				}
				break;

			case 'N'://none
				options = OPTION_WATCH_ALL;
				settings = 0;
				learnSetting = 0;
				wmeSetting = 0;
				break;

			case 'n'://nowmes
				options |= OPTION_WATCH_WME_DETAIL;
				wmeSetting = 0;
				break;

			case 'p'://phases
				options |= OPTION_WATCH_PHASES;
				if (GetOpt::optarg) {
					if (!CheckOptargRemoveOrZero()) return false; //error, code set in CheckOptargRemoveOrZero
					settings &= ~OPTION_WATCH_PHASES;
				} else {
					settings |= OPTION_WATCH_PHASES;
				}
				break;

			case 'P'://productions (default, user, justifications, chunks)
				options |= OPTION_WATCH_DEFAULT;
				options |= OPTION_WATCH_USER;
				options |= OPTION_WATCH_CHUNKS;
				options |= OPTION_WATCH_JUSTIFICATIONS;
				if (GetOpt::optarg) {
					if (!CheckOptargRemoveOrZero()) return false; //error, code set in CheckOptargRemoveOrZero
					settings &= ~OPTION_WATCH_DEFAULT;
					settings &= ~OPTION_WATCH_USER;
					settings &= ~OPTION_WATCH_CHUNKS;
					settings &= ~OPTION_WATCH_JUSTIFICATIONS;
				} else {
					settings |= OPTION_WATCH_DEFAULT;
					settings |= OPTION_WATCH_USER;
					settings |= OPTION_WATCH_CHUNKS;
					settings |= OPTION_WATCH_JUSTIFICATIONS;
				}
				break;

			case 'r'://preferences
				options |= OPTION_WATCH_PREFERENCES;
				if (GetOpt::optarg) {
					if (!CheckOptargRemoveOrZero()) return false; //error, code set in CheckOptargRemoveOrZero
					settings &= ~OPTION_WATCH_PREFERENCES;
				} else {
					settings |= OPTION_WATCH_PREFERENCES;
				}
				break;

			case 't'://timetags
				options |= OPTION_WATCH_WME_DETAIL;
				wmeSetting = 1;
				break;

			case 'u'://user-productions
				options |= OPTION_WATCH_USER;
				if (GetOpt::optarg) {
					if (!CheckOptargRemoveOrZero()) return false; //error, code set in CheckOptargRemoveOrZero
					settings &= ~OPTION_WATCH_USER;
				} else {
					settings |= OPTION_WATCH_USER;
				}
				break;
			case 'w'://wmes
				options |= OPTION_WATCH_WMES;
				if (GetOpt::optarg) {
					if (!CheckOptargRemoveOrZero()) return false; //error, code set in CheckOptargRemoveOrZero
					settings &= ~OPTION_WATCH_WMES;
				} else {
					settings |= OPTION_WATCH_WMES;
				}
				break;
			case ':':
				return m_Error.SetError(CLIError::kMissingOptionArg);
			case '?':
				return m_Error.SetError(CLIError::kUnrecognizedOption);
			default:
				return m_Error.SetError(CLIError::kGetOptError);
		}
	}

	// No non-option arguments allowed
	if ((unsigned)GetOpt::optind != argv.size()) return m_Error.SetError(CLIError::kTooManyArgs);
	return DoWatch(pAgent, options, settings, wmeSetting, learnSetting);
}

int CommandLineInterface::ParseLevelOptarg() {
	std::string optarg(GetOpt::optarg);
	if (!IsInteger(optarg)) {
		m_Error.SetError(CLIError::kIntegerExpected);
		return -1;
	}
	int level = atoi(optarg.c_str());
	if (level < 0) {
		m_Error.SetError(CLIError::kIntegerMustBeNonNegative);
		return -1;
	}
	if (level > 5) {
		m_Error.SetError(CLIError::kIntegerOutOfRange);
		return -1;
	}
	return level;
}

int CommandLineInterface::ParseLearningOptarg() {
	std::string optarg(GetOpt::optarg);
	if (optarg == "noprint" || optarg == "0") {
		return 0;
	}
	if (optarg == "print" || optarg == "1") {
		return 1;
	}
	if (optarg == "fullprint" || optarg == "2") {
		return 2;
	}
	m_Error.SetError(CLIError::kInvalidLearnSetting);
	return -1;
}

bool CommandLineInterface::CheckOptargRemoveOrZero() {
	std::string optarg(GetOpt::optarg);
	if (optarg == "remove" || optarg == "0") return true;
	return m_Error.SetError(CLIError::kRemoveOrZeroExpected);
}

bool CommandLineInterface::DoWatch(gSKI::IAgent* pAgent, const int options, const int settings, const int wmeSetting, const int learnSetting) {
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
		int learning;
		if (!pSysparams[TRACE_CHUNK_NAMES_SYSPARAM] 
			&& !pSysparams[TRACE_CHUNKS_SYSPARAM] 
			&& !pSysparams[TRACE_JUSTIFICATION_NAMES_SYSPARAM] 
			&& !pSysparams[TRACE_JUSTIFICATIONS_SYSPARAM]) 
		{
			learning = 0;
		} else if (!pSysparams[TRACE_CHUNK_NAMES_SYSPARAM] 
			&& !pSysparams[TRACE_CHUNKS_SYSPARAM] 
			&& !pSysparams[TRACE_JUSTIFICATION_NAMES_SYSPARAM] 
			&& !pSysparams[TRACE_JUSTIFICATIONS_SYSPARAM])
		{
			learning = 1;
		} else {
			learning = 2;
		}


		if (m_RawOutput) {
			AppendToResult("Current watch settings:\n  Decisions:  ");
			AppendToResult(pSysparams[TRACE_CONTEXT_DECISIONS_SYSPARAM] ? "on" : "off");
			AppendToResult("\n  Phases:  ");
			AppendToResult(pSysparams[TRACE_PHASES_SYSPARAM] ? "on" : "off");
			AppendToResult("\n  Default productions:  ");
			AppendToResult(pSysparams[TRACE_FIRINGS_OF_DEFAULT_PRODS_SYSPARAM] ? "on" : "off");
			AppendToResult("\n  User productions:  ");
			AppendToResult(pSysparams[TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM] ? "on" : "off");
			AppendToResult("\n  Chunks:  ");
			AppendToResult(pSysparams[TRACE_FIRINGS_OF_CHUNKS_SYSPARAM] ? "on" : "off");
			AppendToResult("\n  Justifications:  ");
			AppendToResult(pSysparams[TRACE_FIRINGS_OF_JUSTIFICATIONS_SYSPARAM] ? "on" : "off");
			AppendToResult("\n    WME detail level:  ");
			switch (pSysparams[TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM]) {
				default:
					//falls through
				case NONE_WME_TRACE:
					AppendToResult("nowmes (0)");
					break;
				case TIMETAG_WME_TRACE:
					AppendToResult("timetags (1)");
					break;
				case FULL_WME_TRACE:
					AppendToResult("fullwmes (2)");
					break;
			}
			AppendToResult("\n  Working memory changes:  ");
			AppendToResult(pSysparams[TRACE_WM_CHANGES_SYSPARAM] ? "on" : "off");
			AppendToResult("\n  Preferences generated by firings/retractions:  ");
			AppendToResult(pSysparams[TRACE_FIRINGS_PREFERENCES_SYSPARAM] ? "on" : "off");
			AppendToResult("\n  Learning:  ");
			switch (learning) {
				default:
					//falls through
				case 0:
					AppendToResult("noprint (0)");
					break;
				case 1:
					AppendToResult("print (1)");
					break;
				case 2:
					AppendToResult("fullprint (2)");
					break;
			}
			AppendToResult("\n  Backtracing:  ");
			AppendToResult(pSysparams[TRACE_BACKTRACING_SYSPARAM] ? "on" : "off");
			AppendToResult("\n  Indifferent selection:  ");
			AppendToResult(pSysparams[TRACE_INDIFFERENT_SYSPARAM] ? "on" : "off");
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

			// Subtract one here because the kernel constants (e.g. TIMETAG_WME_TRACE) are one plus the number we use
			AppendArgTag(Int2String(TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM, buf, kMinBufferSize), sml_Names::kTypeInt, 
				Int2String(pSysparams[TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM]-1, buf2, kMinBufferSize));

			AppendArgTag(Int2String(TRACE_WM_CHANGES_SYSPARAM, buf, kMinBufferSize), sml_Names::kTypeBoolean, 
				pSysparams[TRACE_WM_CHANGES_SYSPARAM] ? sml_Names::kTrue : sml_Names::kFalse);

			AppendArgTag(Int2String(TRACE_FIRINGS_PREFERENCES_SYSPARAM, buf, kMinBufferSize), sml_Names::kTypeBoolean, 
				pSysparams[TRACE_FIRINGS_PREFERENCES_SYSPARAM] ? sml_Names::kTrue : sml_Names::kFalse);

			AppendArgTag(Int2String(TRACE_CHUNKS_SYSPARAM, buf, kMinBufferSize), sml_Names::kTypeInt, 
				Int2String(learning, buf2, kMinBufferSize));

			AppendArgTag(Int2String(TRACE_BACKTRACING_SYSPARAM, buf, kMinBufferSize), sml_Names::kTypeBoolean, 
				pSysparams[TRACE_BACKTRACING_SYSPARAM] ? sml_Names::kTrue : sml_Names::kFalse);

			AppendArgTag(Int2String(TRACE_INDIFFERENT_SYSPARAM, buf, kMinBufferSize), sml_Names::kTypeBoolean, 
				pSysparams[TRACE_INDIFFERENT_SYSPARAM] ? sml_Names::kTrue : sml_Names::kFalse);
		}

		return true;
	}

	// No watch level and no none flags, that means we have to do the rest
	if (options & OPTION_WATCH_BACKTRACING) {
		pKernelHack->SetSysparam(pAgent, TRACE_BACKTRACING_SYSPARAM, settings & OPTION_WATCH_BACKTRACING);
	}

	if (options & OPTION_WATCH_CHUNKS) {
		pKernelHack->SetSysparam(pAgent, TRACE_FIRINGS_OF_CHUNKS_SYSPARAM, settings & OPTION_WATCH_CHUNKS);
	}

	if (options & OPTION_WATCH_DECISIONS) {
		pKernelHack->SetSysparam(pAgent, TRACE_CONTEXT_DECISIONS_SYSPARAM, settings & OPTION_WATCH_DECISIONS);
	}

	if (options & OPTION_WATCH_DEFAULT) {
		pKernelHack->SetSysparam(pAgent, TRACE_FIRINGS_OF_DEFAULT_PRODS_SYSPARAM, settings & OPTION_WATCH_DEFAULT);
	}

	if (options & OPTION_WATCH_INDIFFERENT) {
		pKernelHack->SetSysparam(pAgent, TRACE_INDIFFERENT_SYSPARAM, settings & OPTION_WATCH_INDIFFERENT);
	}

	if (options & OPTION_WATCH_JUSTIFICATIONS) {
		pKernelHack->SetSysparam(pAgent, TRACE_FIRINGS_OF_JUSTIFICATIONS_SYSPARAM, settings & OPTION_WATCH_JUSTIFICATIONS);
	}

	if (options & OPTION_WATCH_PHASES) {
		pKernelHack->SetSysparam(pAgent, TRACE_PHASES_SYSPARAM, settings & OPTION_WATCH_PHASES);
	}

	if (options & OPTION_WATCH_PREFERENCES) {
		pKernelHack->SetSysparam(pAgent, TRACE_FIRINGS_PREFERENCES_SYSPARAM, settings & OPTION_WATCH_PREFERENCES);
	}

	if (options & OPTION_WATCH_USER) {
		pKernelHack->SetSysparam(pAgent, TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM, settings & OPTION_WATCH_USER);
	}

	if (options & OPTION_WATCH_WMES) {
		pKernelHack->SetSysparam(pAgent, TRACE_WM_CHANGES_SYSPARAM, settings & OPTION_WATCH_WMES);
	}

	if (options & OPTION_WATCH_LEARNING) {
		switch (learnSetting) {
			default:
				// falls through
			case 0:
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
		switch (wmeSetting) {
			default:
				// falls through
			case 0:
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
