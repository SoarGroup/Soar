/////////////////////////////////////////////////////////////////
// CommandLineInterface class
//
// Author: Jonathan Voigt
// Date  : Sept 2004
//
/////////////////////////////////////////////////////////////////
#include "cli_CommandLineInterface.h"

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

#define snprintf _snprintf

#endif // WIN32

#include "cli_GetOpt.h"	// Not the real getopt, as that one has crazy side effects with windows libraries
#include "cli_Constants.h"

// gSKI includes
#include "gSKI_Structures.h"
#include "IgSKI_ProductionManager.h"
#include "IgSKI_Agent.h"
#include "IgSKI_AgentManager.h"
#include "IgSKI_Kernel.h"
#include "IgSKI_DoNotTouch.h"
#include "IgSKI_Iterator.h"
#include "IgSKI_Production.h"
#include "IgSKI_MultiAttribute.h"

// SML includes
#include "sml_ElementXML.h"
#include "sml_TagResult.h"
#include "sml_TagError.h"

using namespace std;
using namespace cli;

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
	m_SourceError = false;
	m_SourceDepth = 0;
	m_SourceDirDepth = 0;
}

// ____        _ _     _  ____                                          _ __  __
//| __ ) _   _(_) | __| |/ ___|___  _ __ ___  _ __ ___   __ _ _ __   __| |  \/  | __ _ _ __
//|  _ \| | | | | |/ _` | |   / _ \| '_ ` _ \| '_ ` _ \ / _` | '_ \ / _` | |\/| |/ _` | '_ \
//| |_) | |_| | | | (_| | |__| (_) | | | | | | | | | | | (_| | | | | (_| | |  | | (_| | |_) |
//|____/ \__,_|_|_|\__,_|\____\___/|_| |_| |_|_| |_| |_|\__,_|_| |_|\__,_|_|  |_|\__,_| .__/
//                                                                                    |_|
void CommandLineInterface::BuildCommandMap() {

	// Set the command map up mapping strings to command data
	CommandFunctionAndUsage command;

	// add-wme
	command.first = ParseAddWME; command.second = Constants::kCLIAddWMEUsage;
	m_CommandMap[Constants::kCLIAddWME] = command;

	// cd
	command.first = ParseCD; command.second = Constants::kCLICDUsage;
	m_CommandMap[Constants::kCLICD] = command;

	// dir -> ls
	command.first = ParseLS; command.second = Constants::kCLILSUsage;
	m_CommandMap[Constants::kCLIDir] = command;

	// echo
	command.first = ParseEcho; command.second = Constants::kCLIEchoUsage;
	m_CommandMap[Constants::kCLIEcho] = command;

	// excise
	command.first = ParseExcise; command.second = Constants::kCLIExciseUsage;
	m_CommandMap[Constants::kCLIExcise] = command;

	// exit -> quit
	command.first = ParseQuit; command.second = Constants::kCLIQuitUsage;
	m_CommandMap[Constants::kCLIExit] = command;

	// init -> init-soar
	command.first = ParseInitSoar; command.second = Constants::kCLIInitSoarUsage;
	m_CommandMap[Constants::kCLIInit] = command;

	// init-soar
	command.first = ParseInitSoar; command.second = Constants::kCLIInitSoarUsage;
	m_CommandMap[Constants::kCLIInitSoar] = command;

	// learn
	command.first = ParseLearn; command.second = Constants::kCLILearnUsage;
	m_CommandMap[Constants::kCLILearn] = command;

	// log
	command.first = ParseLog; command.second = Constants::kCLILogUsage;
	m_CommandMap[Constants::kCLILog] = command;

	// ls
	command.first = ParseLS; command.second = Constants::kCLILSUsage;
	m_CommandMap[Constants::kCLILS] = command;

	// multi-attributes
	command.first = ParseMultiAttributes; command.second = Constants::kCLIMultiAttributesUsage;
	m_CommandMap[Constants::kCLIMultiAttributes] = command;

	// popd
	command.first = ParsePopD; command.second = Constants::kCLIPopDUsage;
	m_CommandMap[Constants::kCLIPopD] = command;

	// print
	command.first = ParsePrint; command.second = Constants::kCLIPrintUsage;
	m_CommandMap[Constants::kCLIPrint] = command;

	// pushd
	command.first = ParsePushD; command.second = Constants::kCLIPushDUsage;
	m_CommandMap[Constants::kCLIPushD] = command;

	// pwd
	command.first = ParsePWD; command.second = Constants::kCLIPWDUsage;
	m_CommandMap[Constants::kCLIPWD] = command;

	// quit
	command.first = ParseQuit; command.second = Constants::kCLIQuitUsage;
	m_CommandMap[Constants::kCLIQuit] = command;

	// run
	command.first = ParseRun; command.second = Constants::kCLIRunUsage;
	m_CommandMap[Constants::kCLIRun] = command;

	// source
	command.first = ParseSource; command.second = Constants::kCLISourceUsage;
	m_CommandMap[Constants::kCLISource] = command;

	// sp
	command.first = ParseSP; command.second = Constants::kCLISPUsage;
	m_CommandMap[Constants::kCLISP] = command;

	// stop-soar
	command.first = ParseStopSoar; command.second = Constants::kCLIStopSoarUsage;
	m_CommandMap[Constants::kCLIStopSoar] = command;

	// time
	command.first = ParseTime; command.second = Constants::kCLITimeUsage;
	m_CommandMap[Constants::kCLITime] = command;

	// watch
	command.first = ParseWatch; command.second = Constants::kCLIWatchUsage;
	m_CommandMap[Constants::kCLIWatch] = command;

	// watch-wmes
	command.first = ParseWatchWMEs; command.second = Constants::kCLIWatchWMEsUsage;
	m_CommandMap[Constants::kCLIWatchWMEs] = command;
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

	// Reset source error flag
	m_SourceError = false;

	// Marshall the result text and return
	sml::TagResult* pTag = new sml::TagResult();
	pTag->SetCharacterData(m_Result.c_str());
	pResponse->AddChild(pTag);
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

	// Is the command implemented?
	if (m_CommandMap.find(argv[0]) == m_CommandMap.end()) {
		m_Result += "Command '";
		m_Result += argv[0];
		m_Result += "' not found or implemented.";
		return false;
	}

	// Check for help flags
	if (CheckForHelp(argc, argv)) {
		// Help flags found, add help to line, return true
		m_Result += (m_CommandMap[argv[0]]).second;
		return true;
	}

	// Process command
	CommandFunction pFunction = (m_CommandMap[argv[0]]).first;

	// Just in case...
	if (!pFunction) {
		// Very odd, should be set in BuildCommandMap
		m_Result += "Command found but function pointer is null.";
		return false;
	}
	
	// Make the call
	// TODO: shouldn't this be a const passing since we don't want processCommand to mess with it?
	// But isn't getopt's intended behavior to mess with it?
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

				// TODO: Eat quotes?  I used to but this screws up the sp command

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
			}

			// Add to argument (if we eat quotes, this has to be moved into the else above
			arg += (*iter);

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
	// Only takes one optional argument, the directory to change into
	if (argc > 2) {
		return HandleSyntaxError(Constants::kCLICDUsage);
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
		return true;
	}

	// Chop of quotes if they are there, chdir doesn't like them
	string directoryString = directory;
	if ((directoryString.length() > 2) && (directoryString[0] == '\"') && (directoryString[directoryString.length() - 1] == '\"')) {
		directoryString = directoryString.substr(1, directoryString.length() - 2);
	}

	// Change to passed directory
	if (chdir(directoryString.c_str())) {
		m_Result += "Could not change to directory: ";
		m_Result += directoryString;
		return false;
	}
	return true;
}

