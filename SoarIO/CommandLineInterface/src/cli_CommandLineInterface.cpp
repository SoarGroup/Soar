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

// BADBAD: I think we should be using an error class instead to work with error objects.
#include "../../gSKI/src/gSKI_Error.h"

// SML includes
#include "sml_Connection.h"

using namespace std;
using namespace cli;
using namespace sml;

//  ____                                          _ _     _            ___       _             __
// / ___|___  _ __ ___  _ __ ___   __ _ _ __   __| | |   (_)_ __   ___|_ _|_ __ | |_ ___ _ __ / _| __ _  ___ ___
//| |   / _ \| '_ ` _ \| '_ ` _ \ / _` | '_ \ / _` | |   | | '_ \ / _ \| || '_ \| __/ _ \ '__| |_ / _` |/ __/ _ \
//| |__| (_) | | | | | | | | | | | (_| | | | | (_| | |___| | | | |  __/| || | | | ||  __/ |  |  _| (_| | (_|  __/
// \____\___/|_| |_| |_|_| |_| |_|\__,_|_| |_|\__,_|_____|_|_| |_|\___|___|_| |_|\__\___|_|  |_|  \__,_|\___\___|
//
CommandLineInterface::CommandLineInterface() {

	// Create getopt object
	m_pGetOpt = new GetOpt;

	// Map command names to processing function pointers
	BuildCommandMap();

	// Store current working directory as 'home' dir
	char buf[512];
	getcwd(buf, 512);
	m_HomeDirectory = buf;

	// Give print handlers a reference to us
	m_ResultPrintHandler.SetCLI(this);
	m_LogPrintHandler.SetCLI(this);

	// Initialize other members
	m_QuitCalled = false;
	m_pKernel = 0;
	m_pAgent = 0;
	m_SourceError = false;
	m_SourceDepth = 0;
	m_SourceDirDepth = 0;
	m_pLogFile = 0;
}

// /\/|____                                          _ _     _            ___       _             __
//|/\// ___|___  _ __ ___  _ __ ___   __ _ _ __   __| | |   (_)_ __   ___|_ _|_ __ | |_ ___ _ __ / _| __ _  ___ ___
//   | |   / _ \| '_ ` _ \| '_ ` _ \ / _` | '_ \ / _` | |   | | '_ \ / _ \| || '_ \| __/ _ \ '__| |_ / _` |/ __/ _ \
//   | |__| (_) | | | | | | | | | | | (_| | | | | (_| | |___| | | | |  __/| || | | | ||  __/ |  |  _| (_| | (_|  __/
//    \____\___/|_| |_| |_|_| |_| |_|\__,_|_| |_|\__,_|_____|_|_| |_|\___|___|_| |_|\__\___|_|  |_|  \__,_|\___\___|
//
CommandLineInterface::~CommandLineInterface() {
	delete m_pGetOpt;
	if (m_pLogFile) {
		if (m_pAgent) {
			m_pAgent->RemovePrintListener(gSKIEVENT_PRINT, &m_LogPrintHandler);
		}
		delete m_pLogFile;
	}
}

// ____        _ _     _  ____                                          _ __  __
//| __ ) _   _(_) | __| |/ ___|___  _ __ ___  _ __ ___   __ _ _ __   __| |  \/  | __ _ _ __
//|  _ \| | | | | |/ _` | |   / _ \| '_ ` _ \| '_ ` _ \ / _` | '_ \ / _` | |\/| |/ _` | '_ \
//| |_) | |_| | | | (_| | |__| (_) | | | | | | | | | | | (_| | | | | (_| | |  | | (_| | |_) |
//|____/ \__,_|_|_|\__,_|\____\___/|_| |_| |_|_| |_| |_|\__,_|_| |_|\__,_|_|  |_|\__,_| .__/
//                                                                                    |_|
void CommandLineInterface::BuildCommandMap() {

	m_CommandMap[Constants::kCLICD]					= ParseCD;
	m_CommandMap[Constants::kCLIEcho]				= ParseEcho;
	m_CommandMap[Constants::kCLIExcise]				= ParseExcise;
	m_CommandMap[Constants::kCLIHelp]				= ParseHelp;
	m_CommandMap[Constants::kCLIInitSoar]			= ParseInitSoar;
	m_CommandMap[Constants::kCLILearn]				= ParseLearn;
	m_CommandMap[Constants::kCLILog]				= ParseLog;
	m_CommandMap[Constants::kCLILS]					= ParseLS;
	m_CommandMap[Constants::kCLIMultiAttributes]	= ParseMultiAttributes;
	m_CommandMap[Constants::kCLIPopD]				= ParsePopD;
	m_CommandMap[Constants::kCLIPrint]				= ParsePrint;
	m_CommandMap[Constants::kCLIPushD]				= ParsePushD;
	m_CommandMap[Constants::kCLIPWD]				= ParsePWD;
	m_CommandMap[Constants::kCLIQuit]				= ParseQuit;
	m_CommandMap[Constants::kCLIRun]				= ParseRun;
	m_CommandMap[Constants::kCLISource]				= ParseSource;
	m_CommandMap[Constants::kCLISP]					= ParseSP;
	m_CommandMap[Constants::kCLIStopSoar]			= ParseStopSoar;
	m_CommandMap[Constants::kCLITime]				= ParseTime;
	m_CommandMap[Constants::kCLIWatch]				= ParseWatch;
}

// ____         ____                                          _
//|  _ \  ___  / ___|___  _ __ ___  _ __ ___   __ _ _ __   __| |
//| | | |/ _ \| |   / _ \| '_ ` _ \| '_ ` _ \ / _` | '_ \ / _` |
//| |_| | (_) | |__| (_) | | | | | | | | | | | (_| | | | | (_| |
//|____/ \___/ \____\___/|_| |_| |_|_| |_| |_|\__,_|_| |_|\__,_|
//
bool CommandLineInterface::DoCommand(Connection* pConnection, gSKI::IAgent* pAgent, const char* pCommandLine, sml::ElementXML* pResponse, gSKI::Error* pError) {

	// Clear the result
	m_Result.clear();
	m_CriticalError = false;

	// Save the pointers
	m_pAgent = pAgent;
	m_pError = pError;

	// Process the command, ignoring its result (irrelevant at this level)
	bool ret = DoCommandInternal(pCommandLine);

	// Reset source error flag
	m_SourceError = false;

	if (!m_CriticalError && ret)
	{
		// The command succeeded, so return the result
		pConnection->AddSimpleResultToSMLResponse(pResponse, m_Result.c_str()) ;
	}
	else
	{
		// The command failed, so return an error message
		std::string msg = m_Result ;

		if (pError && isError(*pError))
		{
			msg += "gSKI error was: " ;
			msg += pError->Text ;
			msg += " details: " ;
			msg += pError->ExtendedMsg ;
		}

		// Add the error message to the response
		int errorCode = -1 ;	// -1 reserved for "no error code defined"
		pConnection->AddErrorToSMLResponse(pResponse, msg.c_str(), errorCode) ;
	}

	// Always returns true to indicate that we've generated any needed error message already
	return true ;
}

