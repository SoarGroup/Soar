#include "commandLineInterface.h"

#include <string>
#include <ctype.h>
#include <string.h>
#include <assert.h>
//#include <winx/assertx.h>

//#include <glibc/getopt.h>

#include "getopt.h"

#include "gSKI_Structures.h"
#include "IgSKI_ProductionManager.h"
#include "IgSKI_Agent.h"
#include "IgSKI_Kernel.h"

#include "sml_ElementXML.h"
#include "sml_TagResult.h"
#include "sml_TagError.h"

using namespace std;
using namespace cli;

char const* CLIConstants::kCLIAddWME	= "add-wme";
char const* CLIConstants::kCLICD		= "cd";
char const* CLIConstants::kCLIEcho		= "echo";
char const* CLIConstants::kCLIExcise	= "excise";
char const* CLIConstants::kCLIExit		= "exit";
char const* CLIConstants::kCLIInitSoar	= "init-soar";
char const* CLIConstants::kCLILearn		= "learn";
char const* CLIConstants::kCLINewAgent	= "new-agent";
char const* CLIConstants::kCLIQuit		= "quit";
char const* CLIConstants::kCLIRun		= "run";
char const* CLIConstants::kCLISource	= "source";
char const* CLIConstants::kCLISP		= "sp";
char const* CLIConstants::kCLIStopSoar	= "stop-soar";
char const* CLIConstants::kCLIWatch		= "watch";
char const* CLIConstants::kCLIWatchWMEs	= "watch-wmes";

char const* CLIConstants::kCLIAddWMEUsage    = "Usage:\tadd-wme";
char const* CLIConstants::kCLICDUsage        = "Usage:\tcd [directory]";
char const* CLIConstants::kCLIEchoUsage      = "Usage:\techo [string]";
char const* CLIConstants::kCLIExciseUsage    = "Usage:\texcise production_name\n\texcise -[acdtu]";
char const* CLIConstants::kCLIInitSoarUsage  = "Usage:\tinit-soar";
char const* CLIConstants::kCLILearnUsage     = "Usage:\tlearn [-l]\n\tlearn -[d|e|E|o][ab]";
char const* CLIConstants::kCLINewAgentUsage  = "Usage:\tnew-agent [agent_name]";
char const* CLIConstants::kCLIRunUsage       = "Usage:\trun [count]\n\trun -[d|e|p][fs] [count]\n\trun -[S|o|O][fs] [count]";
char const* CLIConstants::kCLISourceUsage    = "Usage:\tsource filename";
char const* CLIConstants::kCLISPUsage	      = "Usage:\tsp { production }";
char const* CLIConstants::kCLIStopSoarUsage  = "Usage:\tstop-soar [-s] [reason_string]";
char const* CLIConstants::kCLIWatchUsage     = "Usage:\twatch [level] [-n] [-a <switch>] [-b <switch>] [-c <switch>] [-d <switch>] \
											   [-D <switch>] [-i <switch>] [-j <switch>] [-l <detail>] [-L <switch>] [-p <switch>] \
											   [-P <switch>] [-r <switch>] [-u <switch>] [-w <switch>] [-W <detail>]";
char const* CLIConstants::kCLIWatchWMEsUsage = "Usage:\twatch-wmes –[a|r] –t <type> pattern\n\twatch-wmes –[l|R] [–t <type>]";

//  ____                                          _ _     _            ___       _             __
// / ___|___  _ __ ___  _ __ ___   __ _ _ __   __| | |   (_)_ __   ___|_ _|_ __ | |_ ___ _ __ / _| __ _  ___ ___
//| |   / _ \| '_ ` _ \| '_ ` _ \ / _` | '_ \ / _` | |   | | '_ \ / _ \| || '_ \| __/ _ \ '__| |_ / _` |/ __/ _ \
//| |__| (_) | | | | | | | | | | | (_| | | | | (_| | |___| | | | |  __/| || | | | ||  __/ |  |  _| (_| | (_|  __/
// \____\___/|_| |_| |_|_| |_| |_|\__,_|_| |_|\__,_|_____|_|_| |_|\___|___|_| |_|\__\___|_|  |_|  \__,_|\___\___|
//
CommandLineInterface::CommandLineInterface(void)
{
	BuildCommandMap();
	m_QuitCalled = false;
}

// /\/|____                                          _ _     _            ___       _             __
//|/\// ___|___  _ __ ___  _ __ ___   __ _ _ __   __| | |   (_)_ __   ___|_ _|_ __ | |_ ___ _ __ / _| __ _  ___ ___
//   | |   / _ \| '_ ` _ \| '_ ` _ \ / _` | '_ \ / _` | |   | | '_ \ / _ \| || '_ \| __/ _ \ '__| |_ / _` |/ __/ _ \
//   | |__| (_) | | | | | | | | | | | (_| | | | | (_| | |___| | | | |  __/| || | | | ||  __/ |  |  _| (_| | (_|  __/
//    \____\___/|_| |_| |_|_| |_| |_|\__,_|_| |_|\__,_|_____|_|_| |_|\___|___|_| |_|\__\___|_|  |_|  \__,_|\___\___|
//
CommandLineInterface::~CommandLineInterface(void)
{
}

