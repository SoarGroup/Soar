/////////////////////////////////////////////////////////////////
// CommandLineInterface class
//
// Author: Jonathan Voigt
// Date  : Sept 2004
//
/////////////////////////////////////////////////////////////////
#include "commandLineInterface.h"

#include <string>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <fstream>

#ifdef WIN32
#include <windows.h>
#include <winbase.h>
#include <direct.h>
#endif

// Not the real getopt, as that one has crazy side effects with windows libraries
#include "getopt.h"

// gSKI includes
#include "gSKI_Structures.h"
#include "IgSKI_ProductionManager.h"
#include "IgSKI_Agent.h"
#include "IgSKI_AgentManager.h"
#include "IgSKI_Kernel.h"
#include "IgSKI_DoNotTouch.h"

// SML includes
#include "sml_ElementXML.h"
#include "sml_TagResult.h"
#include "sml_TagError.h"

using namespace std;
using namespace cli;

//  ____ _     ___ ____                _              _
// / ___| |   |_ _/ ___|___  _ __  ___| |_ __ _ _ __ | |_ ___
//| |   | |    | | |   / _ \| '_ \/ __| __/ _` | '_ \| __/ __|
//| |___| |___ | | |__| (_) | | | \__ \ || (_| | | | | |_\__ \
// \____|_____|___\____\___/|_| |_|___/\__\__,_|_| |_|\__|___/
//
char const* CommandLineInterface::CLIConstants::kCLISyntaxError		= "Command syntax error.\n";

char const* CommandLineInterface::CLIConstants::kCLIAddWME			= "add-wme";
char const* CommandLineInterface::CLIConstants::kCLICD				= "cd";			// alias for ls
char const* CommandLineInterface::CLIConstants::kCLIDir				= "ls";
char const* CommandLineInterface::CLIConstants::kCLIEcho			= "echo";
char const* CommandLineInterface::CLIConstants::kCLIExcise			= "excise";
char const* CommandLineInterface::CLIConstants::kCLIExit			= "exit";		// alias for quit
char const* CommandLineInterface::CLIConstants::kCLIInitSoar		= "init-soar";
char const* CommandLineInterface::CLIConstants::kCLILearn			= "learn";
char const* CommandLineInterface::CLIConstants::kCLILS				= "ls";
char const* CommandLineInterface::CLIConstants::kCLIMultiAttributes	= "multi-attributes";
char const* CommandLineInterface::CLIConstants::kCLINewAgent		= "new-agent";
char const* CommandLineInterface::CLIConstants::kCLIPopD			= "popd";
char const* CommandLineInterface::CLIConstants::kCLIPrint			= "print";
char const* CommandLineInterface::CLIConstants::kCLIPushD			= "pushd";
char const* CommandLineInterface::CLIConstants::kCLIPWD				= "pwd";
char const* CommandLineInterface::CLIConstants::kCLIQuit			= "quit";
char const* CommandLineInterface::CLIConstants::kCLIRun				= "run";
char const* CommandLineInterface::CLIConstants::kCLISource			= "source";
char const* CommandLineInterface::CLIConstants::kCLISP				= "sp";
char const* CommandLineInterface::CLIConstants::kCLIStopSoar		= "stop-soar";
char const* CommandLineInterface::CLIConstants::kCLIWatch			= "watch";
char const* CommandLineInterface::CLIConstants::kCLIWatchWMEs		= "watch-wmes";

// _   _                         ____                _              _
//| | | |___  __ _  __ _  ___   / ___|___  _ __  ___| |_ __ _ _ __ | |_ ___
//| | | / __|/ _` |/ _` |/ _ \ | |   / _ \| '_ \/ __| __/ _` | '_ \| __/ __|
//| |_| \__ \ (_| | (_| |  __/ | |__| (_) | | | \__ \ || (_| | | | | |_\__ \
// \___/|___/\__,_|\__, |\___|  \____\___/|_| |_|___/\__\__,_|_| |_|\__|___/
//                 |___/
char const* CommandLineInterface::CLIConstants::kCLIAddWMEUsage				= "Usage:\tadd-wme";
char const* CommandLineInterface::CLIConstants::kCLICDUsage					= "Usage:\tcd [directory]";
char const* CommandLineInterface::CLIConstants::kCLIEchoUsage				= "Usage:\techo [string]";
char const* CommandLineInterface::CLIConstants::kCLIExciseUsage				= "Usage:\texcise production_name\n\texcise -[acdtu]";
char const* CommandLineInterface::CLIConstants::kCLIInitSoarUsage			= "Usage:\tinit-soar";
char const* CommandLineInterface::CLIConstants::kCLILearnUsage				= "Usage:\tlearn [-l]\n\tlearn -[d|e|E|o][ab]";
char const* CommandLineInterface::CLIConstants::kCLILSUsage					= "Usage:\tls";
char const* CommandLineInterface::CLIConstants::kCLIMultiAttributesUsage	= "Usage:\tmulti-attributes";
char const* CommandLineInterface::CLIConstants::kCLINewAgentUsage			= "Usage:\tnew-agent [agent_name]";
char const* CommandLineInterface::CLIConstants::kCLIPopDUsage				= "Usage:\tpopd";
char const* CommandLineInterface::CLIConstants::kCLIPrintUsage				= "Usage:\tprint [-fFin] production_name\n\tprint -[a|c|D|j|u][fFin]\n\tprint [-i] \
[-d <depth>] identifier|timetag|pattern\n\tprint -s[oS]";
char const* CommandLineInterface::CLIConstants::kCLIPushDUsage				= "Usage:\tpushd directory";
char const* CommandLineInterface::CLIConstants::kCLIPWDUsage				= "Usage:\tpwd";
char const* CommandLineInterface::CLIConstants::kCLIRunUsage				= "Usage:\trun [count]\n\trun -[d|e|p][fs] [count]\n\trun -[S|o|O][fs] [count]";
char const* CommandLineInterface::CLIConstants::kCLISourceUsage				= "Usage:\tsource filename";
char const* CommandLineInterface::CLIConstants::kCLISPUsage					= "Usage:\tsp { production }";
char const* CommandLineInterface::CLIConstants::kCLIStopSoarUsage			= "Usage:\tstop-soar [-s] [reason_string]";
char const* CommandLineInterface::CLIConstants::kCLIWatchUsage				= "Usage:\twatch [level] [-n] [-a <switch>] [-b <switch>] [-c <switch>] [-d <switch>] \
[-D <switch>] [-i <switch>] [-j <switch>] [-l <detail>] [-L <switch>] [-p <switch>] \
[-P <switch>] [-r <switch>] [-u <switch>] [-w <switch>] [-W <detail>]";
char const* CommandLineInterface::CLIConstants::kCLIWatchWMEsUsage			= "Usage:\twatch-wmes –[a|r] –t <type> pattern\n\twatch-wmes –[l|R] [–t <type>]";