// ____                     _____     _
//|  _ \ __ _ _ __ ___  ___| ____|___| |__   ___
//| |_) / _` | '__/ __|/ _ \  _| / __| '_ \ / _ \
//|  __/ (_| | |  \__ \  __/ |__| (__| | | | (_) |
//|_|   \__,_|_|  |___/\___|_____\___|_| |_|\___/
//
bool CommandLineInterface::ParseEcho(int argc, char** argv) {
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
				return HandleSyntaxError(Constants::kCLIExciseUsage, "Unrecognized option.\n");
			default:
				return HandleGetOptError(option);
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
	// Must have agent to excise from
	if (!RequireAgent()) {
		return false;
	}

	// Acquire production manager
	gSKI::IProductionManager *pProductionManager = m_pAgent->GetProductionManager();

	// Process the general options
	if (options & OPTION_EXCISE_ALL) {
		ExciseInternal(pProductionManager->GetAllProductions());
	}
	if (options & OPTION_EXCISE_CHUNKS) {
		ExciseInternal(pProductionManager->GetChunks());
		ExciseInternal(pProductionManager->GetJustifications());
	}
	if (options & OPTION_EXCISE_DEFAULT) {
		ExciseInternal(pProductionManager->GetDefaultProductions());
	}
	if (options & OPTION_EXCISE_TASK) {
		ExciseInternal(pProductionManager->GetChunks());
		ExciseInternal(pProductionManager->GetJustifications());
		ExciseInternal(pProductionManager->GetUserProductions());
	}
	if (options & OPTION_EXCISE_USER) {
		ExciseInternal(pProductionManager->GetUserProductions());
	}

	// Excise specific productions
	gSKI::tIProductionIterator* pProdIter;
	for (int i = 0; i < productionCount; ++i) {
		// Iterate through productions
		pProdIter = pProductionManager->GetProduction(productions[i]);
		if (pProdIter->GetNumElements()) {
			ExciseInternal(pProdIter);
		} else {
			m_Result += "Production not found: ";
			m_Result += productions[i];
			return false;
		}
	}
	return true;
}

