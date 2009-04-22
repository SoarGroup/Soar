/////////////////////////////////////////////////////////////////
// CommandLineInterface class file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "sml_Utils.h"
#include "cli_CommandLineInterface.h"

#include <assert.h>

#include <iostream>
#include <fstream>

#include "cli_Commands.h"
#include "cli_CLIError.h"

// SML includes
#include "sml_Connection.h"
#include "sml_TagResult.h"
#include "sml_TagArg.h"
#include "sml_StringOps.h"
#include "sml_KernelSML.h"
#include "sml_AgentSML.h"
#include "XMLTrace.h"
#include "sml_KernelHelpers.h"
#include "KernelHeaders.h"

#include "agent.h"

using namespace cli;
using namespace sml;
using namespace soarxml;

std::ostringstream CommandLineInterface::m_Result;	

EXPORT CommandLineInterface::CommandLineInterface() {

	// Map command names to processing function pointers
	m_CommandMap[Commands::kCLIAddWME]						= &cli::CommandLineInterface::ParseAddWME;
	m_CommandMap[Commands::kCLIAlias]						= &cli::CommandLineInterface::ParseAlias;
	m_CommandMap[Commands::kCLICaptureInput]				= &cli::CommandLineInterface::ParseCaptureInput;
	m_CommandMap[Commands::kCLICD]							= &cli::CommandLineInterface::ParseCD;
	m_CommandMap[Commands::kCLIChunkNameFormat]				= &cli::CommandLineInterface::ParseChunkNameFormat;
	m_CommandMap[Commands::kCLICLog]						= &cli::CommandLineInterface::ParseCLog;
	m_CommandMap[Commands::kCLICommandToFile]				= &cli::CommandLineInterface::ParseCommandToFile;
	m_CommandMap[Commands::kCLIDefaultWMEDepth]				= &cli::CommandLineInterface::ParseDefaultWMEDepth;
	m_CommandMap[Commands::kCLIDirs]						= &cli::CommandLineInterface::ParseDirs;
	m_CommandMap[Commands::kCLIEcho]						= &cli::CommandLineInterface::ParseEcho;
	m_CommandMap[Commands::kCLIEchoCommands]				= &cli::CommandLineInterface::ParseEchoCommands;
	m_CommandMap[Commands::kCLIEditProduction]				= &cli::CommandLineInterface::ParseEditProduction;
	m_CommandMap[Commands::kCLIEpMem]						= &cli::CommandLineInterface::ParseEpMem;
	m_CommandMap[Commands::kCLIExcise]						= &cli::CommandLineInterface::ParseExcise;
	m_CommandMap[Commands::kCLIExplainBacktraces]			= &cli::CommandLineInterface::ParseExplainBacktraces;
	m_CommandMap[Commands::kCLIFiringCounts]				= &cli::CommandLineInterface::ParseFiringCounts;
	m_CommandMap[Commands::kCLIGDSPrint]					= &cli::CommandLineInterface::ParseGDSPrint;
	m_CommandMap[Commands::kCLIGP]							= &cli::CommandLineInterface::ParseGP;
	m_CommandMap[Commands::kCLIHelp]						= &cli::CommandLineInterface::ParseHelp;
	m_CommandMap[Commands::kCLIIndifferentSelection]		= &cli::CommandLineInterface::ParseIndifferentSelection;
	m_CommandMap[Commands::kCLIInitSoar]					= &cli::CommandLineInterface::ParseInitSoar;
	m_CommandMap[Commands::kCLIInternalSymbols]				= &cli::CommandLineInterface::ParseInternalSymbols;
	m_CommandMap[Commands::kCLILearn]						= &cli::CommandLineInterface::ParseLearn;
	m_CommandMap[Commands::kCLILoadLibrary]					= &cli::CommandLineInterface::ParseLoadLibrary;
	m_CommandMap[Commands::kCLILS]							= &cli::CommandLineInterface::ParseLS;
	m_CommandMap[Commands::kCLIMatches]						= &cli::CommandLineInterface::ParseMatches;
	m_CommandMap[Commands::kCLIMaxChunks]					= &cli::CommandLineInterface::ParseMaxChunks;
	m_CommandMap[Commands::kCLIMaxElaborations]				= &cli::CommandLineInterface::ParseMaxElaborations;
	m_CommandMap[Commands::kCLIMaxMemoryUsage]				= &cli::CommandLineInterface::ParseMaxMemoryUsage;
	m_CommandMap[Commands::kCLIMaxNilOutputCycles]			= &cli::CommandLineInterface::ParseMaxNilOutputCycles;
	m_CommandMap[Commands::kCLIMemories]					= &cli::CommandLineInterface::ParseMemories;
	m_CommandMap[Commands::kCLIMultiAttributes]				= &cli::CommandLineInterface::ParseMultiAttributes;
	m_CommandMap[Commands::kCLINumericIndifferentMode]		= &cli::CommandLineInterface::ParseNumericIndifferentMode;
	m_CommandMap[Commands::kCLIOSupportMode]				= &cli::CommandLineInterface::ParseOSupportMode;
	m_CommandMap[Commands::kCLIPopD]						= &cli::CommandLineInterface::ParsePopD;
	m_CommandMap[Commands::kCLIPredict]						= &cli::CommandLineInterface::ParsePredict;
	m_CommandMap[Commands::kCLIPreferences]					= &cli::CommandLineInterface::ParsePreferences;
	m_CommandMap[Commands::kCLIPrint]						= &cli::CommandLineInterface::ParsePrint;
	m_CommandMap[Commands::kCLIProductionFind]				= &cli::CommandLineInterface::ParseProductionFind;
	m_CommandMap[Commands::kCLIPushD]						= &cli::CommandLineInterface::ParsePushD;
	m_CommandMap[Commands::kCLIPWatch]						= &cli::CommandLineInterface::ParsePWatch;
	m_CommandMap[Commands::kCLIPWD]							= &cli::CommandLineInterface::ParsePWD;
	m_CommandMap[Commands::kCLIQuit]						= &cli::CommandLineInterface::ParseQuit;
	m_CommandMap[Commands::kCLIRand]						= &cli::CommandLineInterface::ParseRand;
	m_CommandMap[Commands::kCLIRemoveWME]					= &cli::CommandLineInterface::ParseRemoveWME;
	m_CommandMap[Commands::kCLIReplayInput]					= &cli::CommandLineInterface::ParseReplayInput;
	m_CommandMap[Commands::kCLIReteNet]						= &cli::CommandLineInterface::ParseReteNet;
	m_CommandMap[Commands::kCLIRL]							= &cli::CommandLineInterface::ParseRL;
	m_CommandMap[Commands::kCLIRun]							= &cli::CommandLineInterface::ParseRun;
	m_CommandMap[Commands::kCLISaveBacktraces]				= &cli::CommandLineInterface::ParseSaveBacktraces;
	m_CommandMap[Commands::kCLISelect]						= &cli::CommandLineInterface::ParseSelect;
	m_CommandMap[Commands::kCLISetLibraryLocation]			= &cli::CommandLineInterface::ParseSetLibraryLocation;
	m_CommandMap[Commands::kCLISoarNews]					= &cli::CommandLineInterface::ParseSoarNews;
	m_CommandMap[Commands::kCLISource]						= &cli::CommandLineInterface::ParseSource;
	m_CommandMap[Commands::kCLISP]							= &cli::CommandLineInterface::ParseSP;
	m_CommandMap[Commands::kCLISRand]						= &cli::CommandLineInterface::ParseSRand;
	m_CommandMap[Commands::kCLIStats]						= &cli::CommandLineInterface::ParseStats;
	m_CommandMap[Commands::kCLISetStopPhase]				= &cli::CommandLineInterface::ParseSetStopPhase;
	m_CommandMap[Commands::kCLIStopSoar]					= &cli::CommandLineInterface::ParseStopSoar;
	m_CommandMap[Commands::kCLITime]						= &cli::CommandLineInterface::ParseTime;
	m_CommandMap[Commands::kCLITimers]						= &cli::CommandLineInterface::ParseTimers;
	m_CommandMap[Commands::kCLIUnalias]						= &cli::CommandLineInterface::ParseUnalias;
	m_CommandMap[Commands::kCLIVerbose]						= &cli::CommandLineInterface::ParseVerbose;
	m_CommandMap[Commands::kCLIVersion]						= &cli::CommandLineInterface::ParseVersion;
	m_CommandMap[Commands::kCLIWaitSNC]						= &cli::CommandLineInterface::ParseWaitSNC;
	m_CommandMap[Commands::kCLIWarnings]					= &cli::CommandLineInterface::ParseWarnings;
	m_CommandMap[Commands::kCLIWatch]						= &cli::CommandLineInterface::ParseWatch;
	m_CommandMap[Commands::kCLIWatchWMEs]					= &cli::CommandLineInterface::ParseWatchWMEs;
	m_CommandMap[Commands::kCLIWMA]							= &cli::CommandLineInterface::ParseWMA;

	// Indicate which commands should be echoed so that all users can see them when doing a shared debugging session
	// FIXME: missing stuff like GDSPRINT?
	m_EchoMap[Commands::kCLIAddWME]						= true ;
	m_EchoMap[Commands::kCLIAlias]						= true ;
	m_EchoMap[Commands::kCLICaptureInput]				= true ;
	m_EchoMap[Commands::kCLICD]							= true ;
	m_EchoMap[Commands::kCLIChunkNameFormat]			= true ;
	m_EchoMap[Commands::kCLICLog]						= true ;
	m_EchoMap[Commands::kCLICommandToFile]				= true ;
	m_EchoMap[Commands::kCLIDefaultWMEDepth]			= true ;
	m_EchoMap[Commands::kCLIEcho]						= true ;
	m_EchoMap[Commands::kCLIEchoCommands]				= true ;
	m_EchoMap[Commands::kCLIEpMem]						= true ;
	m_EchoMap[Commands::kCLIExcise]						= true ;
	m_EchoMap[Commands::kCLIGP]							= true ;
	m_EchoMap[Commands::kCLIIndifferentSelection]		= true ;
	m_EchoMap[Commands::kCLIInitSoar]					= true ;
	m_EchoMap[Commands::kCLILearn]						= true ;
	m_EchoMap[Commands::kCLILoadLibrary]				= true ; // TODO: figure out if we actually want to echo this
	m_EchoMap[Commands::kCLIMaxChunks]					= true ;
	m_EchoMap[Commands::kCLIMaxElaborations]			= true ;
	m_EchoMap[Commands::kCLIMaxNilOutputCycles]			= true ;
	m_EchoMap[Commands::kCLIMultiAttributes]			= true ;
	m_EchoMap[Commands::kCLINumericIndifferentMode]		= true ;
	m_EchoMap[Commands::kCLIOSupportMode]				= true ;
	m_EchoMap[Commands::kCLIPopD]						= true ;
	m_EchoMap[Commands::kCLIPreferences]				= true ;
	m_EchoMap[Commands::kCLIPushD]						= true ;
	m_EchoMap[Commands::kCLIQuit]						= true ;
	m_EchoMap[Commands::kCLIRand]						= true ;
	m_EchoMap[Commands::kCLIRemoveWME]					= true ;
	m_EchoMap[Commands::kCLIReplayInput]				= true ;
	m_EchoMap[Commands::kCLIReteNet]					= true ;
	m_EchoMap[Commands::kCLIRL]							= true ;
	m_EchoMap[Commands::kCLIRun]						= true ;
	m_EchoMap[Commands::kCLISelect]						= true ;
	m_EchoMap[Commands::kCLISetLibraryLocation]			= true ;
	m_EchoMap[Commands::kCLISource]						= true ;
	m_EchoMap[Commands::kCLISP]							= true ;
	m_EchoMap[Commands::kCLISRand]						= true ;
	m_EchoMap[Commands::kCLISetStopPhase]				= true ;
	m_EchoMap[Commands::kCLIStopSoar]					= true ;
	m_EchoMap[Commands::kCLITimers]						= true ;
	m_EchoMap[Commands::kCLIVerbose]					= true ;
	m_EchoMap[Commands::kCLIWaitSNC]					= true ;
	m_EchoMap[Commands::kCLIWatch]						= true ;
	m_EchoMap[Commands::kCLIWatchWMEs]					= true ;

	// Initialize other members
	m_SourceError = false;
	m_SourceDepth = 0;
	m_SourceDirDepth = 0;
	m_pLogFile = 0;
	m_LastError = CLIError::kNoError;
	m_Initialized = true;
	m_TrapPrintEvents = false;
	m_EchoResult = false ;
	m_pAgentSML = 0 ;
	m_pAgentSoar = 0;
	m_VarPrint = false;

	m_XMLResult = new XMLTrace() ;
}