//  ____      _   _              _   ____                 _ _
// / ___| ___| |_| |    __ _ ___| |_|  _ \ ___  ___ _   _| | |_
//| |  _ / _ \ __| |   / _` / __| __| |_) / _ \/ __| | | | | __|
//| |_| |  __/ |_| |__| (_| \__ \ |_|  _ <  __/\__ \ |_| | | |_
// \____|\___|\__|_____\__,_|___/\__|_| \_\___||___/\__,_|_|\__|
//
void CommandLineInterface::GetLastResult(std::string* pResult) {
	*pResult = m_Result;
}

// ____         ____                                          _
//|  _ \  ___  / ___|___  _ __ ___  _ __ ___   __ _ _ __   __| |
//| | | |/ _ \| |   / _ \| '_ ` _ \| '_ ` _ \ / _` | '_ \ / _` |
//| |_| | (_) | |__| (_) | | | | | | | | | | | (_| | | | | (_| |
//|____/ \___/ \____\___/|_| |_| |_|_| |_| |_|\__,_|_| |_|\__,_|
//
bool CommandLineInterface::DoCommand(gSKI::IAgent* pAgent, gSKI::IKernel* pKernel, const char* pCommandLine, sml::ElementXML* pResponse, gSKI::Error* pError) {

	// Save the pointers
	m_pKernel = pKernel;
	m_pAgent = pAgent;
	m_pError = pError;

	bool ret = DoCommandInternal(pCommandLine);

	sml::TagResult* pTag = new sml::TagResult() ;
	pTag->SetCharacterData(m_Result.c_str()) ;
	pResponse->AddChild(pTag) ;

	return ret;
}

// ____         ____                                          _ ___       _                        _
//|  _ \  ___  / ___|___  _ __ ___  _ __ ___   __ _ _ __   __| |_ _|_ __ | |_ ___ _ __ _ __   __ _| |
//| | | |/ _ \| |   / _ \| '_ ` _ \| '_ ` _ \ / _` | '_ \ / _` || || '_ \| __/ _ \ '__| '_ \ / _` | |
//| |_| | (_) | |__| (_) | | | | | | | | | | | (_| | | | | (_| || || | | | ||  __/ |  | | | | (_| | |
//|____/ \___/ \____\___/|_| |_| |_|_| |_| |_|\__,_|_| |_|\__,_|___|_| |_|\__\___|_|  |_| |_|\__,_|_|
//
bool CommandLineInterface::DoCommandInternal(const char* commandLine) {

	vector<string> argumentVector;

	// clear the result
	m_Result.clear();

	// parse command:
	int argc = Tokenize(commandLine, argumentVector);
	if (!argc) {
		// nothing on the command line, so consider it processed OK
		return true;
	} else if (argc == -1) {
		// parsing failed, error in result
		return false;
	}

	// marshall arg{c/v}:
	char** argv = new char*[argc + 1]; // leave space for extra null
	int arglen;

	// for each arg
	for (int i = 0; i < argc; ++i) {
		// save its length
		arglen = argumentVector[i].length();

		// leave space for null
		argv[i] = new char[ arglen + 1 ];

		// copy the string
		strncpy(argv[i], argumentVector[i].data(), arglen);

		// set final index to null
		argv[i][ arglen ] = 0;
	}
	// set final index to null
	argv[argc] = 0;

	// process command:
	// TODO: shouldn't this be a const passing since we don't want processCommand to mess with it?
	CommandFunction pFunction = m_CommandMap[argv[0]];

	if (!pFunction) {
		// command not found or implemented
		m_Result += "Command '";
		m_Result += argv[0];
		m_Result += "' not found or implemented.";
		return false;
	}

	bool ret = (this->*pFunction)(argc, argv);

	// free memory
	for (int j = 0; j < argc; ++j) {
		delete [] argv[j];
	}
	delete [] argv;

	return ret;
}

// _____     _              _
//|_   _|__ | | _____ _ __ (_)_______
//  | |/ _ \| |/ / _ \ '_ \| |_  / _ \
//  | | (_) |   <  __/ | | | |/ /  __/
//  |_|\___/|_|\_\___|_| |_|_/___\___|
//
int CommandLineInterface::Tokenize(const char* commandLine, vector<string>& argumentVector) {
	int argc = 0;
	string cmdline(commandLine);
	string::iterator iter;
	string arg;
	bool quotes = false;
	int brackets = 0;

	for (;;) {

		// is there anything to work with?
		if(cmdline.empty()) {
			break;
		}

		// remove leading whitespace
		iter = cmdline.begin();
		while (isspace(*iter)) {
			cmdline.erase(iter);

			if (!cmdline.length()) {
				//nothing but space left
				break;
			}

			iter = cmdline.begin();
		}

		// was it actually trailing whitespace?
		if (!cmdline.length()) {
			// nothing left to do
			break;
		}

		// we have an argument
		++argc;
		arg.clear();
		// use space as a delimiter unless inside quotes or brackets (nestable)
		while (!isspace(*iter) || quotes || brackets) {
			// eat quotes but leave brackets
			if (*iter == '"') {
				// flip the quotes flag
				quotes = !quotes;

				// eat quotes (not adding them to arg)

			} else {
				if (*iter == '{') {
					++brackets;
				} else if (*iter == '}') {
					--brackets;
					if (brackets < 0) {
						m_Result += "Closing bracket found without opening counterpart.";
						return -1;
					}
				}

				// add to argument
				arg += (*iter);
			}

			// move on
			cmdline.erase(iter);
			iter = cmdline.begin();

			// are we at the end of the string?
			if (iter == cmdline.end()) {

				// did they close their quotes or brackets?
				if (quotes || brackets) {
					m_Result += "No closing quotes/brackets found.";
					return -1;
				}
				break;
			}
		}

		// store the arg
		argumentVector.push_back(arg);
	}
	return argc;
}

