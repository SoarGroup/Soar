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
#include "IgSKI_Agent.h"

// SML includes
#include "sml_Connection.h"
#include "sml_TagResult.h"
#include "sml_TagArg.h"
#include "sml_StringOps.h"
#include "sml_KernelSML.h"

using namespace cli;
using namespace sml;

EXPORT CommandLineInterface::CommandLineInterface() {

	// Create getopt object
	m_pGetOpt = new GetOpt;

	// Map command names to processing function pointers
	BuildCommandMap();

	// Set up the current working directory, create usage and aliases
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
	m_KernelVersion.major = m_KernelVersion.minor = 0;
}

EXPORT CommandLineInterface::~CommandLineInterface() {
	if (m_pGetOpt) delete m_pGetOpt;
	if (m_pLogFile) {
		(*m_pLogFile) << "Log file closed due to shutdown." << std::endl;
		delete m_pLogFile;
	}
}

void CommandLineInterface::BuildCommandMap() {

	m_CommandMap[Constants::kCLIAddWME]					= &cli::CommandLineInterface::ParseAddWME;
	m_CommandMap[Constants::kCLIAlias]					= &cli::CommandLineInterface::ParseAlias;
	m_CommandMap[Constants::kCLICD]						= &cli::CommandLineInterface::ParseCD;
	m_CommandMap[Constants::kCLIDirs]					= &cli::CommandLineInterface::ParseDirs;
	m_CommandMap[Constants::kCLIEcho]					= &cli::CommandLineInterface::ParseEcho;
	m_CommandMap[Constants::kCLIExcise]					= &cli::CommandLineInterface::ParseExcise;
	m_CommandMap[Constants::kCLIFiringCounts]			= &cli::CommandLineInterface::ParseFiringCounts;
	m_CommandMap[Constants::kCLIGDSPrint]				= &cli::CommandLineInterface::ParseGDSPrint;
	m_CommandMap[Constants::kCLIHelp]					= &cli::CommandLineInterface::ParseHelp;
	m_CommandMap[Constants::kCLIHelpEx]					= &cli::CommandLineInterface::ParseHelpEx;
	m_CommandMap[Constants::kCLIHome]					= &cli::CommandLineInterface::ParseHome;
	m_CommandMap[Constants::kCLIIndifferentSelection]	= &cli::CommandLineInterface::ParseIndifferentSelection;
	m_CommandMap[Constants::kCLIInitSoar]				= &cli::CommandLineInterface::ParseInitSoar;
	m_CommandMap[Constants::kCLILearn]					= &cli::CommandLineInterface::ParseLearn;
	m_CommandMap[Constants::kCLILog]					= &cli::CommandLineInterface::ParseLog;
	m_CommandMap[Constants::kCLILS]						= &cli::CommandLineInterface::ParseLS;
	m_CommandMap[Constants::kCLIMatches]				= &cli::CommandLineInterface::ParseMatches;
	m_CommandMap[Constants::kCLIMaxChunks]				= &cli::CommandLineInterface::ParseMaxChunks;
	m_CommandMap[Constants::kCLIMaxElaborations]		= &cli::CommandLineInterface::ParseMaxElaborations;
	m_CommandMap[Constants::kCLIMaxNilOutputCycles]		= &cli::CommandLineInterface::ParseMaxNilOutputCycles;
	m_CommandMap[Constants::kCLIMemories]				= &cli::CommandLineInterface::ParseMemories;
	m_CommandMap[Constants::kCLIMultiAttributes]		= &cli::CommandLineInterface::ParseMultiAttributes;
	m_CommandMap[Constants::kCLINumericIndifferentMode]	= &cli::CommandLineInterface::ParseNumericIndifferentMode;
	m_CommandMap[Constants::kCLIOSupportMode]			= &cli::CommandLineInterface::ParseOSupportMode;
	m_CommandMap[Constants::kCLIPopD]					= &cli::CommandLineInterface::ParsePopD;
	m_CommandMap[Constants::kCLIPreferences]			= &cli::CommandLineInterface::ParsePreferences;
	m_CommandMap[Constants::kCLIPrint]					= &cli::CommandLineInterface::ParsePrint;
	m_CommandMap[Constants::kCLIProductionFind]			= &cli::CommandLineInterface::ParseProductionFind;
	m_CommandMap[Constants::kCLIPushD]					= &cli::CommandLineInterface::ParsePushD;
	m_CommandMap[Constants::kCLIPWatch]					= &cli::CommandLineInterface::ParsePWatch;
	m_CommandMap[Constants::kCLIPWD]					= &cli::CommandLineInterface::ParsePWD;
	m_CommandMap[Constants::kCLIQuit]					= &cli::CommandLineInterface::ParseQuit;
	m_CommandMap[Constants::kCLIRemoveWME]				= &cli::CommandLineInterface::ParseRemoveWME;
	m_CommandMap[Constants::kCLIReteNet]				= &cli::CommandLineInterface::ParseReteNet;
	m_CommandMap[Constants::kCLIRun]					= &cli::CommandLineInterface::ParseRun;
	m_CommandMap[Constants::kCLISoar8]					= &cli::CommandLineInterface::ParseSoar8;
	m_CommandMap[Constants::kCLISource]					= &cli::CommandLineInterface::ParseSource;
	m_CommandMap[Constants::kCLISP]						= &cli::CommandLineInterface::ParseSP;
	m_CommandMap[Constants::kCLIStats]					= &cli::CommandLineInterface::ParseStats;
	m_CommandMap[Constants::kCLIStopSoar]				= &cli::CommandLineInterface::ParseStopSoar;
	m_CommandMap[Constants::kCLITime]					= &cli::CommandLineInterface::ParseTime;
	m_CommandMap[Constants::kCLITimers]					= &cli::CommandLineInterface::ParseTimers;
	m_CommandMap[Constants::kCLIVerbose]				= &cli::CommandLineInterface::ParseVerbose;
	m_CommandMap[Constants::kCLIVersion]				= &cli::CommandLineInterface::ParseVersion;
	m_CommandMap[Constants::kCLIWaitSNC]				= &cli::CommandLineInterface::ParseWaitSNC;
	m_CommandMap[Constants::kCLIWarnings]				= &cli::CommandLineInterface::ParseWarnings;
	m_CommandMap[Constants::kCLIWatch]					= &cli::CommandLineInterface::ParseWatch;
}	