// ____         ____                                          _
//|  _ \  ___  / ___|___  _ __ ___  _ __ ___   __ _ _ __   __| |
//| | | |/ _ \| |   / _ \| '_ ` _ \| '_ ` _ \ / _` | '_ \ / _` |
//| |_| | (_) | |__| (_) | | | | | | | | | | | (_| | | | | (_| |
//|____/ \___/ \____\___/|_| |_| |_|_| |_| |_|\__,_|_| |_|\__,_|
//
bool CommandLineInterface::DoCommand(gSKI::IAgent* pAgent, const char* pCommandLine, char const* pResponse, gSKI::Error* pError) {
	// This function is for processing a command without the SML layer
	// Clear the result
	m_Result.clear();
	m_CriticalError = false;

	// Save the pointers
	m_pAgent = pAgent;
	m_pError = pError;

	// Process the command, ignoring its result (irrelevant at this level)
	bool ret = DoCommandInternal(pCommandLine);

	// Reset source error flag
	m_SourceError = false;

	pResponse = m_Result.c_str();

	return ret;
}

// ____         ____                                          _ ___       _                        _
//|  _ \  ___  / ___|___  _ __ ___  _ __ ___   __ _ _ __   __| |_ _|_ __ | |_ ___ _ __ _ __   __ _| |
//| | | |/ _ \| |   / _ \| '_ ` _ \| '_ ` _ \ / _` | '_ \ / _` || || '_ \| __/ _ \ '__| '_ \ / _` | |
//| |_| | (_) | |__| (_) | | | | | | | | | | | (_| | | | | (_| || || | | | ||  __/ |  | | | | (_| | |
//|____/ \___/ \____\___/|_| |_| |_|_| |_| |_|\__,_|_| |_|\__,_|___|_| |_|\__\___|_|  |_| |_|\__,_|_|
//
bool CommandLineInterface::DoCommandInternal(const std::string& commandLine) {

	vector<string> argv;

	// Parse command:
	if (Tokenize(commandLine, argv) == -1) {
		return false;	// Parsing failed, error in result
	}

	return DoCommandInternal(argv);
}

// ____         ____                                          _ ___       _                        _
//|  _ \  ___  / ___|___  _ __ ___  _ __ ___   __ _ _ __   __| |_ _|_ __ | |_ ___ _ __ _ __   __ _| |
//| | | |/ _ \| |   / _ \| '_ ` _ \| '_ ` _ \ / _` | '_ \ / _` || || '_ \| __/ _ \ '__| '_ \ / _` | |
//| |_| | (_) | |__| (_) | | | | | | | | | | | (_| | | | | (_| || || | | | ||  __/ |  | | | | (_| | |
//|____/ \___/ \____\___/|_| |_| |_|_| |_| |_|\__,_|_| |_|\__,_|___|_| |_|\__\___|_|  |_| |_|\__,_|_|
//
bool CommandLineInterface::DoCommandInternal(vector<string>& argv) {
	if (!argv.size()) {
		// Nothing on command line!
		return true;
	}

	// Is the command implemented?
	if (m_CommandMap.find(argv[0]) == m_CommandMap.end()) {
		m_Result += "Command '";
		m_Result += argv[0];
		m_Result += "' not found or implemented.";

		// Critical error
		m_CriticalError = true;
		return false;
	}

	// Check for help flags
	if (CheckForHelp(argv)) {
		// Help flags found, add help to line, return true
		string output;
		if (!m_Constants.IsUsageFileAvailable()) {
			m_Result += Constants::kCLINoUsageFile;
		} else if (m_Constants.GetUsageFor(argv[0], output)) {
			m_Result += output;
		} else {
			m_Result += Constants::kCLINoUsageInfo;
		}
		return true;
	}

	// Process command
	CommandFunction pFunction = m_CommandMap[argv[0]];

	// Just in case...
	if (!pFunction) {
		// Very odd, should be set in BuildCommandMap
		m_Result += "Command found but function pointer is null.";

		// This is definately a critical error
		m_CriticalError = true;
		return false;
	}
	
	// Make the call
	return (this->*pFunction)(argv);
}