// _____          _          ___       _                        _
//| ____|_  _____(_)___  ___|_ _|_ __ | |_ ___ _ __ _ __   __ _| |
//|  _| \ \/ / __| / __|/ _ \| || '_ \| __/ _ \ '__| '_ \ / _` | |
//| |___ >  < (__| \__ \  __/| || | | | ||  __/ |  | | | | (_| | |
//|_____/_/\_\___|_|___/\___|___|_| |_|\__\___|_|  |_| |_|\__,_|_|
//
void CommandLineInterface::ExciseInternal(gSKI::tIProductionIterator *pProdIter) {
	// Iterate through the productions using the production iterator and
	// excise and release.  Print one # per excised production.
	while(pProdIter->IsValid()) {
		gSKI::IProduction* ip = pProdIter->GetVal();
		ip->Excise();
		ip->Release();
		pProdIter->Next();
		m_Result += '#';
	}
	pProdIter->Release();
}


// ____                     ___       _ _   ____
//|  _ \ __ _ _ __ ___  ___|_ _|_ __ (_) |_/ ___|  ___   __ _ _ __
//| |_) / _` | '__/ __|/ _ \| || '_ \| | __\___ \ / _ \ / _` | '__|
//|  __/ (_| | |  \__ \  __/| || | | | | |_ ___) | (_) | (_| | |
//|_|   \__,_|_|  |___/\___|___|_| |_|_|\__|____/ \___/ \__,_|_|
//
bool CommandLineInterface::ParseInitSoar(int argc, char** argv) {
	// No arguments
	if (argc != 1) {
		return HandleSyntaxError(Constants::kCLIInitSoarUsage);
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
	static struct GetOpt::option longOptions[] = {
		{"all-levels",		0, 0, 'a'},
		{"bottom-up",		0, 0, 'b'},
		{"disable",			0, 0, 'd'},
		{"off",				0, 0, 'd'},
		{"enable",			0, 0, 'e'},
		{"on",				0, 0, 'e'},
		{"except",			0, 0, 'E'},
		{"list",			0, 0, 'l'},
		{"only",			0, 0, 'o'},
		{0, 0, 0, 0}
	};

	GetOpt::optind = 0;
	GetOpt::opterr = 0;

	int option;
	unsigned short options = 0;

	for (;;) {
		option = m_GetOpt.GetOpt_Long(argc, argv, "abdeElo", longOptions, 0);
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
				return HandleSyntaxError(Constants::kCLILearnUsage, "Unrecognized option.\n");
			default:
				return HandleGetOptError(option);
		}
	}

	// No non-option arguments
	if (GetOpt::optind != argc) {
		return HandleSyntaxError(Constants::kCLILearnUsage);
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
	// Need agent pointer for function calls
	if (!RequireAgent()) {
		return false;
	}


	// No options means print current settings
	if (!options) {
		// TODO: This is legacy output.  Bad.
		m_Result += "Current learn settings:\n   -";
		m_Result += m_pAgent->IsLearningOn() ? "on" : "off";
		return true;
	}

	// Check for unimplemented options
	if ((options & OPTION_LEARN_ALL_LEVELS) || (options & OPTION_LEARN_BOTTOM_UP) || (options & OPTION_LEARN_EXCEPT) || (options & OPTION_LEARN_LIST) || (options & OPTION_LEARN_ONLY)) {
		m_Result += "Options {abElo} are not implemented.";
		return false;
	}

	// Enable or disable, priority to disable
	if (options & OPTION_LEARN_ENABLE) {
		m_pAgent->SetLearning(true);
	}

	if (options & OPTION_LEARN_DISABLE) {
		m_pAgent->SetLearning(false);
	}
	return true;
}

// ____                     _
//|  _ \ __ _ _ __ ___  ___| |    ___   __ _
//| |_) / _` | '__/ __|/ _ \ |   / _ \ / _` |
//|  __/ (_| | |  \__ \  __/ |__| (_) | (_| |
//|_|   \__,_|_|  |___/\___|_____\___/ \__, |
//                                     |___/
bool CommandLineInterface::ParseLog(int argc, char** argv) {
	return DoLog(false);
}

// ____        _
//|  _ \  ___ | |    ___   __ _
//| | | |/ _ \| |   / _ \ / _` |
//| |_| | (_) | |__| (_) | (_| |
//|____/ \___/|_____\___/ \__, |
//                        |___/
bool CommandLineInterface::DoLog(bool close, const char* filename) {
	m_Result += "TODO: log";
	return true;
}

// ____                     _     ____
//|  _ \ __ _ _ __ ___  ___| |   / ___|
//| |_) / _` | '__/ __|/ _ \ |   \___ \
//|  __/ (_| | |  \__ \  __/ |___ ___) |
//|_|   \__,_|_|  |___/\___|_____|____/
//
bool CommandLineInterface::ParseLS(int argc, char** argv) {
	// No arguments
	if (argc != 1) {
		return HandleSyntaxError(Constants::kCLILSUsage);
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
		if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			m_Result += '[';
		}
		m_Result += FindFileData.cFileName;
		if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			m_Result += ']';
		}
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
	// No more than three arguments
	if (argc > 3) {
		return HandleSyntaxError(Constants::kCLIMultiAttributesUsage);
	}

	char* attribute = 0;
	int n = 0;

	// If we have 3 arguments, third one is an integer
	if (argc > 2) {
		if (!IsInteger(argv[2])) {
			// Must be an integer
			return HandleSyntaxError(Constants::kCLIMultiAttributesUsage, "Third argument must be an integer.\n");
		}
		n = atoi(argv[2]);
		if (n <= 0) {
			// Must be non-negative and greater than 0
			return HandleSyntaxError(Constants::kCLIMultiAttributesUsage, "Third argument must be greater than 0.\n");
		}
	}

	// If we have two arguments, second arg is an attribute/identifer/whatever
	if (argc > 1) {
		attribute = argv[1];
	}

	return DoMultiAttributes(attribute, n);
}

