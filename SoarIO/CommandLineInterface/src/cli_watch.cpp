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

	WatchBitset options(0);
	WatchBitset settings(0);
	int learnSetting = 0;
	int wmeSetting = 0;

	for (;;) {
		int option = m_pGetOpt->GetOpt_Long(argv, ":b::c::d::D::fi::j::L:l:Nnp::P::r::tu::w::", longOptions, 0);
		if (option == -1) break;

		switch (option) {
			case 'b'://backtracing
				options.set(WATCH_BACKTRACING);
				if (m_pGetOpt->GetOptArg()) {
					if (!CheckOptargRemoveOrZero()) return false; //error, code set in CheckOptargRemoveOrZero
					settings.reset(WATCH_BACKTRACING);
				} else {
					settings.set(WATCH_BACKTRACING);
				}
				break;

			case 'c'://chunks
				options.set(WATCH_CHUNKS);
				if (m_pGetOpt->GetOptArg()) {
					if (!CheckOptargRemoveOrZero()) return false; //error, code set in CheckOptargRemoveOrZero
					settings.reset(WATCH_CHUNKS);
				} else {
					settings.set(WATCH_CHUNKS);
				}
				break;

			case 'd'://decisions
				options.set(WATCH_DECISIONS);
				if (m_pGetOpt->GetOptArg()) {
					if (!CheckOptargRemoveOrZero()) return false; //error, code set in CheckOptargRemoveOrZero
					settings.reset(WATCH_DECISIONS);
				} else {
					settings.set(WATCH_DECISIONS);
				}
				break;

			case 'D'://default-productions
				options.set(WATCH_DEFAULT);
				if (m_pGetOpt->GetOptArg()) {
					if (!CheckOptargRemoveOrZero()) return false; //error, code set in CheckOptargRemoveOrZero
					settings.reset(WATCH_DEFAULT);
				} else {
					settings.set(WATCH_DEFAULT);
				}
				break;

			case 'f'://fullwmes
				options.set(WATCH_WME_DETAIL);
				wmeSetting = 2;
				break;

			case 'i'://indifferent-selection
				options.set(WATCH_INDIFFERENT);
				if (m_pGetOpt->GetOptArg()) {
					if (!CheckOptargRemoveOrZero()) return false; //error, code set in CheckOptargRemoveOrZero
					settings.reset(WATCH_INDIFFERENT);
				} else {
					settings.set(WATCH_INDIFFERENT);
				}
				break;

			case 'j'://justifications
				options.set(WATCH_JUSTIFICATIONS);
				if (m_pGetOpt->GetOptArg()) {
					if (!CheckOptargRemoveOrZero()) return false; //error, code set in CheckOptargRemoveOrZero
					settings.reset(WATCH_JUSTIFICATIONS);
				} else {
					settings.set(WATCH_JUSTIFICATIONS);
				}
				break;

			case 'L'://learning
				options.set(WATCH_LEARNING);
				learnSetting = ParseLearningOptarg();
				if (learnSetting == -1) return false; //error, code set in ParseLearningOptarg
				break;

			case 'l'://level
				if (!ProcessWatchLevelSettings(ParseLevelOptarg(), options, settings, wmeSetting, learnSetting)) return false; //error, code set in ProcessWatchLevel
				break;

			case 'N'://none
				options.reset();  // clear every bit
				options.flip();   // flip every bit
				settings.reset(); // clear every bit
				learnSetting = 0;
				wmeSetting = 0;
				break;

			case 'n'://nowmes
				options.set(WATCH_WME_DETAIL);
				wmeSetting = 0;
				break;

			case 'p'://phases
				options.set(WATCH_PHASES);
				if (m_pGetOpt->GetOptArg()) {
					if (!CheckOptargRemoveOrZero()) return false; //error, code set in CheckOptargRemoveOrZero
					settings.reset(WATCH_PHASES);
				} else {
					settings.set(WATCH_PHASES);
				}
				break;

			case 'P'://productions (default, user, justifications, chunks)
				options.set(WATCH_DEFAULT);
				options.set(WATCH_USER);
				options.set(WATCH_CHUNKS);
				options.set(WATCH_JUSTIFICATIONS);
				if (m_pGetOpt->GetOptArg()) {
					if (!CheckOptargRemoveOrZero()) return false; //error, code set in CheckOptargRemoveOrZero
					settings.reset(WATCH_DEFAULT);
					settings.reset(WATCH_USER);
					settings.reset(WATCH_CHUNKS);
					settings.reset(WATCH_JUSTIFICATIONS);
				} else {
					settings.set(WATCH_DEFAULT);
					settings.set(WATCH_USER);
					settings.set(WATCH_CHUNKS);
					settings.set(WATCH_JUSTIFICATIONS);
				}
				break;

			case 'r'://preferences
				options.set(WATCH_PREFERENCES);
				if (m_pGetOpt->GetOptArg()) {
					if (!CheckOptargRemoveOrZero()) return false; //error, code set in CheckOptargRemoveOrZero
					settings.reset(WATCH_PREFERENCES);
				} else {
					settings.set(WATCH_PREFERENCES);
				}
				break;

			case 't'://timetags
				options.set(WATCH_WME_DETAIL);
				wmeSetting = 1;
				break;

			case 'u'://user-productions
				options.set(WATCH_USER);
				if (m_pGetOpt->GetOptArg()) {
					if (!CheckOptargRemoveOrZero()) return false; //error, code set in CheckOptargRemoveOrZero
					settings.reset(WATCH_USER);
				} else {
					settings.set(WATCH_USER);
				}
				break;
			case 'w'://wmes
				options.set(WATCH_WMES);
				if (m_pGetOpt->GetOptArg()) {
					if (!CheckOptargRemoveOrZero()) return false; //error, code set in CheckOptargRemoveOrZero
					settings.reset(WATCH_WMES);
				} else {
					settings.set(WATCH_WMES);
				}
				break;
			case ':':
				return SetError(CLIError::kMissingOptionArg);
			case '?':
				return SetError(CLIError::kUnrecognizedOption);
			default:
				return SetError(CLIError::kGetOptError);
		}
	}

	if (m_pGetOpt->GetAdditionalArgCount() > 1) return SetError(CLIError::kTooManyArgs);

	// Allow watch level by itself
	if (m_pGetOpt->GetAdditionalArgCount() == 1) {
		int optind = m_pGetOpt->GetOptind();
		if (!IsInteger(argv[optind])) return SetError(CLIError::kIntegerExpected);
		if (!ProcessWatchLevelSettings(atoi(argv[optind].c_str()), options, settings, wmeSetting, learnSetting)) return false; //error, code set in ProcessWatchLevel
	}

	return DoWatch(pAgent, options, settings, wmeSetting, learnSetting);
}