//  ____                                          _ _     _            ___       _             __
// / ___|___  _ __ ___  _ __ ___   __ _ _ __   __| | |   (_)_ __   ___|_ _|_ __ | |_ ___ _ __ / _| __ _  ___ ___
//| |   / _ \| '_ ` _ \| '_ ` _ \ / _` | '_ \ / _` | |   | | '_ \ / _ \| || '_ \| __/ _ \ '__| |_ / _` |/ __/ _ \
//| |__| (_) | | | | | | | | | | | (_| | | | | (_| | |___| | | | |  __/| || | | | ||  __/ |  |  _| (_| | (_|  __/
// \____\___/|_| |_| |_|_| |_| |_|\__,_|_| |_|\__,_|_____|_|_| |_|\___|___|_| |_|\__\___|_|  |_|  \__,_|\___\___|
//
CommandLineInterface::CommandLineInterface() {

	// Map command names to processing function pointers
	BuildCommandMap();

	// Store current working directory as 'home' dir
	char buf[512];
	getcwd(buf, 512);
	m_HomeDirectory = buf;

	// Give print handler a reference to us
	m_PrintHandler.SetCLI(this);

	// Initialize other members
	m_QuitCalled = false;
	m_pKernel = 0;
	m_pAgent = 0;
}

// ____        _ _     _  ____                                          _ __  __
//| __ ) _   _(_) | __| |/ ___|___  _ __ ___  _ __ ___   __ _ _ __   __| |  \/  | __ _ _ __
//|  _ \| | | | | |/ _` | |   / _ \| '_ ` _ \| '_ ` _ \ / _` | '_ \ / _` | |\/| |/ _` | '_ \
//| |_) | |_| | | | (_| | |__| (_) | | | | | | | | | | | (_| | | | | (_| | |  | | (_| | |_) |
//|____/ \__,_|_|_|\__,_|\____\___/|_| |_| |_|_| |_| |_|\__,_|_| |_|\__,_|_|  |_|\__,_| .__/
//                                                                                    |_|
void CommandLineInterface::BuildCommandMap() {

	// Set the command map up mapping strings to function pointers
	m_CommandMap[CLIConstants::kCLIAddWME]			= CommandLineInterface::ParseAddWME;
	m_CommandMap[CLIConstants::kCLICD]				= CommandLineInterface::ParseCD;
	m_CommandMap[CLIConstants::kCLIDir]				= CommandLineInterface::ParseLS;		// dir->ls
	m_CommandMap[CLIConstants::kCLIEcho]			= CommandLineInterface::ParseEcho;
	m_CommandMap[CLIConstants::kCLIExcise]			= CommandLineInterface::ParseExcise;
	m_CommandMap[CLIConstants::kCLIExit]			= CommandLineInterface::ParseQuit;		// exit->quit
	m_CommandMap[CLIConstants::kCLIInitSoar]		= CommandLineInterface::ParseInitSoar;
	m_CommandMap[CLIConstants::kCLILearn]			= CommandLineInterface::ParseLearn;
	m_CommandMap[CLIConstants::kCLILS]				= CommandLineInterface::ParseLS;
	m_CommandMap[CLIConstants::kCLIMultiAttributes]	= CommandLineInterface::ParseMultiAttributes;
	m_CommandMap[CLIConstants::kCLINewAgent]		= CommandLineInterface::ParseNewAgent;
	m_CommandMap[CLIConstants::kCLIPopD]			= CommandLineInterface::ParsePopD;
	m_CommandMap[CLIConstants::kCLIPrint]			= CommandLineInterface::ParsePrint;
	m_CommandMap[CLIConstants::kCLIPushD]			= CommandLineInterface::ParsePushD;
	m_CommandMap[CLIConstants::kCLIPWD]				= CommandLineInterface::ParsePWD;
	m_CommandMap[CLIConstants::kCLIQuit]			= CommandLineInterface::ParseQuit;
	m_CommandMap[CLIConstants::kCLIRun]				= CommandLineInterface::ParseRun;
	m_CommandMap[CLIConstants::kCLISource]			= CommandLineInterface::ParseSource;
	m_CommandMap[CLIConstants::kCLISP]				= CommandLineInterface::ParseSP;
	m_CommandMap[CLIConstants::kCLIStopSoar]		= CommandLineInterface::ParseStopSoar;
	m_CommandMap[CLIConstants::kCLIWatch]			= CommandLineInterface::ParseWatch;
	m_CommandMap[CLIConstants::kCLIWatchWMEs]		= CommandLineInterface::ParseWatchWMEs;
}

// ____         ____                                          _
//|  _ \  ___  / ___|___  _ __ ___  _ __ ___   __ _ _ __   __| |
//| | | |/ _ \| |   / _ \| '_ ` _ \| '_ ` _ \ / _` | '_ \ / _` |
//| |_| | (_) | |__| (_) | | | | | | | | | | | (_| | | | | (_| |
//|____/ \___/ \____\___/|_| |_| |_|_| |_| |_|\__,_|_| |_|\__,_|
//
bool CommandLineInterface::DoCommand(gSKI::IAgent* pAgent, const char* pCommandLine, sml::ElementXML* pResponse, gSKI::Error* pError) {

	// Clear the result
	m_Result.clear();

	// Save the pointers
	m_pAgent = pAgent;
	m_pError = pError;

	// Process the command
	bool ret = DoCommandInternal(pCommandLine);

	// Marshall the result text and return
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

	// Parse command:
	int argc = Tokenize(commandLine, argumentVector);
	if (!argc) {
		return true;	// Nothing on the command line, so consider it processed OK
	} else if (argc == -1) {
		return false;	// Parsing failed, error in result
	}

	// Marshall arg{c/v} for getopt:
	// TODO: since we're not using gnu getopt anymore, we should lose
	// this step of using char* and just stick to the vector
	char** argv = new char*[argc + 1]; // leave space for extra null
	int arglen;

	// For each arg
	for (int i = 0; i < argc; ++i) {
		// Save its length
		arglen = argumentVector[i].length();

		// Leave space for null
		argv[i] = new char[ arglen + 1 ];

		// Copy the string
		strncpy(argv[i], argumentVector[i].data(), arglen);

		// Set final index to null
		argv[i][ arglen ] = 0;
	}
	// Set final index to null
	argv[argc] = 0;

	// Process command:
	// TODO: shouldn't this be a const passing since we don't want processCommand to mess with it?
	CommandFunction pFunction = m_CommandMap[argv[0]];

	// Is the command implemented
	if (!pFunction) {
		m_Result += "Command '";
		m_Result += argv[0];
		m_Result += "' not found or implemented.";
		return false;
	}
	
	// Make the call
	bool ret = (this->*pFunction)(argc, argv);

	// Free memory from argv
	for (int j = 0; j < argc; ++j) {
		delete [] argv[j];
	}
	delete [] argv;

	// Return what the Do function returned
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

		// Is there anything to work with?
		if(cmdline.empty()) {
			break;
		}

		// Remove leading whitespace
		iter = cmdline.begin();
		while (isspace(*iter)) {
			cmdline.erase(iter);

			if (!cmdline.length()) {
				//Nothing but space left
				break;
			}
			
			// Next character
			iter = cmdline.begin();
		}

		// Was it actually trailing whitespace?
		if (!cmdline.length()) {
			// Nothing left to do
			break;
		}

		// We have an argument
		++argc;
		arg.clear();
		// Use space as a delimiter unless inside quotes or brackets (nestable)
		while (!isspace(*iter) || quotes || brackets) {
			// Eat quotes but leave brackets
			if (*iter == '"') {
				// Flip the quotes flag
				quotes = !quotes;

				// Eat quotes (not adding them to arg)

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

				// Add to argument
				arg += (*iter);
			}

			// Delete the character and move on on
			cmdline.erase(iter);
			iter = cmdline.begin();

			// Are we at the end of the string?
			if (iter == cmdline.end()) {

				// Did they close their quotes or brackets?
				if (quotes || brackets) {
					m_Result += "No closing quotes/brackets found.";
					return -1;
				}
				break;
			}
		}

		// Store the arg
		argumentVector.push_back(arg);
	}

	// Return the number of args found
	return argc;
}

