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
#include <vector>
#include <string>
#include <stack>
#ifdef _MSC_VER
#pragma warning (disable : 4702)  // warning C4702: unreachable code, need to disable for VS.NET 2003 due to STL "bug" in certain cases
#endif
#include <map>
#include <list>
#ifdef _MSC_VER
#pragma warning (default : 4702)
#endif
#include <iostream>
#include <fstream>
#include <sstream>

// Local includes
#include "cli_CommandData.h"
#include "cli_Aliases.h"
#include "cli_CLIError.h"

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

class CommandLineInterface : public gSKI::IPrintListener{
public:

	EXPORT CommandLineInterface();
	EXPORT ~CommandLineInterface();

	/*************************************************************
	* @brief Set the kernel this command line module is interfacing with.
	* @param pKernel The pointer to the gSKI kernel interface
	* @param kernelVersion The gSKI version, available from the KernelFactory
	* @param pKernelSML The pointer to the KernelSML object, optional, used to disable print callbacks
	*************************************************************/
	EXPORT void SetKernel(gSKI::IKernel* pKernel, gSKI::Version kernelVersion, sml::KernelSML* pKernelSML = 0);

	/*************************************************************
	* @brief Process a command.  Give it a command line and it will parse
	*		 and execute the command using gSKI or system calls.
	*		 This version of DoCommand is used when the command line
	*		 interface is created by sml::KernelSML
	*************************************************************/
	EXPORT bool DoCommand(sml::Connection* pConnection, gSKI::IAgent* pAgent, const char* pCommandLine, sml::ElementXML* pResponse, bool rawOutput, gSKI::Error* pError);

	/*************************************************************
	* @brief Takes a command line and expands any aliases and returns
	*		 the result.  The command is NOT executed.
	*************************************************************/
	EXPORT bool ExpandCommand(sml::Connection* pConnection, const char* pCommandLine, sml::ElementXML* pResponse, gSKI::Error* pError);

	/*************************************************************
	* @brief 
	*************************************************************/
	EXPORT bool DoAddWME(gSKI::IAgent* pAgent, std::string id, std::string attribute, std::string value, bool acceptable);

	/*************************************************************
	* @brief 
	*************************************************************/
	EXPORT bool DoAlias(bool disable, const std::string& command, const std::vector<std::string>* pSubstitution);

	/*************************************************************
	* @brief Change the current working directory.  If a null pointer passed,
	*		 the current working directory is changed to the home directory.
	*		 The home directory is defined as the initial working directory.
	*************************************************************/
	EXPORT bool DoCD(std::string* pDirectory = 0);

	/*************************************************************
	* @brief 
	*************************************************************/
	EXPORT bool DoChunkNameFormat(gSKI::IAgent* pAgent, bool changeFormat = false, bool longFormat = true, int* pCount = 0, std::string* pPrefix = 0);

	/*************************************************************
	* @brief 
	*************************************************************/
	EXPORT bool DoDefaultWMEDepth(gSKI::IAgent* pAgent, int n);

	/*************************************************************
	* @brief 
	*************************************************************/
	EXPORT bool DoDirs();

	/*************************************************************
	* @brief Simply concatenates all arguments, adding them to the result.
	*************************************************************/
	EXPORT bool DoEcho(std::vector<std::string>& argv);

	/*************************************************************
	* @brief See CommandData.h for the list of flags for the options
	*		 parameter.  If there is a specific production to excise,
	*		 it is passed, otherwise, pProduction is null.
	*************************************************************/
	EXPORT bool DoExcise(gSKI::IAgent* pAgent, const unsigned int options, std::string* pProduction = 0);

	/*************************************************************
	*************************************************************/
	EXPORT bool DoExplainBacktraces(gSKI::IAgent* pAgent, std::string* pProduction = 0, bool full = false, int condition = false);

	/*************************************************************
	* @brief 
	*************************************************************/
	EXPORT bool DoFiringCounts(gSKI::IAgent* pAgent, std::string* pProduction, int numberToList);

	/*************************************************************
	* @brief 
	*************************************************************/
	EXPORT bool DoGDSPrint(gSKI::IAgent* pAgent);

	/*************************************************************
	* @brief Generate a help string for the passed command if it exists.
	*		 Passing null generates a general help string that lists the
	*		 available commands along with some other help.
	*************************************************************/
	EXPORT bool DoHelp(std::string* pCommand = 0);

