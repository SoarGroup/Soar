#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

/////////////////////////////////////////////////////////////////
// CommandLineInterface class
//
// Author: Jonathan Voigt
// Date  : Sept 2004
//
/////////////////////////////////////////////////////////////////

#include "cli_CommandLineInterface.h"

#ifdef WIN32
#include <direct.h>
#endif // WIN32

#include "cli_GetOpt.h"
#include "cli_Constants.h"
#include "cli_Aliases.h"

// BADBAD: I think we should be using an error class instead to work with error objects.
#include "../../gSKI/src/gSKI_Error.h"

// SML includes
#include "sml_Connection.h"
#include "sml_TagResult.h"
#include "sml_TagArg.h"

using namespace cli;
using namespace sml;

EXPORT CommandLineInterface::CommandLineInterface() {

	// Create getopt object
	m_pGetOpt = new GetOpt;

	// Map command names to processing function pointers
	BuildCommandMap();

	// Set up the current working directory, create usage and aliases
	m_pAliases = 0;
	m_pConstants = 0;
	if (!DoHome()) {
		// TODO: figure out what to do here!
		// ignore for now!
	}

	// Give print handlers a reference to us
	m_ResultPrintHandler.SetCLI(this);
	m_LogPrintHandler.SetCLI(this);

	// Initialize other members
	m_QuitCalled = false;
	m_pKernel = 0;
	m_SourceError = false;
	m_SourceDepth = 0;
	m_SourceDirDepth = 0;
	m_pLogFile = 0;
}

EXPORT CommandLineInterface::~CommandLineInterface() {
	if (m_pGetOpt) {
		delete m_pGetOpt;
	}
	if (m_pLogFile) {
		delete m_pLogFile;
	}
}

void CommandLineInterface::BuildCommandMap() {

	m_CommandMap[Constants::kCLICD]					= &cli::CommandLineInterface::ParseCD;
	m_CommandMap[Constants::kCLIEcho]				= &cli::CommandLineInterface::ParseEcho;
	m_CommandMap[Constants::kCLIExcise]				= &cli::CommandLineInterface::ParseExcise;
	m_CommandMap[Constants::kCLIHelp]				= &cli::CommandLineInterface::ParseHelp;
	m_CommandMap[Constants::kCLIHelpEx]				= &cli::CommandLineInterface::ParseHelpEx;
	m_CommandMap[Constants::kCLIHome]				= &cli::CommandLineInterface::ParseHome;
	m_CommandMap[Constants::kCLIInitSoar]			= &cli::CommandLineInterface::ParseInitSoar;
	m_CommandMap[Constants::kCLILearn]				= &cli::CommandLineInterface::ParseLearn;
	m_CommandMap[Constants::kCLILog]				= &cli::CommandLineInterface::ParseLog;
	m_CommandMap[Constants::kCLILS]					= &cli::CommandLineInterface::ParseLS;
	m_CommandMap[Constants::kCLIMultiAttributes]	= &cli::CommandLineInterface::ParseMultiAttributes;
	m_CommandMap[Constants::kCLIPopD]				= &cli::CommandLineInterface::ParsePopD;
	m_CommandMap[Constants::kCLIPrint]				= &cli::CommandLineInterface::ParsePrint;
	m_CommandMap[Constants::kCLIPushD]				= &cli::CommandLineInterface::ParsePushD;
	m_CommandMap[Constants::kCLIPWD]				= &cli::CommandLineInterface::ParsePWD;
	m_CommandMap[Constants::kCLIQuit]				= &cli::CommandLineInterface::ParseQuit;
	m_CommandMap[Constants::kCLIRun]				= &cli::CommandLineInterface::ParseRun;
	m_CommandMap[Constants::kCLISource]				= &cli::CommandLineInterface::ParseSource;
	m_CommandMap[Constants::kCLISP]					= &cli::CommandLineInterface::ParseSP;
	m_CommandMap[Constants::kCLIStats]				= &cli::CommandLineInterface::ParseStats;
	m_CommandMap[Constants::kCLIStopSoar]			= &cli::CommandLineInterface::ParseStopSoar;
	m_CommandMap[Constants::kCLITime]				= &cli::CommandLineInterface::ParseTime;
	m_CommandMap[Constants::kCLIWarnings]			= &cli::CommandLineInterface::ParseWarnings;
	m_CommandMap[Constants::kCLIWatch]				= &cli::CommandLineInterface::ParseWatch;
}