//bool CommandLineInterface::Parse(int argc, char**& argv) {
//   if (CheckForHelp(argc, argv)) {
//      m_Result += CLIConstants::kCLIUsage;
//      return true;
//   }
//   return Do();
//}

//bool CommandLineInterface::Do() {
//   m_Result += "TODO: ";
//   return true;
//}

// ____                        _       _     ___        ____  __ _____
//|  _ \ __ _ _ __ ___  ___   / \   __| | __| \ \      / /  \/  | ____|
//| |_) / _` | '__/ __|/ _ \ / _ \ / _` |/ _` |\ \ /\ / /| |\/| |  _|
//|  __/ (_| | |  \__ \  __// ___ \ (_| | (_| | \ V  V / | |  | | |___
//|_|   \__,_|_|  |___/\___/_/   \_\__,_|\__,_|  \_/\_/  |_|  |_|_____|
//
bool CommandLineInterface::ParseAddWME(int argc, char**& argv) {
	if (CheckForHelp(argc, argv)) {
		m_Result += CLIConstants::kCLIAddWMEUsage;
		return true;
	}

	return DoAddWME();
}

// ____          _       _     ___        ____  __ _____
//|  _ \  ___   / \   __| | __| \ \      / /  \/  | ____|
//| | | |/ _ \ / _ \ / _` |/ _` |\ \ /\ / /| |\/| |  _|
//| |_| | (_) / ___ \ (_| | (_| | \ V  V / | |  | | |___
//|____/ \___/_/   \_\__,_|\__,_|  \_/\_/  |_|  |_|_____|
//
bool CommandLineInterface::DoAddWME() {
	m_Result += "TODO: add-wme";
	return true;
}

// ____                      ____ ____
//|  _ \ __ _ _ __ ___  ___ / ___|  _ \
//| |_) / _` | '__/ __|/ _ \ |   | | | |
//|  __/ (_| | |  \__ \  __/ |___| |_| |
//|_|   \__,_|_|  |___/\___|\____|____/
//
bool CommandLineInterface::ParseCD(int argc, char**& argv) {
	if (CheckForHelp(argc, argv)) {
		m_Result += CLIConstants::kCLICDUsage;
		return true;
	}

	if (argc > 2) {
		m_Result += "Too many arguments.\n";
		m_Result += CLIConstants::kCLICDUsage;
		return false;
	}
	return DoCD(argv[1]);
}

// ____         ____ ____
//|  _ \  ___  / ___|  _ \
//| | | |/ _ \| |   | | | |
//| |_| | (_) | |___| |_| |
//|____/ \___/ \____|____/
//
bool CommandLineInterface::DoCD(const char* directory) {
	if (!directory) {
		// return to home/original directory
		m_Result += "TODO: return to home or original directory.";
		return true;
	}

	m_Result += "TODO: change to ";
	m_Result += directory;
	return true;
}

// ____                     _____     _
//|  _ \ __ _ _ __ ___  ___| ____|___| |__   ___
//| |_) / _` | '__/ __|/ _ \  _| / __| '_ \ / _ \
//|  __/ (_| | |  \__ \  __/ |__| (__| | | | (_) |
//|_|   \__,_|_|  |___/\___|_____\___|_| |_|\___/
//
bool CommandLineInterface::ParseEcho(int argc, char**& argv) {
	if (CheckForHelp(argc, argv)) {
		m_Result += CLIConstants::kCLIEchoUsage;
		return true;
	}
	return DoEcho(argc, argv);
}

// ____        _____     _
//|  _ \  ___ | ____|___| |__   ___
//| | | |/ _ \|  _| / __| '_ \ / _ \
//| |_| | (_) | |__| (__| | | | (_) |
//|____/ \___/|_____\___|_| |_|\___/
//
bool CommandLineInterface::DoEcho(int argc, char**& argv) {

	for (int i = 1; i < argc; ++i) {
		m_Result += argv[i];
		m_Result += ' ';
	}
	// TODO: chop off that last space
	return true;
}