EXPORT CommandLineInterface::~CommandLineInterface() {
	if (m_pLogFile) {
		(*m_pLogFile) << "Log file closed due to shutdown." << std::endl;
		delete m_pLogFile;
	}

	delete m_XMLResult ;
	m_XMLResult = NULL ;
}

/*************************************************************
* @brief Returns true if the given command should always be echoed (to any listeners)
*        The current implementation doesn't support aliases or short forms of the commands.
* @param pCommandLine	The command line being tested
*************************************************************/
EXPORT bool CommandLineInterface::ShouldEchoCommand(char const* pCommandLine)
{
	if (!pCommandLine)
		return false ;

	std::string command = pCommandLine ;

	char const* pSpace = strchr(pCommandLine, ' ') ;
	if (pSpace)
	{
		// Trim everything from space on
		command.erase(pSpace-pCommandLine, command.length()) ;
	}

	// See if there's an entry in the echo map for this command
	// BADBAD: This won't work for short forms of the command or aliases; but making this test
	// happen later in the command line processing causes too many re-entrancy problem within the command line module.
	return (m_EchoMap.find(command) != m_EchoMap.end()) ;
}

/*************************************************************
* @brief Process a command.  Give it a command line and it will parse
*		 and execute the command using system calls.
* @param pConnection The connection, for communication to the client
* @param pAgent The pointer to the agent interface
* @param pCommandLine The command line string, arguments separated by spaces
* @param echoResults If true send a copy of the result to the echo event
* @param pResponse Pointer to XML response object
*************************************************************/
EXPORT bool CommandLineInterface::DoCommand(Connection* pConnection, sml::AgentSML* pAgent, const char* pCommandLine, bool echoResults, bool rawOutput, ElementXML* pResponse) {
	if (!m_pKernelSML) return false;

	// No way to return data
	if (!pConnection) return false;
	if (!pResponse) return false;
	PushCall( CallData(pAgent, rawOutput) );

	// Log input
	if (m_pLogFile) {
		if (pAgent) (*m_pLogFile) << pAgent->GetName() << "> ";
		(*m_pLogFile) << pCommandLine << std::endl;
	}

	m_EchoResult = echoResults ;

	SetTrapPrintCallbacks( true );

	m_SourceDepth = 0;
	m_SourceMode = SOURCE_DEFAULT;
	m_SourceVerbose = false; 

	// Process the command, ignoring its result (errors detected with m_LastError)
	//DoCommandInternal(pCommandLine);
	std::stringstream soarStream;
	soarStream << pCommandLine;
	StreamSource( soarStream, 0 );

	SetTrapPrintCallbacks( false );

	GetLastResultSML(pConnection, pResponse);

	PopCall();

	// Always returns true to indicate that we've generated any needed error message already
	return true;
}