EXPORT bool CommandLineInterface::DoCommand(Connection* pConnection, gSKI::IAgent* pAgent, const char* pCommandLine, ElementXML* pResponse, bool rawOutput, gSKI::Error* pError) {

	// Clear the result
	m_Result.clear();
	m_ResponseTags.clear();
	m_Error.SetError(CLIError::kNoError);

	// Fail if quit has been called
	if (m_QuitCalled) return false;

	// Save the pointers
	m_pgSKIError = pError;

	// Save the raw output flag
	m_RawOutput = rawOutput;

	// Log input
	if (m_pLogFile) {
		if (pAgent) (*m_pLogFile) << pAgent->GetName() << "> ";
		(*m_pLogFile) << pCommandLine << endl;
	}

	// Process the command, ignoring its result (irrelevant at this level)
	bool ret = DoCommandInternal(pAgent, pCommandLine);

	// Log output
	if (m_pLogFile) (*m_pLogFile) << m_Result << endl;

	// Handle source error output
	if (m_SourceError) {
		m_Error.SetError(m_SourceErrorDetail);
		m_SourceError = false;
	}

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
		string errorDescription = m_Error.GetErrorDescription();
		if (m_pgSKIError && (m_pgSKIError->Id != gSKI::gSKIERR_NONE)) {
			errorDescription += "\ngSKI Error code: ";
			char buf[kMinBufferSize];
			Int2String((int)m_pgSKIError->Id, buf, kMinBufferSize);
			errorDescription += buf;
			errorDescription += "\ngSKI Error text: ";
			errorDescription += m_pgSKIError->Text;
			errorDescription += "\ngSKI Error details: ";
			errorDescription += m_pgSKIError->ExtendedMsg;
		}
		pConnection->AddErrorToSMLResponse(pResponse, errorDescription.c_str(), m_Error.GetErrorCode());
	}

	// Always returns true to indicate that we've generated any needed error message already
	return true ;
}