// ____                     _____          _
//|  _ \ __ _ _ __ ___  ___| ____|_  _____(_)___  ___
//| |_) / _` | '__/ __|/ _ \  _| \ \/ / __| / __|/ _ \
//|  __/ (_| | |  \__ \  __/ |___ >  < (__| \__ \  __/
//|_|   \__,_|_|  |___/\___|_____/_/\_\___|_|___/\___|
//
bool CommandLineInterface::ParseExcise(int argc, char**& argv) {
	if (CheckForHelp(argc, argv)) {
		m_Result += CLIConstants::kCLIExciseUsage;
		return true;
	}

	static struct GetOpt::option longOptions[] = {
		{"all",		0, 0, 'a'},
		{"chunks",	0, 0, 'c'},
		{"default", 0, 0, 'd'},
		{"task",	0, 0, 't'},
		{"user",	0, 0, 'u'},
		{0, 0, 0, 0}
	};

	GetOpt::optind = 0;
	GetOpt::opterr = 0;

	int option;
	unsigned short options = 0;

	for (;;) {
		option = m_GetOpt.GetOpt_Long (argc, argv, "acdtu", longOptions, 0);
		if (option == -1) {
			break;
		}

		switch (option) {
			case 'a':
				options |= OPTION_EXCISE_ALL;
				break;
			case 'c':
				options |= OPTION_EXCISE_CHUNKS;
				break;
			case 'd':
				options |= OPTION_EXCISE_DEFAULT;
				break;
			case 't':
				options |= OPTION_EXCISE_TASK;
				break;
			case 'u':
				options |= OPTION_EXCISE_USER;
				break;
			case '?':
				m_Result += "Unrecognized option.\n";
				m_Result += CLIConstants::kCLIExciseUsage;
				return false;
			default:
				m_Result += "Internal error: m_GetOpt.GetOpt_Long returned '";
				m_Result += option;
				m_Result += "'!";
				return false;
		}
	}

	int productionCount = 0;
	char** productions = 0;
	if (GetOpt::optind < argc) {
		productions = new char*[argc - GetOpt::optind + 1];
		while (GetOpt::optind < argc) {
			productions[productionCount] = new char[strlen(argv[GetOpt::optind]) + 1];
			strcpy(productions[productionCount], argv[GetOpt::optind]);
			productions[productionCount][strlen(argv[GetOpt::optind])] = 0;
			++GetOpt::optind;
			++productionCount;
		}
		productions[productionCount] = 0;
	}

	bool ret = DoExcise(options, productionCount, productions);

	for (int i = 0; i < productionCount; ++i) {
		delete [] (productions[i]);
	}
	delete [] (productions);
	return ret;
}

// ____        _____          _
//|  _ \  ___ | ____|_  _____(_)___  ___
//| | | |/ _ \|  _| \ \/ / __| / __|/ _ \
//| |_| | (_) | |___ >  < (__| \__ \  __/
//|____/ \___/|_____/_/\_\___|_|___/\___|
//
bool CommandLineInterface::DoExcise(unsigned short options, int productionCount, char**& productions) {
	m_Result += "TODO: do excise";
	return true;
}

// ____                     ___       _ _   ____
//|  _ \ __ _ _ __ ___  ___|_ _|_ __ (_) |_/ ___|  ___   __ _ _ __
//| |_) / _` | '__/ __|/ _ \| || '_ \| | __\___ \ / _ \ / _` | '__|
//|  __/ (_| | |  \__ \  __/| || | | | | |_ ___) | (_) | (_| | |
//|_|   \__,_|_|  |___/\___|___|_| |_|_|\__|____/ \___/ \__,_|_|
//
bool CommandLineInterface::ParseInitSoar(int argc, char**& argv) {
	if (CheckForHelp(argc, argv)) {
		m_Result += CLIConstants::kCLIInitSoarUsage;
		return true;
	}

	if (argc > 1) {
		m_Result += "Too many arguments.\n";
		m_Result += CLIConstants::kCLIInitSoarUsage;
		return false;
	}
	return DoInitSoar();
}

// ____       ___       _ _   ____
//|  _ \  ___|_ _|_ __ (_) |_/ ___|  ___   __ _ _ __
//| | | |/ _ \| || '_ \| | __\___ \ / _ \ / _` | '__|
//| |_| | (_) | || | | | | |_ ___) | (_) | (_| | |
//|____/ \___/___|_| |_|_|\__|____/ \___/ \__,_|_|
//
bool CommandLineInterface::DoInitSoar() {
	m_Result += "TODO: init-soar";
	return true;
}

// ____                     _
//|  _ \ __ _ _ __ ___  ___| |    ___  __ _ _ __ _ __
//| |_) / _` | '__/ __|/ _ \ |   / _ \/ _` | '__| '_ \
//|  __/ (_| | |  \__ \  __/ |__|  __/ (_| | |  | | | |
//|_|   \__,_|_|  |___/\___|_____\___|\__,_|_|  |_| |_|
//
bool CommandLineInterface::ParseLearn(int argc, char**& argv) {
	if (CheckForHelp(argc, argv)) {
		m_Result += CLIConstants::kCLILearnUsage;
		return true;
	}

	static struct GetOpt::option longOptions[] = {
		{"all-levels",		0, 0, 'a'},
		{"bottom-up",		0, 0, 'b'},
		{"disable",			0, 0, 'd'},
		{"off",				0, 0, 'd'},
		{"enable",			0, 0, 'e'},
		{"on",				0, 0, 'e'},
		{"except",			0, 0, 'E'},
		{"list",			   0, 0, 'l'},
		{"only",			   0, 0, 'o'},
		{0, 0, 0, 0}
	};

	GetOpt::optind = 0;
	GetOpt::opterr = 0;

	int option;
	unsigned short options = 0;

	for (;;) {
		option = m_GetOpt.GetOpt_Long (argc, argv, "abdeElo", longOptions, 0);
		if (option == -1) {
			break;
		}

		switch (option) {
			case 'a':
				options |= OPTION_LEARN_ALL_LEVELS;
				break;
			case 'b':
				options |= OPTION_LEARN_BOTTOM_UP;
				break;
			case 'd':
				options |= OPTION_LEARN_DISABLE;
				break;
			case 'e':
				options |= OPTION_LEARN_ENABLE;
				break;
			case 'E':
				options |= OPTION_LEARN_EXCEPT;
				break;
			case 'l':
				options |= OPTION_LEARN_LIST;
				break;
			case 'o':
				options |= OPTION_LEARN_ONLY;
				break;
			case '?':
				m_Result += "Unrecognized option.\n";
				m_Result += CLIConstants::kCLILearnUsage;
				return false;
			default:
				m_Result += "Internal error: m_GetOpt.GetOpt_Long returned '";
				m_Result += option;
				m_Result += "'!";
				return false;
		}
	}

	if (GetOpt::optind < argc) {
		m_Result += "Too many arguments.\n";
		m_Result += CLIConstants::kCLILearnUsage;
		return false;
	}

	return DoLearn(options);
}

