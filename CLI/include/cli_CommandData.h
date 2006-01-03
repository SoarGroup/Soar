/////////////////////////////////////////////////////////////////
// Command data file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
// This file contains enumerations important for the command
// line interface exposed function calls.
//
/////////////////////////////////////////////////////////////////

#ifndef COMMAND_DATA_H
#define COMMAND_DATA_H

namespace cli {

	enum eExciseOptions {
		EXCISE_ALL,
		EXCISE_CHUNKS,
		EXCISE_DEFAULT,
		EXCISE_TASK,
		EXCISE_USER,
		EXCISE_NUM_OPTIONS, // must be last
	};

	enum eIndifferentMode {
		INDIFFERENT_QUERY,
		INDIFFERENT_RANDOM,
		INDIFFERENT_FIRST,
		INDIFFERENT_LAST,
		INDIFFERENT_ASK,
	};

	enum eLearnOptions {
		LEARN_ALL_LEVELS,
		LEARN_BOTTOM_UP,
		LEARN_DISABLE,
		LEARN_ENABLE,
		LEARN_EXCEPT,
		LEARN_LIST,
		LEARN_ONLY,
		LEARN_NUM_OPTIONS, // must be last
	};

	enum eLogMode { 
		LOG_QUERY,
		LOG_NEW,
		LOG_NEWAPPEND,
		LOG_CLOSE,
		LOG_ADD,
	};

	enum eMatchesMode {
		MATCHES_PRODUCTION,
		MATCHES_ASSERTIONS,
		MATCHES_RETRACTIONS,
		MATCHES_ASSERTIONS_RETRACTIONS,
	};

	enum eWMEDetail {
		WME_DETAIL_NONE,
		WME_DETAIL_TIMETAG,
		WME_DETAIL_FULL,
	};

	enum eMemoriesOptions {
		MEMORIES_CHUNKS,
		MEMORIES_DEFAULT,
		MEMORIES_JUSTIFICATIONS,
		MEMORIES_USER,
		MEMORIES_NUM_OPTIONS, // must be last
	};

	enum eNumericIndifferentMode {
		NUMERIC_INDIFFERENT_QUERY,
		NUMERIC_INDIFFERENT_AVERAGE,
		NUMERIC_INDIFFERENT_SUM,
	};

	enum ePreferencesDetail {
		PREFERENCES_ONLY,
		PREFERENCES_NAMES,
		PREFERENCES_TIMETAGS,
		PREFERENCES_WMES,
	};

	enum ePrintOptions {
		PRINT_ALL,
		PRINT_CHUNKS,
		PRINT_DEPTH,
		PRINT_DEFAULTS,
		PRINT_FULL,
		PRINT_FILENAME,
		PRINT_INTERNAL,
		PRINT_JUSTIFICATIONS,
		PRINT_NAME,
		PRINT_OPERATORS,
		PRINT_STACK,
		PRINT_STATES,
		PRINT_USER,
		PRINT_NUM_OPTIONS, // must be last
	};

	enum eProductionFindOptions {
		PRODUCTION_FIND_INCLUDE_LHS,
		PRODUCTION_FIND_INCLUDE_RHS,
		PRODUCTION_FIND_ONLY_CHUNKS,
		PRODUCTION_FIND_NO_CHUNKS,
		PRODUCTION_FIND_SHOWBINDINGS,
		PRODUCTION_FIND_NUM_OPTIONS, // must be last
	};

	enum eRunOptions {
		RUN_DECISION,
		RUN_ELABORATION,
		RUN_FOREVER,
		RUN_OUTPUT,
		RUN_PHASE,
		RUN_SELF,
		RUN_UPDATE,
		RUN_NO_UPDATE,
		RUN_NUM_OPTIONS, // must be last
	};

	enum eSourceMode { 
		SOURCE_DISABLE, 
		SOURCE_DEFAULT, 
		SOURCE_ALL, 
	};

	enum eStatsOptions {
		STATS_MEMORY,
		STATS_RETE,
		STATS_SYSTEM,
		STATS_NUM_OPTIONS, // must be last
	};

	enum eWatchOptions {
		WATCH_DECISIONS,
		WATCH_PHASES,
		WATCH_DEFAULT,
		WATCH_USER,
		WATCH_CHUNKS,
		WATCH_JUSTIFICATIONS,
		WATCH_WMES,
		WATCH_PREFERENCES,
		WATCH_WME_DETAIL,
		WATCH_LEARNING,
		WATCH_BACKTRACING,
		WATCH_INDIFFERENT,
		WATCH_NUM_OPTIONS, // must be last
	};

	enum eWatchWMEsMode {
		WATCH_WMES_ADD,
		WATCH_WMES_REMOVE,
		WATCH_WMES_LIST,
		WATCH_WMES_RESET,
	};

	enum eWatchWMEsOptions {
		WATCH_WMES_TYPE_ADDS,
		WATCH_WMES_TYPE_REMOVES,
		WATCH_WMES_TYPE_NUM_OPTIONS, // must be last
	};

} // namespace cli

#endif // COMMAND_DATA_H