void CommandLineInterface::PushCall( CallData callData )
{
	m_pCallDataStack.push( callData );

	if (callData.pAgent) 
	{
		m_pAgentSML = callData.pAgent;
		m_pAgentSoar = m_pAgentSML->GetSoarAgent();
		assert( m_pAgentSoar );
	} else {
		m_pAgentSML = 0;
		m_pAgentSoar = 0;
	}

	m_RawOutput = callData.rawOutput;

	// For kernel callback class we inherit
	SetAgentSML(m_pAgentSML) ;
}

void CommandLineInterface::PopCall()
{
	m_pCallDataStack.pop();
	sml::AgentSML* pAgent = 0;
	
	if ( m_pCallDataStack.size() )
	{
		const CallData& callData = m_pCallDataStack.top();
		pAgent = callData.pAgent;
		m_RawOutput = callData.rawOutput;
	}

	// reset these for the next command
	SetAgentSML( pAgent ) ;

	m_pAgentSML = pAgent;
	if (pAgent) 
	{
		m_pAgentSoar = pAgent->GetSoarAgent();
		assert( m_pAgentSoar );
	}
	else 
	{
		m_pAgentSoar = 0;
	}
}

void CommandLineInterface::SetTrapPrintCallbacks(bool setting)
{
	if (!m_pAgentSML)
	{
		return;
	}

	// If we've already set it, don't re-set it
	if ( m_TrapPrintEvents == setting )
	{
		return;
	}

	if (setting)
	{
		// Trap print callbacks
		m_pAgentSML->DisablePrintCallback();
		m_TrapPrintEvents = true;
		if (!m_pLogFile) 
		{
			// If we're logging, we're already registered for this.
			RegisterWithKernel(smlEVENT_PRINT);
		}

		// Tell kernel to collect result in command buffer as opposed to trace buffer
		xml_begin_command_mode( m_pAgentSML->GetSoarAgent() );
	}
	else
	{
		// Retrieve command buffer, tell kernel to use trace buffer again
		ElementXML* pXMLCommandResult = xml_end_command_mode( m_pAgentSML->GetSoarAgent() );

		// The root object is just a <trace> tag.  The substance is in the children
		// Add childrend of the command buffer to response tags
		for ( int i = 0; i < pXMLCommandResult->GetNumberChildren(); ++i )
		{
			ElementXML* pChildXML = new ElementXML();
			pXMLCommandResult->GetChild( pChildXML, i );

			m_ResponseTags.push_back( pChildXML );
		}

		delete pXMLCommandResult;

		if ( !m_RawOutput )
		{
			// Add text result to response tags
			if ( m_Result.str().length() )
			{
				AppendArgTagFast( sml_Names::kParamMessage, sml_Names::kTypeString, m_Result.str().c_str() );
				m_Result.str("");
			}
		}

		// Re-enable print callbacks
		if (!m_pLogFile) 
		{
			// If we're logging, we want to stay registered for this
			UnregisterWithKernel(smlEVENT_PRINT);
		}
		m_TrapPrintEvents = false;
		m_pAgentSML->EnablePrintCallback();
	}
}