// ____        _
//|  _ \  ___ | |    ___  __ _ _ __ _ __
//| | | |/ _ \| |   / _ \/ _` | '__| '_ \
//| |_| | (_) | |__|  __/ (_| | |  | | | |
//|____/ \___/|_____\___|\__,_|_|  |_| |_|
//
bool CommandLineInterface::DoLearn(const unsigned short options) {
	m_Result += "TODO: do Learn";
	return true;
}

// ____                     _   _                _                    _
//|  _ \ __ _ _ __ ___  ___| \ | | _____      __/ \   __ _  ___ _ __ | |_
//| |_) / _` | '__/ __|/ _ \  \| |/ _ \ \ /\ / / _ \ / _` |/ _ \ '_ \| __|
//|  __/ (_| | |  \__ \  __/ |\  |  __/\ V  V / ___ \ (_| |  __/ | | | |_
//|_|   \__,_|_|  |___/\___|_| \_|\___| \_/\_/_/   \_\__, |\___|_| |_|\__|
//                                                   |___/
bool CommandLineInterface::ParseNewAgent(int argc, char**& argv) {
	if (CheckForHelp(argc, argv)) {
		m_Result += CLIConstants::kCLINewAgentUsage;
		return true;
	}

	if (argc > 2) {
		m_Result += "Too many arguments.\n";
		m_Result += CLIConstants::kCLINewAgentUsage;
		return false;
	}
	return DoNewAgent(argv[1]);
}

// ____        _   _                _                    _
//|  _ \  ___ | \ | | _____      __/ \   __ _  ___ _ __ | |_
//| | | |/ _ \|  \| |/ _ \ \ /\ / / _ \ / _` |/ _ \ '_ \| __|
//| |_| | (_) | |\  |  __/\ V  V / ___ \ (_| |  __/ | | | |_
//|____/ \___/|_| \_|\___| \_/\_/_/   \_\__, |\___|_| |_|\__|
//                                      |___/
bool CommandLineInterface::DoNewAgent(char const* agentName) {
	m_Result += "TODO: create new-agent \"";
	m_Result += agentName;
	m_Result += "\"";
	return true;
}

// ____                      ___        _ _
//|  _ \ __ _ _ __ ___  ___ / _ \ _   _(_) |_
//| |_) / _` | '__/ __|/ _ \ | | | | | | | __|
//|  __/ (_| | |  \__ \  __/ |_| | |_| | | |_
//|_|   \__,_|_|  |___/\___|\__\_\\__,_|_|\__|
//
bool CommandLineInterface::ParseQuit(int argc, char**& argv) {
	return DoQuit();
}

// ____         ___        _ _
//|  _ \  ___  / _ \ _   _(_) |_
//| | | |/ _ \| | | | | | | | __|
//| |_| | (_) | |_| | |_| | | |_
//|____/ \___/ \__\_\\__,_|_|\__|
//
bool CommandLineInterface::DoQuit() {
	m_QuitCalled = true; 
	m_Result = "Goodbye.";
	return true;
}

// ____                     ____
//|  _ \ __ _ _ __ ___  ___|  _ \ _   _ _ __
//| |_) / _` | '__/ __|/ _ \ |_) | | | | '_ \
//|  __/ (_| | |  \__ \  __/  _ <| |_| | | | |
//|_|   \__,_|_|  |___/\___|_| \_\\__,_|_| |_|
//
bool CommandLineInterface::ParseRun(int argc, char**& argv) {
	if (CheckForHelp(argc, argv)) {
		m_Result += CLIConstants::kCLIRunUsage;
		return true;
	}

	static struct GetOpt::option longOptions[] = {
		{"decision",		0, 0, 'd'},
		{"elaboration",	0, 0, 'e'},
		{"forever",			0, 0, 'f'},
		{"operator",		0, 0, 'o'},
		{"output",			0, 0, 'O'},
		{"phase",			0, 0, 'p'},
		{"self",			   0, 0, 's'},
		{"state",			0, 0, 'S'},
		{0, 0, 0, 0}
	};

	GetOpt::optind = 0;
	GetOpt::opterr = 0;

	int option;
	unsigned short options = 0;

	for (;;) {
		option = m_GetOpt.GetOpt_Long (argc, argv, "defoOpsS", longOptions, 0);
		if (option == -1) {
			break;
		}

		switch (option) {
			case 'd':
				options |= OPTION_RUN_DECISION;
				break;
			case 'e':
				options |= OPTION_RUN_ELABORATION;
				break;
			case 'f':
				options |= OPTION_RUN_FOREVER;
				break;
			case 'o':
				options |= OPTION_RUN_OPERATOR;
				break;
			case 'O':
				options |= OPTION_RUN_OUTPUT;
				break;
			case 'p':
				options |= OPTION_RUN_PHASE;
				break;
			case 's':
				options |= OPTION_RUN_SELF;
				break;
			case 'S':
				options |= OPTION_RUN_STATE;
				break;
			case '?':
				m_Result += "Unrecognized option.\n";
				m_Result += CLIConstants::kCLIRunUsage;
				return false;
			default:
				m_Result += "Internal error: m_GetOpt.GetOpt_Long returned '";
				m_Result += option;
				m_Result += "'!";
				return false;
		}
	}

	if (GetOpt::optind == argc - 1) {
		m_Result += "Count: ";
		m_Result += argv[GetOpt::optind];
	} else if (GetOpt::optind < argc) {
		m_Result += "Too many arguments.\n";
		m_Result += CLIConstants::kCLIRunUsage;
		return false;
	}

	return DoRun(options);
}