// ____        __  __       _ _   _    _   _   _        _ _           _
//|  _ \  ___ |  \/  |_   _| | |_(_)  / \ | |_| |_ _ __(_) |__  _   _| |_ ___  ___
//| | | |/ _ \| |\/| | | | | | __| | / _ \| __| __| '__| | '_ \| | | | __/ _ \/ __|
//| |_| | (_) | |  | | |_| | | |_| |/ ___ \ |_| |_| |  | | |_) | |_| | ||  __/\__ \
//|____/ \___/|_|  |_|\__,_|_|\__|_/_/   \_\__|\__|_|  |_|_.__/ \__,_|\__\___||___/
//
bool CommandLineInterface::DoMultiAttributes(const char* attribute, int n) {
	if (!RequireAgent()) {
		return false;
	}

	if (!attribute && !n) {
		// No args, print current setting
		gSKI::tIMultiAttributeIterator* pIt = m_pAgent->GetMultiAttributes();
		if (!pIt->GetNumElements()) {
			m_Result += "No multi-attributes declared for this agent.";

		} else {
			m_Result += "Value\tSymbol";
			gSKI::IMultiAttribute* pMA;

			// Arbitrary buffer size
			char buf[1024];

			for(; pIt->IsValid(); pIt->Next()) {
				pMA = pIt->GetVal();
				memset(buf, 0, sizeof(buf));
				snprintf(buf, sizeof(buf) - 1, "\n%d\t%s", pMA->GetMatchingPriority(), pMA->GetAttributeName());
				m_Result += buf;
				pMA->Release();
			}

		}
		pIt->Release();
		return true;
	}

	// TODO: Check whether attribute is a valid symbolic constant. The way the old kernel
	// does this is to call get_lexeme_from_string(m_agent, argv[1]) and check that
	// the lex type is symbolic constant. At this point, that functionality isn't
	// exposed by gSKI...

	// Setting defaults to 10
	if (!n) {
		n = 10;
	}

	// Set it
	m_pAgent->SetMultiAttribute(attribute, n);

 	return true;
}