bool CommandLineInterface::ProcessWatchLevelSettings(const int level, WatchBitset& options, WatchBitset& settings, int& wmeSetting, int& learnSetting) {

	if (level < 0) return SetError(CLIError::kIntegerMustBeNonNegative);
	if (level > 5) return SetError(CLIError::kIntegerOutOfRange);

	// All of these are going to change
	options.set(WATCH_PREFERENCES);
	options.set(WATCH_WMES);
	options.set(WATCH_DEFAULT);
	options.set(WATCH_USER);
	options.set(WATCH_CHUNKS);
	options.set(WATCH_JUSTIFICATIONS);
	options.set(WATCH_PHASES);
	options.set(WATCH_DECISIONS);

	// Start with all off, turn on as appropriate
	settings.reset(WATCH_PREFERENCES);
	settings.reset(WATCH_WMES);
	settings.reset(WATCH_DEFAULT);
	settings.reset(WATCH_USER);
	settings.reset(WATCH_CHUNKS);
	settings.reset(WATCH_JUSTIFICATIONS);
	settings.reset(WATCH_PHASES);
	settings.reset(WATCH_DECISIONS);

	switch (level) {
		case 0:// none
			options.reset();
			options.flip();
			settings.reset();
			learnSetting = 0;
			wmeSetting = 0;
			break;
			
		case 5:// preferences
			settings.set(WATCH_PREFERENCES);
			// falls through
		case 4:// wmes
			settings.set(WATCH_WMES);
			// falls through
		case 3:// productions (default, user, chunks, justifications)
			settings.set(WATCH_DEFAULT);
			settings.set(WATCH_USER);
			settings.set(WATCH_CHUNKS);
			settings.set(WATCH_JUSTIFICATIONS);
			// falls through
		case 2:// phases
			settings.set(WATCH_PHASES);
			// falls through
		case 1:// decisions
			settings.set(WATCH_DECISIONS);
			break;
	}
	return true;
}
int CommandLineInterface::ParseLevelOptarg() {
	std::string optarg(m_pGetOpt->GetOptArg());
	if (!IsInteger(optarg)) {
		SetError(CLIError::kIntegerExpected);
		return -1;
	}
	return atoi(optarg.c_str());
}

int CommandLineInterface::ParseLearningOptarg() {
	std::string optarg(m_pGetOpt->GetOptArg());
	if (optarg == "noprint"   || optarg == "0") return 0;
	if (optarg == "print"     || optarg == "1") return 1;
	if (optarg == "fullprint" || optarg == "2") return 2;

	SetError(CLIError::kInvalidLearnSetting);
	return -1;
}

bool CommandLineInterface::CheckOptargRemoveOrZero() {
	std::string optarg(m_pGetOpt->GetOptArg());
	if (optarg == "remove" || optarg == "0") return true;
	return SetError(CLIError::kRemoveOrZeroExpected);
}

