/////////////////////////////////////////////////////////////////
// Command Line Interface class
//
// Author: Jonathan Voigt
// Date  : Sept 2004
//
// This class is used as a general command line interface to Soar
// through gSKI.
//
/////////////////////////////////////////////////////////////////
#ifndef COMMAND_LINE_INTERFACE_H
#define COMMAND_LINE_INTERFACE_H

// STL includes
#include <map>
#include <vector>
#include <string>
#include <stack>
#include <list>
#include <iostream>
#include <fstream>

// Local includes
#include "cli_CommandData.h"
//#include "cli_Constants.h"
#include "cli_Aliases.h"

// gSKI includes
#include "gSKI_Events.h"
#include "IgSKI_KernelFactory.h"

#ifdef _WIN32
#ifdef _USRDLL
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __declspec(dllimport)
#endif	// DLL
#else
#define EXPORT
#endif	// WIN32

#ifndef unused
#define unused(x) (void)(x)
#endif

// Forward Declarations
namespace gSKI {
	class IAgent;
	class IKernel;
	struct Error;
}
namespace sml {
	class ElementXML;
	class KernelSML;
	class Connection ;
}

namespace cli {

// Forward declarations
class CommandLineInterface;
class GetOpt;
class Constants;

// Define the CommandFunction which we'll call to process commands
typedef bool (CommandLineInterface::*CommandFunction)(gSKI::IAgent* pAgent, std::vector<std::string>& argv);

// Used to store a map from command name to function handler for that command
typedef std::map<std::string, CommandFunction>	CommandMap;
typedef CommandMap::iterator					CommandMapIter;
typedef CommandMap::const_iterator				CommandMapConstIter;

// Define the stack for pushd/popd
typedef std::stack<std::string> StringStack;

// Define the list for structured responses
typedef std::list<sml::ElementXML*> ElementXMLList;
typedef ElementXMLList::iterator ElementXMLListIter;

// Log command enum
	enum OPTION_LOG { 
		OPTION_LOG_NEW,
		OPTION_LOG_NEWAPPEND,
		OPTION_LOG_CLOSE,
		OPTION_LOG_ADD,
		OPTION_LOG_QUERY,
	};

class CommandLineInterface
{
public:

	EXPORT CommandLineInterface();
	EXPORT ~CommandLineInterface();

	/*************************************************************
	* @brief Set the kernel this command line module is interfacing with.
	*************************************************************/
	EXPORT void SetKernel(gSKI::IKernel* pKernel, gSKI::Version kernelVersion);

	/*************************************************************
	* @brief Process a command.  Give it a command line and it will parse
	*		 and execute the command using gSKI or system calls.
	*		 This version of DoCommand is used when the command line
	*		 interface is created by sml::KernelSML
	*************************************************************/
	EXPORT bool DoCommand(sml::Connection* pConnection, gSKI::IAgent* pAgent, const char* pCommandLine, sml::ElementXML* pResponse, bool rawOutput, gSKI::Error* pError);

	/*************************************************************
	* @brief Process a command.  Give it a command line and it will parse
	*		 and execute the command using gSKI or system calls.
	*		 This version of DoCommand is used when the command line
	*		 interface is created by the application that is using it
	*		 directly (no SML layer).
	*************************************************************/
	EXPORT bool DoCommand(gSKI::IAgent* pAgent, const char* pCommandLine, char const* pResponse, gSKI::Error* pError);

	/*************************************************************
	* @brief This will return true after the quit command has been processed.
	*************************************************************/
	bool IsQuitCalled() { return m_QuitCalled; }

	// Template for new commands:
	///*************************************************************
	//* @brief 
	//*************************************************************/
	//bool Parse(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	///*************************************************************
	//* @brief 
	//*************************************************************/
	//bool Do();