// ____        ____
//|  _ \  ___ |  _ \ _   _ _ __
//| | | |/ _ \| |_) | | | | '_ \
//| |_| | (_) |  _ <| |_| | | | |
//|____/ \___/|_| \_\\__,_|_| |_|
//
bool CommandLineInterface::DoRun(const unsigned short options) {
	m_Result += "TODO: do run";
	return true;
}

// ____                     ____
//|  _ \ __ _ _ __ ___  ___/ ___|  ___  _   _ _ __ ___ ___
//| |_) / _` | '__/ __|/ _ \___ \ / _ \| | | | '__/ __/ _ \
//|  __/ (_| | |  \__ \  __/___) | (_) | |_| | | | (_|  __/
//|_|   \__,_|_|  |___/\___|____/ \___/ \__,_|_|  \___\___|
//
bool CommandLineInterface::ParseSource(int argc, char**& argv) {
	if (CheckForHelp(argc, argv)) {
		m_Result += CLIConstants::kCLISourceUsage;
		return true;
	}

	if (argc < 2) {
		m_Result += "Too few arguments.\n";
		m_Result += CLIConstants::kCLISourceUsage;
		return false;

	} else if (argc > 2) {
		m_Result += "Too many arguments.\n";
		m_Result += CLIConstants::kCLISourceUsage;
		return false;
	}
	return DoSource(argv[1]);
}

// ____       ____
//|  _ \  ___/ ___|  ___  _   _ _ __ ___ ___
//| | | |/ _ \___ \ / _ \| | | | '__/ __/ _ \
//| |_| | (_) |__) | (_) | |_| | | | (_|  __/
//|____/ \___/____/ \___/ \__,_|_|  \___\___|
//
bool CommandLineInterface::DoSource(const char* filename) {
	if (!filename) {
		m_Result += "Please supply a filename to source.";
		return false;
	}

	if (!m_pAgent) {
		m_Result += "No agent pointer.";
		return false;
	}

	gSKI::IProductionManager *pProductionManager = m_pAgent->GetProductionManager();

	pProductionManager->LoadSoarFile(filename, m_pError);

	if(m_pError->Id != gSKI::gSKIERR_NONE) {
		m_Result += "Unable to source the file: ";
		m_Result += filename;
		return false;
	}

	// Print one * per loaded production
	m_Result += "*";
	return true;
}

// ____                     ____  ____
//|  _ \ __ _ _ __ ___  ___/ ___||  _ \
//| |_) / _` | '__/ __|/ _ \___ \| |_) |
//|  __/ (_| | |  \__ \  __/___) |  __/
//|_|   \__,_|_|  |___/\___|____/|_|
//
bool CommandLineInterface::ParseSP(int argc, char**& argv) {
	if (CheckForHelp(argc, argv)) {
		m_Result += CLIConstants::kCLISPUsage;
		return true;
	}

	if (argc != 2) {
		m_Result += "Incorrect format.\n";
		m_Result += CLIConstants::kCLISPUsage;
		return false;
	}

	string production = argv[0];
	production += ' ';
	production += argv[1];

	return DoSP(production.c_str());
}

// ____       ____  ____
//|  _ \  ___/ ___||  _ \
//| | | |/ _ \___ \| |_) |
//| |_| | (_) |__) |  __/
//|____/ \___/____/|_|
//
bool CommandLineInterface::DoSP(const char* production) {
	if (!production) {
		m_Result += "Production body is empty.";
		return false;
	}

	if (!m_pAgent) {
		m_Result += "No agent pointer.";
		return false;
	}

	gSKI::IProductionManager *pProductionManager = m_pAgent->GetProductionManager();

	pProductionManager->AddProduction(const_cast<char*>(production), m_pError);

	if(m_pError->Id != gSKI::gSKIERR_NONE) {
		m_Result += "Unable to add the production: ";
		m_Result += production;
		return false;
	}

	// Print one * per loaded production
	m_Result += "*";
	return true;
}