/*************************************************************
* @brief watch command
* @param pAgent The pointer to the gSKI agent interface
* @param options Options for the watch command, see cli_CommandData.h
* @param settings Settings for the watch command, if a flag (option) is set, its setting is set using this (true/on or false/off)
* @param wmeSetting Setting for wme detail, not binary so it has its own arg
* @param learnSetting Setting for learn level, not binary so it has its own arg
*************************************************************/
EXPORT bool CommandLineInterface::DoWatch(gSKI::IAgent* pAgent, const WatchBitset& options, const WatchBitset& settings, const int wmeSetting, const int learnSetting) {
	// Need agent pointer for function calls
	if (!RequireAgent(pAgent)) return false;

	// Attain the evil back door of doom, even though we aren't the TgD, because we'll probably need it
	gSKI::EvilBackDoor::ITgDWorkArounds* pKernelHack = m_pKernel->getWorkaroundObject();

	if (options.none()) {
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
			m_Result << "Current watch settings:\n  Decisions:  " << pSysparams[TRACE_CONTEXT_DECISIONS_SYSPARAM] ? "on" : "off";
			m_Result << "\n  Phases:  " << pSysparams[TRACE_PHASES_SYSPARAM] ? "on" : "off";
			m_Result << "\n  Default productions:  " << pSysparams[TRACE_FIRINGS_OF_DEFAULT_PRODS_SYSPARAM] ? "on" : "off";
			m_Result << "\n  User productions:  " << pSysparams[TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM] ? "on" : "off";
			m_Result << "\n  Chunks:  " << pSysparams[TRACE_FIRINGS_OF_CHUNKS_SYSPARAM] ? "on" : "off";
			m_Result << "\n  Justifications:  " << pSysparams[TRACE_FIRINGS_OF_JUSTIFICATIONS_SYSPARAM] ? "on" : "off";
			m_Result << "\n    WME detail level:  ";
			switch (pSysparams[TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM]) {
				default://falls through					
				case NONE_WME_TRACE:
					m_Result << "nowmes (0)";
					break;
				case TIMETAG_WME_TRACE:
					m_Result << "timetags (1)";
					break;
				case FULL_WME_TRACE:
					m_Result << "fullwmes (2)";
					break;
			}
			m_Result << "\n  Working memory changes:  " << pSysparams[TRACE_WM_CHANGES_SYSPARAM] ? "on" : "off";
			m_Result << "\n  Preferences generated by firings/retractions:  " << pSysparams[TRACE_FIRINGS_PREFERENCES_SYSPARAM] ? "on" : "off";
			m_Result << "\n  Learning:  ";
			switch (learning) {
				default://falls through
				case 0:
					m_Result << "noprint (0)";
					break;
				case 1:
					m_Result << "print (1)";
					break;
				case 2:
					m_Result << "fullprint (2)";
					break;
			}
			m_Result << "\n  Backtracing:  " << pSysparams[TRACE_BACKTRACING_SYSPARAM] ? "on" : "off";
			m_Result << "\n  Indifferent selection:  " << pSysparams[TRACE_INDIFFERENT_SYSPARAM] ? "on" : "off";

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
	if (options.test(WATCH_BACKTRACING)) {
		pKernelHack->SetSysparam(pAgent, TRACE_BACKTRACING_SYSPARAM, settings.test(WATCH_BACKTRACING));
	}

	if (options.test(WATCH_CHUNKS)) {
		pKernelHack->SetSysparam(pAgent, TRACE_FIRINGS_OF_CHUNKS_SYSPARAM, settings.test(WATCH_CHUNKS));
	}

	if (options.test(WATCH_DECISIONS)) {
		pKernelHack->SetSysparam(pAgent, TRACE_CONTEXT_DECISIONS_SYSPARAM, settings.test(WATCH_DECISIONS));
	}

	if (options.test(WATCH_DEFAULT)) {
		pKernelHack->SetSysparam(pAgent, TRACE_FIRINGS_OF_DEFAULT_PRODS_SYSPARAM, settings.test(WATCH_DEFAULT));
	}

	if (options.test(WATCH_INDIFFERENT)) {
		pKernelHack->SetSysparam(pAgent, TRACE_INDIFFERENT_SYSPARAM, settings.test(WATCH_INDIFFERENT));
	}

	if (options.test(WATCH_JUSTIFICATIONS)) {
		pKernelHack->SetSysparam(pAgent, TRACE_FIRINGS_OF_JUSTIFICATIONS_SYSPARAM, settings.test(WATCH_JUSTIFICATIONS));
	}

	if (options.test(WATCH_PHASES)) {
		pKernelHack->SetSysparam(pAgent, TRACE_PHASES_SYSPARAM, settings.test(WATCH_PHASES));
	}

	if (options.test(WATCH_PREFERENCES)) {
		pKernelHack->SetSysparam(pAgent, TRACE_FIRINGS_PREFERENCES_SYSPARAM, settings.test(WATCH_PREFERENCES));
	}

	if (options.test(WATCH_USER)) {
		pKernelHack->SetSysparam(pAgent, TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM, settings.test(WATCH_USER));
	}

	if (options.test(WATCH_WMES)) {
		pKernelHack->SetSysparam(pAgent, TRACE_WM_CHANGES_SYSPARAM, settings.test(WATCH_WMES));
	}

	if (options.test(WATCH_LEARNING)) {
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

	if (options.test(WATCH_WME_DETAIL)) {
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