	/*************************************************************
	* @brief 
	*************************************************************/
	bool ParseAddWME(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	/*************************************************************
	* @brief 
	*************************************************************/
	bool DoAddWME(gSKI::IAgent* pAgent, std::string id, std::string attribute, std::string value, bool acceptable);

	/*************************************************************
	* @brief 
	*************************************************************/
	bool ParseAlias(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	/*************************************************************
	* @brief 
	*************************************************************/
	bool DoAlias(bool disable, const std::string& command, const std::vector<std::string>* pSubstitution);

	/*************************************************************
	* @brief cd command, see usage.txt for details.
	*************************************************************/
	bool ParseCD(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	/*************************************************************
	* @brief Change the current working directory.  If a null pointer passed,
	*		 the current working directory is changed to the home directory.
	*		 The home directory is defined as the initial working directory.
	*************************************************************/
	bool DoCD(std::string* pDirectory = 0);

	/*************************************************************
	* @brief 
	*************************************************************/
	bool ParseDirs(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	/*************************************************************
	* @brief 
	*************************************************************/
	bool DoDirs();

	/*************************************************************
	* @brief echo command, see usage.txt for details.
	*************************************************************/
	bool ParseEcho(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	/*************************************************************
	* @brief Simply concatenates all arguments, adding them to the result.
	*************************************************************/
	bool DoEcho(std::vector<std::string>& argv, bool noNewLine);

	/*************************************************************
	* @brief excise command, see usage.txt for details.
	*************************************************************/
	bool ParseExcise(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	/*************************************************************
	* @brief See CommandData.h for the list of flags for the options
	*		 parameter.  If there is a specific production to excise,
	*		 it is passed, otherwise, pProduction is null.
	*************************************************************/
	bool DoExcise(gSKI::IAgent* pAgent, const unsigned int options, std::string* pProduction = 0);

	/*************************************************************
	* @brief firing-counts command, see usage.txt for details.
	*************************************************************/
	bool ParseFiringCounts(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	/*************************************************************
	* @brief 
	*************************************************************/
	bool DoFiringCounts(gSKI::IAgent* pAgent, std::string* pProduction, int numberToList);

	/*************************************************************
	* @brief 
	*************************************************************/
	bool ParseGDSPrint(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	/*************************************************************
	* @brief 
	*************************************************************/
	bool DoGDSPrint(gSKI::IAgent* pAgent);

	/*************************************************************
	* @brief help command, see usage.txt for details.
	*************************************************************/
	bool ParseHelp(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	/*************************************************************
	* @brief Generate a help string for the passed command if it exists.
	*		 Passing null generates a general help string that lists the
	*		 available commands along with some other help.
	*************************************************************/
	bool DoHelp(std::string* pCommand = 0);

	/*************************************************************
	* @brief helpex command, see usage.txt for details.
	*************************************************************/
	bool ParseHelpEx(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	/*************************************************************
	* @brief Generate the extended help string for the passed command
	*		 if it exists.
	*************************************************************/
	bool DoHelpEx(const std::string& command);

	/*************************************************************
	* @brief Change the home directory to the current or passed directory.
	*************************************************************/
	bool ParseHome(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	/*************************************************************
	* @brief If passed directory is null, get the current directory and change
	*        to it.  Otherwise, set the home directory to pDirectory.
	*************************************************************/
	bool DoHome(std::string* pDirectory = 0);

	/*************************************************************
	* @brief init-soar command, see usage.txt for details.
	*************************************************************/
	bool ParseInitSoar(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	/*************************************************************
	* @brief Reinitializes the current agent.  No arguments necessary.
	*************************************************************/
	bool DoInitSoar(gSKI::IAgent* pAgent);

	/*************************************************************
	* @brief 
	*************************************************************/
	bool ParseIO(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	/*************************************************************
	* @brief 
	*************************************************************/
	bool DoIO();

	/*************************************************************
	* @brief learn command, see usage.txt for details.
	*************************************************************/
	bool ParseLearn(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	/*************************************************************
	* @brief See CommandData.h for the list of flags used in the options
	*		 parameter.  Passing no options simply prints current learn
	*		 settings.
	*************************************************************/
	bool DoLearn(gSKI::IAgent* pAgent, const unsigned int options = 0);

	/*************************************************************
	* @brief log command, see usage.txt for details.
	*************************************************************/
	bool ParseLog(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	/*************************************************************
	* @brief Presence of filename opens a file for logging.  If option is
	*		 true, then the logging is appended to the end of the file.
	*		 If the filename is null and the option is true, the log file is
	*		 closed.  If the filename is null and the option false, 
	*		 the current logging status is queried.
	*************************************************************/
	bool DoLog(gSKI::IAgent* pAgent, OPTION_LOG operation, const std::string& filename, const std::string& toAdd);

	/*************************************************************
	* @brief ls/dir command, see usage.txt for details.
	*************************************************************/
	bool ParseLS(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	/*************************************************************
	* @brief Lists current working directory, no arguments necessary.
	*************************************************************/
	bool DoLS();

	/*************************************************************
	* @brief 
	*************************************************************/
	bool ParseMatches(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	/*************************************************************
	* @brief 
	*************************************************************/
	bool DoMatches(gSKI::IAgent* pAgent, unsigned int matches, int wmeDetail, const std::string& production);

	/*************************************************************
	* @brief 
	*************************************************************/
	bool ParseMaxElaborations(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	/*************************************************************
	* @brief 
	*************************************************************/
	bool DoMaxElaborations();

	/*************************************************************
	* @brief 
	*************************************************************/
	bool ParseMaxNilOutputCycles(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	/*************************************************************
	* @brief 
	*************************************************************/
	bool DoMaxNilOutputCycles();

	/*************************************************************
	* @brief 
	*************************************************************/
	bool ParseMemories(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	/*************************************************************
	* @brief 
	*************************************************************/
	bool DoMemories(gSKI::IAgent* pAgent, unsigned int productionType, int n, std::string production);

	/*************************************************************
	* @brief multi-attributes command, see usage.txt for details.
	*************************************************************/
	bool ParseMultiAttributes(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	/*************************************************************
	* @brief Two optional arguments, attribute and n.  If no arguments,
	*		 prints current settings.
	*************************************************************/
	bool DoMultiAttributes(gSKI::IAgent* pAgent, std::string* pAttribute = 0, int n = 0);

	/*************************************************************
	* @brief 
	*************************************************************/
	bool ParseNumericIndifferentMode(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	/*************************************************************
	* @brief 
	*************************************************************/
	bool DoNumericIndifferentMode();

	/*************************************************************
	* @brief popd command, see usage.txt for details.
	*************************************************************/
	bool ParsePopD(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	/*************************************************************
	* @brief No arguments, pops a directory off the directory stack
	*		 and changes to it.
	*************************************************************/
	bool DoPopD();

	/*************************************************************
	* @brief 
	*************************************************************/
	bool ParsePreferences(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	/*************************************************************
	* @brief 
	*************************************************************/
	bool DoPreferences();

	/*************************************************************
	* @brief print command, see usage.txt for details.
	*************************************************************/
	bool ParsePrint(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	/*************************************************************
	* @brief Issues a print command to the kernel depending on the options
	*		 (see commanddata.h), depth argument, and optional string
	*		 argument.
	*************************************************************/
	bool DoPrint(gSKI::IAgent* pAgent, const unsigned int options, int depth, std::string* pArg = 0);

	/*************************************************************
	* @brief 
	*************************************************************/
	bool ParseProductionFind(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	/*************************************************************
	* @brief 
	*************************************************************/
	bool DoProductionFind();

	/*************************************************************
	* @brief pushd command, see usage.txt for details.
	*************************************************************/
	bool ParsePushD(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	/*************************************************************
	* @brief Pushes a directory on to the directory stack.  Operates
	*		 like the unix command of the same name.  Used with popd.
	*************************************************************/
	bool DoPushD(std::string& directory);

	/*************************************************************
	* @brief pwd command, see usage.txt for details.
	*************************************************************/
	bool ParsePWD(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	/*************************************************************
	* @brief Prints the current working directory.
	*************************************************************/
	bool DoPWD();

	/*************************************************************
	* @brief exit/quit command, see usage.txt for details.
	*************************************************************/
	bool ParseQuit(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	/*************************************************************
	* @brief 
	*************************************************************/
	bool DoQuit();

	/*************************************************************
	* @brief 
	*************************************************************/
	bool ParseRemoveWME(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	/*************************************************************
	* @brief 
	*************************************************************/
	bool DoRemoveWME(gSKI::IAgent*, int timetag);

	/*************************************************************
	* @brief 
	*************************************************************/
	bool ParseReteNet(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	/*************************************************************
	* @brief 
	*************************************************************/
	bool DoReteNet();

	/*************************************************************
	* @brief run command, see usage.txt for details.
	*************************************************************/
	bool ParseRun(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	/*************************************************************
	* @brief Starts execution of the Soar kernel.  The options decide
	*		 the length and size of steps, the count determines the
	*		 number of steps to execute.  Returns when the count is achieved
	*		 or until execution is interrupted by an interrupt or an
	*		 error.
	*************************************************************/
	bool DoRun(gSKI::IAgent* pAgent, const unsigned int options, int count);

	/*************************************************************
	* @brief 
	*************************************************************/
	bool ParseSoar8(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	/*************************************************************
	* @brief 
	*************************************************************/
	bool DoSoar8(bool query, bool soar8);

	/*************************************************************
	* @brief source command, see usage.txt for details.
	*************************************************************/
	bool ParseSource(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	/*************************************************************
	* @brief Sources a file.  Basically executes the lines of the file
	*		 through DoCommandInternal() ignoring comments (lines starting
	*		 with '#').
	*************************************************************/
	bool DoSource(gSKI::IAgent* pAgent, std::string filename);

	/*************************************************************
	* @brief sp command, see usage.txt for details.
	*************************************************************/
	bool ParseSP(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	/*************************************************************
	* @brief Soar production command, takes a production and loads it
	*		 into production memory.  The production argument should be
	*		 in the form accepted by gSKI, that is with the sp and 
	*		 braces removed.
	*************************************************************/
	bool DoSP(gSKI::IAgent* pAgent, const std::string& production);

	/*************************************************************
	* @brief stats command, see usage.txt for details.
	*************************************************************/
	bool ParseStats(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	/*************************************************************
	* @brief Assembles a string representing the Soar kernel stats
	*		 generated by the kernel.
	*************************************************************/
	bool DoStats(gSKI::IAgent* pAgent);

	/*************************************************************
	* @brief stop-soar command, see usage.txt for details.
	*************************************************************/
	bool ParseStopSoar(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	/*************************************************************
	* @brief 
	*************************************************************/
	bool DoStopSoar(gSKI::IAgent* pAgent, bool self, const std::string& reasonForStopping);

	/*************************************************************
	* @brief time command, see usage.txt for details.
	*************************************************************/
	bool ParseTime(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	/*************************************************************
	* @brief Executes the arguments as a command and times how long it
	*		 takes for the DoCommandInternal function to return.
	*************************************************************/
	bool DoTime(gSKI::IAgent* pAgent, std::vector<std::string>& argv);

	/*************************************************************
	* @brief timers command, see usage.txt for details.
	*************************************************************/
	bool ParseTimers(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	/*************************************************************
	* @brief If print is true, returns the current setting of the
	*		 stats timers. If setting is true, enables stats timers.  
	*        False disables stats timers.
	*************************************************************/
	bool DoTimers(gSKI::IAgent* pAgent, bool print, bool setting);

	/*************************************************************
	* @brief 
	*************************************************************/
	bool ParseVersion(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	/*************************************************************
	* @brief 
	*************************************************************/
	bool DoVersion();

	/*************************************************************
	* @brief 
	*************************************************************/
	bool ParseWarnings(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	/*************************************************************
	* @brief 
	*************************************************************/
	bool DoWarnings(gSKI::IAgent* pAgent, bool query, bool setting);

	/*************************************************************
	* @brief watch command, see usage.txt for details.
	*************************************************************/
	bool ParseWatch(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	/*************************************************************
	* @brief Set watch settings.  The options flag contains the changed
	*		 flags and the values flag contains their new values.
	*************************************************************/
	bool DoWatch(gSKI::IAgent* pAgent, const unsigned int options, unsigned int values);

	/*************************************************************
	* @brief 
	*************************************************************/
	bool ParseWMEs(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	/*************************************************************
	* @brief 
	*************************************************************/
	bool DoWMEs();


protected:

	// Print handler for gSKI print callbacks
	class PrintHandler : public gSKI::IPrintListener
	{
	public:
		PrintHandler() { m_pCLI = 0; }

		void SetCLI(CommandLineInterface* pCLI) { m_pCLI = pCLI; }
		virtual void HandleEvent(egSKIEventId, gSKI::IAgent*, const char* msg) = 0;
	protected:
		CommandLineInterface*	m_pCLI;	// pointer to command line interface
	};

	// ResultPrint handler for result string additions
	class ResultPrintHandler : public PrintHandler
	{
	public:
		ResultPrintHandler() {}

		virtual void HandleEvent(egSKIEventId, gSKI::IAgent*, const char* msg) {
			if (!m_pCLI) return;

			// Simply append to message result
			m_pCLI->AppendToResult(msg);
		}
	};

	// Log Print handler for log printing
	class LogPrintHandler : public PrintHandler
	{
	public:
		LogPrintHandler() {}

		virtual void HandleEvent(egSKIEventId, gSKI::IAgent*, const char* msg) {
			if (!m_pCLI) return;
			if (!m_pCLI->m_pLogFile) return;
			(*(m_pCLI->m_pLogFile)) << msg;
		}
	};

	friend class PrintHandler;			// Allows calling of AppendToResult and log writing

	/*************************************************************
	* @brief Currently used as an output hack, the print handler calls 
	*		 this to add output to the result that gets sent back upon
	*		 completion of a command.
	*************************************************************/
	inline void CommandLineInterface::AppendToResult(const char* pMessage) {
		// Simply add to result
		m_Result += pMessage;
	}

	inline void CommandLineInterface::AppendToResult(const std::string& message) {
		// Simply add to result
		m_Result += message;
	}

	inline void CommandLineInterface::AppendToResult(const bool boolean) {
		// Simply add to result
		m_Result += boolean;
	}

	inline void CommandLineInterface::AppendToResult(const char character) {
		// Simply add to result
		m_Result += character;
	}

	/*************************************************************
	* @brief Does the bulk of command parsing and chooses what function
	*		 to call to process the command.  DoCommand mainly does
	*		 SML stuff.
	*************************************************************/
	bool DoCommandInternal(gSKI::IAgent* pAgent, const std::string& commandLine);
	bool DoCommandInternal(gSKI::IAgent* pAgent, std::vector<std::string>& argv);

	/*************************************************************
	* @brief A utility function, splits the command line into argument
	*		 tokens and stores them in the argumentVector string.
	*************************************************************/
	int Tokenize(std::string commandLine, std::vector<std::string>& argumentVector);

	/*************************************************************
	* @brief A utility function used in the constructor to build the mapping
	*		 of command names to function pointers.
	*************************************************************/
	void BuildCommandMap();

	/*************************************************************
	* @brief Standard parsing of -h and --help flags.  Returns
	*		 true if the flag is present.
	*************************************************************/
	bool CheckForHelp(std::vector<std::string>& argv);

	/*************************************************************
	* @brief 
	*************************************************************/
	bool GetCurrentWorkingDirectory(std::string& directory);

	/*************************************************************
	* @brief 
	*************************************************************/
	void ExciseInternal(gSKI::tIProductionIterator* pProdIter, int& exciseCount);

	/*************************************************************
	* @brief 
	*************************************************************/
	bool WatchArg(unsigned int& values, const unsigned int option, const char* arg);
	bool WatchArg(unsigned int& values, const unsigned int option, int argInt);

	/*************************************************************
	* @brief 
	*************************************************************/
	bool IsInteger(const std::string& s);

	/*************************************************************
	* @brief 
	*************************************************************/
	bool RequireAgent(gSKI::IAgent* pAgent);

	/*************************************************************
	* @brief 
	*************************************************************/
	bool RequireKernel();

	/*************************************************************
	* @brief 
	*************************************************************/
	bool HandleError(std::string errorMessage, gSKI::Error* pError = 0);

	/*************************************************************
	* @brief 
	*************************************************************/
	void HandleSourceError(int errorLine, const std::string& filename);

	/*************************************************************
	* @brief 
	*************************************************************/
	bool HandleSyntaxError(const char* command, const std::string& details);

	/*************************************************************
	* @brief 
	*************************************************************/
	bool HandleSyntaxError(const char* command, const char* details = 0);

	/*************************************************************
	* @brief 
	*************************************************************/
	bool HandleGetOptError(char option);

	/*************************************************************
	* @brief 
	*************************************************************/
	void AppendArgTag(const char* pParam, const char* pType, const char* pValue);

	/*************************************************************
	* @brief 
	*************************************************************/
	void AppendArgTagFast(const char* pParam, const char* pType, const char* pValue);

	/*************************************************************
	* @brief 
	*************************************************************/
	void PrependArgTagFast(const char* pParam, const char* pType, const char* pValue);

	/************************************************************* 	 
	* @brief This is a utility function used by DoLS 	 
	*************************************************************/ 	 
	void PrintFilename(const std::string& name, bool isDirectory); 	 
  	 
	Constants*			m_pConstants;			// Constants management object
	Aliases				m_Aliases;				// Alias management object
	GetOpt*				m_pGetOpt;				// Pointer to GetOpt utility class

	CommandMap			m_CommandMap;			// Mapping of command names to function pointers

	gSKI::IKernel*		m_pKernel;				// Pointer to the current gSKI kernel
	gSKI::Version		m_KernelVersion;		// Kernel version number

	bool				m_RawOutput;			// True if we want string output.
	std::string			m_HomeDirectory;		// The initial working directory, server side
	bool				m_QuitCalled;			// True after DoQuit is called
	StringStack			m_DirectoryStack;		// Directory stack for pushd/popd
	ElementXMLList		m_ResponseTags;			// List of tags for the response.

	bool				m_SourceError;			// Used to control debug printing for source command errors
	int					m_SourceDepth;			// Depth of source command calls.
	int					m_SourceDirDepth;		// Depth of directory stack since source command, used to return to the dir that source was issued in.

	std::string			m_Result;				// String output from the command
	std::string			m_ErrorMessage;			// String output from the command
	gSKI::Error*		m_pError;				// gSKI error output from calls made to process the command

	ResultPrintHandler	m_ResultPrintHandler;	// The print callback handler, used for catching kernel/gSKI output and writing it to result
	LogPrintHandler		m_LogPrintHandler;		// The print callback handler, used for catching kernel/gSKI output and writing it to a log

	std::string			m_LogFilename;			// Used for logging to a file.
	std::ofstream*		m_pLogFile;				// The log file stream

};

} // namespace cli

#endif //COMMAND_LINE_INTERFACE_H
