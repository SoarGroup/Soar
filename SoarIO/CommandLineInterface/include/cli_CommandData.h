#ifndef COMMAND_DATA_H
#define COMMAND_DATA_H

namespace cli {

	//               _
	//  _____  _____(_)___  ___
	// / _ \ \/ / __| / __|/ _ \
	//|  __/>  < (__| \__ \  __/
	// \___/_/\_\___|_|___/\___|
	//
	const unsigned int OPTION_EXCISE_ALL		= 0x01;
	const unsigned int OPTION_EXCISE_CHUNKS		= 0x02;
	const unsigned int OPTION_EXCISE_DEFAULT	= 0x04;
	const unsigned int OPTION_EXCISE_TASK		= 0x08;
	const unsigned int OPTION_EXCISE_USER		= 0x10;

	// _
	//| | ___  __ _ _ __ _ __
	//| |/ _ \/ _` | '__| '_ \
	//| |  __/ (_| | |  | | | |
	//|_|\___|\__,_|_|  |_| |_|
	//
	const unsigned int OPTION_LEARN_ALL_LEVELS = 0x01;
	const unsigned int OPTION_LEARN_BOTTOM_UP  = 0x02;
	const unsigned int OPTION_LEARN_DISABLE    = 0x04;
	const unsigned int OPTION_LEARN_ENABLE     = 0x08;
	const unsigned int OPTION_LEARN_EXCEPT     = 0x10;
	const unsigned int OPTION_LEARN_LIST       = 0x20;
	const unsigned int OPTION_LEARN_ONLY       = 0x40;

	//            _       _
	// _ __  _ __(_)_ __ | |_
	//| '_ \| '__| | '_ \| __|
	//| |_) | |  | | | | | |_
	//| .__/|_|  |_|_| |_|\__|
	//|_|
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

	// _ __ _   _ _ __
	//| '__| | | | '_ \
	//| |  | |_| | | | |
	//|_|   \__,_|_| |_|
	//
	const unsigned int OPTION_RUN_DECISION     = 0x01;
	const unsigned int OPTION_RUN_ELABORATION  = 0x02;
	const unsigned int OPTION_RUN_FOREVER      = 0x04;
	const unsigned int OPTION_RUN_OPERATOR     = 0x08;
	const unsigned int OPTION_RUN_OUTPUT       = 0x10;
	const unsigned int OPTION_RUN_PHASE        = 0x20;
	const unsigned int OPTION_RUN_SELF         = 0x40;
	const unsigned int OPTION_RUN_STATE        = 0x80;

	//     _        _
	// ___| |_ __ _| |_ ___
	/// __| __/ _` | __/ __|
	//\__ \ || (_| | |_\__ \
	//|___/\__\__,_|\__|___/
	//
	//const unsigned int OPTION_STATS_MEMORY	= 0x01;
	//const unsigned int OPTION_STATS_RETE		= 0x02;
	//const unsigned int OPTION_STATS_STATS		= 0x04;
	//const unsigned int OPTION_STATS_SYSTEM	= 0x08;

	//               _       _
	//__      ____ _| |_ ___| |__
	//\ \ /\ / / _` | __/ __| '_ \
	// \ V  V / (_| | || (__| | | |
	//  \_/\_/ \__,_|\__\___|_| |_|
	//
	const unsigned int OPTION_WATCH_LEARNING               = 0x0003;	// 2 bits (detail arg)
	const unsigned int OPTION_WATCH_WME_DETAIL             = 0x000c;
	const unsigned int OPTION_WATCH_BACKTRACING            = 0x0010;	// 1 bit (switch arg)
	const unsigned int OPTION_WATCH_CHUNKS                 = 0x0020;
	const unsigned int OPTION_WATCH_DECISIONS              = 0x0040;
	const unsigned int OPTION_WATCH_DEFAULT_PRODUCTIONS    = 0x0080;
	const unsigned int OPTION_WATCH_INDIFFERENT_SELECTION  = 0x0100;
	const unsigned int OPTION_WATCH_JUSTIFICATIONS         = 0x0200;
	const unsigned int OPTION_WATCH_LOADING                = 0x0400;
	const unsigned int OPTION_WATCH_PHASES                 = 0x0800;
	const unsigned int OPTION_WATCH_PRODUCTIONS            = 0x1000;
	const unsigned int OPTION_WATCH_PREFERENCES            = 0x2000;
	const unsigned int OPTION_WATCH_USER_PRODUCTIONS       = 0x4000;
	const unsigned int OPTION_WATCH_WMES                   = 0x8000;
	const unsigned int OPTION_WATCH_NONE                   = 0xffff;	// All bits

} // namespace cli

#endif // COMMAND_DATA_H