	/*************************************************************
	* @brief Generate the extended help string for the passed command
	*		 if it exists.
	*************************************************************/
	EXPORT bool DoHelpEx(const std::string& command);

	/*************************************************************
	* @brief If passed directory is null, get the current directory and change
	*        to it.  Otherwise, set the home directory to pDirectory.
	*************************************************************/
	EXPORT bool DoHome(std::string* pDirectory = 0);

	/*************************************************************
	* @brief 
	*************************************************************/
	EXPORT bool DoIndifferentSelection(gSKI::IAgent* pAgent, unsigned int mode);

	/*************************************************************
	* @brief Reinitializes the current agent.  No arguments necessary.
	*************************************************************/
	EXPORT bool DoInitSoar(gSKI::IAgent* pAgent);

	/*************************************************************
	*************************************************************/
	EXPORT bool DoInternalSymbols(gSKI::IAgent* pAgent);

	/*************************************************************
	* @brief 
	*************************************************************/
	EXPORT bool DoIO();

	/*************************************************************
	* @brief See CommandData.h for the list of flags used in the options
	*		 parameter.  Passing no options simply prints current learn
	*		 settings.
	*************************************************************/
	EXPORT bool DoLearn(gSKI::IAgent* pAgent, const unsigned int options = 0);

	/*************************************************************
	* @brief Presence of filename opens a file for logging.  If option is
	*		 true, then the logging is appended to the end of the file.
	*		 If the filename is null and the option is true, the log file is
	*		 closed.  If the filename is null and the option false, 
	*		 the current logging status is queried.
	*************************************************************/
	EXPORT bool DoLog(gSKI::IAgent* pAgent, OPTION_LOG operation, const std::string& filename, const std::string& toAdd);

	/*************************************************************
	* @brief Lists current working directory, no arguments necessary.
	*************************************************************/
	EXPORT bool DoLS();

	/*************************************************************
	* @brief 
	*************************************************************/
	EXPORT bool DoMatches(gSKI::IAgent* pAgent, unsigned int matches, int wmeDetail, const std::string& production);

	/*************************************************************
	* @brief 
	*************************************************************/
	EXPORT bool DoMaxChunks(gSKI::IAgent* pAgent, int n);

	/*************************************************************
	* @brief 
	*************************************************************/
	EXPORT bool DoMaxElaborations(gSKI::IAgent* pAgent, int n);

	/*************************************************************
	* @brief 
	*************************************************************/
	EXPORT bool DoMaxNilOutputCycles(gSKI::IAgent* pAgent, int n);

	/*************************************************************
	* @brief 
	*************************************************************/
	EXPORT bool DoMemories(gSKI::IAgent* pAgent, unsigned int productionType, int n, std::string production);

	/*************************************************************
	* @brief Two optional arguments, attribute and n.  If no arguments,
	*		 prints current settings.
	*************************************************************/
	EXPORT bool DoMultiAttributes(gSKI::IAgent* pAgent, std::string* pAttribute = 0, int n = 0);

	/*************************************************************
	* @brief 
	*************************************************************/
	EXPORT bool DoNumericIndifferentMode(gSKI::IAgent* pAgent, unsigned int mode);

	/*************************************************************
	* @brief 
	*************************************************************/
	EXPORT bool DoOSupportMode(gSKI::IAgent* pAgent, int mode);

	/*************************************************************
	* @brief No arguments, pops a directory off the directory stack
	*		 and changes to it.
	*************************************************************/
	EXPORT bool DoPopD();

	/*************************************************************
	* @brief 
	*************************************************************/
	EXPORT bool DoPreferences(gSKI::IAgent* pAgent, int detail, std::string* pId = 0, std::string* pAttribute = 0);

	/*************************************************************
	* @brief Issues a print command to the kernel depending on the options
	*		 (see commanddata.h), depth argument, and optional string
	*		 argument.
	*************************************************************/
	EXPORT bool DoPrint(gSKI::IAgent* pAgent, const unsigned int options, int depth, std::string* pArg = 0);

	/*************************************************************
	* @brief 
	*************************************************************/
	EXPORT bool DoProductionFind(gSKI::IAgent* pAgent, unsigned int mode, std::string pattern);

	/*************************************************************
	* @brief Pushes a directory on to the directory stack.  Operates
	*		 like the unix command of the same name.  Used with popd.
	*************************************************************/
	EXPORT bool DoPushD(std::string& directory);

	/*************************************************************
	*************************************************************/
	EXPORT bool DoPWatch(gSKI::IAgent* pAgent, bool query = true, std::string* pProduction = 0, bool setting = false);

