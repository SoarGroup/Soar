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
#include <stack>
#include <fstream>

// Local includes
#include "cli_CommandData.h"
#include "cli_Constants.h"

// gSKI includes
#include "gSKI_Events.h"
#include "IgSKI_Iterator.h"

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
typedef bool (CommandLineInterface::*CommandFunction)(std::vector<std::string>& argv);

// Used to store a map from command name to function handler for that command
typedef std::map<std::string, CommandFunction>	CommandMap;
typedef CommandMap::iterator					CommandMapIter;
typedef CommandMap::const_iterator				CommandMapConstIter;

// Define the stack for pushd/popd
typedef std::stack<std::string> StringStack;
 
class CommandLineInterface
{
public:

	CommandLineInterface();
	~CommandLineInterface();

	/*************************************************************
	* @brief 
	*************************************************************/
	void SetKernel(gSKI::IKernel* pKernel);

	/*************************************************************
	* @brief Process a command.  Give it a command line and it will parse
	*		 and execute the command using gSKI or system calls.
	*************************************************************/
	bool DoCommand(sml::Connection* pConnection, gSKI::IAgent* pAgent, const char* pCommandLine, sml::ElementXML* pResponse, gSKI::Error* pError);

	/*************************************************************
	* @brief Process a command.  Give it a command line and it will parse
	*		 and execute the command using gSKI or system calls.
	*************************************************************/
	bool DoCommand(gSKI::IAgent* pAgent, const char* pCommandLine, char const* pResponse, gSKI::Error* pError);

	// Template for new commands:
	///*************************************************************
	//* @brief 
	//*************************************************************/
	//bool Parse(std::vector<std::string>& argv);
	///*************************************************************
	//* @brief 
	//*************************************************************/
	//bool Do();

	bool IsQuitCalled() { return m_QuitCalled; }

	/*************************************************************
	* @brief cd command, see command line spec document for details
	*************************************************************/
	bool ParseCD(std::vector<std::string>& argv);
	/*************************************************************
	* @brief Change the current working directory.  If an empty string is passed
	*		 the current working directory is changed to the home directory.
	*		 The home directory is defined as the initial working directory.
	*************************************************************/
	bool DoCD(std::string& directory);

	/*************************************************************
	* @brief echo command, see command line spec document for details
	*************************************************************/
	bool ParseEcho(std::vector<std::string>& argv);
	/*************************************************************
	* @brief Simply concatenate all arguments, adding them to the result.
	*************************************************************/
	bool DoEcho(std::vector<std::string>& argv);

	/*************************************************************
	* @brief excise command, see command line spec document for details
	*************************************************************/
	bool ParseExcise(std::vector<std::string>& argv);
	/*************************************************************
	* @brief See CommandData.h for the list of flags for the options
	*		 parameter.  If there are specific productions to excise,
	*		 they are passed as an array of character strings with their
	*		 number indicated by production count.
	*************************************************************/
	bool DoExcise(const unsigned short options, int optind, std::vector<std::string>& argv);

	/*************************************************************
	* @brief 
	*************************************************************/
	bool ParseHelp(std::vector<std::string>& argv);
	/*************************************************************
	* @brief 
	*************************************************************/
	bool DoHelp(const std::string& command);

	/*************************************************************
	* @brief init-soar command, see command line spec document for details
	*************************************************************/
	bool ParseInitSoar(std::vector<std::string>& argv);
	/*************************************************************
	* @brief Reinitializes the current agent.  No arguments necessary.
	*		 The agent pointer member must be valid.
	*************************************************************/
	bool DoInitSoar();

	/*************************************************************
	* @brief learn command, see command line spec document for details
	*************************************************************/
	bool ParseLearn(std::vector<std::string>& argv);
	/*************************************************************
	* @brief See CommandData.h for the list of flags used in the options
	*		 parameter.  Passing no options simply prints current learn
	*		 settings.
	*************************************************************/
	bool DoLearn(const unsigned short options = 0);

	/*************************************************************
	* @brief 
	*************************************************************/
	bool ParseLog(std::vector<std::string>& argv);
	/*************************************************************
	* @brief 
	*************************************************************/
	bool DoLog(bool option, const char* filename = 0);

	/*************************************************************
	* @brief ls/dir command, see command line spec document for details
	*************************************************************/
	bool ParseLS(std::vector<std::string>& argv);
	/*************************************************************
	* @brief Lists current working directory, no arguments necessary.
	*************************************************************/
	bool DoLS();

	/*************************************************************
	* @brief multi-attributes command, see command line spec document
	*		 for details.
	*************************************************************/
	bool ParseMultiAttributes(std::vector<std::string>& argv);
	/*************************************************************
	* @brief Two optional arguments, attribute and n.  If no arguments,
	*		 prints current settings.
	*************************************************************/
	bool DoMultiAttributes(const std::string& attribute, int n = 0);

	/*************************************************************
	* @brief popd command, see command line spec document for details
	*************************************************************/
	bool ParsePopD(std::vector<std::string>& argv);
	/*************************************************************
	* @brief No arguments, pops a directory off the directory stack
	*		 and changes to it.
	*************************************************************/
	bool DoPopD();

	/*************************************************************
	* @brief print command, see command line spec document for details
	*************************************************************/
	bool ParsePrint(std::vector<std::string>& argv);
	/*************************************************************
	* @brief 
	*************************************************************/
	bool DoPrint(const unsigned short options, int depth, const std::string& arg);