// ____                     ____             ____
//|  _ \ __ _ _ __ ___  ___|  _ \ ___  _ __ |  _ \
//| |_) / _` | '__/ __|/ _ \ |_) / _ \| '_ \| | | |
//|  __/ (_| | |  \__ \  __/  __/ (_) | |_) | |_| |
//|_|   \__,_|_|  |___/\___|_|   \___/| .__/|____/
//                                    |_|
bool CommandLineInterface::ParsePopD(int argc, char** argv) {
	// No arguments
	if (argc != 1) {
		return HandleSyntaxError(Constants::kCLIPopDUsage);
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

	// If we're sourcing, this will be non-negative
	if (m_SourceDirDepth >= 0) {
		// And if it is, decrement it for each dir removed from the stack
		--m_SourceDirDepth;
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
				if (!IsInteger(GetOpt::optarg)) {
					return HandleSyntaxError(Constants::kCLIPrintUsage, "Depth must be an integer.\n");
				}
				depth = atoi(GetOpt::optarg);
				if (depth < 0) {
					return HandleSyntaxError(Constants::kCLIPrintUsage, "Depth must be non-negative.\n");
				}
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
				return HandleSyntaxError(Constants::kCLIPrintUsage, "Unrecognized option.\n");
			default:
				return HandleGetOptError(option);
		}
	}

	// One additional optional argument
	if ((argc - GetOpt::optind) > 1) {
		return HandleSyntaxError(Constants::kCLIPrintUsage);
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
	// Need agent pointer for function calls
	if (!RequireAgent()) {
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
	// Only takes one argument, the directory to change into
	if (argc != 2) {
		return HandleSyntaxError(Constants::kCLIPushDUsage);
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

	// If we're sourcing, this will be non-negative
	if (m_SourceDirDepth >= 0) {
		// And if it is, increment it for each dir placed on the stack
		++m_SourceDirDepth;
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
	// No arguments to print working directory
	if (argc != 1) {
		return HandleSyntaxError(Constants::kCLIPWDUsage);
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
				return HandleSyntaxError(Constants::kCLIRunUsage, "Unrecognized option.\n");
			default:
				return HandleGetOptError(option);
		}
	}

	// Count defaults to 1 (which are ignored if the options default, since they default to forever)
	int count = 1;

	// Only one non-option argument allowed, count
	if (GetOpt::optind == argc - 1) {

		if (!IsInteger(argv[GetOpt::optind])) {
			return HandleSyntaxError(Constants::kCLIRunUsage, "Count must be an integer.\n");
		}
		count = atoi(argv[GetOpt::optind]);
		if (count <= 0) {
			return HandleSyntaxError(Constants::kCLIRunUsage, "Count must be greater than 0.\n");
		}

	} else if (GetOpt::optind < argc) {
		return HandleSyntaxError(Constants::kCLIRunUsage);
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
		return false;
	}

	// If running self, an agent pointer is necessary.  Otherwise, a Kernel pointer is necessary.
	egSKIRunResult runResult;
	if (options & OPTION_RUN_SELF) {
		if (!RequireAgent()) {
			return false;
		}

		m_pAgent->AddPrintListener(gSKIEVENT_PRINT, &m_PrintHandler);
		runResult = m_pAgent->RunInClientThread(runType, count, m_pError);
		m_pAgent->RemovePrintListener(gSKIEVENT_PRINT, &m_PrintHandler);
	} else {
		m_pAgent->AddPrintListener(gSKIEVENT_PRINT, &m_PrintHandler);
        m_pKernel->GetAgentManager()->ClearAllInterrupts();
        m_pKernel->GetAgentManager()->AddAllAgentsToRunList();
		runResult = m_pKernel->GetAgentManager()->RunInClientThread(runType, count, gSKI_INTERLEAVE_SMALLEST_STEP, m_pError);
		m_pAgent->RemovePrintListener(gSKIEVENT_PRINT, &m_PrintHandler);
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
	if (argc != 2) {
		// Source requires a filename
		return HandleSyntaxError(Constants::kCLISourceUsage);

	} else if (argc > 2) {
		// but only one filename
		return HandleSyntaxError(Constants::kCLISourceUsage, "Source only one file at a time.\n");
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

	if (!RequireAgent()) {
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
	int lineCount = 0;				// Count the lines per file
	int lineCountCache = 0;			// Used to save a line number

	// Set dir depth to zero on first call to source, even though it should be zero anyway
	if (m_SourceDepth == 0) {
		m_SourceDirDepth = 0;
	}
	++m_SourceDepth;

	// Go through each line of the file (Yay! C++ file parsing!)
	while (getline(soarFile, line)) {
	
		// Increment line count
		++lineCount;

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
			
			// Save this line number for error messages
			lineCountCache = lineCount;

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

				// Did we go negative?
				if (braces < 0) {
					// Yes, break out on error
					break;
				}

				// We're getting another line, increment count now
				++lineCount;

				// Get the next line from the file and repeat
			} while (getline(soarFile, line));

			// Did we break out because of closed braces or EOF?
			if (braces > 0) {
				// EOF while still nested
				m_Result += "Unexpected end of file. Unmatched opening brace.";
				HandleSourceError(lineCountCache, filename);
				return false;

			} else if (braces < 0) {
				m_Result += "Closing brace(s) found without matching opening brace.";
				HandleSourceError(lineCountCache, filename);
				return false;
			}

			// We're good to go

		} else {
			// No braces on line, set command to line
			command = line;

			// Set cache to same line for error message
			lineCountCache = lineCount;
		}

		// Fire off the command
		if (!DoCommandInternal(command.c_str())) {
			// Command failed, error in result
			HandleSourceError(lineCountCache, filename);
			return false;
		}	
	}

	// Completion
	--m_SourceDepth;

	// if we're returing to the user and there is stuff on the source dir stack, print warning
	if (!m_SourceDepth) {
		if (m_SourceDirDepth != 0) {
			// And it should be zero, if it isn't, it's gotta be positive
			// which means the source files didn't have a popd for every pushd
			// so warn the user
			m_Result += "\nWarning: Source command left ";

			// It would be insane for there to be more than 32 characters in the integer here
			char buf[32];
			memset(buf, 0, 32);
			m_Result += itoa(m_SourceDirDepth, buf, 10); // 10 for base 10

			m_Result += " directories on the pushd/popd stack.";
		}
		m_SourceDirDepth = 0;
	}

	soarFile.close();
	return true;
}

// _   _                 _ _      ____                           _____
//| | | | __ _ _ __   __| | | ___/ ___|  ___  _   _ _ __ ___ ___| ____|_ __ _ __ ___  _ __
//| |_| |/ _` | '_ \ / _` | |/ _ \___ \ / _ \| | | | '__/ __/ _ \  _| | '__| '__/ _ \| '__|
//|  _  | (_| | | | | (_| | |  __/___) | (_) | |_| | | | (_|  __/ |___| |  | | | (_) | |
//|_| |_|\__,_|_| |_|\__,_|_|\___|____/ \___/ \__,_|_|  \___\___|_____|_|  |_|  \___/|_|
//
void CommandLineInterface::HandleSourceError(int errorLine, const char* filename) {
	if (!m_SourceError) {
		// PopD to original source directory
		while (m_SourceDirDepth) {
			DoPopD(); // Ignore error here since it will be rare and a message confusing
			--m_SourceDirDepth;
		}

		// Reset depths to zero
		m_SourceDepth = 0;
		m_SourceDirDepth = 0;

		m_SourceError = true;
		m_Result += "\nSource command error: error on line ";
		// TODO: arbitrary buffer size here
		char buf[256];
		memset(buf, 0, 256);
		m_Result += itoa(errorLine, buf, 10);

		m_Result += " of file: ";
		
		string directory;
		GetCurrentWorkingDirectory(directory); // Again, ignore error here

		m_Result += filename;
		m_Result += " (" + directory + ")";

	} else {
		m_Result += "\n\t--> Sourced by: ";
		m_Result += filename;
	}
}

// ____                     ____  ____
//|  _ \ __ _ _ __ ___  ___/ ___||  _ \
//| |_) / _` | '__/ __|/ _ \___ \| |_) |
//|  __/ (_| | |  \__ \  __/___) |  __/
//|_|   \__,_|_|  |___/\___|____/|_|
//
bool CommandLineInterface::ParseSP(int argc, char** argv) {
	// One argument (in brackets)
	if (argc != 2) {
		return HandleSyntaxError(Constants::kCLISPUsage);
	}

	// Remove first and last characters (the braces)
	string production = argv[1];
	if (production.length() < 3) {
		return HandleSyntaxError(Constants::kCLISPUsage);
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
	if (!RequireAgent()) {
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
				return HandleSyntaxError(Constants::kCLIStopSoarUsage, "Unrecognized option.\n");
			default:
				return HandleGetOptError(option);
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

// ____                    _____ _
//|  _ \ __ _ _ __ ___  __|_   _(_)_ __ ___   ___
//| |_) / _` | '__/ __|/ _ \| | | | '_ ` _ \ / _ \
//|  __/ (_| | |  \__ \  __/| | | | | | | | |  __/
//|_|   \__,_|_|  |___/\___||_| |_|_| |_| |_|\___|
//
bool CommandLineInterface::ParseTime(int argc, char** argv) {
	// There must at least be a command
	if (argc < 2) {
		return HandleSyntaxError(Constants::kCLITimeUsage);
	}

	char** newArgv = argv + 1;

	return DoTime(argc - 1, newArgv);
}

// ____      _____ _
//|  _ \  __|_   _(_)_ __ ___   ___
//| | | |/ _ \| | | | '_ ` _ \ / _ \
//| |_| | (_) | | | | | | | | |  __/
//|____/ \___/|_| |_|_| |_| |_|\___|
//
bool CommandLineInterface::DoTime(int argc, char** argv) {

#ifdef WIN32
	// I know this is lame marshalling it back 
	// up again only so it can get unmarshalled
	string command;
	for (int i = 0;i < argc; ++i) {
		command += argv[i];
		command += ' ';
	}

	// Look at clock
	DWORD start = GetTickCount();

	// Execute command
	bool ret = DoCommandInternal(command.c_str());

	// Look at clock again, subtracting first value
	DWORD elapsed = GetTickCount() - start;

	// calculate elapsed in seconds
	float seconds = elapsed / 1000.0f;

	// Print time elapsed and return
	char buf[32];
	memset(buf, 0, 32);
	snprintf(buf, 31, "%f", seconds);
	m_Result += "\n(";
	m_Result += buf;
	m_Result += "s) real";
	return ret;

#else
	m_Result += "TODO: time on non-windows platform";
	return true;
#endif
}

// ____                   __        __    _       _
//|  _ \ __ _ _ __ ___  __\ \      / /_ _| |_ ___| |__
//| |_) / _` | '__/ __|/ _ \ \ /\ / / _` | __/ __| '_ \
//|  __/ (_| | |  \__ \  __/\ V  V / (_| | || (__| | | |
//|_|   \__,_|_|  |___/\___| \_/\_/ \__,_|\__\___|_| |_|
//
bool CommandLineInterface::ParseWatch(int argc, char** argv) {
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
				return HandleSyntaxError(Constants::kCLIWatchUsage, "Unrecognized option.\n");
			default:
				return HandleGetOptError(option);
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

// ___     ___       _
//|_ _|___|_ _|_ __ | |_ ___  __ _  ___ _ __
// | |/ __|| || '_ \| __/ _ \/ _` |/ _ \ '__|
// | |\__ \| || | | | ||  __/ (_| |  __/ |
//|___|___/___|_| |_|\__\___|\__, |\___|_|
//                           |___/
bool CommandLineInterface::IsInteger(const char* s) {
	const string str = s;
	string::const_iterator iter = str.begin();
	while (iter != str.end()) {
		if (!isdigit(*iter)) {
			return false;
		}
		++iter;
	}
	return true;
}

// _   _                 _ _      ____              _             _____
//| | | | __ _ _ __   __| | | ___/ ___| _   _ _ __ | |_ __ ___  _| ____|_ __ _ __ ___  _ __
//| |_| |/ _` | '_ \ / _` | |/ _ \___ \| | | | '_ \| __/ _` \ \/ /  _| | '__| '__/ _ \| '__|
//|  _  | (_| | | | | (_| | |  __/___) | |_| | | | | || (_| |>  <| |___| |  | | | (_) | |
//|_| |_|\__,_|_| |_|\__,_|_|\___|____/ \__, |_| |_|\__\__,_/_/\_\_____|_|  |_|  \___/|_|
//                                      |___/
bool CommandLineInterface::HandleSyntaxError(const char* usage, const char* details) {
	m_Result += Constants::kCLISyntaxError;
	if (details) {
		m_Result += details;
	}
	m_Result += usage;
	return false;
}

// ____                  _             _                    _
//|  _ \ ___  __ _ _   _(_)_ __ ___   / \   __ _  ___ _ __ | |_
//| |_) / _ \/ _` | | | | | '__/ _ \ / _ \ / _` |/ _ \ '_ \| __|
//|  _ <  __/ (_| | |_| | | | |  __// ___ \ (_| |  __/ | | | |_
//|_| \_\___|\__, |\__,_|_|_|  \___/_/   \_\__, |\___|_| |_|\__|
//              |_|                        |___/
bool CommandLineInterface::RequireAgent() {
	if (!m_pAgent) {
		m_Result += "An agent pointer is required for this command.";
		return false;
	}
	return true;
}

// _   _                 _ _       ____      _    ___        _   _____
//| | | | __ _ _ __   __| | | ___ / ___| ___| |_ / _ \ _ __ | |_| ____|_ __ _ __ ___  _ __
//| |_| |/ _` | '_ \ / _` | |/ _ \ |  _ / _ \ __| | | | '_ \| __|  _| | '__| '__/ _ \| '__|
//|  _  | (_| | | | | (_| | |  __/ |_| |  __/ |_| |_| | |_) | |_| |___| |  | | | (_) | |
//|_| |_|\__,_|_| |_|\__,_|_|\___|\____|\___|\__|\___/| .__/ \__|_____|_|  |_|  \___/|_|
//                                                    |_|
bool CommandLineInterface::HandleGetOptError(char option) {
	m_Result += "Internal error: m_GetOpt.GetOpt_Long returned '";
	m_Result += option;
	m_Result += "'!";
	return false;
}

// _                _   _                 _ _
//| |    ___   __ _| | | | __ _ _ __   __| | | ___ _ __
//| |   / _ \ / _` | |_| |/ _` | '_ \ / _` | |/ _ \ '__|
//| |__| (_) | (_| |  _  | (_| | | | | (_| | |  __/ |
//|_____\___/ \__, |_| |_|\__,_|_| |_|\__,_|_|\___|_|
//            |___/
CommandLineInterface::LogHandler::LogHandler() {

}

// ___     _                      _
//|_ _|___| |    ___   __ _  __ _(_)_ __   __ _
// | |/ __| |   / _ \ / _` |/ _` | | '_ \ / _` |
// | |\__ \ |__| (_) | (_| | (_| | | | | | (_| |
//|___|___/_____\___/ \__, |\__, |_|_| |_|\__, |
//                    |___/ |___/         |___/
bool CommandLineInterface::LogHandler::IsLogging() {
	return m_pFile ? true : false;
}

// ____  _             _   _                      _
/// ___|| |_ __ _ _ __| |_| |    ___   __ _  __ _(_)_ __   __ _
//\___ \| __/ _` | '__| __| |   / _ \ / _` |/ _` | | '_ \ / _` |
// ___) | || (_| | |  | |_| |__| (_) | (_| | (_| | | | | | (_| |
//|____/ \__\__,_|_|   \__|_____\___/ \__, |\__, |_|_| |_|\__, |
//                                    |___/ |___/         |___/
bool CommandLineInterface::LogHandler::StartLogging(const char* pFilename) {
	m_pFile = new ofstream(pFilename);
	if (!m_pFile) {
		return false;
	}

	if (!m_pFile->is_open()) {
		return false;
	}
	return true;
}

// _____           _ _                      _
//| ____|_ __   __| | |    ___   __ _  __ _(_)_ __   __ _
//|  _| | '_ \ / _` | |   / _ \ / _` |/ _` | | '_ \ / _` |
//| |___| | | | (_| | |__| (_) | (_| | (_| | | | | | (_| |
//|_____|_| |_|\__,_|_____\___/ \__, |\__, |_|_| |_|\__, |
//                              |___/ |___/         |___/
void CommandLineInterface::LogHandler::EndLogging() {
	HandleEvent(gSKIEVENT_PRINT, 0, "*** Log file closed ***");
	m_pFile->close();
	delete m_pFile;
}

// _   _                 _ _      _____                 _
//| | | | __ _ _ __   __| | | ___| ____|_   _____ _ __ | |_
//| |_| |/ _` | '_ \ / _` | |/ _ \  _| \ \ / / _ \ '_ \| __|
//|  _  | (_| | | | | (_| | |  __/ |___ \ V /  __/ | | | |_
//|_| |_|\__,_|_| |_|\__,_|_|\___|_____| \_/ \___|_| |_|\__|
//
void CommandLineInterface::LogHandler::HandleEvent(egSKIEventId, gSKI::IAgent*, const char* msg) {
	m_pFile->write(msg, strlen(msg));
}