void CommandLineInterface::GetLastResultSML(sml::Connection* pConnection, soarxml::ElementXML* pResponse) {
	assert(pConnection);
	assert(pResponse);

	// Handle source error output
	if (m_SourceError) {
		//SetError(CLIError::kSourceError);
		SetErrorDetail(m_SourceErrorDetail);
		m_SourceError = false;
	}

	if (m_LastError == CLIError::kNoError) {
        // Log output
        if (m_pLogFile) (*m_pLogFile) << m_Result.str() << std::endl;

        // The command succeeded, so return the result if raw output
		if (m_RawOutput) {
			pConnection->AddSimpleResultToSMLResponse(pResponse, m_Result.str().c_str());
			EchoString(pConnection, m_Result.str().c_str()) ;
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
				// Or, simply return true
				pConnection->AddSimpleResultToSMLResponse(pResponse, sml_Names::kTrue);
			}
		}
	} else {
		// The command failed, add the error message
		std::string errorDescription = GenerateErrorString();

		pConnection->AddErrorToSMLResponse(pResponse, errorDescription.c_str(), m_LastError);
		EchoString(pConnection, errorDescription.c_str()) ;

        // Log error
        if (m_pLogFile) (*m_pLogFile) << errorDescription << std::endl;
	}

	// reset state
	m_Result.str("");

	// Delete all remaining xml objects
	for ( ElementXMLListIter cleanupIter = m_ResponseTags.begin(); cleanupIter != m_ResponseTags.end(); ++cleanupIter )
	{
		delete *cleanupIter;
	}
	m_ResponseTags.clear();	

	m_LastError = CLIError::kNoError;	
	m_LastErrorDetail.clear();			
}