// Templates for new additions
//bool CommandLineInterface::Parse(int argc, char** argv) {
//	if (CheckForHelp(argc, argv)) {
//		m_Result += CLIConstants::kCLIUsage;
//		return true;
//	}
//	return Do();
//}
//
//bool CommandLineInterface::Do() {
//	m_Result += "TODO: ";
//	return true;
//}

// ____                        _       _     ___        ____  __ _____
//|  _ \ __ _ _ __ ___  ___   / \   __| | __| \ \      / /  \/  | ____|
//| |_) / _` | '__/ __|/ _ \ / _ \ / _` |/ _` |\ \ /\ / /| |\/| |  _|
//|  __/ (_| | |  \__ \  __// ___ \ (_| | (_| | \ V  V / | |  | | |___
//|_|   \__,_|_|  |___/\___/_/   \_\__,_|\__,_|  \_/\_/  |_|  |_|_____|
//
bool CommandLineInterface::ParseAddWME(int argc, char** argv) {
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
bool CommandLineInterface::ParseCD(int argc, char** argv) {
	if (CheckForHelp(argc, argv)) {
		m_Result += CLIConstants::kCLICDUsage;
		return true;
	}

	// Only takes one optional argument, the directory to change into
	if (argc > 2) {
		m_Result += CLIConstants::kCLISyntaxError;
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

	// If cd is typed by itself, return to original (home) directory
	if (!directory) {

		// Home dir set in constructor
		if (chdir(m_HomeDirectory.c_str())) {
			m_Result += "Could not change to home directory: ";
			m_Result += m_HomeDirectory;
			return false;
		}

		// Lets print the current working directory on success
		DoPWD();
		return true;
	}

	// Change to passed directory
	if (chdir(directory)) {
		m_Result += "Could not change to directory: ";
		m_Result += directory;
		return false;
	}

	// Lets print the current working directory on success
	DoPWD();
	return true;
}

// ____                     _____     _
//|  _ \ __ _ _ __ ___  ___| ____|___| |__   ___
//| |_) / _` | '__/ __|/ _ \  _| / __| '_ \ / _ \
//|  __/ (_| | |  \__ \  __/ |__| (__| | | | (_) |
//|_|   \__,_|_|  |___/\___|_____\___|_| |_|\___/
//
bool CommandLineInterface::ParseEcho(int argc, char** argv) {
	if (CheckForHelp(argc, argv)) {
		m_Result += CLIConstants::kCLIEchoUsage;
		return true;
	}

	// Very simple command, any number of arguments
	return DoEcho(argc, argv);
}

// ____        _____     _
//|  _ \  ___ | ____|___| |__   ___
//| | | |/ _ \|  _| / __| '_ \ / _ \
//| |_| | (_) | |__| (__| | | | (_) |
//|____/ \___/|_____\___|_| |_|\___/
//
bool CommandLineInterface::DoEcho(int argc, char** argv) {

	// Concatenate arguments (spaces between arguments are lost unless enclosed in quotes)
	for (int i = 1; i < argc; ++i) {
		m_Result += argv[i];
		m_Result += ' ';
	}

	// Chop off that last space we just added in the loop
	m_Result = m_Result.substr(0, m_Result.length() - 1);
	return true;
}

// ____                     _____          _
//|  _ \ __ _ _ __ ___  ___| ____|_  _____(_)___  ___
//| |_) / _` | '__/ __|/ _ \  _| \ \/ / __| / __|/ _ \
//|  __/ (_| | |  \__ \  __/ |___ >  < (__| \__ \  __/
//|_|   \__,_|_|  |___/\___|_____/_/\_\___|_|___/\___|
//
bool CommandLineInterface::ParseExcise(int argc, char** argv) {
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
		option = m_GetOpt.GetOpt_Long(argc, argv, "acdtu", longOptions, 0);
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

	// Pass the productions to the DoExcise function
	int productionCount = argc - GetOpt::optind;	// Number of Productions = total productions - processed productions
	char** productions = argv + GetOpt::optind;		// Advance pointer to first productions (GetOpt puts all non-options at the end)
	// productions == 0 if optind == argc because argv[argc] == 0

	// Make the call
	return DoExcise(options, productionCount, productions);
}

// ____        _____          _
//|  _ \  ___ | ____|_  _____(_)___  ___
//| | | |/ _ \|  _| \ \/ / __| / __|/ _ \
//| |_| | (_) | |___ >  < (__| \__ \  __/
//|____/ \___/|_____/_/\_\___|_|___/\___|
//
bool CommandLineInterface::DoExcise(unsigned short options, int productionCount, char** productions) {
	m_Result += "TODO: do excise";
	return true;
}

// ____                     ___       _ _   ____
//|  _ \ __ _ _ __ ___  ___|_ _|_ __ (_) |_/ ___|  ___   __ _ _ __
//| |_) / _` | '__/ __|/ _ \| || '_ \| | __\___ \ / _ \ / _` | '__|
//|  __/ (_| | |  \__ \  __/| || | | | | |_ ___) | (_) | (_| | |
//|_|   \__,_|_|  |___/\___|___|_| |_|_|\__|____/ \___/ \__,_|_|
//
bool CommandLineInterface::ParseInitSoar(int argc, char** argv) {
	if (CheckForHelp(argc, argv)) {
		m_Result += CLIConstants::kCLIInitSoarUsage;
		return true;
	}

	// No arguments
	if (argc != 1) {
		m_Result += CLIConstants::kCLISyntaxError;
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
	// Simply call reinitialize
	m_pAgent->Reinitialize();
	m_Result += "Agent reinitialized.";
	return true;
}

// ____                     _
//|  _ \ __ _ _ __ ___  ___| |    ___  __ _ _ __ _ __
//| |_) / _` | '__/ __|/ _ \ |   / _ \/ _` | '__| '_ \
//|  __/ (_| | |  \__ \  __/ |__|  __/ (_| | |  | | | |
//|_|   \__,_|_|  |___/\___|_____\___|\__,_|_|  |_| |_|
//
bool CommandLineInterface::ParseLearn(int argc, char** argv) {
	if (CheckForHelp(argc, argv)) {
		m_Result += CLIConstants::kCLILearnUsage;
		return true;
	}

	// Just return true for debugging of source command
	// TODO: Update towers of hanoi or something for the new syntax
	return DoLearn(0);

	//static struct GetOpt::option longOptions[] = {
	//	{"all-levels",		0, 0, 'a'},
	//	{"bottom-up",		0, 0, 'b'},
	//	{"disable",			0, 0, 'd'},
	//	{"off",				0, 0, 'd'},
	//	{"enable",			0, 0, 'e'},
	//	{"on",				0, 0, 'e'},
	//	{"except",			0, 0, 'E'},
	//	{"list",			   0, 0, 'l'},
	//	{"only",			   0, 0, 'o'},
	//	{0, 0, 0, 0}
	//};

	//GetOpt::optind = 0;
	//GetOpt::opterr = 0;

	//int option;
	//unsigned short options = 0;

	//for (;;) {
	//	option = m_GetOpt.GetOpt_Long(argc, argv, "abdeElo", longOptions, 0);
	//	if (option == -1) {
	//		break;
	//	}

	//	switch (option) {
	//		case 'a':
	//			options |= OPTION_LEARN_ALL_LEVELS;
	//			break;
	//		case 'b':
	//			options |= OPTION_LEARN_BOTTOM_UP;
	//			break;
	//		case 'd':
	//			options |= OPTION_LEARN_DISABLE;
	//			break;
	//		case 'e':
	//			options |= OPTION_LEARN_ENABLE;
	//			break;
	//		case 'E':
	//			options |= OPTION_LEARN_EXCEPT;
	//			break;
	//		case 'l':
	//			options |= OPTION_LEARN_LIST;
	//			break;
	//		case 'o':
	//			options |= OPTION_LEARN_ONLY;
	//			break;
	//		case '?':
	//			m_Result += "Unrecognized option.\n";
	//			m_Result += CLIConstants::kCLILearnUsage;
	//			return false;
	//		default:
	//			m_Result += "Internal error: m_GetOpt.GetOpt_Long returned '";
	//			m_Result += option;
	//			m_Result += "'!";
	//			return false;
	//	}
	//}

	//// No non-option arguments
	//if (GetOpt::optind != argc) {
	//	m_Result += CLIConstants::kCLISyntaxError;
	//	m_Result += CLIConstants::kCLILearnUsage;
	//	return false;
	//}

	//return DoLearn(options);
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

// ____                     _     ____
//|  _ \ __ _ _ __ ___  ___| |   / ___|
//| |_) / _` | '__/ __|/ _ \ |   \___ \
//|  __/ (_| | |  \__ \  __/ |___ ___) |
//|_|   \__,_|_|  |___/\___|_____|____/
//
bool CommandLineInterface::ParseLS(int argc, char** argv) {
	if (CheckForHelp(argc, argv)) {
		m_Result += CLIConstants::kCLILSUsage;
		return true;
	}

	// No arguments
	if (argc != 1) {
		m_Result += CLIConstants::kCLISyntaxError;
		m_Result += CLIConstants::kCLILSUsage;
		return false;
	}
	return DoLS();
}

// ____        _     ____
//|  _ \  ___ | |   / ___|
//| | | |/ _ \| |   \___ \
//| |_| | (_) | |___ ___) |
//|____/ \___/|_____|____/
//
bool CommandLineInterface::DoLS() {

#ifdef WIN32

	// Windows-specific directory listing
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;

	// Open a file find using the universal dos wildcard *.*
	hFind = FindFirstFile("*.*", &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE) {
		
		// Not a single file, or file system error, we'll just assume no files
		m_Result += "File not found.";

		// TODO: Although file not found isn't an error, this could be an error 
		// like permission denied, we should check for this
		return true;	
	}

	// At least one file found, concatinate additional ones with newlines
	do {
		// TODO: Columns and stats
		m_Result += FindFileData.cFileName;
		m_Result += '\n';
	} while (FindNextFile(hFind, &FindFileData));

	// Close the file find
	FindClose(hFind);

#else // WIN32
	m_Result += "TODO: ls on non-windows platforms";
#endif // WIN32

	return true;
}

// ____                     __  __       _ _   _    _   _   _        _ _           _
//|  _ \ __ _ _ __ ___  ___|  \/  |_   _| | |_(_)  / \ | |_| |_ _ __(_) |__  _   _| |_ ___  ___
//| |_) / _` | '__/ __|/ _ \ |\/| | | | | | __| | / _ \| __| __| '__| | '_ \| | | | __/ _ \/ __|
//|  __/ (_| | |  \__ \  __/ |  | | |_| | | |_| |/ ___ \ |_| |_| |  | | |_) | |_| | ||  __/\__ \
//|_|   \__,_|_|  |___/\___|_|  |_|\__,_|_|\__|_/_/   \_\__|\__|_|  |_|_.__/ \__,_|\__\___||___/
//
bool CommandLineInterface::ParseMultiAttributes(int argc, char** argv) {
	if (CheckForHelp(argc, argv)) {
		m_Result += CLIConstants::kCLIMultiAttributesUsage;
		return true;
	}
	return DoMultiAttributes();
}

// ____        __  __       _ _   _    _   _   _        _ _           _
//|  _ \  ___ |  \/  |_   _| | |_(_)  / \ | |_| |_ _ __(_) |__  _   _| |_ ___  ___
//| | | |/ _ \| |\/| | | | | | __| | / _ \| __| __| '__| | '_ \| | | | __/ _ \/ __|
//| |_| | (_) | |  | | |_| | | |_| |/ ___ \ |_| |_| |  | | |_) | |_| | ||  __/\__ \
//|____/ \___/|_|  |_|\__,_|_|\__|_/_/   \_\__|\__|_|  |_|_.__/ \__,_|\__\___||___/
//
bool CommandLineInterface::DoMultiAttributes() {
	m_Result += "TODO: multi-attributes";
	return true;
}

// ____                     _   _                _                    _
//|  _ \ __ _ _ __ ___  ___| \ | | _____      __/ \   __ _  ___ _ __ | |_
//| |_) / _` | '__/ __|/ _ \  \| |/ _ \ \ /\ / / _ \ / _` |/ _ \ '_ \| __|
//|  __/ (_| | |  \__ \  __/ |\  |  __/\ V  V / ___ \ (_| |  __/ | | | |_
//|_|   \__,_|_|  |___/\___|_| \_|\___| \_/\_/_/   \_\__, |\___|_| |_|\__|
//                                                   |___/
bool CommandLineInterface::ParseNewAgent(int argc, char** argv) {
	if (CheckForHelp(argc, argv)) {
		m_Result += CLIConstants::kCLINewAgentUsage;
		return true;
	}

	// One argument (agent name)
	if (argc != 2) {
		m_Result += CLIConstants::kCLISyntaxError;
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

// ____                     ____             ____
//|  _ \ __ _ _ __ ___  ___|  _ \ ___  _ __ |  _ \
//| |_) / _` | '__/ __|/ _ \ |_) / _ \| '_ \| | | |
//|  __/ (_| | |  \__ \  __/  __/ (_) | |_) | |_| |
//|_|   \__,_|_|  |___/\___|_|   \___/| .__/|____/
//                                    |_|
bool CommandLineInterface::ParsePopD(int argc, char** argv) {
	if (CheckForHelp(argc, argv)) {
		m_Result += CLIConstants::kCLIPopDUsage;
		return true;
	}

	// No arguments
	if (argc != 1) {
		m_Result += CLIConstants::kCLISyntaxError;
		m_Result += CLIConstants::kCLIPopDUsage;
		return false;
	}
	return DoPopD();
}

// ____        ____             ____
//|  _ \  ___ |  _ \ ___  _ __ |  _ \
//| | | |/ _ \| |_) / _ \| '_ \| | | |
//| |_| | (_) |  __/ (_) | |_) | |_| |
//|____/ \___/|_|   \___/| .__/|____/
//                       |_|
bool CommandLineInterface::DoPopD() {

	// There must be a directory on the stack to pop
	if (m_DirectoryStack.empty()) {
		m_Result += "Directory stack empty, no directory to change to.";
		return false;
	}

	// Change to the directory
	if (!DoCD(m_DirectoryStack.top().c_str())) {
		// cd failed, error message added in cd function
		return false;
	}

	// Pop the directory stack
	m_DirectoryStack.pop();
	return true;
}

// ____                     ____       _       _
//|  _ \ __ _ _ __ ___  ___|  _ \ _ __(_)_ __ | |_
//| |_) / _` | '__/ __|/ _ \ |_) | '__| | '_ \| __|
//|  __/ (_| | |  \__ \  __/  __/| |  | | | | | |_
//|_|   \__,_|_|  |___/\___|_|   |_|  |_|_| |_|\__|
//
bool CommandLineInterface::ParsePrint(int argc, char** argv) {
	if (CheckForHelp(argc, argv)) {
		m_Result += CLIConstants::kCLIPrintUsage;
		return true;
	}

	static struct GetOpt::option longOptions[] = {
		{"all",				0, 0, 'a'},
		{"chunks",			0, 0, 'c'},
		{"depth",			1, 0, 'd'},
		{"defaults",		0, 0, 'D'},
		{"full",			0, 0, 'f'},
		{"filename",		0, 0, 'F'},
		{"internal",		0, 0, 'i'},
		{"justifications",	0, 0, 'j'},
		{"name",			0, 0, 'n'},
		{"operators",		0, 0, 'o'},
		{"stack",			0, 0, 's'},
		{"states",			0, 0, 'S'},
		{"user",			0, 0, 'u'},
		{0, 0, 0, 0}
	};

	GetOpt::optind = 0;
	GetOpt::opterr = 0;

	int option;
	// TODO: in 8.5.2 this is current_agent(default_wme_depth)
	int depth = 1;
	unsigned short options = 0;

	for (;;) {
		option = m_GetOpt.GetOpt_Long(argc, argv, "acd:DfFijnosSu", longOptions, 0);
		if (option == -1) {
			break;
		}

		switch (option) {
			case 'a':
				options |= OPTION_PRINT_ALL;
				break;
			case 'c':
				options |= OPTION_PRINT_CHUNKS;
				break;
			case 'd':
				options |= OPTION_PRINT_DEPTH;
				depth = atoi(GetOpt::optarg);
				break;
			case 'D':
				options |= OPTION_PRINT_DEFAULTS;
				break;
			case 'f':
				options |= OPTION_PRINT_FULL;
				break;
			case 'F':
				options |= OPTION_PRINT_FILENAME;
				break;
			case 'i':
				options |= OPTION_PRINT_INTERNAL;
				break;
			case 'j':
				options |= OPTION_PRINT_JUSTIFICATIONS;
				break;
			case 'n':
				options |= OPTION_PRINT_NAME;
				break;
			case 'o':
				options |= OPTION_PRINT_OPERATORS;
				break;
			case 's':
				options |= OPTION_PRINT_STACK;
				break;
			case 'S':
				options |= OPTION_PRINT_STATES;
				break;
			case 'u':
				options |= OPTION_PRINT_USER;
				break;
			case '?':
				m_Result += "Unrecognized option.\n";
				m_Result += CLIConstants::kCLIPrintUsage;
				return false;
			default:
				m_Result += "Internal error: m_GetOpt.GetOpt_Long returned '";
				m_Result += option;
				m_Result += "'!";
				return false;
		}
	}

	// One additional optional argument
	if ((argc - GetOpt::optind) > 1) {
		m_Result += CLIConstants::kCLISyntaxError;
		m_Result += CLIConstants::kCLIPrintUsage;
		return false;
	}

	char* pArg = argv[GetOpt::optind]; // argv == 0 if optind == argc because argv[argc] == 0

	return DoPrint(options, depth, pArg);
}

// ____        ____       _       _
//|  _ \  ___ |  _ \ _ __(_)_ __ | |_
//| | | |/ _ \| |_) | '__| | '_ \| __|
//| |_| | (_) |  __/| |  | | | | | |_
//|____/ \___/|_|   |_|  |_|_| |_|\__|
//
bool CommandLineInterface::DoPrint(const unsigned short options, int depth, const char* pArg) {

	// Need kernel pointer for back door
	if (!m_pKernel) {
		m_Result += "No kernel pointer.";
		return false;
	}

	// Need agent pointer for function calls
	if (!m_pAgent) {
		m_Result += "No agent pointer.";
		return false;
	}

	// Attain the evil back door of doom, even though we aren't the TgD
	gSKI::EvilBackDoor::ITgDWorkArounds* pKernelHack = m_pKernel->getWorkaroundObject();

	// Check for stack print
	if (options & OPTION_PRINT_STACK) {
		m_pAgent->AddPrintListener(gSKIEVENT_PRINT, &m_PrintHandler);
		pKernelHack->PrintStackTrace(m_pAgent, (options & OPTION_PRINT_STATES) ? true : false, (options & OPTION_PRINT_OPERATORS) ? true : false);
		m_pAgent->RemovePrintListener(gSKIEVENT_PRINT, &m_PrintHandler);
		return true;
	}

	// Cache the flags since it makes function calls huge
	bool internal = (options & OPTION_PRINT_INTERNAL) ? true : false;
	bool filename = (options & OPTION_PRINT_FILENAME) ? true : false;
	bool full = (options & OPTION_PRINT_FULL) ? true : false;
	bool name = (options & OPTION_PRINT_NAME) ? true : false;

	// Check for the five general print options (all, chunks, defaults, justifications, user)
	if (options & OPTION_PRINT_ALL) {
		// TODO: Find out what is pArg is for
		// TODO: This would be much easier to read if we cached those flags
		m_pAgent->AddPrintListener(gSKIEVENT_PRINT, &m_PrintHandler);
        pKernelHack->PrintUser(m_pAgent, const_cast<char*>(pArg), internal, filename, full, DEFAULT_PRODUCTION_TYPE);
        pKernelHack->PrintUser(m_pAgent, const_cast<char*>(pArg), internal, filename, full, USER_PRODUCTION_TYPE);
        pKernelHack->PrintUser(m_pAgent, const_cast<char*>(pArg), internal, filename, full, CHUNK_PRODUCTION_TYPE);
        pKernelHack->PrintUser(m_pAgent, const_cast<char*>(pArg), internal, filename, full, JUSTIFICATION_PRODUCTION_TYPE);
		m_pAgent->RemovePrintListener(gSKIEVENT_PRINT, &m_PrintHandler);
		return true;
	}
	if (options & OPTION_PRINT_CHUNKS) {
		m_pAgent->AddPrintListener(gSKIEVENT_PRINT, &m_PrintHandler);
        pKernelHack->PrintUser(m_pAgent, const_cast<char*>(pArg), internal, filename, full, CHUNK_PRODUCTION_TYPE);
		m_pAgent->RemovePrintListener(gSKIEVENT_PRINT, &m_PrintHandler);
		return true;
	}
	if (options & OPTION_PRINT_DEFAULTS) {
		m_pAgent->AddPrintListener(gSKIEVENT_PRINT, &m_PrintHandler);
        pKernelHack->PrintUser(m_pAgent, const_cast<char*>(pArg), internal, filename, full, DEFAULT_PRODUCTION_TYPE);
		m_pAgent->RemovePrintListener(gSKIEVENT_PRINT, &m_PrintHandler);
		return true;
	}
	if (options & OPTION_PRINT_JUSTIFICATIONS) {
		m_pAgent->AddPrintListener(gSKIEVENT_PRINT, &m_PrintHandler);
        pKernelHack->PrintUser(m_pAgent, const_cast<char*>(pArg), internal, filename, full, JUSTIFICATION_PRODUCTION_TYPE);
		m_pAgent->RemovePrintListener(gSKIEVENT_PRINT, &m_PrintHandler);
		return true;
	}
	if (options & OPTION_PRINT_USER) {
		m_pAgent->AddPrintListener(gSKIEVENT_PRINT, &m_PrintHandler);
        pKernelHack->PrintUser(m_pAgent, const_cast<char*>(pArg), internal, filename, full, USER_PRODUCTION_TYPE);
		m_pAgent->RemovePrintListener(gSKIEVENT_PRINT, &m_PrintHandler);
		return true;
	}

	// Default to symbol print
	m_pAgent->AddPrintListener(gSKIEVENT_PRINT, &m_PrintHandler);
	pKernelHack->PrintSymbol(m_pAgent, const_cast<char*>(pArg), name, filename, internal, full, depth);
	m_pAgent->RemovePrintListener(gSKIEVENT_PRINT, &m_PrintHandler);
	return true;
}

// ____                     ____            _     ____
//|  _ \ __ _ _ __ ___  ___|  _ \ _   _ ___| |__ |  _ \
//| |_) / _` | '__/ __|/ _ \ |_) | | | / __| '_ \| | | |
//|  __/ (_| | |  \__ \  __/  __/| |_| \__ \ | | | |_| |
//|_|   \__,_|_|  |___/\___|_|    \__,_|___/_| |_|____/
//
bool CommandLineInterface::ParsePushD(int argc, char** argv) {
	if (CheckForHelp(argc, argv)) {
		m_Result += CLIConstants::kCLIPushDUsage;
		return true;
	}

	// Only takes one argument, the directory to change into
	if (argc != 2) {
		m_Result += CLIConstants::kCLISyntaxError;
		m_Result += CLIConstants::kCLIPushDUsage;
		return false;
	}
	return DoPushD(argv[1]);
}

// ____        ____            _     ____
//|  _ \  ___ |  _ \ _   _ ___| |__ |  _ \
//| | | |/ _ \| |_) | | | / __| '_ \| | | |
//| |_| | (_) |  __/| |_| \__ \ | | | |_| |
//|____/ \___/|_|    \__,_|___/_| |_|____/
//
bool CommandLineInterface::DoPushD(const char* pDirectory) {
	
	// Target directory required, checked in DoCD call.

	// Save the current (soon to be old) directory
	string oldDirectory;
	if (!GetCurrentWorkingDirectory(oldDirectory)) {
		// Error message added in function
		return false;
	}

	// Change to the new directory.
	if (!DoCD(pDirectory)) {
		return false;
	}

	// Directory change successful, store old directory and move on
	m_DirectoryStack.push(oldDirectory);
	return true;
}

// ____                     ______        ______
//|  _ \ __ _ _ __ ___  ___|  _ \ \      / /  _ \
//| |_) / _` | '__/ __|/ _ \ |_) \ \ /\ / /| | | |
//|  __/ (_| | |  \__ \  __/  __/ \ V  V / | |_| |
//|_|   \__,_|_|  |___/\___|_|     \_/\_/  |____/
//
bool CommandLineInterface::ParsePWD(int argc, char** argv) {
	if (CheckForHelp(argc, argv)) {
		m_Result += CLIConstants::kCLIPWDUsage;
		return true;
	}

	// No arguments to print working directory
	if (argc != 1) {
		m_Result += CLIConstants::kCLISyntaxError;
		m_Result += CLIConstants::kCLIPWDUsage;
		return false;
	}
	return DoPWD();
}

// ____        ______        ______
//|  _ \  ___ |  _ \ \      / /  _ \
//| | | |/ _ \| |_) \ \ /\ / /| | | |
//| |_| | (_) |  __/ \ V  V / | |_| |
//|____/ \___/|_|     \_/\_/  |____/
//
bool CommandLineInterface::DoPWD() {

	string directory;
	bool ret = GetCurrentWorkingDirectory(directory);

	// On success, working dir is in parameter, on failure it is empty so this statement has no effect
	m_Result += directory;

	return ret;
}

// ____                      ___        _ _
//|  _ \ __ _ _ __ ___  ___ / _ \ _   _(_) |_
//| |_) / _` | '__/ __|/ _ \ | | | | | | | __|
//|  __/ (_| | |  \__ \  __/ |_| | |_| | | |_
//|_|   \__,_|_|  |___/\___|\__\_\\__,_|_|\__|
//
bool CommandLineInterface::ParseQuit(int argc, char** argv) {
	// Quit needs no help
	return DoQuit();
}

// ____         ___        _ _
//|  _ \  ___  / _ \ _   _(_) |_
//| | | |/ _ \| | | | | | | | __|
//| |_| | (_) | |_| | |_| | | |_
//|____/ \___/ \__\_\\__,_|_|\__|
//
bool CommandLineInterface::DoQuit() {
	// Simply flip the quit flag
	m_QuitCalled = true; 

	// Toodles!
	m_Result += "Goodbye.";
	return true;
}

// ____                     ____
//|  _ \ __ _ _ __ ___  ___|  _ \ _   _ _ __
//| |_) / _` | '__/ __|/ _ \ |_) | | | | '_ \
//|  __/ (_| | |  \__ \  __/  _ <| |_| | | | |
//|_|   \__,_|_|  |___/\___|_| \_\\__,_|_| |_|
//
bool CommandLineInterface::ParseRun(int argc, char** argv) {
	if (CheckForHelp(argc, argv)) {
		m_Result += CLIConstants::kCLIRunUsage;
		return true;
	}

	static struct GetOpt::option longOptions[] = {
		{"decision",	0, 0, 'd'},
		{"elaboration",	0, 0, 'e'},
		{"forever",		0, 0, 'f'},
		{"operator",	0, 0, 'o'},
		{"output",		0, 0, 'O'},
		{"phase",		0, 0, 'p'},
		{"self",		0, 0, 's'},
		{"state",		0, 0, 'S'},
		{0, 0, 0, 0}
	};

	GetOpt::optind = 0;
	GetOpt::opterr = 0;

	int option;
	unsigned short options = 0;

	for (;;) {
		option = m_GetOpt.GetOpt_Long(argc, argv, "defoOpsS", longOptions, 0);
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

	// Count defaults to 1 (which are ignored if the options default, since they default to forever)
	int count = 1;

	// Only one non-option argument allowed, count
	if (GetOpt::optind == argc - 1) {

		// TODO: Probably should check and make sure this is an integer before calling atoi
		count = atoi(argv[GetOpt::optind]);

	} else if (GetOpt::optind < argc) {
		m_Result += CLIConstants::kCLISyntaxError;
		m_Result += CLIConstants::kCLIRunUsage;
		return false;
	}

	return DoRun(options, count);
}

// ____        ____
//|  _ \  ___ |  _ \ _   _ _ __
//| | | |/ _ \| |_) | | | | '_ \
//| |_| | (_) |  _ <| |_| | | | |
//|____/ \___/|_| \_\\__,_|_| |_|
//
bool CommandLineInterface::DoRun(const unsigned short options, int count) {

	// TODO: Rather tricky options
	if ((options & OPTION_RUN_OPERATOR) || (options & OPTION_RUN_OUTPUT) || (options & OPTION_RUN_STATE)) {
		m_Result += "Options { o, O, S } not implemented yet.";
		return false;
	}

	// Determine run unit, mutually exclusive so give smaller steps precedence, default to gSKI_RUN_DECISION_CYCLE
	egSKIRunType runType = gSKI_RUN_DECISION_CYCLE;
	if (options & OPTION_RUN_ELABORATION) {
		runType = gSKI_RUN_SMALLEST_STEP;

	} else if (options & OPTION_RUN_DECISION) {
		runType = gSKI_RUN_DECISION_CYCLE;

	} else if (options & OPTION_RUN_FOREVER) {
		// TODO: Forever is going to hang unless a lucky halt is achieved until we implement 
		// a way to interrupt it, so lets just avoid it with an error
		//runType = gSKI_RUN_FOREVER;	
		m_Result += "Forever option is not implemented yet.";
	}

	// If running self, an agent pointer is necessary.  Otherwise, a Kernel pointer is necessary.
	egSKIRunResult runResult;
	if (options & OPTION_RUN_SELF) {
		if (!m_pAgent) {
			m_Result += "Run self: no agent pointer.";
			return false;
		}
		m_pAgent->AddPrintListener(gSKIEVENT_PRINT, &m_PrintHandler);
		runResult = m_pAgent->RunInClientThread(runType, count, m_pError);
		m_pAgent->RemovePrintListener(gSKIEVENT_PRINT, &m_PrintHandler);
	} else {
		if (!m_pKernel) {
			m_Result += "Run: no kernel pointer.";
			return false;
		}
        m_pKernel->GetAgentManager()->ClearAllInterrupts();
        m_pKernel->GetAgentManager()->AddAllAgentsToRunList();
		runResult = m_pKernel->GetAgentManager()->RunInClientThread(runType, count, gSKI_INTERLEAVE_SMALLEST_STEP, m_pError);
	}

	// Check for error
	if (runResult == gSKI_RUN_ERROR) {
		m_Result += "Run failed.";
		return false;	// Hopefully details are in gSKI error message
	}

	m_Result += "\nRun successful: ";
	switch (runResult) {
		case gSKI_RUN_EXECUTING:
			m_Result += "(gSKI_RUN_EXECUTING)";						// the run is still executing
			break;
		case gSKI_RUN_INTERRUPTED:
			m_Result += "(gSKI_RUN_INTERRUPTED)";					// the run was interrupted
			break;
		case gSKI_RUN_COMPLETED:
			m_Result += "(gSKI_RUN_COMPLETED)";						// the run completed normally
			break;
		case gSKI_RUN_COMPLETED_AND_INTERRUPTED:					// an interrupt was requested, but the run completed first
			m_Result += "(gSKI_RUN_COMPLETED_AND_INTERRUPTED)";
			break;
		default:
			m_Result += "Unknown egSKIRunResult code returned.";
			return false;
	}
	return true;
}

// ____                     ____
//|  _ \ __ _ _ __ ___  ___/ ___|  ___  _   _ _ __ ___ ___
//| |_) / _` | '__/ __|/ _ \___ \ / _ \| | | | '__/ __/ _ \
//|  __/ (_| | |  \__ \  __/___) | (_) | |_| | | | (_|  __/
//|_|   \__,_|_|  |___/\___|____/ \___/ \__,_|_|  \___\___|
//
bool CommandLineInterface::ParseSource(int argc, char** argv) {
	if (CheckForHelp(argc, argv)) {
		m_Result += CLIConstants::kCLISourceUsage;
		return true;
	}

	if (argc != 2) {
		// Source requires a filename
		m_Result += CLIConstants::kCLISyntaxError;
		m_Result += CLIConstants::kCLISourceUsage;
		return false;

	} else if (argc > 2) {
		// but only one filename
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

	// Open the file
	ifstream soarFile;
	soarFile.open(filename);
	if (!soarFile.is_open()) {
		m_Result += "Failed to open file '";
		m_Result += filename;
		m_Result += "' for reading.";
		return false;
	}

	string line;					// Each line removed from the file
	string command;					// The command, sometimes spanning multiple lines
	string::size_type pos;			// Used to find braces on a line (triggering multiple line spanning commands)
	int braces = 0;					// Brace nest level (hopefully all braces are supposed to be closed)
	string::iterator iter;			// Iterator when parsing for braces and pounds

	// Go through each line of the file (Yay! C++ file parsing!)
	while (getline(soarFile, line)) {

		// Clear out the old command
		command.clear();

		// Remove leading whitespace
		iter = line.begin();
		while (isspace(*iter)) {
			line.erase(iter);

			if (!line.length()) {
				// Nothing but space left, next line
				continue;
			}
			
			// Next character
			iter = line.begin();
		}

		// Was it actually trailing whitespace?
		if (!line.length()) {
			// Nothing left to do
			continue;
		}

		// Is the first character a comment?
		if (*iter == '#') {
			// Yes, ignore
			continue;
		}

		// If there is a brace on the line, concatenate lines until the closing brace
		pos = line.find('{');

		if (pos != string::npos) {
			// While we are inside braces, stay in special parsing mode
			do {
				// Enter special parsing mode
				iter = line.begin();
				while (iter != line.end()) {
					// Go through each of the characters, counting brace nesting level
					if (*iter == '{') {
						++braces;
					}
					if (*iter == '}') {
						--braces;
					}
					// Next character
					++iter;
				}

				// We finished that line, add it to the command, and put the newline back on it (getline eats the newline)
				command += line + '\n';

				// Did we close all of the braces?
				if (!braces) {
					// Yes, break out of special parsing mode
					break;
				}

				// Get the next line from the file and repeat
			} while (getline(soarFile, line));

			// Did we break out because of closed braces or EOF?
			if (braces) {
				// EOF while still nested
				m_Result += "Unexpected end of file. Did you close all of your braces?";
				return false;
			}

			// We're good to go

		} else {
			// No braces on line, set command to line
			command = line;
		}

		// Fire off the command
		if (!DoCommandInternal(command.c_str())) {
			// Command failed, error in result
			return false;
		}	
	}

	soarFile.close();
	return true;

	//// Old gSKI LoadSourceFile implementation:
	////////////////////////////////////////////

	//// TODO: This needs to be reimplemented, since there are commands in with the soar
	//// productions and I'm not sure what the gSKI LoadSoarFile does with those commands
	//// (such as 'learn -on' 'pushd' etc.

	//// Load the file
	//pProductionManager->LoadSoarFile(filename, m_pError);

	//if(m_pError->Id != gSKI::gSKIERR_NONE) {
	//	m_Result += "Unable to source the file: ";
	//	m_Result += filename;
	//	return false;
	//}

	//// TODO: Print one * per loaded production, this will be easy if we parse it here.
	//m_Result += "File sourced successfully.";
	//return true;
}

// ____                     ____  ____
//|  _ \ __ _ _ __ ___  ___/ ___||  _ \
//| |_) / _` | '__/ __|/ _ \___ \| |_) |
//|  __/ (_| | |  \__ \  __/___) |  __/
//|_|   \__,_|_|  |___/\___|____/|_|
//
bool CommandLineInterface::ParseSP(int argc, char** argv) {
	if (CheckForHelp(argc, argv)) {
		m_Result += CLIConstants::kCLISPUsage;
		return true;
	}

	// One argument (in brackets)
	if (argc != 2) {
		m_Result += CLIConstants::kCLISyntaxError;
		m_Result += CLIConstants::kCLISPUsage;
		return false;
	}

	// Remove first and last characters (the braces)
	string production = argv[1];
	if (production.length() < 3) {
		m_Result += CLIConstants::kCLISyntaxError;
		m_Result += CLIConstants::kCLISPUsage;
		return false;
	}
	production = production.substr(1, production.length() - 2);

	return DoSP(production.c_str());
}

// ____       ____  ____
//|  _ \  ___/ ___||  _ \
//| | | |/ _ \___ \| |_) |
//| |_| | (_) |__) |  __/
//|____/ \___/____/|_|
//
bool CommandLineInterface::DoSP(const char* production) {
	// Must have production
	if (!production) {
		m_Result += "sp command requires a production.";
		return false;
	}

	// Must have agent to give production to
	if (!m_pAgent) {
		m_Result += "No agent pointer.";
		return false;
	}

	// Acquire production manager
	gSKI::IProductionManager *pProductionManager = m_pAgent->GetProductionManager();

	// Load the production
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
bool CommandLineInterface::ParseStopSoar(int argc, char** argv) {
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
		option = m_GetOpt.GetOpt_Long(argc, argv, "s", longOptions, 0);
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

	// Concatinate remaining args for 'reason'
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
bool CommandLineInterface::ParseWatch(int argc, char** argv) {
	if (CheckForHelp(argc, argv)) {
		m_Result += CLIConstants::kCLIWatchUsage;
		return true;
	}

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

	int option;
	bool self = false;
	unsigned short options = 0;   // what flag changed
	unsigned short states = 0;    // new setting

	for (;;) {
		option = m_GetOpt.GetOpt_Long(argc, argv, "a:b:c:d:D:i:j:l:L:np:P:r:u:w:W:", longOptions, 0);
		if (option == -1) {
			break;
		}

		// TODO: Arguments and more

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

	// TODO: arguments, if any
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
bool CommandLineInterface::ParseWatchWMEs(int argc, char** argv) {
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

//  ____ _               _    _____          _   _      _
// / ___| |__   ___  ___| | _|  ___|__  _ __| | | | ___| |_ __
//| |   | '_ \ / _ \/ __| |/ / |_ / _ \| '__| |_| |/ _ \ | '_ \
//| |___| | | |  __/ (__|   <|  _| (_) | |  |  _  |  __/ | |_) |
// \____|_| |_|\___|\___|_|\_\_|  \___/|_|  |_| |_|\___|_| .__/
//                                                       |_|
bool CommandLineInterface::CheckForHelp(int argc, char** argv) {

	// Standard help check if there is more than one argument
	if (argc > 1) {
		string argv1 = argv[1];

		// Is one of the two help strings present?
		if (argv1 == "-h" || argv1 == "--help") {
			return true;
		}
	}
	return false;
}

//    _                               _ _____     ____                 _ _
//   / \   _ __  _ __   ___ _ __   __| |_   _|__ |  _ \ ___  ___ _   _| | |_
//  / _ \ | '_ \| '_ \ / _ \ '_ \ / _` | | |/ _ \| |_) / _ \/ __| | | | | __|
// / ___ \| |_) | |_) |  __/ | | | (_| | | | (_) |  _ <  __/\__ \ |_| | | |_
///_/   \_\ .__/| .__/ \___|_| |_|\__,_| |_|\___/|_| \_\___||___/\__,_|_|\__|
//        |_|   |_|
void CommandLineInterface::AppendToResult(const char* pMessage) {
	// Simply add to result
	m_Result += pMessage;
}

// ____       _   _  __                    _
/// ___|  ___| |_| |/ /___ _ __ _ __   ___| |
//\___ \ / _ \ __| ' // _ \ '__| '_ \ / _ \ |
// ___) |  __/ |_| . \  __/ |  | | | |  __/ |
//|____/ \___|\__|_|\_\___|_|  |_| |_|\___|_|
//
void CommandLineInterface::SetKernel(gSKI::IKernel* pKernel) {
	m_pKernel = pKernel;
}

//  ____      _    ____                          _ __        __         _    _             ____  _               _
// / ___| ___| |_ / ___|   _ _ __ _ __ ___ _ __ | |\ \      / /__  _ __| | _(_)_ __   __ _|  _ \(_)_ __ ___  ___| |_ ___  _ __ _   _
//| |  _ / _ \ __| |  | | | | '__| '__/ _ \ '_ \| __\ \ /\ / / _ \| '__| |/ / | '_ \ / _` | | | | | '__/ _ \/ __| __/ _ \| '__| | | |
//| |_| |  __/ |_| |__| |_| | |  | | |  __/ | | | |_ \ V  V / (_) | |  |   <| | | | | (_| | |_| | | | |  __/ (__| || (_) | |  | |_| |
// \____|\___|\__|\____\__,_|_|  |_|  \___|_| |_|\__| \_/\_/ \___/|_|  |_|\_\_|_| |_|\__, |____/|_|_|  \___|\___|\__\___/|_|   \__, |
//                                                                                   |___/                                     |___/
bool CommandLineInterface::GetCurrentWorkingDirectory(string& directory) {
	// Pull an arbitrary buffer size of 1024 out of a hat and use it
	char buf[1024];
	char* ret = getcwd(buf, 1024);

	// If getcwd returns 0, that is bad
	if (!ret) {
		m_Result += "Couldn't get working directory.";
		return false;
	}

	// Store directory in output parameter and return success
	directory = buf;
	return true;
}