	/*************************************************************
	* @brief 
	*************************************************************/
	bool ParsePushD(std::vector<std::string>& argv);
	/*************************************************************
	* @brief 
	*************************************************************/
	bool DoPushD(std::string& directory);

	/*************************************************************
	* @brief pwd command, see command line spec document for details
	*************************************************************/
	bool ParsePWD(std::vector<std::string>& argv);
	/*************************************************************
	* @brief 
	*************************************************************/
	bool DoPWD();

	/*************************************************************
	* @brief exit/quit command, see command line spec document for details
	*************************************************************/
	bool ParseQuit(std::vector<std::string>& argv);
	/*************************************************************
	* @brief 
	*************************************************************/
	bool DoQuit();

	/*************************************************************
	* @brief run command, see command line spec document for details
	*************************************************************/
	bool ParseRun(std::vector<std::string>& argv);
	/*************************************************************
	* @brief 
	*************************************************************/
	bool DoRun(const unsigned short options, int count);

	/*************************************************************
	* @brief source command, see command line spec document for details
	*************************************************************/
	bool ParseSource(std::vector<std::string>& argv);
	/*************************************************************
	* @brief 
	*************************************************************/
	bool DoSource(const std::string& filename);

	/*************************************************************
	* @brief sp command, see command line spec document for details
	*************************************************************/
	bool ParseSP(std::vector<std::string>& argv);
	/*************************************************************
	* @brief 
	*************************************************************/
	bool DoSP(const std::string& production);

	/*************************************************************
	* @brief 
	*************************************************************/
	bool ParseStats(std::vector<std::string>& argv);
	/*************************************************************
	* @brief 
	*************************************************************/
	bool DoStats(const unsigned short options);

	/*************************************************************
	* @brief stop-soar command, see command line spec document for details
	*************************************************************/
	bool ParseStopSoar(std::vector<std::string>& argv);
	/*************************************************************
	* @brief 
	*************************************************************/
	bool DoStopSoar(bool self, const std::string& reasonForStopping);

	/*************************************************************
	* @brief 
	*************************************************************/
	bool ParseTime(std::vector<std::string>& argv);
	/*************************************************************
	* @brief 
	*************************************************************/
	bool DoTime(std::vector<std::string>& argv);

	/*************************************************************
	* @brief watch command, see command line spec document for details
	*************************************************************/
	bool ParseWatch(std::vector<std::string>& argv);
	/*************************************************************
	* @brief 
	*************************************************************/
	bool DoWatch(const unsigned int options, unsigned int values);

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
			m_pCLI->m_pLogFile->write(msg, (std::streamsize)strlen(msg));
		}
	};

	friend class PrintHandler;			// Allows calling of AppendToResult and log writing
	friend class sml::KernelSML;		// Allows calling of SetKernel

	/*************************************************************
	* @brief Currently used as an output hack, the print handler calls 
	*		 this to add output to the result that gets sent back upon
	*		 completion of a command.
	*************************************************************/
	void AppendToResult(const char* pMessage);

	/*************************************************************
	* @brief Does the bulk of command parsing and chooses what function
	*		 to call to process the command.  DoCommand mainly does
	*		 SML stuff.
	*************************************************************/
	bool DoCommandInternal(const std::string& commandLine);
	bool DoCommandInternal(std::vector<std::string>& argv);

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
	void ExciseInternal(gSKI::tIProductionIterator* pProdIter);

	/*************************************************************
	* @brief 
	*************************************************************/
	void HandleSourceError(int errorLine, const std::string& filename);

	/*************************************************************
	* @brief 
	*************************************************************/
	bool IsInteger(const std::string& s);

	/*************************************************************
	* @brief 
	*************************************************************/
	bool HandleSyntaxError(const char* command, const char* details = 0);

	/*************************************************************
	* @brief 
	*************************************************************/
	bool RequireAgent();

	/*************************************************************
	* @brief 
	*************************************************************/
	bool HandleGetOptError(char option);

	/*************************************************************
	* @brief 
	*************************************************************/
	bool WatchArg(unsigned int& values, const unsigned int option, const char* arg);
	bool WatchArg(unsigned int& values, const unsigned int option, int argInt);

	Constants			m_Constants;			// Pointer to constants management object
	GetOpt*				m_pGetOpt;				// Pointer to GetOpt utility class
	CommandMap			m_CommandMap;			// Mapping of command names to function pointers
	gSKI::IKernel*		m_pKernel;				// Pointer to the current gSKI kernel
	gSKI::IAgent*		m_pAgent;				// Pointer to the gSKI agent the command is valid for
	std::string			m_Result;				// String output from the command
	gSKI::Error*		m_pError;				// gSKI error output from calls made to process the command
	std::string			m_HomeDirectory;		// The initial working directory, server side
	ResultPrintHandler	m_ResultPrintHandler;	// The print callback handler, used for catching kernel/gSKI output and writing it to result
	LogPrintHandler		m_LogPrintHandler;		// The print callback handler, used for catching kernel/gSKI output and writing it to a log
	bool				m_QuitCalled;			// True after DoQuit is called
	StringStack			m_DirectoryStack;		// Directory stack for pushd/popd
	bool				m_SourceError;			// Used to control debug printing for source command errors
	int					m_SourceDepth;			// Depth of source command calls.
	int					m_SourceDirDepth;		// Depth of directory stack since source command.
	std::string			m_LogFilename;			// Used for logging to a file.
	std::ofstream*		m_pLogFile;				// The log file stream
	bool				m_CriticalError;		// True if DoCommand should return false.
};

} // namespace cli

#endif //COMMAND_LINE_INTERFACE_H