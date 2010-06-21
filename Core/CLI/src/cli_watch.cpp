/////////////////////////////////////////////////////////////////
// watch command file.
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

#include "sml_KernelSML.h"
#include "gsysparam.h"
#include "misc.h"
#include "agent.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseWatch(std::vector<std::string>& argv) {
	Options optionsData[] = {
		{'b',"backtracing",				OPTARG_OPTIONAL},
		{'c',"chunks",					OPTARG_OPTIONAL},
		{'d',"decisions",				OPTARG_OPTIONAL},
		{'D',"default-productions",		OPTARG_OPTIONAL},
		{'e',"epmem",					OPTARG_OPTIONAL},
		{'f',"fullwmes",				OPTARG_NONE},
		{'g',"gds",						OPTARG_OPTIONAL},
		{'i',"indifferent-selection",	OPTARG_OPTIONAL},
		{'j',"justifications",			OPTARG_OPTIONAL},
		{'L',"learning",				OPTARG_REQUIRED},
		{'l',"level",					OPTARG_REQUIRED},
		{'N',"none",					OPTARG_NONE},
		{'n',"nowmes",					OPTARG_NONE},
		{'p',"phases",					OPTARG_OPTIONAL},
		{'P',"productions",				OPTARG_OPTIONAL},
		{'r',"preferences",				OPTARG_OPTIONAL},
		{'R',"rl",						OPTARG_OPTIONAL},
		{'s',"smem",					OPTARG_OPTIONAL},
		{'t',"timetags",				OPTARG_NONE},
		{'T',"template",				OPTARG_OPTIONAL},
		{'u',"user-productions",		OPTARG_OPTIONAL},
		{'w',"wmes",					OPTARG_OPTIONAL},
		{'W',"waterfall",				OPTARG_OPTIONAL}, // TODO: document. note: added to watch 5
		{0, 0, OPTARG_NONE}
	};
			 
	WatchBitset options(0);
	WatchBitset settings(0);
	int learnSetting = 0;
	int wmeSetting = 0;

	for (;;) {
		if (!ProcessOptions(argv, optionsData)) return false;
		if (m_Option == -1) break;

		switch (m_Option) {
			case 'b'://backtracing
				options.set(WATCH_BACKTRACING);
				if (m_OptionArgument.size()) {
					if (!CheckOptargRemoveOrZero()) return false; //error, code set in CheckOptargRemoveOrZero
					settings.reset(WATCH_BACKTRACING);
				} else {
					settings.set(WATCH_BACKTRACING);
				}
				break;

			case 'c'://chunks
				options.set(WATCH_CHUNKS);
				if (m_OptionArgument.size()) {
					if (!CheckOptargRemoveOrZero()) return false; //error, code set in CheckOptargRemoveOrZero
					settings.reset(WATCH_CHUNKS);
				} else {
					settings.set(WATCH_CHUNKS);
				}
				break;

			case 'd'://decisions
				options.set(WATCH_DECISIONS);
				if (m_OptionArgument.size()) {
					if (!CheckOptargRemoveOrZero()) return false; //error, code set in CheckOptargRemoveOrZero
					settings.reset(WATCH_DECISIONS);
				} else {
					settings.set(WATCH_DECISIONS);
				}
				break;

			case 'D'://default-productions
				options.set(WATCH_DEFAULT);
				if (m_OptionArgument.size()) {
					if (!CheckOptargRemoveOrZero()) return false; //error, code set in CheckOptargRemoveOrZero
					settings.reset(WATCH_DEFAULT);
				} else {
					settings.set(WATCH_DEFAULT);
				}
				break;
				
			case 'e'://epmem
				options.set(WATCH_EPMEM);
				if (m_OptionArgument.size()) {
					if (!CheckOptargRemoveOrZero()) return false; //error, code set in CheckOptargRemoveOrZero
					settings.reset(WATCH_EPMEM);
				} else {
					settings.set(WATCH_EPMEM);
				}
				break;

			case 'f'://fullwmes
				options.set(WATCH_WME_DETAIL);
				wmeSetting = 2;
				break;

			case 'g'://gds
				options.set(WATCH_GDS);
				if (m_OptionArgument.size()) {
					if (!CheckOptargRemoveOrZero()) return false; //error, code set in CheckOptargRemoveOrZero
					settings.reset(WATCH_GDS);
				} else {
					settings.set(WATCH_GDS);
				}
				break;

			case 'i'://indifferent-selection
				options.set(WATCH_INDIFFERENT);
				if (m_OptionArgument.size()) {
					if (!CheckOptargRemoveOrZero()) return false; //error, code set in CheckOptargRemoveOrZero
					settings.reset(WATCH_INDIFFERENT);
				} else {
					settings.set(WATCH_INDIFFERENT);
				}
				break;

			case 'j'://justifications
				options.set(WATCH_JUSTIFICATIONS);
				if (m_OptionArgument.size()) {
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
				{
					int level = 0;
					if ( !from_string( level, m_OptionArgument ) ) return SetError( CLIError::kIntegerExpected );
					if (!ProcessWatchLevelSettings(level, options, settings, wmeSetting, learnSetting)) return false; //error, code set in ProcessWatchLevel
				}
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
				if (m_OptionArgument.size()) {
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
				if (m_OptionArgument.size()) {
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
				if (m_OptionArgument.size()) {
					if (!CheckOptargRemoveOrZero()) return false; //error, code set in CheckOptargRemoveOrZero
					settings.reset(WATCH_PREFERENCES);
				} else {
					settings.set(WATCH_PREFERENCES);
				}
				break;

			case 'R'://rl
				options.set(WATCH_RL);
				if (m_OptionArgument.size()) {
					if (!CheckOptargRemoveOrZero()) return false; //error, code set in CheckOptargRemoveOrZero
					settings.reset(WATCH_RL);
				} else {
					settings.set(WATCH_RL);
				}
				break;

			case 's'://smem
				options.set(WATCH_SMEM);
				if (m_OptionArgument.size()) {
					if (!CheckOptargRemoveOrZero()) return false; //error, code set in CheckOptargRemoveOrZero
					settings.reset(WATCH_SMEM);
				} else {
					settings.set(WATCH_SMEM);
				}
				break;

			case 't'://timetags
				options.set(WATCH_WME_DETAIL);
				wmeSetting = 1;
				break;

			case 'T'://templates
				options.set(WATCH_TEMPLATES);
				if (m_OptionArgument.size()) {
					if (!CheckOptargRemoveOrZero()) return false; //error, code set in CheckOptargRemoveOrZero
					settings.reset(WATCH_TEMPLATES);
				} else {
					settings.set(WATCH_TEMPLATES);
				}
				break;

			case 'u'://user-productions
				options.set(WATCH_USER);
				if (m_OptionArgument.size()) {
					if (!CheckOptargRemoveOrZero()) return false; //error, code set in CheckOptargRemoveOrZero
					settings.reset(WATCH_USER);
				} else {
					settings.set(WATCH_USER);
				}
				break;
			case 'w'://wmes
				options.set(WATCH_WMES);
				if (m_OptionArgument.size()) {
					if (!CheckOptargRemoveOrZero()) return false; //error, code set in CheckOptargRemoveOrZero
					settings.reset(WATCH_WMES);
				} else {
					settings.set(WATCH_WMES);
				}
				break;
			case 'W'://waterfall
				options.set(WATCH_WATERFALL);
				if (m_OptionArgument.size()) {
					if (!CheckOptargRemoveOrZero()) return false; //error, code set in CheckOptargRemoveOrZero
					settings.reset(WATCH_WATERFALL);
				} else {
					settings.set(WATCH_WATERFALL);
				}
				break;
			default:
				return SetError(CLIError::kGetOptError);
		}
	}

	if (m_NonOptionArguments > 1) {
		SetErrorDetail("Only non option argument allowed is watch level.");
		return SetError(CLIError::kTooManyArgs);
	}

	// Allow watch level by itself
	if (m_NonOptionArguments == 1) {
		int optind = m_Argument - m_NonOptionArguments;
		int level = 0;
		if ( !from_string( level, argv[optind] ) ) return SetError(CLIError::kIntegerExpected);
		if (!ProcessWatchLevelSettings(level, options, settings, wmeSetting, learnSetting)) return false; //error, code set in ProcessWatchLevel
	}

	return DoWatch(options, settings, wmeSetting, learnSetting);
}

bool CommandLineInterface::ProcessWatchLevelSettings(const int level, WatchBitset& options, WatchBitset& settings, int& wmeSetting, int& learnSetting) {

	if (level < 0)  {
		SetErrorDetail("Expected watch level from 0 to 5.");
		return SetError(CLIError::kIntegerMustBeNonNegative);
	}
	if (level > 5) {
		SetErrorDetail("Expected watch level from 0 to 5.");
		return SetError(CLIError::kIntegerOutOfRange);
	}

	// All of these are going to change
	options.set(WATCH_PREFERENCES);
	options.set(WATCH_WMES);
	options.set(WATCH_DEFAULT);
	options.set(WATCH_USER);
	options.set(WATCH_CHUNKS);
	options.set(WATCH_JUSTIFICATIONS);
	options.set(WATCH_TEMPLATES);
	options.set(WATCH_PHASES);
	options.set(WATCH_DECISIONS);
	options.set(WATCH_WATERFALL);
	options.set(WATCH_GDS);

	// Start with all off, turn on as appropriate
	settings.reset(WATCH_PREFERENCES);
	settings.reset(WATCH_WMES);
	settings.reset(WATCH_DEFAULT);
	settings.reset(WATCH_USER);
	settings.reset(WATCH_CHUNKS);
	settings.reset(WATCH_JUSTIFICATIONS);
	settings.reset(WATCH_TEMPLATES);
	settings.reset(WATCH_PHASES);
	settings.reset(WATCH_DECISIONS);
	settings.reset(WATCH_WATERFALL);
	settings.reset(WATCH_GDS);

	switch (level) {
		case 0:// none
			options.reset();
			options.flip();
			settings.reset();
			learnSetting = 0;
			wmeSetting = 0;
			break;
			
		case 5:// preferences, waterfall
			settings.set(WATCH_PREFERENCES);
			settings.set(WATCH_WATERFALL);
			// falls through
		case 4:// wmes
			settings.set(WATCH_WMES);
			// falls through
		case 3:// productions (default, user, chunks, justifications, templates)
			settings.set(WATCH_DEFAULT);
			settings.set(WATCH_USER);
			settings.set(WATCH_CHUNKS);
			settings.set(WATCH_JUSTIFICATIONS);
			settings.set(WATCH_TEMPLATES);
			// falls through
		case 2:// phases, gds
			settings.set(WATCH_PHASES);
			settings.set(WATCH_GDS);
			// falls through
		case 1:// decisions
			settings.set(WATCH_DECISIONS);
			break;
	}
	return true;
}
int CommandLineInterface::ParseLearningOptarg() {
	if (m_OptionArgument == "noprint"   || m_OptionArgument == "0") return 0;
	if (m_OptionArgument == "print"     || m_OptionArgument == "1") return 1;
	if (m_OptionArgument == "fullprint" || m_OptionArgument == "2") return 2;

	SetErrorDetail("Got: " + m_OptionArgument);
	SetError(CLIError::kInvalidLearnSetting);
	return -1;
}

bool CommandLineInterface::CheckOptargRemoveOrZero() {
	if (m_OptionArgument == "remove" || m_OptionArgument == "0") return true;
	SetErrorDetail("Got: " + m_OptionArgument);
	return SetError(CLIError::kRemoveOrZeroExpected);
}

bool CommandLineInterface::DoWatch(const WatchBitset& options, const WatchBitset& settings, const int wmeSetting, const int learnSetting) {
	if (options.none()) {
		// Print watch settings.
		int learning;
		if (!m_pAgentSoar->sysparams[TRACE_CHUNK_NAMES_SYSPARAM] 
			&& !m_pAgentSoar->sysparams[TRACE_CHUNKS_SYSPARAM] 
			&& !m_pAgentSoar->sysparams[TRACE_JUSTIFICATION_NAMES_SYSPARAM] 
			&& !m_pAgentSoar->sysparams[TRACE_JUSTIFICATIONS_SYSPARAM]) 
		{
			learning = 0;
		} else if (m_pAgentSoar->sysparams[TRACE_CHUNK_NAMES_SYSPARAM] 
			&& !m_pAgentSoar->sysparams[TRACE_CHUNKS_SYSPARAM] 
			&& m_pAgentSoar->sysparams[TRACE_JUSTIFICATION_NAMES_SYSPARAM] 
			&& !m_pAgentSoar->sysparams[TRACE_JUSTIFICATIONS_SYSPARAM])
		{
			learning = 1;
		} else {
			learning = 2;
		}


		if (m_RawOutput) {
			m_Result << "Current watch settings:\n  Decisions:  " << (m_pAgentSoar->sysparams[TRACE_CONTEXT_DECISIONS_SYSPARAM] ? "on" : "off");
			m_Result << "\n  Phases:  " << (m_pAgentSoar->sysparams[TRACE_PHASES_SYSPARAM] ? "on" : "off");
			m_Result << "\n  Default productions:  " << (m_pAgentSoar->sysparams[TRACE_FIRINGS_OF_DEFAULT_PRODS_SYSPARAM] ? "on" : "off");
			m_Result << "\n  User productions:  " << (m_pAgentSoar->sysparams[TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM] ? "on" : "off");
			m_Result << "\n  Chunks:  " << (m_pAgentSoar->sysparams[TRACE_FIRINGS_OF_CHUNKS_SYSPARAM] ? "on" : "off");
			m_Result << "\n  Justifications:  " << (m_pAgentSoar->sysparams[TRACE_FIRINGS_OF_JUSTIFICATIONS_SYSPARAM] ? "on" : "off");
			m_Result << "\n  Templates:  " << (m_pAgentSoar->sysparams[TRACE_FIRINGS_OF_TEMPLATES_SYSPARAM] ? "on" : "off");
			m_Result << "\n    WME detail level:  ";
			switch (m_pAgentSoar->sysparams[TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM]) {
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
			m_Result << "\n  Working memory changes:  " << (m_pAgentSoar->sysparams[TRACE_WM_CHANGES_SYSPARAM] ? "on" : "off");
			m_Result << "\n  Preferences generated by firings/retractions:  " << (m_pAgentSoar->sysparams[TRACE_FIRINGS_PREFERENCES_SYSPARAM] ? "on" : "off");
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
			m_Result << "\n  Backtracing:  " << (m_pAgentSoar->sysparams[TRACE_BACKTRACING_SYSPARAM] ? "on" : "off");
			m_Result << "\n  Indifferent selection:  " << (m_pAgentSoar->sysparams[TRACE_INDIFFERENT_SYSPARAM] ? "on" : "off");
			m_Result << "\n  Soar-RL:  " << (m_pAgentSoar->sysparams[TRACE_RL_SYSPARAM] ? "on" : "off");
			m_Result << "\n  Waterfall:  " << (m_pAgentSoar->sysparams[TRACE_WATERFALL_SYSPARAM] ? "on" : "off");
			m_Result << "\n  EpMem:  " << (m_pAgentSoar->sysparams[TRACE_EPMEM_SYSPARAM] ? "on" : "off");
			m_Result << "\n  SMem:  " << (m_pAgentSoar->sysparams[TRACE_SMEM_SYSPARAM] ? "on" : "off");
			m_Result << "\n  GDS:  " << (m_pAgentSoar->sysparams[TRACE_GDS_SYSPARAM] ? "on" : "off");
		} else {
			std::string temp;
			AppendArgTag(sml_Names::kParamWatchDecisions, sml_Names::kTypeBoolean, 
				m_pAgentSoar->sysparams[TRACE_CONTEXT_DECISIONS_SYSPARAM] ? sml_Names::kTrue : sml_Names::kFalse);

			AppendArgTag(sml_Names::kParamWatchPhases, sml_Names::kTypeBoolean, 
				m_pAgentSoar->sysparams[TRACE_PHASES_SYSPARAM] ? sml_Names::kTrue : sml_Names::kFalse);

			AppendArgTag(sml_Names::kParamWatchProductionDefault, sml_Names::kTypeBoolean, 
				m_pAgentSoar->sysparams[TRACE_FIRINGS_OF_DEFAULT_PRODS_SYSPARAM] ? sml_Names::kTrue : sml_Names::kFalse);

			AppendArgTag(sml_Names::kParamWatchProductionUser, sml_Names::kTypeBoolean, 
				m_pAgentSoar->sysparams[TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM] ? sml_Names::kTrue : sml_Names::kFalse);

			AppendArgTag(sml_Names::kParamWatchProductionChunks, sml_Names::kTypeBoolean, 
				m_pAgentSoar->sysparams[TRACE_FIRINGS_OF_CHUNKS_SYSPARAM] ? sml_Names::kTrue : sml_Names::kFalse);

			AppendArgTag(sml_Names::kParamWatchProductionJustifications, sml_Names::kTypeBoolean, 
				m_pAgentSoar->sysparams[TRACE_FIRINGS_OF_JUSTIFICATIONS_SYSPARAM] ? sml_Names::kTrue : sml_Names::kFalse);

			AppendArgTag(sml_Names::kParamWatchProductionTemplates, sml_Names::kTypeBoolean, 
				m_pAgentSoar->sysparams[TRACE_FIRINGS_OF_TEMPLATES_SYSPARAM] ? sml_Names::kTrue : sml_Names::kFalse);

			// Subtract one here because the kernel constants (e.g. TIMETAG_WME_TRACE) are one plus the number we use
			AppendArgTag(sml_Names::kParamWatchWMEDetail, sml_Names::kTypeInt, 
				to_string(m_pAgentSoar->sysparams[TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM]-1, temp));

			AppendArgTag(sml_Names::kParamWatchWorkingMemoryChanges, sml_Names::kTypeBoolean, 
				m_pAgentSoar->sysparams[TRACE_WM_CHANGES_SYSPARAM] ? sml_Names::kTrue : sml_Names::kFalse);

			AppendArgTag(sml_Names::kParamWatchPreferences, sml_Names::kTypeBoolean, 
				m_pAgentSoar->sysparams[TRACE_FIRINGS_PREFERENCES_SYSPARAM] ? sml_Names::kTrue : sml_Names::kFalse);

			AppendArgTag(sml_Names::kParamWatchLearning, sml_Names::kTypeInt, 
				to_string(learning, temp));

			AppendArgTag(sml_Names::kParamWatchBacktracing, sml_Names::kTypeBoolean, 
				m_pAgentSoar->sysparams[TRACE_BACKTRACING_SYSPARAM] ? sml_Names::kTrue : sml_Names::kFalse);

			AppendArgTag(sml_Names::kParamWatchIndifferentSelection, sml_Names::kTypeBoolean, 
				m_pAgentSoar->sysparams[TRACE_INDIFFERENT_SYSPARAM] ? sml_Names::kTrue : sml_Names::kFalse);

			AppendArgTag(sml_Names::kParamWatchRL, sml_Names::kTypeBoolean, 
				m_pAgentSoar->sysparams[TRACE_RL_SYSPARAM] ? sml_Names::kTrue : sml_Names::kFalse);

			AppendArgTag(sml_Names::kParamWatchWaterfall, sml_Names::kTypeBoolean, 
				m_pAgentSoar->sysparams[TRACE_WATERFALL_SYSPARAM] ? sml_Names::kTrue : sml_Names::kFalse);
			
			AppendArgTag(sml_Names::kParamWatchEpMem, sml_Names::kTypeBoolean, 
				m_pAgentSoar->sysparams[TRACE_EPMEM_SYSPARAM] ? sml_Names::kTrue : sml_Names::kFalse);

			AppendArgTag(sml_Names::kParamWatchSMem, sml_Names::kTypeBoolean, 
				m_pAgentSoar->sysparams[TRACE_SMEM_SYSPARAM] ? sml_Names::kTrue : sml_Names::kFalse);

			AppendArgTag(sml_Names::kParamWatchGDS, sml_Names::kTypeBoolean, 
				m_pAgentSoar->sysparams[TRACE_GDS_SYSPARAM] ? sml_Names::kTrue : sml_Names::kFalse);
		}

		return true;
	}

	// No watch level and no none flags, that means we have to do the rest
	if (options.test(WATCH_BACKTRACING)) {
		set_sysparam(m_pAgentSoar, TRACE_BACKTRACING_SYSPARAM, settings.test(WATCH_BACKTRACING));
	}

	if (options.test(WATCH_CHUNKS)) {
		set_sysparam(m_pAgentSoar, TRACE_FIRINGS_OF_CHUNKS_SYSPARAM, settings.test(WATCH_CHUNKS));
	}

	if (options.test(WATCH_DECISIONS)) {
		set_sysparam(m_pAgentSoar, TRACE_CONTEXT_DECISIONS_SYSPARAM, settings.test(WATCH_DECISIONS));
	}

	if (options.test(WATCH_DEFAULT)) {
		set_sysparam(m_pAgentSoar, TRACE_FIRINGS_OF_DEFAULT_PRODS_SYSPARAM, settings.test(WATCH_DEFAULT));
	}

	if (options.test(WATCH_GDS)) {
		set_sysparam(m_pAgentSoar, TRACE_GDS_SYSPARAM, settings.test(WATCH_GDS));
	}

	if (options.test(WATCH_INDIFFERENT)) {
		set_sysparam(m_pAgentSoar, TRACE_INDIFFERENT_SYSPARAM, settings.test(WATCH_INDIFFERENT));
	}

	if (options.test(WATCH_RL)) {
		set_sysparam(m_pAgentSoar, TRACE_RL_SYSPARAM, settings.test(WATCH_RL));
	}
	
	if (options.test(WATCH_EPMEM)) {
		set_sysparam(m_pAgentSoar, TRACE_EPMEM_SYSPARAM, settings.test(WATCH_EPMEM));
	}

	if (options.test(WATCH_JUSTIFICATIONS)) {
		set_sysparam(m_pAgentSoar, TRACE_FIRINGS_OF_JUSTIFICATIONS_SYSPARAM, settings.test(WATCH_JUSTIFICATIONS));
	}

	if (options.test(WATCH_TEMPLATES)) {
		set_sysparam(m_pAgentSoar, TRACE_FIRINGS_OF_TEMPLATES_SYSPARAM, settings.test(WATCH_TEMPLATES));
	}

	if (options.test(WATCH_PHASES)) {
		set_sysparam(m_pAgentSoar, TRACE_PHASES_SYSPARAM, settings.test(WATCH_PHASES));
	}

	if (options.test(WATCH_PREFERENCES)) {
		set_sysparam(m_pAgentSoar, TRACE_FIRINGS_PREFERENCES_SYSPARAM, settings.test(WATCH_PREFERENCES));
	}

	if (options.test(WATCH_SMEM)) {
		set_sysparam(m_pAgentSoar, TRACE_SMEM_SYSPARAM, settings.test(WATCH_SMEM));
	}

	if (options.test(WATCH_USER)) {
		set_sysparam(m_pAgentSoar, TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM, settings.test(WATCH_USER));
	}

	if (options.test(WATCH_WMES)) {
		set_sysparam(m_pAgentSoar, TRACE_WM_CHANGES_SYSPARAM, settings.test(WATCH_WMES));
	}

	if (options.test(WATCH_WATERFALL)) {
		set_sysparam(m_pAgentSoar, TRACE_WATERFALL_SYSPARAM, settings.test(WATCH_WATERFALL));
	}

	if (options.test(WATCH_LEARNING)) {
		switch (learnSetting) {
			default:
				// falls through
			case 0:
				set_sysparam(m_pAgentSoar, TRACE_CHUNK_NAMES_SYSPARAM, false);
				set_sysparam(m_pAgentSoar, TRACE_CHUNKS_SYSPARAM, false);
				set_sysparam(m_pAgentSoar, TRACE_JUSTIFICATION_NAMES_SYSPARAM, false);
				set_sysparam(m_pAgentSoar, TRACE_JUSTIFICATIONS_SYSPARAM, false);
				break;
			case 1:
				set_sysparam(m_pAgentSoar, TRACE_CHUNK_NAMES_SYSPARAM, true);
				set_sysparam(m_pAgentSoar, TRACE_CHUNKS_SYSPARAM, false);
				set_sysparam(m_pAgentSoar, TRACE_JUSTIFICATION_NAMES_SYSPARAM, true);
				set_sysparam(m_pAgentSoar, TRACE_JUSTIFICATIONS_SYSPARAM, false);
				break;
			case 2:
				set_sysparam(m_pAgentSoar, TRACE_CHUNK_NAMES_SYSPARAM, true);
				set_sysparam(m_pAgentSoar, TRACE_CHUNKS_SYSPARAM, true);
				set_sysparam(m_pAgentSoar, TRACE_JUSTIFICATION_NAMES_SYSPARAM, true);
				set_sysparam(m_pAgentSoar, TRACE_JUSTIFICATIONS_SYSPARAM, true);
				break;
		}
	}

	if (options.test(WATCH_WME_DETAIL)) {
		switch (wmeSetting) {
			default:
				// falls through
			case 0:
				set_sysparam(m_pAgentSoar, TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM, NONE_WME_TRACE);
				break;
			case 1:
				set_sysparam(m_pAgentSoar, TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM, TIMETAG_WME_TRACE);
				break;
			case 2:
				set_sysparam(m_pAgentSoar, TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM, FULL_WME_TRACE);
				break;
		}
	}

	return true;
}
