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

	const unsigned int OPTION_NUMERIC_INDIFFERENT_AVERAGE = 0x1;
	const unsigned int OPTION_NUMERIC_INDIFFERENT_SUM	  = 0x2;

	const unsigned int OPTION_PRINT_ALL					= 0x0001;
	const unsigned int OPTION_PRINT_CHUNKS				= 0x0002;
	const unsigned int OPTION_PRINT_DEPTH				= 0x0004;
	const unsigned int OPTION_PRINT_DEFAULTS			= 0x0008;
	const unsigned int OPTION_PRINT_FULL				= 0x0010;
	const unsigned int OPTION_PRINT_FILENAME			= 0x0020;
	const unsigned int OPTION_PRINT_INTERNAL			= 0x0040;
	const unsigned int OPTION_PRINT_JUSTIFICATIONS		= 0x0080;
	const unsigned int OPTION_PRINT_NAME				= 0x0100;
	const unsigned int OPTION_PRINT_OPERATORS			= 0x0200;
	const unsigned int OPTION_PRINT_STACK				= 0x0400;
	const unsigned int OPTION_PRINT_STATES				= 0x0800;
	const unsigned int OPTION_PRINT_USER				= 0x1000;

	const unsigned int OPTION_PRODUCTION_FIND_INCLUDE_LHS	 = 0x1;
	const unsigned int OPTION_PRODUCTION_FIND_INCLUDE_RHS	 = 0x2;
	const unsigned int OPTION_PRODUCTION_FIND_INCLUDE_CHUNKS = 0x4;
	const unsigned int OPTION_PRODUCTION_FIND_SHOWBINDINGS	 = 0x8;

	const unsigned int OPTION_RUN_DECISION     = 0x01;
	const unsigned int OPTION_RUN_ELABORATION  = 0x02;
	const unsigned int OPTION_RUN_FOREVER      = 0x04;
	const unsigned int OPTION_RUN_OUTPUT       = 0x08;
	const unsigned int OPTION_RUN_PHASE        = 0x10;
	const unsigned int OPTION_RUN_SELF         = 0x20;

	const unsigned int OPTION_STATS_MEMORY	= 0x01;
	const unsigned int OPTION_STATS_RETE	= 0x02;
	const unsigned int OPTION_STATS_SYSTEM	= 0x04;

	const unsigned int OPTION_WATCH_DECISIONS		= 0x001;
	const unsigned int OPTION_WATCH_PHASES			= 0x002;
	const unsigned int OPTION_WATCH_DEFAULT			= 0x004;
	const unsigned int OPTION_WATCH_USER			= 0x008;
	const unsigned int OPTION_WATCH_CHUNKS			= 0x010;
	const unsigned int OPTION_WATCH_JUSTIFICATIONS	= 0x020;
	const unsigned int OPTION_WATCH_WMES			= 0x040;
	const unsigned int OPTION_WATCH_PREFERENCES		= 0x080;
	const unsigned int OPTION_WATCH_WME_DETAIL		= 0x100;
	const unsigned int OPTION_WATCH_LEARNING		= 0x200;
	const unsigned int OPTION_WATCH_BACKTRACING		= 0x400;
	const unsigned int OPTION_WATCH_INDIFFERENT		= 0x800;
	const unsigned int OPTION_WATCH_ALL				= 0xfff;

	const unsigned int OPTION_WATCH_WMES_MODE_ADD		= 0x1;
	const unsigned int OPTION_WATCH_WMES_MODE_REMOVE	= 0x2;
	const unsigned int OPTION_WATCH_WMES_MODE_LIST		= 0x3;
	const unsigned int OPTION_WATCH_WMES_MODE_RESET		= 0x4;

} // namespace cli

#endif // COMMAND_DATA_H