std::string CommandLineInterface::GenerateErrorString()
{
	std::string errorDescription = CLIError::GetErrorDescription(m_LastError);
	if (m_LastErrorDetail.size()) {
		errorDescription += "\nError detail: ";
		errorDescription += m_LastErrorDetail;
	}
	if (m_Result.str().size()) {
		errorDescription += "\nResult before error happened:\n";
		errorDescription += m_Result.str();
	}
	return errorDescription;
}

/************************************************************* 	 
* @brief Echo the given std::string through the smlEVENT_ECHO event
*		 if the call requested that commands be echoed.
*************************************************************/ 	 
void CommandLineInterface::EchoString(sml::Connection* pConnection, char const* pString)
{
	if (!m_EchoResult)
		return ;

	// BUGBUG: We may need to support this for kernel level commands without an agent
	if (m_pAgentSML)
		m_pAgentSML->FireEchoEvent(pConnection, pString) ;
}

/*************************************************************
* @brief Takes a command line and expands any aliases and returns
*		 the result.  The command is NOT executed.
* @param pCommandLine The command line string, arguments separated by spaces
* @param pExpandedLine The return value -- the expanded version of the command
*************************************************************/
bool CommandLineInterface::ExpandCommandToString(const char* pCommandLine, std::string* pExpandedLine)
{
	SetError(CLIError::kNoError);

	std::vector<std::string> argv;

	// 1) Parse command
	if (CLITokenize(pCommandLine, argv) == -1)
		return false ;

	if (!argv.empty())
	{
		// 2) Translate aliases, irrelevant return value ignored
		m_Aliases.Translate(argv);

		// 3) Partial match, irrelevant return value ignored
		PartialMatch(argv);

		// 4) Reassemble the command line
		for (unsigned int i = 0 ; i < argv.size() ; i++)
		{
			*pExpandedLine += argv[i] ;
			if (i != argv.size()-1) *pExpandedLine += " " ;
		}
	}

	return true ;
}

/*************************************************************
* @brief Takes a command line and expands any aliases and returns
*		 the result.  The command is NOT executed.
* @param pConnection The connection, for communication to the client
* @param pCommandLine The command line string, arguments separated by spaces
* @param pResponse Pointer to XML response object
*************************************************************/
EXPORT bool CommandLineInterface::ExpandCommand(sml::Connection* pConnection, const char* pCommandLine, soarxml::ElementXML* pResponse)
{
	std::string result ;

	bool ok = ExpandCommandToString(pCommandLine, &result) ;

	if (ok)
		pConnection->AddSimpleResultToSMLResponse(pResponse, result.c_str());

	return ok ;
}

bool CommandLineInterface::DoCommandInternal(const std::string& commandLine) {
	std::vector<std::string> argv;
	// Parse command:
	if (CLITokenize(commandLine, argv) == -1)  return false;	// Parsing failed

	// Execute the command
	return DoCommandInternal(argv);
}

bool CommandLineInterface::PartialMatch(std::vector<std::string>& argv) {
	// Not an alias, check for partial match
	std::list<std::string> possibilities;
	std::list<std::string>::iterator liter;
	bool exactMatch = false;

	for(unsigned index = 0; index < (argv[0].size()); ++index) {

		if (index == 0) {
			// Bootstrap the list of possibilities
			CommandMapConstIter citer = m_CommandMap.begin();

			while (citer != m_CommandMap.end()) {
				if (citer->first[index] == argv[0][index]) {
					possibilities.push_back(citer->first);
				}
				++citer;
			}

		} else {
			// Update the list of possiblities

			// A more efficient search here would be nice.
			liter = possibilities.begin();
			while (liter != possibilities.end()) {
				if ((*liter)[index] != argv[0][index]) {
					// Remove this possibility from the list
					liter = possibilities.erase(liter);
				} else {
					// check for exact match
					if (argv[0] == (*liter)) {
						// Exact match, we're done
						argv[0] = (*liter);
						exactMatch = true;
						break;
					}
					++liter;
				}
			}
			if (exactMatch) break;
		}

		if (!possibilities.size()) {
			// Not implemented
			SetErrorDetail("(No such command: " + argv[0] + ")");
			return SetError(CLIError::kCommandNotImplemented);

		} 
	}

	if (!exactMatch) {
		if (possibilities.size() != 1) {
			// Ambiguous
			std::stringstream detail;
			detail << "Ambiguous command, possibilities: ";
			liter = possibilities.begin();
			while (liter != possibilities.end()) {
				detail << "'" << (*liter) << "' ";
				++liter;
			}
			SetErrorDetail(detail.str());
			return SetError(CLIError::kAmbiguousCommand);

		} else {
			// We have a partial match
			argv[0] = (*(possibilities.begin()));
		}
		return true;
	}
	return true;
}