// ____                     ____  _             ____
//|  _ \ __ _ _ __ ___  ___/ ___|| |_ ___  _ __/ ___|  ___   __ _ _ __
//| |_) / _` | '__/ __|/ _ \___ \| __/ _ \| '_ \___ \ / _ \ / _` | '__|
//|  __/ (_| | |  \__ \  __/___) | || (_) | |_) |__) | (_) | (_| | |
//|_|   \__,_|_|  |___/\___|____/ \__\___/| .__/____/ \___/ \__,_|_|
//                                        |_|
bool CommandLineInterface::ParseStopSoar(int argc, char**& argv) {
	if (CheckForHelp(argc, argv)) {
		m_Result += CLIConstants::kCLIStopSoarUsage;
		return true;
	}

	static struct GetOpt::option longOptions[] = {
		{"self",		0, 0, 's'},
		{0, 0, 0, 0}
	};

	GetOpt::optind = 0;
	GetOpt::opterr = 0;

	int option;
	bool self = false;

	for (;;) {
		option = m_GetOpt.GetOpt_Long (argc, argv, "s", longOptions, 0);
		if (option == -1) {
			break;
		}

		switch (option) {
			case 's':
				self = true;
				break;
			case '?':
				m_Result += "Unrecognized option.\n";
				m_Result += CLIConstants::kCLIStopSoarUsage;
				return false;
			default:
				m_Result += "Internal error: m_GetOpt.GetOpt_Long returned '";
				m_Result += option;
				m_Result += "'!";
				return false;
		}
	}

	string reasonForStopping;
	if (GetOpt::optind < argc) {
		while (GetOpt::optind < argc) {
			reasonForStopping += argv[GetOpt::optind++];
			reasonForStopping += ' ';
		}
	}
	return DoStopSoar(self, reasonForStopping.c_str());
}

// ____       ____  _             ____
//|  _ \  ___/ ___|| |_ ___  _ __/ ___|  ___   __ _ _ __
//| | | |/ _ \___ \| __/ _ \| '_ \___ \ / _ \ / _` | '__|
//| |_| | (_) |__) | || (_) | |_) |__) | (_) | (_| | |
//|____/ \___/____/ \__\___/| .__/____/ \___/ \__,_|_|
//                          |_|
bool CommandLineInterface::DoStopSoar(bool self, char const* reasonForStopping) {
	m_Result += "TODO: do stop-soar";
	return true;
}

// ____                   __        __    _       _
//|  _ \ __ _ _ __ ___  __\ \      / /_ _| |_ ___| |__
//| |_) / _` | '__/ __|/ _ \ \ /\ / / _` | __/ __| '_ \
//|  __/ (_| | |  \__ \  __/\ V  V / (_| | || (__| | | |
//|_|   \__,_|_|  |___/\___| \_/\_/ \__,_|\__\___|_| |_|
//
bool CommandLineInterface::ParseWatch(int argc, char**& argv) {
	if (CheckForHelp(argc, argv)) {
		m_Result += CLIConstants::kCLIWatchUsage;
		return true;
	}

	static struct GetOpt::option longOptions[] = {
		{"aliases",		            1, 0, 'a'},
		{"backtracing",		      1, 0, 'b'},
		{"chunks",		            1, 0, 'c'},
		{"decisions",		         1, 0, 'd'},
		{"default-productions",		1, 0, 'D'},
		{"indifferent-selection",  1, 0, 'i'},
		{"justifications",		   1, 0, 'j'},
		{"learning",		         1, 0, 'l'},
		{"loading",		            1, 0, 'L'},
		{"none",		               0, 0, 'n'},
		{"phases",		            1, 0, 'p'},
		{"productions",		      1, 0, 'P'},
		{"preferences",		      1, 0, 'r'},
		{"user-productions",		   1, 0, 'u'},
		{"wmes",		               1, 0, 'w'},
		{"wme-detail",		         1, 0, 'W'},
		{0, 0, 0, 0}
	};

	GetOpt::optind = 0;
	GetOpt::opterr = 0;

	int option;
	bool self = false;
	unsigned short options = 0;   // what flag changed
	unsigned short states = 0;    // new setting

	for (;;) {
		option = m_GetOpt.GetOpt_Long (argc, argv, "a:b:c:d:D:i:j:l:L:np:P:r:u:w:W:", longOptions, 0);
		if (option == -1) {
			break;
		}

		// convert the argument
		//      switch (option) {

		//      }

		switch (option) {
			case 'a':
				options |= OPTION_WATCH_ALIASES;
				break;
			case 'b':
				options |= OPTION_WATCH_BACKTRACING;
				break;
			case 'c':
				options |= OPTION_WATCH_CHUNKS;
				break;
			case 'd':
				options |= OPTION_WATCH_DECISIONS;
				break;
			case 'D':
				options |= OPTION_WATCH_DEFAULT_PRODUCTIONS;
				break;
			case 'i':
				options |= OPTION_WATCH_INDIFFERENT_SELECTION;
				break;
			case 'j':
				options |= OPTION_WATCH_JUSTIFICATIONS;
				break;
			case 'l':
				options |= OPTION_WATCH_LEARNING;
				break;
			case 'L':
				options |= OPTION_WATCH_LOADING;
				break;
			case 'n':
				options |= OPTION_WATCH_NONE;
				break;
			case 'p':
				options |= OPTION_WATCH_PHASES;
				break;
			case 'P':
				options |= OPTION_WATCH_PRODUCTIONS;
				break;
			case 'r':
				options |= OPTION_WATCH_PREFERENCES;
				break;
			case 'u':
				options |= OPTION_WATCH_USER_PRODUCTIONS;
				break;
			case 'w':
				options |= OPTION_WATCH_WMES;
				break;
			case 'W':
				options |= OPTION_WATCH_WME_DETAIL;
				break;
			case '?':
				m_Result += "Unrecognized option.\n";
				m_Result += CLIConstants::kCLIWatchUsage;
				return false;
			default:
				m_Result += "Internal error: m_GetOpt.GetOpt_Long returned '";
				m_Result += option;
				m_Result += "'!";
				return false;
		}
	}

	return DoWatch();
}