EXPORT bool CommandLineInterface::DoCommand(Connection* pConnection, gSKI::IAgent* pAgent, const char* pCommandLine, ElementXML* pResponse, bool rawOutput, gSKI::Error* pError) {

	// Clear the result
	m_Result.clear();
	m_ErrorMessage.clear();
	m_ResponseTags.clear();

	// Save the pointers
	m_pError = pError;

	// Save the raw output flag
	m_RawOutput = rawOutput;

	// Process the command, ignoring its result (irrelevant at this level)
	bool ret = DoCommandInternal(pAgent, pCommandLine);

	// Reset source error flag
	m_SourceError = false;

	if (ret) {
		// The command succeeded, so return the result if raw output
		if (m_RawOutput) {
			pConnection->AddSimpleResultToSMLResponse(pResponse, m_Result.c_str());
		} else {
			// If there are tags in the response list, add them and return
			if (m_ResponseTags.size()) {
				TagResult* pTag = new TagResult();

				ElementXMLListIter iter = m_ResponseTags.begin();
				while (iter != m_ResponseTags.end()) {
					pTag->AddChild(*iter);
					m_ResponseTags.erase(iter);
					iter = m_ResponseTags.begin();
				}

				pResponse->AddChild(pTag);

			} else {
				// Otherwise, return result as simple result if there is one
				if (m_Result.size()) {
					pConnection->AddSimpleResultToSMLResponse(pResponse, m_Result.c_str());
				} else {
					// Or, simply return true
					pConnection->AddSimpleResultToSMLResponse(pResponse, sml_Names::kTrue);
				}
			}
		}
	} else {
		// The command failed, add the error message
		pConnection->AddErrorToSMLResponse(pResponse, m_ErrorMessage.c_str());
	}

	// Always returns true to indicate that we've generated any needed error message already
	return true ;
}

EXPORT bool CommandLineInterface::DoCommand(gSKI::IAgent* pAgent, const char* pCommandLine, char const* pResponse, gSKI::Error* pError) {
	// This function is for processing a command without the SML layer
	// Clear the result
	m_Result.clear();
	m_ErrorMessage.clear();

	// Save the pointers
	m_pError = pError;

	// Process the command, ignoring its result (irrelevant at this level)
	bool ret = DoCommandInternal(pAgent, pCommandLine);

	// Reset source error flag
	m_SourceError = false;

	pResponse = m_Result.c_str();

	return ret;
}

bool CommandLineInterface::DoCommandInternal(gSKI::IAgent* pAgent, const std::string& commandLine) {

	vector<string> argv;

	// Parse command:
	if (Tokenize(commandLine, argv) == -1) {
		return false;	// Parsing failed
	}

	return DoCommandInternal(pAgent, argv);
}

