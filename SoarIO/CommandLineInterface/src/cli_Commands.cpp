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
#include "IgSKI_AgentPerformanceMonitor.h"

// BADBAD: I think we should be using an error class instead to work with error objects.
#include "../../gSKI/src/gSKI_Error.h"

// SML includes
#include "sml_Connection.h"
#include "sml_StringOps.h"

using namespace std;
using namespace cli;
using namespace sml;

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
bool CommandLineInterface::ParseCD(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	unused(pAgent);

	// Only takes one optional argument, the directory to change into
	if (argv.size() > 2) {
		return HandleSyntaxError(Constants::kCLICD, Constants::kCLITooManyArgs);
	}
	if (argv.size() > 1) {
		return DoCD(&(argv[1]));
	}
	return DoCD();
}

// ____         ____ ____
//|  _ \  ___  / ___|  _ \
//| | | |/ _ \| |   | | | |
//| |_| | (_) | |___| |_| |
//|____/ \___/ \____|____/
//
bool CommandLineInterface::DoCD(string* pDirectory) {

	// If cd is typed by itself, return to original (home) directory
	if (!pDirectory) {

		// Home dir set in constructor
		if (chdir(m_HomeDirectory.c_str())) {
			return HandleError("Could not change to home directory: " + m_HomeDirectory);
		}
		return true;
	}

	// Chop of quotes if they are there, chdir doesn't like them
	if ((pDirectory->length() > 2) && ((*pDirectory)[0] == '\"') && ((*pDirectory)[pDirectory->length() - 1] == '\"')) {
		*pDirectory = pDirectory->substr(1, pDirectory->length() - 2);
	}

	// Change to passed directory
	if (chdir(pDirectory->c_str())) {
		return HandleError("Could not change to directory: " + *pDirectory);
	}
	return true;
}

// ____                     _____     _
//|  _ \ __ _ _ __ ___  ___| ____|___| |__   ___
//| |_) / _` | '__/ __|/ _ \  _| / __| '_ \ / _ \
//|  __/ (_| | |  \__ \  __/ |__| (__| | | | (_) |
//|_|   \__,_|_|  |___/\___|_____\___|_| |_|\___/
//
bool CommandLineInterface::ParseEcho(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	unused(pAgent);

	if (argv.size() != 2) {
		return HandleSyntaxError(Constants::kCLIEcho);
	}

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
bool CommandLineInterface::ParseExcise(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
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
	unsigned int options = 0;

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
				return HandleSyntaxError(Constants::kCLIExcise, Constants::kCLIUnrecognizedOption);
			default:
				return HandleGetOptError((char)option);
		}
	}

	// If there are options, no additional argument.
	if (options) {
		if (argv.size() != (unsigned)GetOpt::optind) {
			return HandleSyntaxError(Constants::kCLIExcise, Constants::kCLITooManyArgs);
		}
		return DoExcise(pAgent, options);
	}

	// If there are no options, there must be only one production name argument
	if ((argv.size() - GetOpt::optind) != 1) {
		return HandleSyntaxError(Constants::kCLIExcise);		
	}

	// Pass the productions to the DoExcise function
	return DoExcise(pAgent, options, &(argv[GetOpt::optind]));
}

// ____        _____          _
//|  _ \  ___ | ____|_  _____(_)___  ___
//| | | |/ _ \|  _| \ \/ / __| / __|/ _ \
//| |_| | (_) | |___ >  < (__| \__ \  __/
//|____/ \___/|_____/_/\_\___|_|___/\___|
//
bool CommandLineInterface::DoExcise(gSKI::IAgent* pAgent, const unsigned int options, string* pProduction) {
	if (!RequireAgent(pAgent)) return false;

	// Acquire production manager
	gSKI::IProductionManager *pProductionManager = pAgent->GetProductionManager();
	if (!pProductionManager) {
		return HandleError("Unable to get production manager!");
	}

	// Listen for #s
	pAgent->AddPrintListener(gSKIEVENT_PRINT, &m_ResultPrintHandler);

	int exciseCount = 0;

	// Process the general options
	if (options & OPTION_EXCISE_ALL) {
		ExciseInternal(pProductionManager->GetAllProductions(), exciseCount);
	}
	if (options & OPTION_EXCISE_CHUNKS) {
		ExciseInternal(pProductionManager->GetChunks(), exciseCount);
		ExciseInternal(pProductionManager->GetJustifications(), exciseCount);
	}
	if (options & OPTION_EXCISE_DEFAULT) {
		ExciseInternal(pProductionManager->GetDefaultProductions(), exciseCount);
	}
	if (options & OPTION_EXCISE_TASK) {
		ExciseInternal(pProductionManager->GetChunks(), exciseCount);
		ExciseInternal(pProductionManager->GetJustifications(), exciseCount);
		ExciseInternal(pProductionManager->GetUserProductions(), exciseCount);
	}
	if (options & OPTION_EXCISE_USER) {
		ExciseInternal(pProductionManager->GetUserProductions(), exciseCount);
	}

	// Excise specific production
	if (pProduction) {
		// Check for the production
		gSKI::tIProductionIterator* pProdIter = pProductionManager->GetProduction((*pProduction).c_str());
		if (!pProdIter->GetNumElements()) {
			pAgent->RemovePrintListener(gSKIEVENT_PRINT, &m_ResultPrintHandler);
			return HandleError("Production not found: " + (*pProduction));
		}

		ExciseInternal(pProdIter, exciseCount);
	}
	pAgent->RemovePrintListener(gSKIEVENT_PRINT, &m_ResultPrintHandler);

	if (!m_RawOutput) {
		// Add the count tag to the front
		char buffer[kMinBufferSize];
		PrependArgTagFast(sml_Names::kParamCount, sml_Names::kTypeInt, Int2String(exciseCount, buffer, sizeof(buffer)));
	}

	return true;
}

// _____          _          ___       _                        _
//| ____|_  _____(_)___  ___|_ _|_ __ | |_ ___ _ __ _ __   __ _| |
//|  _| \ \/ / __| / __|/ _ \| || '_ \| __/ _ \ '__| '_ \ / _` | |
//| |___ >  < (__| \__ \  __/| || | | | ||  __/ |  | | | | (_| | |
//|_____/_/\_\___|_|___/\___|___|_| |_|\__\___|_|  |_| |_|\__,_|_|
//
void CommandLineInterface::ExciseInternal(gSKI::tIProductionIterator *pProdIter, int& exciseCount) {
	// Iterate through the productions using the production iterator and
	// excise and release.
	while(pProdIter->IsValid()) {
		gSKI::IProduction* ip = pProdIter->GetVal();

		if (!m_RawOutput) {
			// Save the name for the structured response
			AppendArgTagFast(sml_Names::kParamName, sml_Names::kTypeString, ip->GetName());
		}

		// Increment the count for the structured response
		++exciseCount;	// Don't *need* to outside of if above but why not.

		ip->Excise();
		ip->Release();
		pProdIter->Next();
	}
	pProdIter->Release();

}