EXPORT bool CommandLineInterface::DoCommand(gSKI::IAgent* pAgent, const char* pCommandLine, char const* pResponse, gSKI::Error* pError) {
	// This function is for processing a command without the SML layer
	// Clear the result
	m_Result.clear();
	m_Error.SetError(CLIError::kNoError);

	// Save the pointers
	m_pgSKIError = pError;

	// Process the command, ignoring its result (irrelevant at this level)
	bool ret = DoCommandInternal(pAgent, pCommandLine);

	// Reset source error flag
	m_SourceError = false;

	if (ret) {
		pResponse = m_Result.c_str();
	} else {
		pResponse = m_Error.GetErrorDescription();
	}

	return ret;
}

bool CommandLineInterface::DoCommandInternal(gSKI::IAgent* pAgent, const std::string& commandLine) {
	vector<string> argv;
	// Parse command:
	if (Tokenize(commandLine, argv) == -1)  return false;	// Parsing failed
	return DoCommandInternal(pAgent, argv);
}

bool CommandLineInterface::DoCommandInternal(gSKI::IAgent* pAgent, vector<string>& argv) {
	if (!argv.size()) return true;

	// Translate aliases
	m_Aliases.Translate(argv);

	// Is the command implemented?
	if (m_CommandMap.find(argv[0]) == m_CommandMap.end()) return m_Error.SetError(CLIError::kCommandNotFound);

	// Check for help flags
	if (CheckForHelp(argv)) {
		// Help flags found, add help to line, return true
		string output;
		if (!m_pConstants->IsUsageFileAvailable()) return m_Error.SetError(CLIError::kNoUsageFile);
		if (!m_pConstants->GetUsageFor(argv[0], output)) return m_Error.SetError(CLIError::kNoUsageInfo);
		AppendToResult(output);
		return true;
	}

	// Process command
	CommandFunction pFunction = m_CommandMap[argv[0]];

	// Just in case...
	if (!pFunction) return m_Error.SetError(CLIError::kNoCommandPointer); // Very odd, should be set in BuildCommandMap
	
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
		if(cmdline.empty()) break;

		// Remove leading whitespace
		iter = cmdline.begin();
		while (isspace(*iter)) {
			cmdline.erase(iter);

			if (!cmdline.length()) break; //Nothing but space left
			
			// Next character
			iter = cmdline.begin();
		}

		// Was it actually trailing whitespace?
		if (!cmdline.length()) break;// Nothing left to do

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
						m_Error.SetError(CLIError::kExtraClosingParen);
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
					m_Error.SetError(CLIError::kUnmatchedBracketOrQuote);
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

EXPORT void CommandLineInterface::SetKernel(gSKI::IKernel* pKernel, gSKI::Version kernelVersion, sml::KernelSML* pKernelSML) {
	m_pKernel = pKernel;
	m_KernelVersion = kernelVersion;
	m_pKernelSML = pKernelSML;
}

bool CommandLineInterface::GetCurrentWorkingDirectory(string& directory) {
	// Pull an arbitrary buffer size of 1024 out of a hat and use it
	char buf[1024];
	char* ret = getcwd(buf, 1024);

	// If getcwd returns 0, that is bad
	if (!ret) return m_Error.SetError(CLIError::kgetcwdFail);

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

bool CommandLineInterface::RequireAgent(gSKI::IAgent* pAgent) {
	// Requiring an agent implies requiring a kernel
	if (!RequireKernel()) return false;
	if (!pAgent) return m_Error.SetError(CLIError::kAgentRequired);
	return true;
}

bool CommandLineInterface::RequireKernel() {
	if (!m_pKernel) return m_Error.SetError(CLIError::kKernelRequired);
	return true;
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

void CommandLineInterface::AddListenerAndDisableCallbacks(gSKI::IAgent* pAgent) {
	if (m_pKernelSML) m_pKernelSML->DisablePrintCallback(pAgent);
	if (pAgent) pAgent->AddPrintListener(gSKIEVENT_PRINT, &m_ResultPrintHandler);
}

void CommandLineInterface::RemoveListenerAndEnableCallbacks(gSKI::IAgent* pAgent) {
	if (pAgent) pAgent->RemovePrintListener(gSKIEVENT_PRINT, &m_ResultPrintHandler);
	if (m_pKernelSML) m_pKernelSML->EnablePrintCallback(pAgent);
}