bool CommandLineInterface::DoCommandInternal(gSKI::IAgent* pAgent, vector<string>& argv) {
	if (!argv.size()) {
		// Nothing on command line!
		return true;
	}

	// Translate aliases
	m_pAliases->Translate(argv);

	// Is the command implemented?
	if (m_CommandMap.find(argv[0]) == m_CommandMap.end()) {
		HandleError("Command '" + argv[0] + "' not found or implemented.");
		return false;
	}

	// Check for help flags
	if (CheckForHelp(argv)) {
		// Help flags found, add help to line, return true
		string output;
		if (!m_pConstants->IsUsageFileAvailable()) {
			AppendToResult(Constants::kCLINoUsageFile);
		} else if (m_pConstants->GetUsageFor(argv[0], output)) {
			AppendToResult(output);
		} else {
			AppendToResult(Constants::kCLINoUsageInfo);
		}
		return true;
	}

	// Process command
	CommandFunction pFunction = m_CommandMap[argv[0]];

	// Just in case...
	if (!pFunction) {
		// Very odd, should be set in BuildCommandMap
		HandleError("Command found but function pointer is null.");
		return false;
	}
	
	// Make the call
	return (this->*pFunction)(pAgent, argv);
}

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
						HandleError("Closing bracket found without opening counterpart.");
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
					HandleError("No closing quotes/brackets found.");
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

EXPORT void CommandLineInterface::SetKernel(gSKI::IKernel* pKernel) {
	m_pKernel = pKernel;
}

bool CommandLineInterface::GetCurrentWorkingDirectory(string& directory) {
	// Pull an arbitrary buffer size of 1024 out of a hat and use it
	char buf[1024];
	char* ret = getcwd(buf, 1024);

	// If getcwd returns 0, that is bad
	if (!ret) {
		HandleError("Couldn't get working directory.");
		return false;
	}

	// Store directory in output parameter and return success
	directory = buf;
	return true;
}

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

bool CommandLineInterface::HandleSyntaxError(const char* command, const char* details) {
	string msg;
	msg += Constants::kCLISyntaxError;
	msg += " (";
	msg += command;
	msg += ")\n";
	if (details) {
		msg += details;
		msg += '\n';
	}
	msg += "Type 'help ";
	msg += command;
	msg += "' or '";
	msg += command;
	msg += " --help' for syntax and usage.";
	HandleError(msg);
	return false;
}

bool CommandLineInterface::RequireAgent(gSKI::IAgent* pAgent) {
	if (!pAgent) {
		HandleError("An agent pointer is required for this command.");
		return false;
	}
	return true;
}

bool CommandLineInterface::RequireKernel() {
	if (!m_pKernel) {
		HandleError("A kernel pointer is required for this command.");
		return false;
	}
	return true;
}

bool CommandLineInterface::HandleGetOptError(char option) {
	string msg;
	msg += "Internal error: m_pGetOpt->GetOpt_Long returned '";
	msg += option;
	msg += "'!";
	HandleError(msg);
	return false;
}

bool CommandLineInterface::HandleError(std::string errorMessage, gSKI::Error* pError) {
	m_ErrorMessage += errorMessage;

	if (pError && isError(*pError)) {
		m_ErrorMessage += "\ngSKI error was: " ;
		m_ErrorMessage += pError->Text ;
		m_ErrorMessage += " details: " ;
		m_ErrorMessage += pError->ExtendedMsg ;
	}
	AppendToResult(m_ErrorMessage);

	// Always return false
	return false;
}

void CommandLineInterface::AppendArgTag(const char* pParam, const char* pType, const char* pValue) {
	TagArg* pTag = new TagArg();
	pTag->SetParam(pParam);
	pTag->SetType(pType);
	pTag->SetValue(pValue);
	m_ResponseTags.push_back(pTag);
}

void CommandLineInterface::AppendArgTagFast(const char* pParam, const char* pType, const char* pValue) {
	TagArg* pTag = new TagArg();
	pTag->SetParamFast(pParam);
	pTag->SetTypeFast(pType);
	pTag->SetValue(pValue);
	m_ResponseTags.push_back(pTag);
}

void CommandLineInterface::PrependArgTagFast(const char* pParam, const char* pType, const char* pValue) {
	TagArg* pTag = new TagArg();
	pTag->SetParamFast(pParam);
	pTag->SetTypeFast(pType);
	pTag->SetValue(pValue);
	m_ResponseTags.push_front(pTag);
}