// ____                     _   _      _
//|  _ \ __ _ _ __ ___  ___| | | | ___| |_ __
//| |_) / _` | '__/ __|/ _ \ |_| |/ _ \ | '_ \
//|  __/ (_| | |  \__ \  __/  _  |  __/ | |_) |
//|_|   \__,_|_|  |___/\___|_| |_|\___|_| .__/
//                                      |_|
bool CommandLineInterface::ParseHelp(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	unused(pAgent);

	if (argv.size() > 2) {
		return HandleSyntaxError(Constants::kCLIHelp, Constants::kCLITooManyArgs);
	}

	if (argv.size() == 2) {
		return DoHelp(&(argv[1]));
	}
	return DoHelp();
}

// ____        _   _      _
//|  _ \  ___ | | | | ___| |_ __
//| | | |/ _ \| |_| |/ _ \ | '_ \
//| |_| | (_) |  _  |  __/ | |_) |
//|____/ \___/|_| |_|\___|_| .__/
//                         |_|
bool CommandLineInterface::DoHelp(string* pCommand) {
	string output;

	if (!m_Constants.IsUsageFileAvailable()) {
		return HandleError(Constants::kCLINoUsageFile);
	}

	if (pCommand) {
		if (!m_Constants.GetUsageFor(*pCommand, output)) {
			return HandleError("Help for command '" + *pCommand + "' not found.");
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

// ____                     _   _      _       _____
//|  _ \ __ _ _ __ ___  ___| | | | ___| |_ __ | ____|_  __
//| |_) / _` | '__/ __|/ _ \ |_| |/ _ \ | '_ \|  _| \ \/ /
//|  __/ (_| | |  \__ \  __/  _  |  __/ | |_) | |___ >  <
//|_|   \__,_|_|  |___/\___|_| |_|\___|_| .__/|_____/_/\_\
//                                      |_|
bool CommandLineInterface::ParseHelpEx(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	unused(pAgent);

	if (argv.size() != 2) {
		return HandleSyntaxError(Constants::kCLIHelpEx);
	}

	return DoHelpEx(argv[1]);
}

// ____        _   _      _       _____
//|  _ \  ___ | | | | ___| |_ __ | ____|_  __
//| | | |/ _ \| |_| |/ _ \ | '_ \|  _| \ \/ /
//| |_| | (_) |  _  |  __/ | |_) | |___ >  <
//|____/ \___/|_| |_|\___|_| .__/|_____/_/\_\
//                         |_|
bool CommandLineInterface::DoHelpEx(const string& command) {
	string output;

	if (!m_Constants.IsUsageFileAvailable()) {
		return HandleError(Constants::kCLINoUsageFile);
	}

	if (!m_Constants.GetExtendedUsageFor(command, output)) {
		return HandleError("Extended help for command '" + command + "' not found.");
	}
	m_Result += output;
	return true;
}

// ____                     ___       _ _   ____
//|  _ \ __ _ _ __ ___  ___|_ _|_ __ (_) |_/ ___|  ___   __ _ _ __
//| |_) / _` | '__/ __|/ _ \| || '_ \| | __\___ \ / _ \ / _` | '__|
//|  __/ (_| | |  \__ \  __/| || | | | | |_ ___) | (_) | (_| | |
//|_|   \__,_|_|  |___/\___|___|_| |_|_|\__|____/ \___/ \__,_|_|
//
bool CommandLineInterface::ParseInitSoar(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	// No arguments
	if (argv.size() != 1) {
		return HandleSyntaxError(Constants::kCLIInitSoar, Constants::kCLITooManyArgs);
	}
	return DoInitSoar(pAgent);
}

// ____       ___       _ _   ____
//|  _ \  ___|_ _|_ __ (_) |_/ ___|  ___   __ _ _ __
//| | | |/ _ \| || '_ \| | __\___ \ / _ \ / _` | '__|
//| |_| | (_) | || | | | | |_ ___) | (_) | (_| | |
//|____/ \___/___|_| |_|_|\__|____/ \___/ \__,_|_|
//
bool CommandLineInterface::DoInitSoar(gSKI::IAgent* pAgent) {
	// Need agent pointer for function calls
	if (!RequireAgent(pAgent)) return false;

	// Simply call reinitialize
	pAgent->Halt();
	pAgent->Reinitialize();
	m_Result += "Agent reinitialized.";
	return true;
}

// ____                     _
//|  _ \ __ _ _ __ ___  ___| |    ___  __ _ _ __ _ __
//| |_) / _` | '__/ __|/ _ \ |   / _ \/ _` | '__| '_ \
//|  __/ (_| | |  \__ \  __/ |__|  __/ (_| | |  | | | |
//|_|   \__,_|_|  |___/\___|_____\___|\__,_|_|  |_| |_|
//
bool CommandLineInterface::ParseLearn(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
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
	unsigned int options = 0;

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
				return HandleSyntaxError(Constants::kCLILearn, Constants::kCLIUnrecognizedOption);
			default:
				return HandleGetOptError((char)option);
		}
	}

	// No non-option arguments
	if ((unsigned)GetOpt::optind != argv.size()) {
		return HandleSyntaxError(Constants::kCLILearn, Constants::kCLITooManyArgs);
	}

	return DoLearn(pAgent, options);
}

// ____        _
//|  _ \  ___ | |    ___  __ _ _ __ _ __
//| | | |/ _ \| |   / _ \/ _` | '__| '_ \
//| |_| | (_) | |__|  __/ (_| | |  | | | |
//|____/ \___/|_____\___|\__,_|_|  |_| |_|
//
bool CommandLineInterface::DoLearn(gSKI::IAgent* pAgent, const unsigned int options) {
	// Need agent pointer for function calls
	if (!RequireAgent(pAgent)) return false;

	// No options means print current settings
	if (!options) {
		if (m_RawOutput) {
			m_Result += "Learning is ";
			m_Result += pAgent->IsLearningOn() ? "enabled." : "disabled.";
		} else {
			const char* setting = pAgent->IsLearningOn() ? sml_Names::kTrue : sml_Names::kFalse;
			AppendArgTagFast(sml_Names::kParamLearnSetting, sml_Names::kTypeBoolean, setting);
		}
		return true;
	}

	// Check for unimplemented options
	if ((options & OPTION_LEARN_ALL_LEVELS) || (options & OPTION_LEARN_BOTTOM_UP) || (options & OPTION_LEARN_EXCEPT) || (options & OPTION_LEARN_LIST) || (options & OPTION_LEARN_ONLY)) {
		return HandleError("Options {abElo} are not implemented.");
	}

	// Enable or disable, priority to disable
	if (options & OPTION_LEARN_ENABLE) {
		pAgent->SetLearning(true);
	}

	if (options & OPTION_LEARN_DISABLE) {
		pAgent->SetLearning(false);
	}
	return true;
}

// ____                     _
//|  _ \ __ _ _ __ ___  ___| |    ___   __ _
//| |_) / _` | '__/ __|/ _ \ |   / _ \ / _` |
//|  __/ (_| | |  \__ \  __/ |__| (_) | (_| |
//|_|   \__,_|_|  |___/\___|_____\___/ \__, |
//                                     |___/
bool CommandLineInterface::ParseLog(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
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
				return HandleSyntaxError(Constants::kCLILog, Constants::kCLIUnrecognizedOption);
			default:
				return HandleGetOptError((char)option);
		}
	}

	// Only one non-option arg allowed, filename
	if ((unsigned)GetOpt::optind == argv.size() - 1) {

		// But not with the close option
		if (close) {
			return HandleSyntaxError(Constants::kCLILog, Constants::kCLITooManyArgs);
		}

		return DoLog(pAgent, argv[GetOpt::optind].c_str(), append);

	} else if ((unsigned)GetOpt::optind < argv.size()) {
		return HandleSyntaxError(Constants::kCLILog, Constants::kCLITooManyArgs);
	}

	// No appending without a filename, and if we got here, there is no filename
	if (append) {
		return HandleSyntaxError(Constants::kCLILog, "Append requires a filename.");
	}

	return DoLog(pAgent, 0, close);
}