// ____     __        __    _       _
//|  _ \  __\ \      / /_ _| |_ ___| |__
//| | | |/ _ \ \ /\ / / _` | __/ __| '_ \
//| |_| | (_) \ V  V / (_| | || (__| | | |
//|____/ \___/ \_/\_/ \__,_|\__\___|_| |_|
//
bool CommandLineInterface::DoWatch() {
	m_Result += "TODO: DoWatch";
	return true;
}

// ____                   __        __    _       _  __        ____  __ _____
//|  _ \ __ _ _ __ ___  __\ \      / /_ _| |_ ___| |_\ \      / /  \/  | ____|___
//| |_) / _` | '__/ __|/ _ \ \ /\ / / _` | __/ __| '_ \ \ /\ / /| |\/| |  _| / __|
//|  __/ (_| | |  \__ \  __/\ V  V / (_| | || (__| | | \ V  V / | |  | | |___\__ \
//|_|   \__,_|_|  |___/\___| \_/\_/ \__,_|\__\___|_| |_|\_/\_/  |_|  |_|_____|___/
//
bool CommandLineInterface::ParseWatchWMEs(int argc, char**& argv) {
	if (CheckForHelp(argc, argv)) {
		m_Result += CLIConstants::kCLIWatchWMEsUsage;
		return true;
	}
	return DoWatchWMEs();
}

// ____     __        __    _       _  __        ____  __ _____
//|  _ \  __\ \      / /_ _| |_ ___| |_\ \      / /  \/  | ____|___
//| | | |/ _ \ \ /\ / / _` | __/ __| '_ \ \ /\ / /| |\/| |  _| / __|
//| |_| | (_) \ V  V / (_| | || (__| | | \ V  V / | |  | | |___\__ \
//|____/ \___/ \_/\_/ \__,_|\__\___|_| |_|\_/\_/  |_|  |_|_____|___/
//
bool CommandLineInterface::DoWatchWMEs() {
	m_Result += "TODO: DoWatchWMEs";
	return true;
}

// ____        _ _     _  ____                                          _ __  __
//| __ ) _   _(_) | __| |/ ___|___  _ __ ___  _ __ ___   __ _ _ __   __| |  \/  | __ _ _ __
//|  _ \| | | | | |/ _` | |   / _ \| '_ ` _ \| '_ ` _ \ / _` | '_ \ / _` | |\/| |/ _` | '_ \
//| |_) | |_| | | | (_| | |__| (_) | | | | | | | | | | | (_| | | | | (_| | |  | | (_| | |_) |
//|____/ \__,_|_|_|\__,_|\____\___/|_| |_| |_|_| |_| |_|\__,_|_| |_|\__,_|_|  |_|\__,_| .__/
//                                                                                    |_|
void CommandLineInterface::BuildCommandMap() {
	m_CommandMap[CLIConstants::kCLIAddWME]		= CommandLineInterface::ParseAddWME;
	m_CommandMap[CLIConstants::kCLICD]			= CommandLineInterface::ParseCD;
	m_CommandMap[CLIConstants::kCLIEcho]		= CommandLineInterface::ParseEcho;
	m_CommandMap[CLIConstants::kCLIExcise]		= CommandLineInterface::ParseExcise;
	m_CommandMap[CLIConstants::kCLIExit]		= CommandLineInterface::ParseQuit;
	m_CommandMap[CLIConstants::kCLIInitSoar]	= CommandLineInterface::ParseInitSoar;
	m_CommandMap[CLIConstants::kCLILearn]		= CommandLineInterface::ParseLearn;
	m_CommandMap[CLIConstants::kCLINewAgent]	= CommandLineInterface::ParseNewAgent;
	m_CommandMap[CLIConstants::kCLIQuit]		= CommandLineInterface::ParseQuit;
	m_CommandMap[CLIConstants::kCLIRun]			= CommandLineInterface::ParseRun;
	m_CommandMap[CLIConstants::kCLISource]		= CommandLineInterface::ParseSource;
	m_CommandMap[CLIConstants::kCLISP]			= CommandLineInterface::ParseSP;
	m_CommandMap[CLIConstants::kCLIStopSoar]	= CommandLineInterface::ParseStopSoar;
	m_CommandMap[CLIConstants::kCLIWatch]		= CommandLineInterface::ParseWatch;
	m_CommandMap[CLIConstants::kCLIWatchWMEs]	= CommandLineInterface::ParseWatchWMEs;
}

bool CommandLineInterface::CheckForHelp(int argc, char**& argv) {
	if (argc > 1) {
		string argv1 = argv[1];
		if (argv1 == "-h" || argv1 == "--help") {
			return true;
		}
	}
	return false;
}