bool CommandLineInterface::DoCommandInternal(std::vector<std::string>& argv) {
	if (!argv.size()) return true;

	// Check for help flags
	bool helpFlag = false;
	if (CheckForHelp(argv)) {
		helpFlag = true;
	}

	// Expand aliases
	if (m_Aliases.Translate(argv)) {
		// Is the alias target implemented?
		if (m_CommandMap.find(argv[0]) == m_CommandMap.end()) {
			SetErrorDetail("(No such command: " + argv[0] + ")");
			return SetError(CLIError::kCommandNotImplemented);
		}

	} else {
		if (!PartialMatch(argv)) 
		{
			// error set inside PartialMatch
			return false;
		}
	}

	// Show help if requested
	if (helpFlag) {
		std::string helpFile = m_LibraryDirectory + "/CLIHelp/" + argv[0];
		return GetHelpString(helpFile);
	}

	// Process command
	CommandFunction pFunction = m_CommandMap[argv[0]];
	assert(pFunction);
	
	// Initialize option parsing each call
	ResetOptions();

	// Make the Parse call
	return (this->*pFunction)(argv);
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

EXPORT void CommandLineInterface::SetKernel(sml::KernelSML* pKernelSML) {
	m_pKernelSML = pKernelSML;

	// Now that we have the kernel, set the home directory to the location of SoarKernelSML's parent directory,
	// SoarLibrary
#ifdef WIN32
	char dllpath[256];
	GetModuleFileName(static_cast<HMODULE>(m_pKernelSML->GetModuleHandle()), dllpath, 256);

	// This sets it to the path + the dll
	m_LibraryDirectory = dllpath;

	// This chops off the dll part to get just the path (...SoarLibrary/bin)
	m_LibraryDirectory = m_LibraryDirectory.substr(0, m_LibraryDirectory.find_last_of("\\"));

	// This takes the parent directory to get ...SoarLibrary
	m_LibraryDirectory = m_LibraryDirectory.substr(0, m_LibraryDirectory.find_last_of("\\"));

#else // WIN32
	// Hopefully ...SoarLibrary/bin
	GetCurrentWorkingDirectory(m_LibraryDirectory);

	// This takes the parent directory to get ...SoarLibrary
	m_LibraryDirectory = m_LibraryDirectory.substr(0, m_LibraryDirectory.find_last_of("/"));

#endif // WIN32
}

bool CommandLineInterface::GetCurrentWorkingDirectory(std::string& directory) {
	// Pull an arbitrary buffer size of 1024 out of a hat and use it
	char buf[1024];
	char* ret = getcwd(buf, 1024);

	// If getcwd returns 0, that is bad
	if (!ret) return SetError(CLIError::kgetcwdFail);

	// Store directory in output parameter and return success
	directory = buf;
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

void CommandLineInterface::PrependArgTag(const char* pParam, const char* pType, const char* pValue) {
	TagArg* pTag = new TagArg();
	pTag->SetParam(pParam);
	pTag->SetType(pType);
	pTag->SetValue(pValue);
	m_ResponseTags.push_front(pTag);
}

void CommandLineInterface::PrependArgTagFast(const char* pParam, const char* pType, const char* pValue) {
	TagArg* pTag = new TagArg();
	pTag->SetParamFast(pParam);
	pTag->SetTypeFast(pType);
	pTag->SetValue(pValue);
	m_ResponseTags.push_front(pTag);
}

bool CommandLineInterface::SetError(cli::ErrorCode code) {
	m_LastError = code;
	return false;
}
bool CommandLineInterface::SetErrorDetail(const std::string detail) {
	m_LastErrorDetail = detail;
	return false;
}

bool CommandLineInterface::StripQuotes(std::string& str) {
    if ((str.size() >= 2) && (str[0] == '"') && (str[str.length() - 1] == '"')) {
        str = str.substr(1, str.length() - 2);
        return true;
    }
    return false;
}

void CommandLineInterface::ResetOptions() {
	m_Argument = 0;
	m_NonOptionArguments = 0;	
}

bool CommandLineInterface::ProcessOptions(std::vector<std::string>& argv, Options* options) {

	// default to indifference
	m_Option = 0;
	m_OptionArgument = "";

	// increment current argument and check bounds
	while (static_cast<unsigned>(++m_Argument) < argv.size()) {

		// args less than 2 characters cannot mean anything to us
		if (argv[m_Argument].size() < 2) {
			++m_NonOptionArguments;
			continue;
		}

		if (argv[m_Argument][0] == '-') {
			if (argv[m_Argument][1] == '-') {
				// possible long m_Option
				if (argv[m_Argument].size() > 2) {
					// long m_Option, save it
					std::string longOption = argv[m_Argument].substr(2);

					// check for partial match
					std::list<Options> possibilities;
					std::list<Options>::iterator liter;
					std::set< std::string > addedLongOptions;

					for(unsigned index = 0; index < longOption.size(); ++index) {

						if (index == 0) {
							// Bootstrap the list of possibilities
							for (int i = 0; options[i].shortOpt != 0; ++i) {
								if (options[i].longOpt[index] == longOption[index]) {
									// don't add duplicates (bug 976)
									if ( addedLongOptions.insert( options[i].longOpt ).second )
									{
										possibilities.push_back(options[i]);
									}
								}
							}

						} else {
							// Update the list of possiblities

							// A more efficient search here would be nice.
							liter = possibilities.begin();
							while (liter != possibilities.end()) {
								if ((*liter).longOpt[index] != longOption[index]) {
									// Remove this possibility from the list
									liter = possibilities.erase(liter);
								} else {
									// check for exact match
									if (longOption == (*liter).longOpt) {
										// exact match, we're done
										m_Option = liter->shortOpt;
										MoveBack(argv, m_Argument, m_NonOptionArguments);
										if (!HandleOptionArgument(argv, liter->longOpt, liter->argument)) return false;
										return true;
									}
									++liter;
								}
							}
						}

						if (!possibilities.size()) {
							SetErrorDetail("No such option: " + longOption);
							return SetError(CLIError::kUnrecognizedOption);
						} 
					}

					if (possibilities.size() != 1) {
						// Ambiguous
						std::stringstream detail;
						detail << "Ambiguous option, possibilities: ";
						liter = possibilities.begin();
						while (liter != possibilities.end()) {
							detail << "'--" << (*liter).longOpt << "' ";
							++liter;
						}
						SetErrorDetail(detail.str());
						return SetError(CLIError::kAmbiguousOption);

					}
					// We have a partial match
					m_Option = (*(possibilities.begin())).shortOpt;
					MoveBack(argv, m_Argument, m_NonOptionArguments);
					if (!HandleOptionArgument(argv, (*(possibilities.begin())).longOpt, (*(possibilities.begin())).argument)) return false;
					return true;
				}

				// end of options special flag '--'
				// FIXME: remove -- argument?
				m_Option = -1; // done
				return true; // no error
			}

			// short m_Option(s)
			for (int i = 0; options[i].shortOpt != 0; ++i) {
				if (argv[m_Argument][1] == options[i].shortOpt) {
					if (argv[m_Argument].size() > 2) {
						std::vector<std::string>::iterator target = argv.begin();
						target += m_Argument;

						std::string original = *target;
						*target = (*target).substr(0,2);
						++target;

						argv.insert(target, "-" + original.substr(2));
					}
					m_Option = options[i].shortOpt;
					MoveBack(argv, m_Argument, m_NonOptionArguments);
					if (!HandleOptionArgument(argv, options[i].longOpt, options[i].argument)) return false;
					return true;
				}
			}
			char theOption = argv.at( m_Argument ).at( 1 );
			SetErrorDetail( std::string("No such option: ") + theOption );
			return SetError(CLIError::kUnrecognizedOption);
		}
		++m_NonOptionArguments;
	}

	// out of arguments
	m_Option = -1;	// done
	return true;	// no error
}

void CommandLineInterface::MoveBack(std::vector<std::string>& argv, int what, int howFar) {

	assert(what >= howFar);
	assert(what > 0);
	assert(howFar >= 0);

	if (howFar == 0) {
		return;
	}

	std::vector<std::string>::iterator target = argv.begin();
	target += what;

	std::vector<std::string>::iterator dest = target - howFar;

	argv.insert(dest, *target);

	target = argv.begin();
	target += what + 1;

	argv.erase(target);
}

bool CommandLineInterface::HandleOptionArgument(std::vector<std::string>& argv, const char* option, eOptionArgument arg) {
	switch (arg) {
		case OPTARG_NONE:
			break;
		case OPTARG_REQUIRED:
			// required argument
			if (static_cast<unsigned>(++m_Argument) >= argv.size()) {
				std::string detail(option);
				SetErrorDetail("Option '" + detail + "' requires an argument.");
				return SetError(CLIError::kMissingOptionArg);
			}
			m_OptionArgument = argv[m_Argument];
			MoveBack(argv, m_Argument, m_NonOptionArguments);
			break;
		case OPTARG_OPTIONAL:
		default:
			// optional argument
			if (static_cast<unsigned>(++m_Argument) < argv.size()) {
				if (argv[m_Argument].size()) {
					if (argv[m_Argument][0] != '-') {
						m_OptionArgument = argv[m_Argument];
						MoveBack(argv, m_Argument, m_NonOptionArguments);
					} 
				}
			}
			if (!m_OptionArgument.size()) {
				--m_Argument;
			}
			break;
	}
	return true;
}

CommandLineInterface* cli::GetCLI()
{
	return sml::KernelSML::GetKernelSML()->GetCommandLineInterface() ;
}

/*************************************************************
* @brief Methods to create an XML element by starting a tag, adding attributes and
*		 closing the tag.
*		 These tags are automatically collected into the result of the current command.
*	
* NOTE: The attribute names must be compile time constants -- i.e. they remain in scope
*		at all times (so we don't have to copy them).
*************************************************************/
void CommandLineInterface::XMLBeginTag(char const* pTagName)
{
	m_XMLResult->BeginTag(pTagName) ;
}

void CommandLineInterface::XMLAddAttribute(char const* pAttribute, char const* pValue)
{
	m_XMLResult->AddAttribute(pAttribute, pValue) ;
}

void CommandLineInterface::XMLEndTag(char const* pTagName)
{
	m_XMLResult->EndTag(pTagName) ;
}

/*************************************************************
* @brief	Occassionally it's helpful to be able to back up
*			in the XML and add some extra elements.
*
*			These calls should only be used once a tag has been completed,
*			so the sequence is something like:
*			beginTag() ;
*			...
*			endTag() ;
*			moveToLastChild() ;	// Goes back to previous tag
*			add an extra attribute() ;
*			moveToParent() ;	// Go back to parent
*			... continue on
*************************************************************/
bool CommandLineInterface::XMLMoveCurrentToParent()
{
	return m_XMLResult->MoveCurrentToParent() ;
}

bool CommandLineInterface::XMLMoveCurrentToChild(int index)
{
	return m_XMLResult->MoveCurrentToChild(index) ;
}

bool CommandLineInterface::XMLMoveCurrentToLastChild()
{
	return m_XMLResult->MoveCurrentToLastChild() ;
}

// The copies over the m_XMLResult object to the response XML object and sets the
// tag name to the command that was just executed.
// The result is XML in this format (e.g. for matches):
// <result><matches>...</matches></result>
// where ... contains the XML specific to that command.
void CommandLineInterface::XMLResultToResponse(char const* pCommandName)
{
	// Extract the XML object from the xmlTrace object and
	// add it as a child of this message.  This is just moving a few pointers around, nothing is getting copied.
	ElementXML_Handle xmlHandle = m_XMLResult->Detach() ;
	ElementXML* pXMLResult = new ElementXML(xmlHandle) ;
	pXMLResult->SetTagName(pCommandName) ;

	m_ResponseTags.push_back(pXMLResult) ;

	// Clear the XML result, so it's ready for use again.
	m_XMLResult->Reset() ;
}

int CommandLineInterface::CLITokenize(std::string cmdline, std::vector<std::string>& argumentVector) {

	int ret = Tokenize(cmdline, argumentVector);
	
	if (ret >= 0) {
		return ret; // no error
	}

	// there is an error

	// handle easy errors with a switch
	switch (ret) {
		case -1:
			SetError(CLIError::kNewlineBeforePipe);
			break;

		case -2:
			SetErrorDetail("An extra '}' was found.");
			SetError(CLIError::kExtraClosingParen);
			break;

		case -3:
			SetErrorDetail("An extra ')' was found.");
			SetError(CLIError::kExtraClosingParen);
			break;

		default:
			{
				int errorCode = abs(ret);

				const int QUOTES_MASK = 4;
				const int BRACKETS_MASK = 8;
				const int PARENS_MASK = 16;
				const int PIPES_MASK = 32;

				std::string errorDetail = "These quoting characters were not closed: ";
				bool foundError = false;
				if (QUOTES_MASK & errorCode) {
					foundError = true;
					errorDetail += "\"";
				}
				if (BRACKETS_MASK & errorCode) {
					foundError = true;
					errorDetail += "{";
				}
				if (PARENS_MASK & errorCode) {
					foundError = true;
					errorDetail += "(";
				}
				if (PIPES_MASK & errorCode) {
					foundError = true;
					errorDetail += "|";
				}
				assert(foundError);
				SetErrorDetail(errorDetail);
			}
			SetError(CLIError::kUnmatchedBracketOrQuote);
			break;
	}

	return ret;
}

void CommandLineInterface::OnKernelEvent(int eventID, AgentSML*, void* pCallData)
{
	// Registered for this event in source command
	if (eventID == smlEVENT_BEFORE_PRODUCTION_REMOVED)
	{
		// Only called when source command is active
		++m_NumProductionsExcised;

		if (m_SourceVerbose) {
			production* p = reinterpret_cast<production*>(pCallData);
			assert(p) ;
			assert(p->name->sc.name) ;

			std::string name( p->name->sc.name );

			m_ExcisedDuringSource.push_back( name );
		}
	}
	else if (eventID == smlEVENT_PRINT)
	{
		char const* msg = reinterpret_cast<char const*>(pCallData);

		if (m_TrapPrintEvents || m_pLogFile)
		{
			if (m_VarPrint)
			{
				// Transform if varprint, see print command
				std::string message(msg);

				regex_t comp;
				regcomp(&comp, "[A-Z][0-9]+", REG_EXTENDED);

				regmatch_t match;
				memset(&match, 0, sizeof(regmatch_t));

				while (regexec(&comp, message.substr(match.rm_eo, message.size() - match.rm_eo).c_str(), 1, &match, 0) == 0) {
					message.insert(match.rm_so, "<");
					message.insert(match.rm_eo + 1, ">");
					match.rm_eo += 2;
				}  

				regfree(&comp);

				// Simply append to message result
				if (m_TrapPrintEvents) {
					CommandLineInterface::m_Result << message;
				}
			} else {
				if (m_TrapPrintEvents) {
					CommandLineInterface::m_Result << msg;
				}
			}
		}
	}
	else
	{
		assert(false);
		// unknown event
		// TODO: gracefully (?) deal with this error
	}
} // function