// ____        _
//|  _ \  ___ | |    ___   __ _
//| | | |/ _ \| |   / _ \ / _` |
//| |_| | (_) | |__| (_) | (_| |
//|____/ \___/|_____\___/ \__, |
//                        |___/
bool CommandLineInterface::DoLog(gSKI::IAgent* pAgent, const char* pFilename, bool option) {
	if (!RequireAgent(pAgent)) return false;

	// Presence of a filename means open, absence means close or query
	if (pFilename) {
		// Open a file
		if (m_pLogFile) {
			// Unless one is already opened
			return HandleError("Close log file '" + m_LogFilename + "' first.");
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
			return HandleError("Failed to open file for logging.");
		}

		// Logging opened, add listener and save filename since we can't get it from ofstream
		pAgent->AddPrintListener(gSKIEVENT_PRINT, &m_LogPrintHandler);
		m_LogFilename = pFilename;

	} else if (option) {		
		// In absence of filename, option true means close
		if (m_pLogFile) {
			// Remove the listener and close the file
			pAgent->RemovePrintListener(gSKIEVENT_PRINT, &m_LogPrintHandler);
			delete m_pLogFile;
			m_pLogFile = 0;

			// Forget the filename
			m_LogFilename.clear();
		}
	}

	// Query at end of successful command, or by default
	if (m_RawOutput) {
		m_Result += m_pLogFile ? "Log file '" + m_LogFilename + "' opened." : "Log file closed.";

	} else {
		const char* setting = m_pLogFile ? sml_Names::kTrue : sml_Names::kFalse;
		AppendArgTagFast(sml_Names::kParamLogSetting, sml_Names::kTypeBoolean, setting);

		if (m_LogFilename.size()) {
			AppendArgTagFast(sml_Names::kParamFilename, sml_Names::kTypeString, m_LogFilename.c_str());
		}
	}

	return true;
}

// ____                     _     ____
//|  _ \ __ _ _ __ ___  ___| |   / ___|
//| |_) / _` | '__/ __|/ _ \ |   \___ \
//|  __/ (_| | |  \__ \  __/ |___ ___) |
//|_|   \__,_|_|  |___/\___|_____|____/
//
bool CommandLineInterface::ParseLS(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	unused(pAgent);

	// No arguments
	if (argv.size() != 1) {
		return HandleSyntaxError(Constants::kCLILS, Constants::kCLITooManyArgs);
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
		return true;	
	}

	// At least one file found, concatinate additional ones with newlines
	do {
		if (m_RawOutput) {
			// TODO: Columns and stats
			if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				m_Result += '[';
			}
			m_Result += FindFileData.cFileName;
			if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				m_Result += ']';
			}
			m_Result += '\n';
		} else {
			if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				AppendArgTagFast(sml_Names::kParamDirectory, sml_Names::kTypeString, FindFileData.cFileName);
			} else {
				AppendArgTagFast(sml_Names::kParamFilename, sml_Names::kTypeString, FindFileData.cFileName);
			}
		}

	} while (FindNextFile(hFind, &FindFileData));

	// Close the file find
	FindClose(hFind);
	return true;

#else // WIN32
	return HandleError("TODO: ls on non-windows platforms");
#endif // WIN32
}

// ____                     __  __       _ _   _    _   _   _        _ _           _
//|  _ \ __ _ _ __ ___  ___|  \/  |_   _| | |_(_)  / \ | |_| |_ _ __(_) |__  _   _| |_ ___  ___
//| |_) / _` | '__/ __|/ _ \ |\/| | | | | | __| | / _ \| __| __| '__| | '_ \| | | | __/ _ \/ __|
//|  __/ (_| | |  \__ \  __/ |  | | |_| | | |_| |/ ___ \ |_| |_| |  | | |_) | |_| | ||  __/\__ \
//|_|   \__,_|_|  |___/\___|_|  |_|\__,_|_|\__|_/_/   \_\__|\__|_|  |_|_.__/ \__,_|\__\___||___/
//
bool CommandLineInterface::ParseMultiAttributes(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	// No more than three arguments
	if (argv.size() > 3) {
		return HandleSyntaxError(Constants::kCLIMultiAttributes, Constants::kCLITooManyArgs);
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
		return DoMultiAttributes(pAgent, &(argv[1]), n);
	} 
	return DoMultiAttributes(pAgent, 0, n);
}

