#ifndef COMMAND_DATA_H
#define COMMAND_DATA_H

namespace cli {

	const unsigned int OPTION_EXCISE_ALL		= 0x01;
	const unsigned int OPTION_EXCISE_CHUNKS		= 0x02;
	const unsigned int OPTION_EXCISE_DEFAULT	= 0x04;
	const unsigned int OPTION_EXCISE_TASK		= 0x08;
	const unsigned int OPTION_EXCISE_USER		= 0x10;

	const unsigned int OPTION_INDIFFERENT_RANDOM = 0x1;
	const unsigned int OPTION_INDIFFERENT_FIRST	 = 0x2;
	const unsigned int OPTION_INDIFFERENT_LAST	 = 0x3;
	const unsigned int OPTION_INDIFFERENT_ASK	 = 0x4;

	const unsigned int OPTION_LEARN_ALL_LEVELS = 0x01;
	const unsigned int OPTION_LEARN_BOTTOM_UP  = 0x02;
	const unsigned int OPTION_LEARN_DISABLE    = 0x04;
	const unsigned int OPTION_LEARN_ENABLE     = 0x08;
	const unsigned int OPTION_LEARN_EXCEPT     = 0x10;
	const unsigned int OPTION_LEARN_LIST       = 0x20;
	const unsigned int OPTION_LEARN_ONLY       = 0x40;

	const unsigned int OPTION_MATCHES_PRODUCTION			 = 0x0;
	const unsigned int OPTION_MATCHES_ASSERTIONS			 = 0x1;
	const unsigned int OPTION_MATCHES_RETRACTIONS			 = 0x2;
	const unsigned int OPTION_MATCHES_ASSERTIONS_RETRACTIONS = 0x3;

	const unsigned int OPTION_MEMORIES_CHUNKS		  = 0x1;
	const unsigned int OPTION_MEMORIES_DEFAULT		  = 0x2;
	const unsigned int OPTION_MEMORIES_JUSTIFICATIONS = 0x4;
	const unsigned int OPTION_MEMORIES_USER			  = 0x8;

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
	const unsigned int OPTION_RUN_OPERATOR     = 0x08;
	const unsigned int OPTION_RUN_OUTPUT       = 0x10;
	const unsigned int OPTION_RUN_PHASE        = 0x20;
	const unsigned int OPTION_RUN_SELF         = 0x40;
	const unsigned int OPTION_RUN_STATE        = 0x80;

	//const unsigned int OPTION_STATS_MEMORY	= 0x01;
	//const unsigned int OPTION_STATS_RETE		= 0x02;
	//const unsigned int OPTION_STATS_STATS		= 0x04;
	//const unsigned int OPTION_STATS_SYSTEM	= 0x08;

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

} // namespace cli

#endif // COMMAND_DATA_H