	/*************************************************************
	* @brief Prints the current working directory.
	*************************************************************/
	EXPORT bool DoPWD();

	/*************************************************************
	* @brief 
	*************************************************************/
	EXPORT bool DoQuit();

	/*************************************************************
	* @brief 
	*************************************************************/
	EXPORT bool DoRemoveWME(gSKI::IAgent*, int timetag);

	/*************************************************************
	* @brief 
	*************************************************************/
	EXPORT bool DoReteNet(gSKI::IAgent* pAgent, bool save, std::string filename);

	/*************************************************************
	* @brief Starts execution of the Soar kernel.  The options decide
	*		 the length and size of steps, the count determines the
	*		 number of steps to execute.  Returns when the count is achieved
	*		 or until execution is interrupted by an interrupt or an
	*		 error.
	*************************************************************/
	EXPORT bool DoRun(gSKI::IAgent* pAgent, const unsigned int options, int count);

	/*************************************************************
	* @brief 
	*************************************************************/
	EXPORT bool DoSaveBacktraces(gSKI::IAgent* pAgent, bool query, bool setting);

	/*************************************************************
	* @brief 
	*************************************************************/
	EXPORT bool DoSoar8(bool query, bool soar8);

	/*************************************************************
	* @brief 
	*************************************************************/
	EXPORT bool DoSoarNews();

	/*************************************************************
	* @brief Sources a file.  Basically executes the lines of the file
	*		 through DoCommandInternal() ignoring comments (lines starting
	*		 with '#').
	*************************************************************/
	EXPORT bool DoSource(gSKI::IAgent* pAgent, std::string filename);

	/*************************************************************
	* @brief Soar production command, takes a production and loads it
	*		 into production memory.  The production argument should be
	*		 in the form accepted by gSKI, that is with the sp and 
	*		 braces removed.
	*************************************************************/
	EXPORT bool DoSP(gSKI::IAgent* pAgent, const std::string& production);

	/*************************************************************
	* @brief Assembles a string representing the Soar kernel stats
	*		 generated by the kernel.
	*************************************************************/
	EXPORT bool DoStats(gSKI::IAgent* pAgent, const int options);

	/*************************************************************
	* @brief 
	*************************************************************/
	EXPORT bool DoStopSoar(gSKI::IAgent* pAgent, bool self, const std::string& reasonForStopping);

	/*************************************************************
	* @brief Executes the arguments as a command and times how long it
	*		 takes for the DoCommandInternal function to return.
	*************************************************************/
	EXPORT bool DoTime(gSKI::IAgent* pAgent, std::vector<std::string>& argv);

	/*************************************************************
	* @brief If print is true, returns the current setting of the
	*		 stats timers. If setting is true, enables stats timers.  
	*        False disables stats timers.
	*************************************************************/
	EXPORT bool DoTimers(gSKI::IAgent* pAgent, bool print, bool setting);

	/*************************************************************
	*************************************************************/
	EXPORT bool DoTopD();

	/*************************************************************
	* @brief 
	*************************************************************/
	EXPORT bool DoVerbose(gSKI::IAgent* pAgent, bool query, bool setting);

	/*************************************************************
	* @brief 
	*************************************************************/
	EXPORT bool DoVersion();

	/*************************************************************
	* @brief 
	*************************************************************/
	EXPORT bool DoWaitSNC(gSKI::IAgent* pAgent, bool query, bool enable);

	/*************************************************************
	* @brief 
	*************************************************************/
	EXPORT bool DoWarnings(gSKI::IAgent* pAgent, bool query, bool setting);

	/*************************************************************
	* @brief Set watch settings.  The options flag contains the changed
	*		 flags and the values flag contains their new values.
	*************************************************************/
	EXPORT bool DoWatch(gSKI::IAgent* pAgent, const int options, const int settings, const int wmeSetting, const int learnSetting);

	/*************************************************************
	*************************************************************/
	EXPORT bool DoWatchWMEs(gSKI::IAgent* pAgent, unsigned int mode, bool adds, bool removes, std::string* pIdString = 0, std::string* pAttributeString = 0, std::string* pValueString = 0);


protected:

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