// ____        __  __       _ _   _    _   _   _        _ _           _
//|  _ \  ___ |  \/  |_   _| | |_(_)  / \ | |_| |_ _ __(_) |__  _   _| |_ ___  ___
//| | | |/ _ \| |\/| | | | | | __| | / _ \| __| __| '__| | '_ \| | | | __/ _ \/ __|
//| |_| | (_) | |  | | |_| | | |_| |/ ___ \ |_| |_| |  | | |_) | |_| | ||  __/\__ \
//|____/ \___/|_|  |_|\__,_|_|\__|_/_/   \_\__|\__|_|  |_|_.__/ \__,_|\__\___||___/
//
bool CommandLineInterface::DoMultiAttributes(gSKI::IAgent* pAgent, string* pAttribute, int n) {
	if (!RequireAgent(pAgent)) return false;

	if (!pAttribute && !n) {
		// No args, print current setting
		int count = 0;
		
		// Arbitrary buffer size
		char buf[1024];

		gSKI::tIMultiAttributeIterator* pIt = pAgent->GetMultiAttributes();
		if (!pIt->GetNumElements()) {
			if (m_RawOutput) {
				m_Result += "No multi-attributes declared for this agent.";
			}

		} else {
			if (m_RawOutput) {
				m_Result += "Value\tSymbol";
			}

			gSKI::IMultiAttribute* pMA;


			for(; pIt->IsValid(); pIt->Next()) {
				pMA = pIt->GetVal();

				if (m_RawOutput) {
					memset(buf, 0, sizeof(buf));
					snprintf(buf, sizeof(buf) - 1, "\n%d\t%s", pMA->GetMatchingPriority(), pMA->GetAttributeName());
					m_Result += buf;
				} else {
					// Value
					AppendArgTagFast(sml_Names::kParamValue, sml_Names::kTypeInt, Int2String(count, buf, sizeof(buf)));
					// Symbol
					AppendArgTagFast(sml_Names::kParamName, sml_Names::kTypeString, pMA->GetAttributeName());
				}

				++count;
				pMA->Release();
			}

		}
		pIt->Release();

		if (!m_RawOutput) {
			// Add the count tag to the front
			PrependArgTagFast(sml_Names::kParamCount, sml_Names::kTypeInt, Int2String(count, buf, sizeof(buf)));
		}
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
	pAgent->SetMultiAttribute(pAttribute->c_str(), n);
 	return true;
}

// ____                     ____             ____
//|  _ \ __ _ _ __ ___  ___|  _ \ ___  _ __ |  _ \
//| |_) / _` | '__/ __|/ _ \ |_) / _ \| '_ \| | | |
//|  __/ (_| | |  \__ \  __/  __/ (_) | |_) | |_| |
//|_|   \__,_|_|  |___/\___|_|   \___/| .__/|____/
//                                    |_|
bool CommandLineInterface::ParsePopD(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	unused(pAgent);

	// No arguments
	if (argv.size() != 1) {
		return HandleSyntaxError(Constants::kCLIPopD, Constants::kCLITooManyArgs);
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
		return HandleError("Directory stack empty, no directory to change to.");
	}

	// Change to the directory
	if (!DoCD(&(m_DirectoryStack.top()))) {
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
bool CommandLineInterface::ParsePrint(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
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
	unsigned int options = 0;

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
				return HandleSyntaxError(Constants::kCLIPrint, Constants::kCLIMissingOptionArg);
			case '?':
				return HandleSyntaxError(Constants::kCLIPrint, Constants::kCLIUnrecognizedOption);
			default:
				return HandleGetOptError((char)option);
		}
	}

	// One additional optional argument
	if ((argv.size() - GetOpt::optind) > 1) {
		return HandleSyntaxError(Constants::kCLIPrint, Constants::kCLITooManyArgs);
	} else if ((argv.size() - GetOpt::optind) == 1) {
		return DoPrint(pAgent, options, depth, &(argv[GetOpt::optind]));
	}
	return DoPrint(pAgent, options, depth);
}

// ____        ____       _       _
//|  _ \  ___ |  _ \ _ __(_)_ __ | |_
//| | | |/ _ \| |_) | '__| | '_ \| __|
//| |_| | (_) |  __/| |  | | | | | |_
//|____/ \___/|_|   |_|  |_|_| |_|\__|
//
bool CommandLineInterface::DoPrint(gSKI::IAgent* pAgent, const unsigned int options, int depth, string* pArg) {
	// TODO: structured output

	// Need agent pointer for function calls
	if (!RequireAgent(pAgent)) return false;
	if (!RequireKernel()) return false;

	// Attain the evil back door of doom, even though we aren't the TgD
	gSKI::EvilBackDoor::ITgDWorkArounds* pKernelHack = m_pKernel->getWorkaroundObject();

	// Check for stack print
	if (options & OPTION_PRINT_STACK) {
		pAgent->AddPrintListener(gSKIEVENT_PRINT, &m_ResultPrintHandler);
		pKernelHack->PrintStackTrace(pAgent, (options & OPTION_PRINT_STATES) ? true : false, (options & OPTION_PRINT_OPERATORS) ? true : false);
		pAgent->RemovePrintListener(gSKIEVENT_PRINT, &m_ResultPrintHandler);
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
		pAgent->AddPrintListener(gSKIEVENT_PRINT, &m_ResultPrintHandler);
        pKernelHack->PrintUser(pAgent, 0, internal, filename, full, DEFAULT_PRODUCTION_TYPE);
        pKernelHack->PrintUser(pAgent, 0, internal, filename, full, USER_PRODUCTION_TYPE);
        pKernelHack->PrintUser(pAgent, 0, internal, filename, full, CHUNK_PRODUCTION_TYPE);
        pKernelHack->PrintUser(pAgent, 0, internal, filename, full, JUSTIFICATION_PRODUCTION_TYPE);
		pAgent->RemovePrintListener(gSKIEVENT_PRINT, &m_ResultPrintHandler);
		return true;
	}
	if (options & OPTION_PRINT_CHUNKS) {
		pAgent->AddPrintListener(gSKIEVENT_PRINT, &m_ResultPrintHandler);
        pKernelHack->PrintUser(pAgent, 0, internal, filename, full, CHUNK_PRODUCTION_TYPE);
		pAgent->RemovePrintListener(gSKIEVENT_PRINT, &m_ResultPrintHandler);
		return true;
	}
	if (options & OPTION_PRINT_DEFAULTS) {
		pAgent->AddPrintListener(gSKIEVENT_PRINT, &m_ResultPrintHandler);
        pKernelHack->PrintUser(pAgent, 0, internal, filename, full, DEFAULT_PRODUCTION_TYPE);
		pAgent->RemovePrintListener(gSKIEVENT_PRINT, &m_ResultPrintHandler);
		return true;
	}
	if (options & OPTION_PRINT_JUSTIFICATIONS) {
		pAgent->AddPrintListener(gSKIEVENT_PRINT, &m_ResultPrintHandler);
        pKernelHack->PrintUser(pAgent, 0, internal, filename, full, JUSTIFICATION_PRODUCTION_TYPE);
		pAgent->RemovePrintListener(gSKIEVENT_PRINT, &m_ResultPrintHandler);
		return true;
	}
	if (options & OPTION_PRINT_USER) {
		pAgent->AddPrintListener(gSKIEVENT_PRINT, &m_ResultPrintHandler);
        pKernelHack->PrintUser(pAgent, 0, internal, filename, full, USER_PRODUCTION_TYPE);
		pAgent->RemovePrintListener(gSKIEVENT_PRINT, &m_ResultPrintHandler);
		return true;
	}

	// Default to symbol print
	pAgent->AddPrintListener(gSKIEVENT_PRINT, &m_ResultPrintHandler);
	pKernelHack->PrintSymbol(pAgent, const_cast<char*>(pArg->c_str()), name, filename, internal, full, depth);
	pAgent->RemovePrintListener(gSKIEVENT_PRINT, &m_ResultPrintHandler);
	return true;
}

// ____                     ____            _     ____
//|  _ \ __ _ _ __ ___  ___|  _ \ _   _ ___| |__ |  _ \
//| |_) / _` | '__/ __|/ _ \ |_) | | | / __| '_ \| | | |
//|  __/ (_| | |  \__ \  __/  __/| |_| \__ \ | | | |_| |
//|_|   \__,_|_|  |___/\___|_|    \__,_|___/_| |_|____/
//
bool CommandLineInterface::ParsePushD(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	unused(pAgent);

	// Only takes one argument, the directory to change into
	if (argv.size() != 2) {
		return HandleSyntaxError(Constants::kCLIPushD, Constants::kCLITooManyArgs);
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
	if (!DoCD(&directory)) {
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
bool CommandLineInterface::ParsePWD(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	unused(pAgent);

	// No arguments to print working directory
	if (argv.size() != 1) {
		return HandleSyntaxError(Constants::kCLIPWD, Constants::kCLITooManyArgs);
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
bool CommandLineInterface::ParseQuit(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	unused(pAgent);

	// Quit needs no help
	argv.clear();
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
bool CommandLineInterface::ParseRun(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
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
	unsigned int options = 0;

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
				return HandleSyntaxError(Constants::kCLIRun, Constants::kCLIUnrecognizedOption);
			default:
				return HandleGetOptError((char)option);
		}
	}

	// Count defaults to 1 (which are ignored if the options default, since they default to forever)
	int count = 1;

	// Only one non-option argument allowed, count
	if ((unsigned)GetOpt::optind == argv.size() - 1) {

		if (!IsInteger(argv[GetOpt::optind])) {
			return HandleSyntaxError(Constants::kCLIRun, "Count must be an integer.");
		}
		count = atoi(argv[GetOpt::optind].c_str());
		if (count <= 0) {
			return HandleSyntaxError(Constants::kCLIRun, "Count must be greater than 0.");
		}

	} else if ((unsigned)GetOpt::optind < argv.size()) {
		return HandleSyntaxError(Constants::kCLIRun, Constants::kCLITooManyArgs);
	}

	return DoRun(pAgent, options, count);
}

// ____        ____
//|  _ \  ___ |  _ \ _   _ _ __
//| | | |/ _ \| |_) | | | | '_ \
//| |_| | (_) |  _ <| |_| | | | |
//|____/ \___/|_| \_\\__,_|_| |_|
//
bool CommandLineInterface::DoRun(gSKI::IAgent* pAgent, const unsigned int options, int count) {
	// TODO: structured output

	if (!RequireAgent(pAgent)) return false;
	if (!RequireKernel()) return false;

	// TODO: Rather tricky options
	if ((options & OPTION_RUN_OPERATOR) || (options & OPTION_RUN_OUTPUT) || (options & OPTION_RUN_STATE)) {
		return HandleError("Options { o, O, S } not implemented yet.");
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
		return HandleError("Forever option is not implemented yet.");
	}

	// If running self, an agent pointer is necessary.  Otherwise, a Kernel pointer is necessary.
	egSKIRunResult runResult;
	if (options & OPTION_RUN_SELF) {
		pAgent->AddPrintListener(gSKIEVENT_PRINT, &m_ResultPrintHandler);
		runResult = pAgent->RunInClientThread(runType, count, m_pError);
		pAgent->RemovePrintListener(gSKIEVENT_PRINT, &m_ResultPrintHandler);
	} else {
		pAgent->AddPrintListener(gSKIEVENT_PRINT, &m_ResultPrintHandler);
        m_pKernel->GetAgentManager()->ClearAllInterrupts();
        m_pKernel->GetAgentManager()->AddAllAgentsToRunList();
		runResult = m_pKernel->GetAgentManager()->RunInClientThread(runType, count, gSKI_INTERLEAVE_SMALLEST_STEP, m_pError);
		pAgent->RemovePrintListener(gSKIEVENT_PRINT, &m_ResultPrintHandler);
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
			return HandleError("Unknown egSKIRunResult code returned.");
	}
	return true;
}

// ____                     ____
//|  _ \ __ _ _ __ ___  ___/ ___|  ___  _   _ _ __ ___ ___
//| |_) / _` | '__/ __|/ _ \___ \ / _ \| | | | '__/ __/ _ \
//|  __/ (_| | |  \__ \  __/___) | (_) | |_| | | | (_|  __/
//|_|   \__,_|_|  |___/\___|____/ \___/ \__,_|_|  \___\___|
//
bool CommandLineInterface::ParseSource(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	if (argv.size() < 2) {
		// Source requires a filename
		return HandleSyntaxError(Constants::kCLISource, Constants::kCLITooFewArgs);

	} else if (argv.size() > 2) {
		// but only one filename
		return HandleSyntaxError(Constants::kCLISource, "Source only one file at a time.");
	}

	return DoSource(pAgent, argv[1]);
}

// ____       ____
//|  _ \  ___/ ___|  ___  _   _ _ __ ___ ___
//| | | |/ _ \___ \ / _ \| | | | '__/ __/ _ \
//| |_| | (_) |__) | (_) | |_| | | | (_|  __/
//|____/ \___/____/ \___/ \__,_|_|  \___\___|
//
bool CommandLineInterface::DoSource(gSKI::IAgent* pAgent, const string& filename) {
	if (!RequireAgent(pAgent)) return false;

	// Open the file
	ifstream soarFile(filename.c_str());
	if (!soarFile) {
		return HandleError("Failed to open file '" + filename + "' for reading.");
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
				HandleError("Unexpected end of file. Unmatched opening brace.");
				HandleSourceError(lineCountCache, filename);
				return false;

			} else if (braces < 0) {
				HandleError("Closing brace(s) found without matching opening brace.");
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
		if (!DoCommandInternal(pAgent, command)) {
			// Command failed, error in result
			HandleSourceError(lineCountCache, filename);
			return false;
		}	
	}

	// Completion
	--m_SourceDepth;

	// if we're returing to the user and there is stuff on the source dir stack, print warning (?)
	if (!m_SourceDepth) {
		
		// Print working directory if source dir depth !=  0
		if (m_SourceDirDepth != 0) {
			DoPWD();	// Ignore error
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
		m_ErrorMessage += "\nSource command error on line ";
		// TODO: arbitrary buffer size here
		char buf[256];
		memset(buf, 0, 256);
		m_ErrorMessage += itoa(errorLine, buf, 10);

		m_ErrorMessage += " of ";
		
		string directory;
		GetCurrentWorkingDirectory(directory); // Again, ignore error here

		m_ErrorMessage += filename + " (" + directory + ")";

	} else {
		m_ErrorMessage += "\n\t--> Sourced by: " + filename;
	}
}

// ____                     ____  ____
//|  _ \ __ _ _ __ ___  ___/ ___||  _ \
//| |_) / _` | '__/ __|/ _ \___ \| |_) |
//|  __/ (_| | |  \__ \  __/___) |  __/
//|_|   \__,_|_|  |___/\___|____/|_|
//
bool CommandLineInterface::ParseSP(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
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

	return DoSP(pAgent, production);
}

// ____       ____  ____
//|  _ \  ___/ ___||  _ \
//| | | |/ _ \___ \| |_) |
//| |_| | (_) |__) |  __/
//|____/ \___/____/|_|
//
bool CommandLineInterface::DoSP(gSKI::IAgent* pAgent, const string& production) {
	// Must have agent to give production to
	if (!RequireAgent(pAgent)) return false;

	// Acquire production manager
	gSKI::IProductionManager *pProductionManager = pAgent->GetProductionManager();

	// Load the production
	pAgent->AddPrintListener(gSKIEVENT_PRINT, &m_ResultPrintHandler);
	pProductionManager->AddProduction(const_cast<char*>(production.c_str()), m_pError);
	pAgent->RemovePrintListener(gSKIEVENT_PRINT, &m_ResultPrintHandler);

	if(m_pError->Id != gSKI::gSKIERR_NONE) {
		return HandleError("Unable to add the production: " + production);
	}

	if (m_RawOutput) {
		// TODO: The kernel is supposed to print this but doesnt!
		m_Result += '*';
	}
	return true;
}

// ____                     ____  _        _
//|  _ \ __ _ _ __ ___  ___/ ___|| |_ __ _| |_ ___
//| |_) / _` | '__/ __|/ _ \___ \| __/ _` | __/ __|
//|  __/ (_| | |  \__ \  __/___) | || (_| | |_\__ \
//|_|   \__,_|_|  |___/\___|____/ \__\__,_|\__|___/
//
bool CommandLineInterface::ParseStats(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	// No arguments
	if (argv.size() != 1) {
		return HandleSyntaxError(Constants::kCLIStats, Constants::kCLITooManyArgs);
	}

	return DoStats(pAgent);
}

// ____       ____  _        _
//|  _ \  ___/ ___|| |_ __ _| |_ ___
//| | | |/ _ \___ \| __/ _` | __/ __|
//| |_| | (_) |__) | || (_| | |_\__ \
//|____/ \___/____/ \__\__,_|\__|___/
//
bool CommandLineInterface::DoStats(gSKI::IAgent* pAgent) {
	// Need agent pointer for function calls
	if (!RequireAgent(pAgent)) return false;

	gSKI::IAgentPerformanceMonitor* pPerfMon = pAgent->GetPerformanceMonitor();

	// This next block needs to be redone pending a rewrite on the gSKI side.
	int argc = 1;
	char* argv[3];
	argv[0] = new char[6];
	memset(argv[0], 0, 6);
	strncpy(argv[0], "stats", 5);

	argv[1] = new char[7];
	memset(argv[1], 0, 7);
	strncpy(argv[1], "-stats", 6);

	argv[2] = 0;

	const char* pResult = 0;

	pAgent->AddPrintListener(gSKIEVENT_PRINT, &m_ResultPrintHandler);
	bool ret = pPerfMon->GetStatsString(argc, argv, &pResult);
	pAgent->RemovePrintListener(gSKIEVENT_PRINT, &m_ResultPrintHandler);

	if(strlen(pResult) > 0) {
		m_Result += pResult;
	}
	delete [] argv[0];
	delete [] argv[1];
	return ret;
}

// ____                     ____  _             ____
//|  _ \ __ _ _ __ ___  ___/ ___|| |_ ___  _ __/ ___|  ___   __ _ _ __
//| |_) / _` | '__/ __|/ _ \___ \| __/ _ \| '_ \___ \ / _ \ / _` | '__|
//|  __/ (_| | |  \__ \  __/___) | || (_) | |_) |__) | (_) | (_| | |
//|_|   \__,_|_|  |___/\___|____/ \__\___/| .__/____/ \___/ \__,_|_|
//                                        |_|
bool CommandLineInterface::ParseStopSoar(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
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
				return HandleSyntaxError(Constants::kCLIStopSoar, Constants::kCLIUnrecognizedOption);
			default:
				return HandleGetOptError((char)option);
		}
	}

	// Concatinate remaining args for 'reason'
	string reasonForStopping;
	if ((unsigned)GetOpt::optind < argv.size()) {
		while ((unsigned)GetOpt::optind < argv.size()) {
			reasonForStopping += argv[GetOpt::optind++] + ' ';
		}
	}
	return DoStopSoar(pAgent, self, reasonForStopping);
}

// ____       ____  _             ____
//|  _ \  ___/ ___|| |_ ___  _ __/ ___|  ___   __ _ _ __
//| | | |/ _ \___ \| __/ _ \| '_ \___ \ / _ \ / _` | '__|
//| |_| | (_) |__) | || (_) | |_) |__) | (_) | (_| | |
//|____/ \___/____/ \__\___/| .__/____/ \___/ \__,_|_|
//                          |_|
bool CommandLineInterface::DoStopSoar(gSKI::IAgent* pAgent, bool self, const string& reasonForStopping) {
	unused(pAgent);
	m_Result += "TODO: do stop-soar ";
	m_Result += self;
	m_Result += reasonForStopping;
	return true;
}

// ____                    _____ _
//|  _ \ __ _ _ __ ___  __|_   _(_)_ __ ___   ___
//| |_) / _` | '__/ __|/ _ \| | | | '_ ` _ \ / _ \
//|  __/ (_| | |  \__ \  __/| | | | | | | | |  __/
//|_|   \__,_|_|  |___/\___||_| |_|_| |_| |_|\___|
//
bool CommandLineInterface::ParseTime(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	// There must at least be a command
	if (argv.size() < 2) {
		return HandleSyntaxError(Constants::kCLITime, Constants::kCLITooFewArgs);
	}

	vector<string>::iterator iter = argv.begin();
	argv.erase(iter);

	return DoTime(pAgent, argv);
}

// ____      _____ _
//|  _ \  __|_   _(_)_ __ ___   ___
//| | | |/ _ \| | | | '_ ` _ \ / _ \
//| |_| | (_) | | | | | | | | |  __/
//|____/ \___/|_| |_|_| |_| |_|\___|
//
bool CommandLineInterface::DoTime(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {

#ifdef WIN32
	// Look at clock
	DWORD start = GetTickCount();

	// Execute command
	bool ret = DoCommandInternal(pAgent, argv);

	// Look at clock again, subtracting first value
	DWORD elapsed = GetTickCount() - start;

	// calculate elapsed in seconds
	float seconds = elapsed / 1000.0f;

	char buf[32];
	memset(buf, 0, 32);
	Double2String(seconds, buf, 31);

	if (m_RawOutput) {
		// Print time elapsed and return
		m_Result += "\n(";
		m_Result += buf;
		m_Result += "s) real";
	} else {
		AppendArgTagFast(sml_Names::kParamSeconds, sml_Names::kTypeDouble, buf);
	}
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
bool CommandLineInterface::ParseWatch(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
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
				return HandleSyntaxError(Constants::kCLIWatch, Constants::kCLIMissingOptionArg);
			case '?':
				return HandleSyntaxError(Constants::kCLIWatch, Constants::kCLIUnrecognizedOption);
			default:
				return HandleGetOptError((char)option);
		}

		// process argument
		if (!WatchArg(values, constant, GetOpt::optarg)) {
			return false;
		}
	}

	// Only one non-option argument allowed, watch level
	if ((unsigned)GetOpt::optind == argv.size() - 1) {

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
			//pKernelHack->SetSysparam(pAgent, TRACE_OPERAND2_REMOVALS_SYSPARAM, false);

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
		return HandleSyntaxError(Constants::kCLIWatch, Constants::kCLITooManyArgs);
	}

	return DoWatch(pAgent, options, values);
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
bool CommandLineInterface::DoWatch(gSKI::IAgent* pAgent, const unsigned int options, unsigned int values) {
	// Need agent pointer for function calls
	if (!RequireAgent(pAgent)) return false;
	if (!RequireKernel()) return false;

	// Attain the evil back door of doom, even though we aren't the TgD, because we'll probably need it
	gSKI::EvilBackDoor::ITgDWorkArounds* pKernelHack = m_pKernel->getWorkaroundObject();

	if (!options) {
		// Print watch settings.
		const long* pSysparams = pKernelHack->GetSysparams(pAgent);
		char buf[kMinBufferSize];
		char buf2[kMinBufferSize];

		if (m_RawOutput) {
			m_Result += "Current watch settings:\n  Decisions:  ";
			m_Result += pSysparams[TRACE_CONTEXT_DECISIONS_SYSPARAM] ? "on" : "off";
			m_Result += "\n  Phases:  ";
			m_Result += pSysparams[TRACE_PHASES_SYSPARAM] ? "on" : "off";
			m_Result += "\n  Production firings/retractions\n    default productions:  ";
			m_Result += pSysparams[TRACE_FIRINGS_OF_DEFAULT_PRODS_SYSPARAM] ? "on" : "off";
			m_Result += "\n    user productions:  ";
			m_Result += pSysparams[TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM] ? "on" : "off";
			m_Result += "\n    chunks:  ";
			m_Result += pSysparams[TRACE_FIRINGS_OF_CHUNKS_SYSPARAM] ? "on" : "off";
			m_Result += "\n    justifications:  ";
			m_Result += pSysparams[TRACE_FIRINGS_OF_JUSTIFICATIONS_SYSPARAM] ? "on" : "off";
			m_Result += "\n    WME detail level:  ";
			m_Result += Int2String(pSysparams[TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM], buf, kMinBufferSize);
			m_Result += "\n  Working memory changes:  ";
			m_Result += pSysparams[TRACE_WM_CHANGES_SYSPARAM] ? "on" : "off";
			m_Result += "\n  Preferences generated by firings/retractions:  ";
			m_Result += pSysparams[TRACE_FIRINGS_PREFERENCES_SYSPARAM] ? "on" : "off";
			m_Result += "\n  Learning:  ";
			m_Result += Int2String(pSysparams[TRACE_CHUNKS_SYSPARAM], buf, kMinBufferSize);
			m_Result += "\n  Backtracing:  ";
			m_Result += pSysparams[TRACE_BACKTRACING_SYSPARAM] ? "on" : "off";
			m_Result += "\n  Loading:  ";
			m_Result += pSysparams[TRACE_LOADING_SYSPARAM] ? "on" : "off";
			m_Result += "\n";
		} else {
			AppendArgTag(Int2String(TRACE_CONTEXT_DECISIONS_SYSPARAM, buf, kMinBufferSize), sml_Names::kTypeBoolean, 
				pSysparams[TRACE_CONTEXT_DECISIONS_SYSPARAM] ? sml_Names::kTrue : sml_Names::kFalse);

			AppendArgTag(Int2String(TRACE_PHASES_SYSPARAM, buf, kMinBufferSize), sml_Names::kTypeBoolean, 
				pSysparams[TRACE_PHASES_SYSPARAM] ? sml_Names::kTrue : sml_Names::kFalse);

			AppendArgTag(Int2String(TRACE_FIRINGS_OF_DEFAULT_PRODS_SYSPARAM, buf, kMinBufferSize), sml_Names::kTypeBoolean, 
				pSysparams[TRACE_FIRINGS_OF_DEFAULT_PRODS_SYSPARAM] ? sml_Names::kTrue : sml_Names::kFalse);

			AppendArgTag(Int2String(TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM, buf, kMinBufferSize), sml_Names::kTypeBoolean, 
				pSysparams[TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM] ? sml_Names::kTrue : sml_Names::kFalse);

			AppendArgTag(Int2String(TRACE_FIRINGS_OF_CHUNKS_SYSPARAM, buf, kMinBufferSize), sml_Names::kTypeBoolean, 
				pSysparams[TRACE_FIRINGS_OF_CHUNKS_SYSPARAM] ? sml_Names::kTrue : sml_Names::kFalse);

			AppendArgTag(Int2String(TRACE_FIRINGS_OF_JUSTIFICATIONS_SYSPARAM, buf, kMinBufferSize), sml_Names::kTypeBoolean, 
				pSysparams[TRACE_FIRINGS_OF_JUSTIFICATIONS_SYSPARAM] ? sml_Names::kTrue : sml_Names::kFalse);

			AppendArgTag(Int2String(TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM, buf, kMinBufferSize), sml_Names::kTypeInt, 
				Int2String(pSysparams[TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM], buf2, kMinBufferSize));

			AppendArgTag(Int2String(TRACE_WM_CHANGES_SYSPARAM, buf, kMinBufferSize), sml_Names::kTypeBoolean, 
				pSysparams[TRACE_WM_CHANGES_SYSPARAM] ? sml_Names::kTrue : sml_Names::kFalse);

			AppendArgTag(Int2String(TRACE_FIRINGS_PREFERENCES_SYSPARAM, buf, kMinBufferSize), sml_Names::kTypeBoolean, 
				pSysparams[TRACE_FIRINGS_PREFERENCES_SYSPARAM] ? sml_Names::kTrue : sml_Names::kFalse);

			AppendArgTag(Int2String(TRACE_CHUNKS_SYSPARAM, buf, kMinBufferSize), sml_Names::kTypeInt, 
				Int2String(pSysparams[TRACE_CHUNKS_SYSPARAM], buf2, kMinBufferSize));

			AppendArgTag(Int2String(TRACE_BACKTRACING_SYSPARAM, buf, kMinBufferSize), sml_Names::kTypeBoolean, 
				pSysparams[TRACE_BACKTRACING_SYSPARAM] ? sml_Names::kTrue : sml_Names::kFalse);

			AppendArgTag(Int2String(TRACE_LOADING_SYSPARAM, buf, kMinBufferSize), sml_Names::kTypeBoolean, 
				pSysparams[TRACE_LOADING_SYSPARAM] ? sml_Names::kTrue : sml_Names::kFalse);
		}

		return true;
	}

	// If option is watch none, set values all off
	if (options == OPTION_WATCH_NONE) {
		values = 0;
	}

	// Next, do we have a watch level? (none flag will set this to zero)
	// No watch level and no none flags, that means we have to do the rest
	if (options & OPTION_WATCH_BACKTRACING) {
		pKernelHack->SetSysparam(pAgent, TRACE_BACKTRACING_SYSPARAM, values & OPTION_WATCH_BACKTRACING);
	}

	if (options & OPTION_WATCH_CHUNKS) {
		pKernelHack->SetSysparam(pAgent, TRACE_FIRINGS_OF_CHUNKS_SYSPARAM, values & OPTION_WATCH_CHUNKS);
	}

	if (options & OPTION_WATCH_DECISIONS) {
		pKernelHack->SetSysparam(pAgent, TRACE_CONTEXT_DECISIONS_SYSPARAM, values & OPTION_WATCH_DECISIONS);
	}

	if (options & OPTION_WATCH_DEFAULT_PRODUCTIONS) {
		pKernelHack->SetSysparam(pAgent, TRACE_FIRINGS_OF_DEFAULT_PRODS_SYSPARAM, values & OPTION_WATCH_DEFAULT_PRODUCTIONS);
	}

	if (options & OPTION_WATCH_INDIFFERENT_SELECTION) {
		pKernelHack->SetSysparam(pAgent, TRACE_INDIFFERENT_SYSPARAM, values & OPTION_WATCH_INDIFFERENT_SELECTION);
	}

	if (options & OPTION_WATCH_JUSTIFICATIONS) {
		pKernelHack->SetSysparam(pAgent, TRACE_FIRINGS_OF_JUSTIFICATIONS_SYSPARAM, values & OPTION_WATCH_JUSTIFICATIONS);
	}

	if (options & OPTION_WATCH_LOADING) {
		pKernelHack->SetSysparam(pAgent, TRACE_LOADING_SYSPARAM, values & OPTION_WATCH_LOADING);
	}

	if (options & OPTION_WATCH_PHASES) {
		pKernelHack->SetSysparam(pAgent, TRACE_PHASES_SYSPARAM, values & OPTION_WATCH_PHASES);
	}

	if (options & OPTION_WATCH_PRODUCTIONS) {
		pKernelHack->SetSysparam(pAgent, TRACE_FIRINGS_OF_DEFAULT_PRODS_SYSPARAM, values & OPTION_WATCH_PRODUCTIONS);
		pKernelHack->SetSysparam(pAgent, TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM, values & OPTION_WATCH_PRODUCTIONS);
		pKernelHack->SetSysparam(pAgent, TRACE_FIRINGS_OF_CHUNKS_SYSPARAM, values & OPTION_WATCH_PRODUCTIONS);
		pKernelHack->SetSysparam(pAgent, TRACE_FIRINGS_OF_JUSTIFICATIONS_SYSPARAM, values & OPTION_WATCH_PRODUCTIONS);
	}

	if (options & OPTION_WATCH_PREFERENCES) {
		pKernelHack->SetSysparam(pAgent, TRACE_FIRINGS_PREFERENCES_SYSPARAM, values & OPTION_WATCH_PREFERENCES);
	}

	if (options & OPTION_WATCH_USER_PRODUCTIONS) {
		pKernelHack->SetSysparam(pAgent, TRACE_FIRINGS_OF_USER_PRODS_SYSPARAM, values & OPTION_WATCH_USER_PRODUCTIONS);
	}

	if (options & OPTION_WATCH_WMES) {
		pKernelHack->SetSysparam(pAgent, TRACE_WM_CHANGES_SYSPARAM, values & OPTION_WATCH_WMES);
	}

	if (options & OPTION_WATCH_LEARNING) {
		switch (values & OPTION_WATCH_LEARNING) {
			case 0:
			default:
				pKernelHack->SetSysparam(pAgent, TRACE_CHUNK_NAMES_SYSPARAM, false);
				pKernelHack->SetSysparam(pAgent, TRACE_CHUNKS_SYSPARAM, false);
				pKernelHack->SetSysparam(pAgent, TRACE_JUSTIFICATION_NAMES_SYSPARAM, false);
				pKernelHack->SetSysparam(pAgent, TRACE_JUSTIFICATIONS_SYSPARAM, false);
				break;
			case 1:
				pKernelHack->SetSysparam(pAgent, TRACE_CHUNK_NAMES_SYSPARAM, true);
				pKernelHack->SetSysparam(pAgent, TRACE_CHUNKS_SYSPARAM, false);
				pKernelHack->SetSysparam(pAgent, TRACE_JUSTIFICATION_NAMES_SYSPARAM, true);
				pKernelHack->SetSysparam(pAgent, TRACE_JUSTIFICATIONS_SYSPARAM, false);
				break;
			case 2:
				pKernelHack->SetSysparam(pAgent, TRACE_CHUNK_NAMES_SYSPARAM, true);
				pKernelHack->SetSysparam(pAgent, TRACE_CHUNKS_SYSPARAM, true);
				pKernelHack->SetSysparam(pAgent, TRACE_JUSTIFICATION_NAMES_SYSPARAM, true);
				pKernelHack->SetSysparam(pAgent, TRACE_JUSTIFICATIONS_SYSPARAM, true);
				break;
		}
	}

	if (options & OPTION_WATCH_WME_DETAIL) {
		switch ((values & OPTION_WATCH_WME_DETAIL) >> 2) {
			case 0:
			default:
				pKernelHack->SetSysparam(pAgent, TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM, NONE_WME_TRACE);
				break;
			case 1:
				pKernelHack->SetSysparam(pAgent, TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM, TIMETAG_WME_TRACE);
				break;
			case 2:
				pKernelHack->SetSysparam(pAgent, TRACE_FIRINGS_WME_TRACE_TYPE_SYSPARAM, FULL_WME_TRACE);
				break;
		}
	}

	return true;
}

