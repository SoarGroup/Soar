#include "cli_Constants.h"

using namespace cli;

char const* Constants::kCLISyntaxError		= "Command syntax error.\n";

char const* Constants::kCLIAddWME			= "add-wme";
char const* Constants::kCLICD				= "cd";			
char const* Constants::kCLIDir				= "dir";		// alias for ls
char const* Constants::kCLIEcho				= "echo";
char const* Constants::kCLIExcise			= "excise";
char const* Constants::kCLIExit				= "exit";		// alias for quit
char const* Constants::kCLIInit				= "init";		// alias for init-soar
char const* Constants::kCLIInitSoar			= "init-soar";
char const* Constants::kCLILearn			= "learn";
char const* Constants::kCLILog				= "log";
char const* Constants::kCLILS				= "ls";
char const* Constants::kCLIMultiAttributes	= "multi-attributes";
char const* Constants::kCLIPopD				= "popd";
char const* Constants::kCLIPrint			= "print";
char const* Constants::kCLIPushD			= "pushd";
char const* Constants::kCLIPWD				= "pwd";
char const* Constants::kCLIQuit				= "quit";
char const* Constants::kCLIRun				= "run";
char const* Constants::kCLISource			= "source";
char const* Constants::kCLISP				= "sp";
char const* Constants::kCLIStopSoar			= "stop-soar";
char const* Constants::kCLITime				= "time";
char const* Constants::kCLIWatch			= "watch";
char const* Constants::kCLIWatchWMEs		= "watch-wmes";

char const* Constants::kCLIAddWMEUsage				= "Usage:\tadd-wme";
char const* Constants::kCLICDUsage					= "Usage:\tcd [directory]";
char const* Constants::kCLIEchoUsage				= "Usage:\techo [string]";
char const* Constants::kCLIExciseUsage				= "Usage:\texcise production_name\n\texcise -[acdtu]";
char const* Constants::kCLIInitSoarUsage			= "Usage:\tinit-soar";
char const* Constants::kCLILearnUsage				= "Usage:\tlearn [-l]\n\tlearn -[d|e|E|o][ab]";
char const* Constants::kCLILogUsage					= "Usage:\tlog [filename]\n\tlog -d";
char const* Constants::kCLILSUsage					= "Usage:\tls";
char const* Constants::kCLIMultiAttributesUsage		= "Usage:\tmulti-attributes";
char const* Constants::kCLIPopDUsage				= "Usage:\tpopd";
char const* Constants::kCLIPrintUsage				= "Usage:\tprint [-fFin] production_name\n\tprint -[a|c|D|j|u][fFin]\n\tprint [-i] \
[-d <depth>] identifier|timetag|pattern\n\tprint -s[oS]";
char const* Constants::kCLIPushDUsage				= "Usage:\tpushd directory";
char const* Constants::kCLIPWDUsage					= "Usage:\tpwd";
char const* Constants::kCLIQuitUsage				= "Usage:\tquit";
char const* Constants::kCLIRunUsage					= "Usage:\trun [count]\n\trun -[d|e|p][fs] [count]\n\trun -[S|o|O][fs] [count]";
char const* Constants::kCLISourceUsage				= "Usage:\tsource filename";
char const* Constants::kCLISPUsage					= "Usage:\tsp { production }";
char const* Constants::kCLIStopSoarUsage			= "Usage:\tstop-soar [-s] [reason_string]";
char const* Constants::kCLITimeUsage				= "Usage:\ttime command [arguments]";
char const* Constants::kCLIWatchUsage				= "Usage:\twatch [level] [-n] [-a <switch>] [-b <switch>] [-c <switch>] [-d <switch>] \
[-D <switch>] [-i <switch>] [-j <switch>] [-l <detail>] [-L <switch>] [-p <switch>] \
[-P <switch>] [-r <switch>] [-u <switch>] [-w <switch>] [-W <detail>]";
char const* Constants::kCLIWatchWMEsUsage			= "Usage:\twatch-wmes –[a|r] –t <type> pattern\n\twatch-wmes –[l|R] [–t <type>]";