	// The ParseX functions follow, all have same return value and argument list
	bool ParseAddWME(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	bool ParseAlias(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	bool ParseCD(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	bool ParseChunkNameFormat(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	bool ParseDefaultWMEDepth(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	bool ParseDirs(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	bool ParseEcho(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	bool ParseExcise(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	bool ParseExplainBacktraces(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	bool ParseFiringCounts(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	bool ParseGDSPrint(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	bool ParseHelp(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	bool ParseHelpEx(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	bool ParseHome(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	bool ParseIndifferentSelection(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	bool ParseInitSoar(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	bool ParseInternalSymbols(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	bool ParseIO(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	bool ParseLearn(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	bool ParseLog(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	bool ParseLS(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	bool ParseMatches(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	bool ParseMaxChunks(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	bool ParseMaxElaborations(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	bool ParseMaxNilOutputCycles(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	bool ParseMemories(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	bool ParseMultiAttributes(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	bool ParseNumericIndifferentMode(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	bool ParseOSupportMode(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	bool ParsePopD(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	bool ParsePreferences(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	bool ParsePrint(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	bool ParseProductionFind(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	bool ParsePushD(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	bool ParsePWatch(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	bool ParsePWD(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	bool ParseQuit(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	bool ParseRemoveWME(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	bool ParseReteNet(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	bool ParseRun(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	bool ParseSaveBacktraces(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	bool ParseSoar8(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	bool ParseSoarNews(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	bool ParseSource(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	bool ParseSP(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	bool ParseStats(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	bool ParseStopSoar(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	bool ParseTime(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	bool ParseTimers(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	bool ParseTopD(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	bool ParseVerbose(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	bool ParseVersion(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	bool ParseWaitSNC(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	bool ParseWarnings(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	bool ParseWatch(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	bool ParseWatchWMEs(gSKI::IAgent* pAgent, std::vector<std::string>& argv);

	// Print callback events go here
	virtual void HandleEvent(egSKIEventId, gSKI::IAgent*, const char* msg) {
		// Simply append to message result
		CommandLineInterface::m_Result << msg;
	}

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
	int ParseLevelOptarg();
	int ParseLearningOptarg();
	bool CheckOptargRemoveOrZero();
	bool ProcessWatchLevelSettings(const int level, int& options, int& settings, int& wmeSetting, int& learnSetting);

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
	void HandleSourceError(int errorLine, const std::string& filename);

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
	void PrependArgTag(const char* pParam, const char* pType, const char* pValue);

	/*************************************************************
	* @brief 
	*************************************************************/
	void PrependArgTagFast(const char* pParam, const char* pType, const char* pValue);

	/************************************************************* 	 
	* @brief This is a utility function used by DoLS 	 
	*************************************************************/ 	 
	void PrintFilename(const std::string& name, bool isDirectory); 	 

	void AddListenerAndDisableCallbacks(gSKI::IAgent* pAgent);
	void RemoveListenerAndEnableCallbacks(gSKI::IAgent* pAgent);

	bool SetError(cli::ErrorCode code);				// always returns false
	bool SetErrorDetail(const std::string detail);	// always returns false

	static std::ostringstream m_Result;	// Raw output from the command

	Aliases				m_Aliases;				// Alias management object
	GetOpt*				m_pGetOpt;				// Pointer to GetOpt utility class

	CommandMap			m_CommandMap;			// Mapping of command names to function pointers

	gSKI::IKernel*		m_pKernel;				// Pointer to the current gSKI kernel
	sml::KernelSML*		m_pKernelSML;
	gSKI::Version		m_KernelVersion;		// Kernel version number

	bool				m_RawOutput;			// True if we want string output.
	std::string			m_HomeDirectory;		// The initial working directory, server side
	bool				m_QuitCalled;			// True after DoQuit is called
	StringStack			m_DirectoryStack;		// Directory stack for pushd/popd
	ElementXMLList		m_ResponseTags;			// List of tags for the response.

	bool				m_SourceError;			// Used to control debug printing for source command errors
	std::string			m_SourceErrorDetail;	// Used for detailed source error output
	int					m_SourceDepth;			// Depth of source command calls.
	int					m_SourceDirDepth;		// Depth of directory stack since source command, used to return to the dir that source was issued in.

	cli::ErrorCode		m_LastError;			// Last error code (see cli_CLIError.h)
	std::string			m_LastErrorDetail;		// Additional detail concerning the last error
	gSKI::Error*		m_pgSKIError;			// gSKI error output from calls made to process the command

	std::string			m_LogFilename;			// Used for logging to a file.
	std::ofstream*		m_pLogFile;				// The log file stream

};

} // namespace cli

#endif //COMMAND_LINE_INTERFACE_H