// _____     _              _
//|_   _|__ | | _____ _ __ (_)_______
//  | |/ _ \| |/ / _ \ '_ \| |_  / _ \
//  | | (_) |   <  __/ | | | |/ /  __/
//  |_|\___/|_|\_\___|_| |_|_/___\___|
//
int CommandLineInterface::Tokenize(string cmdline, vector<string>& argumentVector) {
	int argc = 0;
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
//bool CommandLineInterface::Parse(std::vector<std::string>& argv) {
//	return Do();
//}
//
//bool CommandLineInterface::Do() {
//	m_Result += "TODO: ";
//	return true;
//}

// ____                      ____ ____
//|  _ \ __ _ _ __ ___  ___ / ___|  _ \
//| |_) / _` | '__/ __|/ _ \ |   | | | |
//|  __/ (_| | |  \__ \  __/ |___| |_| |
//|_|   \__,_|_|  |___/\___|\____|____/
//
bool CommandLineInterface::ParseCD(std::vector<std::string>& argv) {
	// Only takes one optional argument, the directory to change into
	if (argv.size() > 2) {
		return HandleSyntaxError(Constants::kCLICD);
	}
	if (argv.size() > 1) {
		return DoCD(argv[1]);
	}
	return DoCD(string());
}

// ____         ____ ____
//|  _ \  ___  / ___|  _ \
//| | | |/ _ \| |   | | | |
//| |_| | (_) | |___| |_| |
//|____/ \___/ \____|____/
//
bool CommandLineInterface::DoCD(string& directory) {

	// If cd is typed by itself, return to original (home) directory
	if (!directory.size()) {

		// Home dir set in constructor
		if (chdir(m_HomeDirectory.c_str())) {
			m_Result += "Could not change to home directory: ";
			m_Result += m_HomeDirectory;

			// Critical error
			m_CriticalError = true;
			return false;
		}
		return true;
	}

	// Chop of quotes if they are there, chdir doesn't like them
	if ((directory.length() > 2) && (directory[0] == '\"') && (directory[directory.length() - 1] == '\"')) {
		directory = directory.substr(1, directory.length() - 2);
	}

	// Change to passed directory
	if (chdir(directory.c_str())) {
		m_Result += "Could not change to directory: ";
		m_Result += directory;

		// Critical error
		m_CriticalError = true;
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
bool CommandLineInterface::ParseEcho(std::vector<std::string>& argv) {
	// Very simple command, any number of arguments
	return DoEcho(argv);
}

// ____        _____     _
//|  _ \  ___ | ____|___| |__   ___
//| | | |/ _ \|  _| / __| '_ \ / _ \
//| |_| | (_) | |__| (__| | | | (_) |
//|____/ \___/|_____\___|_| |_|\___/
//
bool CommandLineInterface::DoEcho(std::vector<std::string>& argv) {

	// Concatenate arguments (spaces between arguments are lost unless enclosed in quotes)
	for (unsigned i = 1; i < argv.size(); ++i) {
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
bool CommandLineInterface::ParseExcise(std::vector<std::string>& argv) {
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
		option = m_pGetOpt->GetOpt_Long(argv, "acdtu", longOptions, 0);
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
				return HandleSyntaxError(Constants::kCLIExcise, "Unrecognized option.");
			default:
				return HandleGetOptError(option);
		}
	}

	// Pass the productions to the DoExcise function
	return DoExcise(options, GetOpt::optind, argv);
}

// ____        _____          _
//|  _ \  ___ | ____|_  _____(_)___  ___
//| | | |/ _ \|  _| \ \/ / __| / __|/ _ \
//| |_| | (_) | |___ >  < (__| \__ \  __/
//|____/ \___/|_____/_/\_\___|_|___/\___|
//
bool CommandLineInterface::DoExcise(const unsigned short options, int optind, vector<string>& argv) {
	// Must have agent to excise from
	if (!RequireAgent()) {
		return false;
	}

	// Acquire production manager
	gSKI::IProductionManager *pProductionManager = m_pAgent->GetProductionManager();

	// Listen for #s
	m_pAgent->AddPrintListener(gSKIEVENT_PRINT, &m_ResultPrintHandler);

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
	for (unsigned i = optind; i < argv.size(); ++i) {
		// Iterate through productions
		pProdIter = pProductionManager->GetProduction(argv[i].c_str());
		if (pProdIter->GetNumElements()) {
			ExciseInternal(pProdIter);
		} else {
			m_Result += "Production not found: ";
			m_Result += argv[i];
			m_pAgent->RemovePrintListener(gSKIEVENT_PRINT, &m_ResultPrintHandler);
			return false;
		}
	}
	m_pAgent->RemovePrintListener(gSKIEVENT_PRINT, &m_ResultPrintHandler);
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

// ____                     _   _      _
//|  _ \ __ _ _ __ ___  ___| | | | ___| |_ __
//| |_) / _` | '__/ __|/ _ \ |_| |/ _ \ | '_ \
//|  __/ (_| | |  \__ \  __/  _  |  __/ | |_) |
//|_|   \__,_|_|  |___/\___|_| |_|\___|_| .__/
//                                      |_|
bool CommandLineInterface::ParseHelp(std::vector<std::string>& argv) {
	if (argv.size() > 2) {
		return HandleSyntaxError(Constants::kCLIHelp);
	}

	if (argv.size() == 2) {
		return DoHelp(argv[1]);
	}
	return DoHelp(string());
}

// ____        _   _      _
//|  _ \  ___ | | | | ___| |_ __
//| | | |/ _ \| |_| |/ _ \ | '_ \
//| |_| | (_) |  _  |  __/ | |_) |
//|____/ \___/|_| |_|\___|_| .__/
//                         |_|
bool CommandLineInterface::DoHelp(const string& command) {
	string output;

	if (!m_Constants.IsUsageFileAvailable()) {
		m_Result += Constants::kCLINoUsageFile;
		return false;
	}

	if (command.size()) {
		if (!m_Constants.GetUsageFor(command, output)) {
			m_Result += "Help for command '" + command + "' not found.";
			return false;
		}
		m_Result += output;
		return true;
	}
	m_Result += "Help is available for the following commands:\n";
	list<string> commandList = m_Constants.GetCommandList();
	list<string>::const_iterator iter = commandList.begin();

	int i = 0;
	int tabs;
	while (iter != commandList.end()) {
		m_Result += *iter;
		if (m_CommandMap.find(*iter) == m_CommandMap.end()) {
			m_Result += '*';
		} else {
			m_Result += ' ';
		}
		tabs = (40 - (*iter).length() - 2) / 8; 
		if (i % 2) {
			m_Result += "\n";
		} else {
			do {
				m_Result += '\t';
			} while (--tabs > 0);
		}
		++iter;
		++i;
	}
	if (i % 2) {
		m_Result += '\n';
	}
	m_Result += "Type 'help' followed by the command name for help on a specific command.\n";
	m_Result += "A Star (*) indicates the command is not yet implemented.";
	return true;
}

// ____                     ___       _ _   ____
//|  _ \ __ _ _ __ ___  ___|_ _|_ __ (_) |_/ ___|  ___   __ _ _ __
//| |_) / _` | '__/ __|/ _ \| || '_ \| | __\___ \ / _ \ / _` | '__|
//|  __/ (_| | |  \__ \  __/| || | | | | |_ ___) | (_) | (_| | |
//|_|   \__,_|_|  |___/\___|___|_| |_|_|\__|____/ \___/ \__,_|_|
//
bool CommandLineInterface::ParseInitSoar(std::vector<std::string>& argv) {
	// No arguments
	if (argv.size() != 1) {
		return HandleSyntaxError(Constants::kCLIInitSoar);
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
bool CommandLineInterface::ParseLearn(std::vector<std::string>& argv) {
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
		option = m_pGetOpt->GetOpt_Long(argv, "abdeElo", longOptions, 0);
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
				return HandleSyntaxError(Constants::kCLILearn, "Unrecognized option.");
			default:
				return HandleGetOptError(option);
		}
	}

	// No non-option arguments
	if (GetOpt::optind != argv.size()) {
		return HandleSyntaxError(Constants::kCLILearn);
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
		m_Result += "Learning is ";
		m_Result += m_pAgent->IsLearningOn() ? "enabled." : "disabled.";
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
bool CommandLineInterface::ParseLog(std::vector<std::string>& argv) {
	static struct GetOpt::option longOptions[] = {
		{"append",	0, 0, 'a'},
		{"close",	0, 0, 'c'},
		{"disable",	0, 0, 'c'},
		{"off",		0, 0, 'c'},
		{0, 0, 0, 0}
	};

	GetOpt::optind = 0;
	GetOpt::opterr = 0;

	int option;
	unsigned short options = 0;
	bool append = false;
	bool close = false;

	for (;;) {
		option = m_pGetOpt->GetOpt_Long(argv, "ac", longOptions, 0);
		if (option == -1) {
			break;
		}

		switch (option) {
			case 'a':
				append = true;
				break;
			case 'c':
				close = true;
				break;
			case '?':
				return HandleSyntaxError(Constants::kCLILog, "Unrecognized option.");
			default:
				return HandleGetOptError(option);
		}
	}

	// Only one non-option arg allowed, filename
	if (GetOpt::optind == argv.size() - 1) {

		// But not with the close option
		if (close) {
			return HandleSyntaxError(Constants::kCLILog, "No arguments when disabling.");
		}

		return DoLog(append, argv[GetOpt::optind].c_str());

	} else if ((unsigned)GetOpt::optind < argv.size()) {
		return HandleSyntaxError(Constants::kCLILog);
	}

	// No appending without a filename, and if we got here, there is no filename
	if (append) {
		return HandleSyntaxError(Constants::kCLILog, "Append requires a filename.");
	}

	return DoLog(close);
}

// ____        _
//|  _ \  ___ | |    ___   __ _
//| | | |/ _ \| |   / _ \ / _` |
//| |_| | (_) | |__| (_) | (_| |
//|____/ \___/|_____\___/ \__, |
//                        |___/
bool CommandLineInterface::DoLog(bool option, const char* pFilename) {

	// Presence of a filename means open, absence means close or query
	if (pFilename) {
		// Open a file
		if (m_pLogFile) {
			// Unless one is already opened
			m_Result += "Close log file '" + m_LogFilename + "' first.";

			// Critical error
			m_CriticalError = true;
			return false;
		}

		// Create the stream
		if (option) {
			// Option flag means append in presence of filename
			m_pLogFile = new ofstream(pFilename, ios_base::app);
		} else {
			m_pLogFile = new ofstream(pFilename);
		}

		if (!m_pLogFile) {
			// Creation and opening was not successful
			m_Result += "Failed to open file for logging.";

			// Critical error
			m_CriticalError = true;
			return false;
		}

		// Logging opened, add listener and save filename since we can't get it from ofstream
		m_pAgent->AddPrintListener(gSKIEVENT_PRINT, &m_LogPrintHandler);
		m_LogFilename = pFilename;

	} else if (option) {		
		// In absence of filename, option means close
		if (!m_pLogFile) {
			// Can't close when we're not logging to begin with
			m_Result += "No log is opened.";
			return false;
		}

		// Remove the listener and close the file
		m_pAgent->RemovePrintListener(gSKIEVENT_PRINT, &m_LogPrintHandler);
		delete m_pLogFile;
		m_pLogFile = 0;

		// Forget the filename
		m_LogFilename.clear();

	}

	// Query at end of successful command, or by default
	if (!m_pLogFile) {
		m_Result = "Log file closed.";
	} else {
		m_Result += "Log file '" + m_LogFilename + "' opened.";
	}
	return true;
}

// ____                     _     ____
//|  _ \ __ _ _ __ ___  ___| |   / ___|
//| |_) / _` | '__/ __|/ _ \ |   \___ \
//|  __/ (_| | |  \__ \  __/ |___ ___) |
//|_|   \__,_|_|  |___/\___|_____|____/
//
bool CommandLineInterface::ParseLS(std::vector<std::string>& argv) {
	// No arguments
	if (argv.size() != 1) {
		return HandleSyntaxError(Constants::kCLILS);
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
bool CommandLineInterface::ParseMultiAttributes(std::vector<std::string>& argv) {
	// No more than three arguments
	if (argv.size() > 3) {
		return HandleSyntaxError(Constants::kCLIMultiAttributes);
	}

	int n = 0;

	// If we have 3 arguments, third one is an integer
	if (argv.size() > 2) {
		if (!IsInteger(argv[2])) {
			// Must be an integer
			return HandleSyntaxError(Constants::kCLIMultiAttributes, "Third argument must be an integer.");
		}
		n = atoi(argv[2].c_str());
		if (n <= 0) {
			// Must be greater than 0
			return HandleSyntaxError(Constants::kCLIMultiAttributes, "Third argument must be greater than 0.");
		}
	}

	// If we have two arguments, second arg is an attribute/identifer/whatever
	if (argv.size() > 1) {
		return DoMultiAttributes(argv[1], n);
	} 
	return DoMultiAttributes(string(), n);
}

// ____        __  __       _ _   _    _   _   _        _ _           _
//|  _ \  ___ |  \/  |_   _| | |_(_)  / \ | |_| |_ _ __(_) |__  _   _| |_ ___  ___
//| | | |/ _ \| |\/| | | | | | __| | / _ \| __| __| '__| | '_ \| | | | __/ _ \/ __|
//| |_| | (_) | |  | | |_| | | |_| |/ ___ \ |_| |_| |  | | |_) | |_| | ||  __/\__ \
//|____/ \___/|_|  |_|\__,_|_|\__|_/_/   \_\__|\__|_|  |_|_.__/ \__,_|\__\___||___/
//
bool CommandLineInterface::DoMultiAttributes(const string& attribute, int n) {
	if (!RequireAgent()) {
		return false;
	}

	if (!attribute.size() && !n) {
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
	m_pAgent->SetMultiAttribute(attribute.c_str(), n);

 	return true;
}

// ____                     ____             ____
//|  _ \ __ _ _ __ ___  ___|  _ \ ___  _ __ |  _ \
//| |_) / _` | '__/ __|/ _ \ |_) / _ \| '_ \| | | |
//|  __/ (_| | |  \__ \  __/  __/ (_) | |_) | |_| |
//|_|   \__,_|_|  |___/\___|_|   \___/| .__/|____/
//                                    |_|
bool CommandLineInterface::ParsePopD(std::vector<std::string>& argv) {
	// No arguments
	if (argv.size() != 1) {
		return HandleSyntaxError(Constants::kCLIPopD);
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
		// Critical error
		m_CriticalError = true;
		return false;
	}

	// Change to the directory
	if (!DoCD(m_DirectoryStack.top())) {
		// cd failed, error message added in cd function
		// Critical error (probably redundant)
		m_CriticalError = true;
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
bool CommandLineInterface::ParsePrint(std::vector<std::string>& argv) {
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
		option = m_pGetOpt->GetOpt_Long(argv, ":acd:DfFijnosSu", longOptions, 0);
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
					return HandleSyntaxError(Constants::kCLIPrint, "Depth must be an integer.");
				}
				depth = atoi(GetOpt::optarg);
				if (depth < 0) {
					return HandleSyntaxError(Constants::kCLIPrint, "Depth must be non-negative.");
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
			case ':':
				return HandleSyntaxError(Constants::kCLIPrint, "Missing option argument.");
			case '?':
				return HandleSyntaxError(Constants::kCLIPrint, "Unrecognized option.");
			default:
				return HandleGetOptError(option);
		}
	}

	// One additional optional argument
	if ((argv.size() - GetOpt::optind) > 1) {
		return HandleSyntaxError(Constants::kCLIPrint);
	} else if ((argv.size() - GetOpt::optind) == 1) {
		return DoPrint(options, depth, argv[GetOpt::optind]);
	}
	return DoPrint(options, depth, string());
}

// ____        ____       _       _
//|  _ \  ___ |  _ \ _ __(_)_ __ | |_
//| | | |/ _ \| |_) | '__| | '_ \| __|
//| |_| | (_) |  __/| |  | | | | | |_
//|____/ \___/|_|   |_|  |_|_| |_|\__|
//
bool CommandLineInterface::DoPrint(const unsigned short options, int depth, const string& arg) {
	// Need agent pointer for function calls
	if (!RequireAgent()) {
		return false;
	}

	// Attain the evil back door of doom, even though we aren't the TgD
	gSKI::EvilBackDoor::ITgDWorkArounds* pKernelHack = m_pKernel->getWorkaroundObject();

	// Check for stack print
	if (options & OPTION_PRINT_STACK) {
		m_pAgent->AddPrintListener(gSKIEVENT_PRINT, &m_ResultPrintHandler);
		pKernelHack->PrintStackTrace(m_pAgent, (options & OPTION_PRINT_STATES) ? true : false, (options & OPTION_PRINT_OPERATORS) ? true : false);
		m_pAgent->RemovePrintListener(gSKIEVENT_PRINT, &m_ResultPrintHandler);
		return true;
	}

	// Cache the flags since it makes function calls huge
	bool internal = (options & OPTION_PRINT_INTERNAL) ? true : false;
	bool filename = (options & OPTION_PRINT_FILENAME) ? true : false;
	bool full = (options & OPTION_PRINT_FULL) ? true : false;
	bool name = (options & OPTION_PRINT_NAME) ? true : false;

	// Check for the five general print options (all, chunks, defaults, justifications, user)
	if (options & OPTION_PRINT_ALL) {
		// TODO: Find out what is arg is for
		m_pAgent->AddPrintListener(gSKIEVENT_PRINT, &m_ResultPrintHandler);
        pKernelHack->PrintUser(m_pAgent, 0, internal, filename, full, DEFAULT_PRODUCTION_TYPE);
        pKernelHack->PrintUser(m_pAgent, 0, internal, filename, full, USER_PRODUCTION_TYPE);
        pKernelHack->PrintUser(m_pAgent, 0, internal, filename, full, CHUNK_PRODUCTION_TYPE);
        pKernelHack->PrintUser(m_pAgent, 0, internal, filename, full, JUSTIFICATION_PRODUCTION_TYPE);
		m_pAgent->RemovePrintListener(gSKIEVENT_PRINT, &m_ResultPrintHandler);
		return true;
	}
	if (options & OPTION_PRINT_CHUNKS) {
		m_pAgent->AddPrintListener(gSKIEVENT_PRINT, &m_ResultPrintHandler);
        pKernelHack->PrintUser(m_pAgent, 0, internal, filename, full, CHUNK_PRODUCTION_TYPE);
		m_pAgent->RemovePrintListener(gSKIEVENT_PRINT, &m_ResultPrintHandler);
		return true;
	}
	if (options & OPTION_PRINT_DEFAULTS) {
		m_pAgent->AddPrintListener(gSKIEVENT_PRINT, &m_ResultPrintHandler);
        pKernelHack->PrintUser(m_pAgent, 0, internal, filename, full, DEFAULT_PRODUCTION_TYPE);
		m_pAgent->RemovePrintListener(gSKIEVENT_PRINT, &m_ResultPrintHandler);
		return true;
	}
	if (options & OPTION_PRINT_JUSTIFICATIONS) {
		m_pAgent->AddPrintListener(gSKIEVENT_PRINT, &m_ResultPrintHandler);
        pKernelHack->PrintUser(m_pAgent, 0, internal, filename, full, JUSTIFICATION_PRODUCTION_TYPE);
		m_pAgent->RemovePrintListener(gSKIEVENT_PRINT, &m_ResultPrintHandler);
		return true;
	}
	if (options & OPTION_PRINT_USER) {
		m_pAgent->AddPrintListener(gSKIEVENT_PRINT, &m_ResultPrintHandler);
        pKernelHack->PrintUser(m_pAgent, 0, internal, filename, full, USER_PRODUCTION_TYPE);
		m_pAgent->RemovePrintListener(gSKIEVENT_PRINT, &m_ResultPrintHandler);
		return true;
	}

	// Default to symbol print
	m_pAgent->AddPrintListener(gSKIEVENT_PRINT, &m_ResultPrintHandler);
	pKernelHack->PrintSymbol(m_pAgent, const_cast<char*>(arg.c_str()), name, filename, internal, full, depth);
	m_pAgent->RemovePrintListener(gSKIEVENT_PRINT, &m_ResultPrintHandler);
	return true;
}

// ____                     ____            _     ____
//|  _ \ __ _ _ __ ___  ___|  _ \ _   _ ___| |__ |  _ \
//| |_) / _` | '__/ __|/ _ \ |_) | | | / __| '_ \| | | |
//|  __/ (_| | |  \__ \  __/  __/| |_| \__ \ | | | |_| |
//|_|   \__,_|_|  |___/\___|_|    \__,_|___/_| |_|____/
//
bool CommandLineInterface::ParsePushD(std::vector<std::string>& argv) {
	// Only takes one argument, the directory to change into
	if (argv.size() != 2) {
		return HandleSyntaxError(Constants::kCLIPushD);
	}
	return DoPushD(argv[1]);
}

// ____        ____            _     ____
//|  _ \  ___ |  _ \ _   _ ___| |__ |  _ \
//| | | |/ _ \| |_) | | | / __| '_ \| | | |
//| |_| | (_) |  __/| |_| \__ \ | | | |_| |
//|____/ \___/|_|    \__,_|___/_| |_|____/
//
bool CommandLineInterface::DoPushD(string& directory) {
	
	// Target directory required, checked in DoCD call.

	// Save the current (soon to be old) directory
	string oldDirectory;
	if (!GetCurrentWorkingDirectory(oldDirectory)) {
		// Error message added in function
		return false;
	}

	// Change to the new directory.
	if (!DoCD(directory)) {
		// Critical error
		m_CriticalError = true;
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
bool CommandLineInterface::ParsePWD(std::vector<std::string>& argv) {
	// No arguments to print working directory
	if (argv.size() != 1) {
		return HandleSyntaxError(Constants::kCLIPWD);
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
bool CommandLineInterface::ParseQuit(std::vector<std::string>& argv) {
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
bool CommandLineInterface::ParseRun(std::vector<std::string>& argv) {
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
		option = m_pGetOpt->GetOpt_Long(argv, "defoOpsS", longOptions, 0);
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
				return HandleSyntaxError(Constants::kCLIRun, "Unrecognized option.");
			default:
				return HandleGetOptError(option);
		}
	}

	// Count defaults to 1 (which are ignored if the options default, since they default to forever)
	int count = 1;

	// Only one non-option argument allowed, count
	if (GetOpt::optind == argv.size() - 1) {

		if (!IsInteger(argv[GetOpt::optind])) {
			return HandleSyntaxError(Constants::kCLIRun, "Count must be an integer.");
		}
		count = atoi(argv[GetOpt::optind].c_str());
		if (count <= 0) {
			return HandleSyntaxError(Constants::kCLIRun, "Count must be greater than 0.");
		}

	} else if ((unsigned)GetOpt::optind < argv.size()) {
		return HandleSyntaxError(Constants::kCLIRun);
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

		m_pAgent->AddPrintListener(gSKIEVENT_PRINT, &m_ResultPrintHandler);
		runResult = m_pAgent->RunInClientThread(runType, count, m_pError);
		m_pAgent->RemovePrintListener(gSKIEVENT_PRINT, &m_ResultPrintHandler);
	} else {
		m_pAgent->AddPrintListener(gSKIEVENT_PRINT, &m_ResultPrintHandler);
        m_pKernel->GetAgentManager()->ClearAllInterrupts();
        m_pKernel->GetAgentManager()->AddAllAgentsToRunList();
		runResult = m_pKernel->GetAgentManager()->RunInClientThread(runType, count, gSKI_INTERLEAVE_SMALLEST_STEP, m_pError);
		m_pAgent->RemovePrintListener(gSKIEVENT_PRINT, &m_ResultPrintHandler);
	}

	// Check for error
	if (runResult == gSKI_RUN_ERROR) {
		m_Result += "Run failed.";
		// Critical error
		m_CriticalError = true;
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
			// Critical error
			m_CriticalError = true;
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
bool CommandLineInterface::ParseSource(std::vector<std::string>& argv) {
	if (argv.size() != 2) {
		// Source requires a filename
		return HandleSyntaxError(Constants::kCLISource);

	} else if (argv.size() > 2) {
		// but only one filename
		return HandleSyntaxError(Constants::kCLISource, "Source only one file at a time.");
	}

	return DoSource(argv[1]);
}

// ____       ____
//|  _ \  ___/ ___|  ___  _   _ _ __ ___ ___
//| | | |/ _ \___ \ / _ \| | | | '__/ __/ _ \
//| |_| | (_) |__) | (_) | |_| | | | (_|  __/
//|____/ \___/____/ \___/ \__,_|_|  \___\___|
//
bool CommandLineInterface::DoSource(const string& filename) {
	if (!RequireAgent()) {
		return false;
	}

	// Open the file
	ifstream soarFile(filename.c_str());
	if (!soarFile) {
		m_Result += "Failed to open file '";
		m_Result += filename;
		m_Result += "' for reading.";
		// Critical error
		m_CriticalError = true;
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
		if (!DoCommandInternal(command)) {
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
void CommandLineInterface::HandleSourceError(int errorLine, const string& filename) {
	if (!m_SourceError) {
		// PopD to original source directory
		while (m_SourceDirDepth) {
			DoPopD(); // Ignore error here since it will be rare and a message confusing
			--m_SourceDirDepth;
		}

		// Reset depths to zero
		m_SourceDepth = 0;
		m_SourceDirDepth = 0; // TODO: redundant

		m_SourceError = true;
		m_Result += "\nSource command error: error on line ";
		// TODO: arbitrary buffer size here
		char buf[256];
		memset(buf, 0, 256);
		m_Result += itoa(errorLine, buf, 10);

		m_Result += " of file: ";
		
		string directory;
		GetCurrentWorkingDirectory(directory); // Again, ignore error here

		m_Result += filename + " (" + directory + ")";

		// Critical error
		m_CriticalError = true;

	} else {
		m_Result += "\n\t--> Sourced by: " + filename;
	}
}

// ____                     ____  ____
//|  _ \ __ _ _ __ ___  ___/ ___||  _ \
//| |_) / _` | '__/ __|/ _ \___ \| |_) |
//|  __/ (_| | |  \__ \  __/___) |  __/
//|_|   \__,_|_|  |___/\___|____/|_|
//
bool CommandLineInterface::ParseSP(std::vector<std::string>& argv) {
	// One argument (in brackets)
	if (argv.size() != 2) {
		return HandleSyntaxError(Constants::kCLISP);
	}

	// Remove first and last characters (the braces)
	string production = argv[1];
	if (production.length() < 3) {
		return HandleSyntaxError(Constants::kCLISP);
	}
	production = production.substr(1, production.length() - 2);

	return DoSP(production);
}

// ____       ____  ____
//|  _ \  ___/ ___||  _ \
//| | | |/ _ \___ \| |_) |
//| |_| | (_) |__) |  __/
//|____/ \___/____/|_|
//
bool CommandLineInterface::DoSP(const string& production) {
	// Must have agent to give production to
	if (!RequireAgent()) {
		return false;
	}

	// Acquire production manager
	gSKI::IProductionManager *pProductionManager = m_pAgent->GetProductionManager();

	// Load the production
	m_pAgent->AddPrintListener(gSKIEVENT_PRINT, &m_ResultPrintHandler);
	pProductionManager->AddProduction(const_cast<char*>(production.c_str()), m_pError);
	m_pAgent->RemovePrintListener(gSKIEVENT_PRINT, &m_ResultPrintHandler);

	if(m_pError->Id != gSKI::gSKIERR_NONE) {
		m_Result += "Unable to add the production: " + production;
		// Critical error
		m_CriticalError = true;
		return false;
	}

	// TODO: The kernel is supposed to print this but doesnt!
	m_Result += '*';

	return true;
}

// ____                     ____  _             ____
//|  _ \ __ _ _ __ ___  ___/ ___|| |_ ___  _ __/ ___|  ___   __ _ _ __
//| |_) / _` | '__/ __|/ _ \___ \| __/ _ \| '_ \___ \ / _ \ / _` | '__|
//|  __/ (_| | |  \__ \  __/___) | || (_) | |_) |__) | (_) | (_| | |
//|_|   \__,_|_|  |___/\___|____/ \__\___/| .__/____/ \___/ \__,_|_|
//                                        |_|
bool CommandLineInterface::ParseStopSoar(std::vector<std::string>& argv) {
	static struct GetOpt::option longOptions[] = {
		{"self",		0, 0, 's'},
		{0, 0, 0, 0}
	};

	GetOpt::optind = 0;
	GetOpt::opterr = 0;

	int option;
	bool self = false;

	for (;;) {
		option = m_pGetOpt->GetOpt_Long(argv, "s", longOptions, 0);
		if (option == -1) {
			break;
		}

		switch (option) {
			case 's':
				self = true;
				break;
			case '?':
				return HandleSyntaxError(Constants::kCLIStopSoar, "Unrecognized option.");
			default:
				return HandleGetOptError(option);
		}
	}

	// Concatinate remaining args for 'reason'
	string reasonForStopping;
	if ((unsigned)GetOpt::optind < argv.size()) {
		while ((unsigned)GetOpt::optind < argv.size()) {
			reasonForStopping += argv[GetOpt::optind++] + ' ';
		}
	}
	return DoStopSoar(self, reasonForStopping);
}

// ____       ____  _             ____
//|  _ \  ___/ ___|| |_ ___  _ __/ ___|  ___   __ _ _ __
//| | | |/ _ \___ \| __/ _ \| '_ \___ \ / _ \ / _` | '__|
//| |_| | (_) |__) | || (_) | |_) |__) | (_) | (_| | |
//|____/ \___/____/ \__\___/| .__/____/ \___/ \__,_|_|
//                          |_|
bool CommandLineInterface::DoStopSoar(bool self, const string& reasonForStopping) {
	m_Result += "TODO: do stop-soar";
	return true;
}

// ____                    _____ _
//|  _ \ __ _ _ __ ___  __|_   _(_)_ __ ___   ___
//| |_) / _` | '__/ __|/ _ \| | | | '_ ` _ \ / _ \
//|  __/ (_| | |  \__ \  __/| | | | | | | | |  __/
//|_|   \__,_|_|  |___/\___||_| |_|_| |_| |_|\___|
//
bool CommandLineInterface::ParseTime(std::vector<std::string>& argv) {
	// There must at least be a command
	if (argv.size() < 2) {
		return HandleSyntaxError(Constants::kCLITime);
	}

	vector<string>::iterator iter = argv.begin();
	argv.erase(iter);

	return DoTime(argv);
}

// ____      _____ _
//|  _ \  __|_   _(_)_ __ ___   ___
//| | | |/ _ \| | | | '_ ` _ \ / _ \
//| |_| | (_) | | | | | | | | |  __/
//|____/ \___/|_| |_|_| |_| |_|\___|
//
bool CommandLineInterface::DoTime(std::vector<std::string>& argv) {

#ifdef WIN32
	// Look at clock
	DWORD start = GetTickCount();

	// Execute command
	bool ret = DoCommandInternal(argv);

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
bool CommandLineInterface::ParseWatch(std::vector<std::string>& argv) {
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
	int constant;
	bool self = false;
	unsigned int options = 0;	// what flag changed
	unsigned int values = 0;    // new setting

	for (;;) {
		option = m_pGetOpt->GetOpt_Long(argv, ":b:c:d:D:i:j:l:L:np:P:r:u:w:W:", longOptions, 0);
		if (option == -1) {
			break;
		}

		switch (option) {
			case 'b':
				constant = OPTION_WATCH_BACKTRACING;
				options |= OPTION_WATCH_BACKTRACING;
				break;
			case 'c':
				constant = OPTION_WATCH_CHUNKS;
				options |= OPTION_WATCH_CHUNKS;
				break;
			case 'd':
				constant = OPTION_WATCH_DECISIONS;
				options |= OPTION_WATCH_DECISIONS;
				break;
			case 'D':
				constant = OPTION_WATCH_DEFAULT_PRODUCTIONS;
				options |= OPTION_WATCH_DEFAULT_PRODUCTIONS;
				break;
			case 'i':
				constant = OPTION_WATCH_INDIFFERENT_SELECTION;
				options |= OPTION_WATCH_INDIFFERENT_SELECTION;
				break;
			case 'j':
				constant = OPTION_WATCH_JUSTIFICATIONS;
				options |= OPTION_WATCH_JUSTIFICATIONS;
				break;
			case 'l':
				constant = OPTION_WATCH_LEARNING;
				options |= OPTION_WATCH_LEARNING;
				break;
			case 'L':
				constant = OPTION_WATCH_LOADING;
				options |= OPTION_WATCH_LOADING;
				break;
			case 'n':
				constant = OPTION_WATCH_NONE;
				options |= OPTION_WATCH_NONE;
				break;
			case 'p':
				constant = OPTION_WATCH_PHASES;
				options |= OPTION_WATCH_PHASES;
				break;
			case 'P':
				constant = OPTION_WATCH_PRODUCTIONS;
				options |= OPTION_WATCH_PRODUCTIONS;
				break;
			case 'r':
				constant = OPTION_WATCH_PREFERENCES;
				options |= OPTION_WATCH_PREFERENCES;
				break;
			case 'u':
				constant = OPTION_WATCH_USER_PRODUCTIONS;
				options |= OPTION_WATCH_USER_PRODUCTIONS;
				break;
			case 'w':
				constant = OPTION_WATCH_WMES;
				options |= OPTION_WATCH_WMES;
				break;
			case 'W':
				constant = OPTION_WATCH_WME_DETAIL;
				options |= OPTION_WATCH_WME_DETAIL;
				break;
			case ':':
				return HandleSyntaxError(Constants::kCLIWatch, "Missing option argument.");
			case '?':
				return HandleSyntaxError(Constants::kCLIWatch, "Unrecognized option.");
			default:
				return HandleGetOptError(option);
		}

		// process argument
		if (!WatchArg(values, constant, GetOpt::optarg)) {
			return false;
		}
	}

	// Only one non-option argument allowed, watch level
	if (GetOpt::optind == argv.size() - 1) {

		if (!IsInteger(argv[GetOpt::optind])) {
			return HandleSyntaxError(Constants::kCLIWatch, "Watch level must be an integer.");
		}
		int watchLevel = atoi(argv[GetOpt::optind].c_str());
		if ((watchLevel < 0) || (watchLevel > 5)) {
			return HandleSyntaxError(Constants::kCLIWatch, "Watch level must be 0 to 5.");
		}

		if (watchLevel == 0) {
			// Turn everything off
			options |= OPTION_WATCH_NONE;
		} else {
		
			// Activate the options
			options |= OPTION_WATCH_DECISIONS | OPTION_WATCH_PHASES | OPTION_WATCH_DEFAULT_PRODUCTIONS | OPTION_WATCH_USER_PRODUCTIONS
				| OPTION_WATCH_CHUNKS | OPTION_WATCH_JUSTIFICATIONS | OPTION_WATCH_WMES | OPTION_WATCH_PREFERENCES;

			// Reset some settings per old soar 8.5.2 behavior
			// Don't reset wme detail or learning unless watch 0
			WatchArg(values, OPTION_WATCH_DECISIONS, 0);			// set true in watch 1
			WatchArg(values, OPTION_WATCH_PHASES, 0);				// set true in watch 2
			WatchArg(values, OPTION_WATCH_DEFAULT_PRODUCTIONS, 0);	// set true in watch 3
			WatchArg(values, OPTION_WATCH_USER_PRODUCTIONS, 0);		// set true in watch 3
			WatchArg(values, OPTION_WATCH_CHUNKS, 0);				// set true in watch 3
			WatchArg(values, OPTION_WATCH_JUSTIFICATIONS, 0);		// set true in watch 3
			WatchArg(values, OPTION_WATCH_WMES, 0);					// set true in watch 4
			WatchArg(values, OPTION_WATCH_PREFERENCES, 0);			// set true in watch 5

			// TODO: This is off by default and nothing seems to turn it on
			//pKernelHack->SetSysparam(m_pAgent, TRACE_OPERAND2_REMOVALS_SYSPARAM, false);

			// Switch out the level
			switch (watchLevel) {
				case 5:
					WatchArg(values, OPTION_WATCH_PREFERENCES, 1);
					// 5 includes 4

				case 4:
					WatchArg(values, OPTION_WATCH_WMES, 1);
					// 4 includes 3

				case 3:
					WatchArg(values, OPTION_WATCH_DEFAULT_PRODUCTIONS, 1);
					WatchArg(values, OPTION_WATCH_USER_PRODUCTIONS, 1);
					WatchArg(values, OPTION_WATCH_CHUNKS, 1);
					WatchArg(values, OPTION_WATCH_JUSTIFICATIONS, 1);
					// 3 includes 2

				case 2:
					WatchArg(values, OPTION_WATCH_PHASES, 1);
					// 2 includes 1

				case 1:
				default:
					WatchArg(values, OPTION_WATCH_DECISIONS, 1);
					break;
			}
		}

	} else if ((unsigned)GetOpt::optind < argv.size()) {
		return HandleSyntaxError(Constants::kCLIWatch);
	}

	return DoWatch(options, values);
}

//__        __    _       _        _
//\ \      / /_ _| |_ ___| |__    / \   _ __ __ _
// \ \ /\ / / _` | __/ __| '_ \  / _ \ | '__/ _` |
//  \ V  V / (_| | || (__| | | |/ ___ \| | | (_| |
//   \_/\_/ \__,_|\__\___|_| |_/_/   \_\_|  \__, |
//                                          |___/
bool CommandLineInterface::WatchArg(unsigned int& values, const unsigned int option, const char* arg) {
	if (!arg || !IsInteger(arg)) {
		return HandleSyntaxError(Constants::kCLIWatch, "Arguments must be integers.");
	}
	return WatchArg(values, option, atoi(arg));
}

//__        __    _       _        _
//\ \      / /_ _| |_ ___| |__    / \   _ __ __ _
// \ \ /\ / / _` | __/ __| '_ \  / _ \ | '__/ _` |
//  \ V  V / (_| | || (__| | | |/ ___ \| | | (_| |
//   \_/\_/ \__,_|\__\___|_| |_/_/   \_\_|  \__, |
//                                          |___/
bool CommandLineInterface::WatchArg(unsigned int& values, const unsigned int option, int argInt) {
	// If option is none, values will be ignored anyway
	if (option == OPTION_WATCH_NONE) {
		return true;
	}

	if (option <= OPTION_WATCH_WME_DETAIL) {
		// Detail arguments 
		if ((argInt < 0) || (argInt > 2)) {
			return HandleSyntaxError(Constants::kCLIWatch, "Detail argument must 0, 1, or 2.");
		}

		// First, shift argInt if necessary
		if (option == OPTION_WATCH_WME_DETAIL) {
			argInt <<= 2;
		}

		// Second set the bits to 1
		values |= option;

		// Third, create a value to and
		argInt |= ~option;

		// Finally, and it with the values
		values &= argInt;

	} else {
		// Switch arguments
		if ((argInt < 0) || (argInt > 1)) {
			return HandleSyntaxError(Constants::kCLIWatch, "Switch argument must 0 or 1.");
		}

		if (argInt) {
			// Turn on option
			values |= option;
		} else {
			// Turn off option
			values &= ~option;
		}
	}

	return true;
}

// ____     __        __    _       _
//|  _ \  __\ \      / /_ _| |_ ___| |__
//| | | |/ _ \ \ /\ / / _` | __/ __| '_ \
//| |_| | (_) \ V  V / (_| | || (__| | | |
//|____/ \___/ \_/\_/ \__,_|\__\___|_| |_|
//
bool CommandLineInterface::DoWatch(const unsigned int options, unsigned int values) {
	// Need agent pointer for function calls
	if (!RequireAgent()) {
		return false;
	}

	// Attain the evil back door of doom, even though we aren't the TgD, because we'll probably need it
	gSKI::EvilBackDoor::ITgDWorkArounds* pKernelHack = m_pKernel->getWorkaroundObject();

	// If option is watch none, set values all off
	if (options == OPTION_WATCH_NONE) {
		values = 0;
	}

	// Next, do we have a watch level? (none flag will set this to zero)
	// No watch level and no none flags, that means we have to do the rest
	if (options & OPTION_WATCH_BACKTRACING) {
		pKernelHack->SetSysparam(m_pAgent, TRACE_BACKTRACING_SYSPARAM, values & OPTION_WATCH_BACKTRACING);
	}

	if (options & OPTION_WATCH_CHUNKS) {
		pKernelHack->SetSysparam(m_pAgent, TRACE_FIRINGS_OF_CHUNKS_SYSPARAM, values & OPTION_WATCH_CHUNKS);
	}

	if (options & OPTION_WATCH_DECISIONS) {
		pKernelHack->SetSysparam(m_pAgent, TRACE_CONTEXT_DECISIONS_SYSPARAM, values & OPTION_WATCH_DECISIONS);
	}

	if (options & OPTION_WATCH_DEFAULT_PRODUCTIONS) {
		pKernelHack->SetSysparam(m_pAgent, TRACE_FIRINGS_OF_DEFAULT_PRODS_SYSPARAM, values & OPTION_WATCH_DEFAULT_PRODUCTIONS);
	}

	if (options & OPTION_WATCH_INDIFFERENT_SELECTION) {
		pKernelHack->SetSysparam(m_pAgent, TRACE_INDIFFERENT_SYSPARAM, values & OPTION_WATCH_INDIFFERENT_SELECTION);
	}

	if (options & OPTION_WATCH_JUSTIFICATIONS) {
		pKernelHack->SetSysparam(m_pAgent, TRACE_FIRINGS_OF_JUSTIFICATIONS_SYSPARAM, values & OPTION_WATCH_JUSTIFICATIONS);
	}

	if (options & OPTION_WATCH_LOADING) {
		pKernelHack->SetSysparam(m_pAgent, TRACE_LOADING_SYSPARAM, values & OPTION_WATCH_LOADING);
	}

	if (options & OPTION_WATCH_PHASES) {
		pKernelHack->SetSysparam(m_pAgent, TRACE_PHASES_SYSPARAM, values & OPTION_WATCH_PHASES);
	}

	if (options & OPTION_WATCH_PRODUCTIONS) {
		pKernelHack->SetSysparam(m_pAgent, TRACE_FIRINGS_OF_DEFAULT_PRODS_SYSPARAM, values & OPTION_WATCH_PRODUCTIONS);
		pKernelHack->SetSysparam(m_pAgent, TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM, values & OPTION_WATCH_PRODUCTIONS);
		pKernelHack->SetSysparam(m_pAgent, TRACE_FIRINGS_OF_CHUNKS_SYSPARAM, values & OPTION_WATCH_PRODUCTIONS);
		pKernelHack->SetSysparam(m_pAgent, TRACE_FIRINGS_OF_JUSTIFICATIONS_SYSPARAM, values & OPTION_WATCH_PRODUCTIONS);
	}

	if (options & OPTION_WATCH_PREFERENCES) {
		pKernelHack->SetSysparam(m_pAgent, TRACE_FIRINGS_PREFERENCES_SYSPARAM, values & OPTION_WATCH_PREFERENCES);
	}

	if (options & OPTION_WATCH_USER_PRODUCTIONS) {
		pKernelHack->SetSysparam(m_pAgent, TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM, values & OPTION_WATCH_USER_PRODUCTIONS);
	}

	if (options & OPTION_WATCH_WMES) {
		pKernelHack->SetSysparam(m_pAgent, TRACE_WM_CHANGES_SYSPARAM, values & OPTION_WATCH_WMES);
	}

	if (options & OPTION_WATCH_LEARNING) {
		switch (values & OPTION_WATCH_LEARNING) {
			case 0:
			default:
				pKernelHack->SetSysparam(m_pAgent, TRACE_CHUNK_NAMES_SYSPARAM, false);
				pKernelHack->SetSysparam(m_pAgent, TRACE_CHUNKS_SYSPARAM, false);
				pKernelHack->SetSysparam(m_pAgent, TRACE_JUSTIFICATION_NAMES_SYSPARAM, false);
				pKernelHack->SetSysparam(m_pAgent, TRACE_JUSTIFICATIONS_SYSPARAM, false);
				break;
			case 1:
				pKernelHack->SetSysparam(m_pAgent, TRACE_CHUNK_NAMES_SYSPARAM, true);
				pKernelHack->SetSysparam(m_pAgent, TRACE_CHUNKS_SYSPARAM, false);
				pKernelHack->SetSysparam(m_pAgent, TRACE_JUSTIFICATION_NAMES_SYSPARAM, true);
				pKernelHack->SetSysparam(m_pAgent, TRACE_JUSTIFICATIONS_SYSPARAM, false);
				break;
			case 2:
				pKernelHack->SetSysparam(m_pAgent, TRACE_CHUNK_NAMES_SYSPARAM, true);
				pKernelHack->SetSysparam(m_pAgent, TRACE_CHUNKS_SYSPARAM, true);
				pKernelHack->SetSysparam(m_pAgent, TRACE_JUSTIFICATION_NAMES_SYSPARAM, true);
				pKernelHack->SetSysparam(m_pAgent, TRACE_JUSTIFICATIONS_SYSPARAM, true);
				break;
		}
	}

	if (options & OPTION_WATCH_WME_DETAIL) {
		switch ((values & OPTION_WATCH_WME_DETAIL) >> 2) {
			case 0:
			default:
				pKernelHack->SetSysparam(m_pAgent, TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM, NONE_WME_TRACE);
				break;
			case 1:
				pKernelHack->SetSysparam(m_pAgent, TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM, TIMETAG_WME_TRACE);
				break;
			case 2:
				pKernelHack->SetSysparam(m_pAgent, TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM, FULL_WME_TRACE);
				break;
		}
	}

	return true;
}

//  ____ _               _    _____          _   _      _
// / ___| |__   ___  ___| | _|  ___|__  _ __| | | | ___| |_ __
//| |   | '_ \ / _ \/ __| |/ / |_ / _ \| '__| |_| |/ _ \ | '_ \
//| |___| | | |  __/ (__|   <|  _| (_) | |  |  _  |  __/ | |_) |
// \____|_| |_|\___|\___|_|\_\_|  \___/|_|  |_| |_|\___|_| .__/
//                                                       |_|
bool CommandLineInterface::CheckForHelp(std::vector<std::string>& argv) {

	// Standard help check if there is more than one argument
	if (argv.size() > 1) {
		// Is one of the two help strings present?
		if (argv[1] == "-h" || argv[1] == "--help") {
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
		// Critical error
		m_CriticalError = true;
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
bool CommandLineInterface::IsInteger(const string& s) {
	string::const_iterator iter = s.begin();
	
	// Allow negatives
	if (s.length() > 1) {
		if (*iter == '-') {
			++iter;
		}
	}

	while (iter != s.end()) {
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
bool CommandLineInterface::HandleSyntaxError(const char* command, const char* details) {
	m_Result += Constants::kCLISyntaxError;
	m_Result += " (";
	m_Result += command;
	m_Result += ")\n";
	if (details) {
		m_Result += details;
		m_Result += '\n';
	}
	m_Result += "Type 'help ";
	m_Result += command;
	m_Result += "' or '";
	m_Result += command;
	m_Result += " --help' for syntax and usage.";
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
		// Critical error
		m_CriticalError = true;
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
	m_Result += "Internal error: m_pGetOpt->GetOpt_Long returned '";
	m_Result += option;
	m_Result += "'!";
	// Critical error
	m_CriticalError = true;
	return false;
}
