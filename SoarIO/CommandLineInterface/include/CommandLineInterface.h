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

// Local includes
#include "commanddata.h"
#include "getopt.h"

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
}

namespace cli {

// Forward declarations
class CommandLineInterface;

// We need a comparator to make the map we're about to define work with char*
struct strCompareCommand
{
	bool operator()(const char* s1, const char* s2) const
	{
		return std::strcmp(s1, s2) < 0;
	}
};

// Define the CommandFunction which we'll call to process commands
typedef bool (CommandLineInterface::*CommandFunction)(int argc, char** argv);

// Used to store a map from command name to function handler for that command
typedef std::map<char const*, CommandFunction, strCompareCommand>	CommandMap;
typedef CommandMap::iterator										CommandMapIter;
typedef CommandMap::const_iterator									CommandMapConstIter;

// Define the stack for pushd/popd
typedef std::stack<std::string> StringStack;
 
class CommandLineInterface
{
public:

	// Simple constructor
	CommandLineInterface();

	/*************************************************************
	* @brief Process a command.  Give it a command line and it will parse
	*		 and execute the command using gSKI or system calls.
	*************************************************************/
	bool DoCommand(gSKI::IAgent* pAgent, const char* pCommandLine, sml::ElementXML* pResponse, gSKI::Error* pError);

	// Template for new commands:
	///*************************************************************
	//* @brief 
	//*************************************************************/
	//bool Parse(int argc, char** argv);
	///*************************************************************
	//* @brief 
	//*************************************************************/
	//bool Do();

	bool IsQuitCalled() { return m_QuitCalled; }

	/*************************************************************
	* @brief add-wme command, see command line spec document for details
	*************************************************************/
	bool ParseAddWME(int argc, char** argv);
	/*************************************************************
	* @brief 
	*************************************************************/
	bool DoAddWME();

	/*************************************************************
	* @brief cd command, see command line spec document for details
	*************************************************************/
	bool ParseCD(int argc, char** argv);
	/*************************************************************
	* @brief Change the current working directory.  If null is passed
	*		 the current working directory is changed to the home directory.
	*		 The home directory is defined as the initial working directory.
	*************************************************************/
	bool DoCD(const char* directory = 0);

	/*************************************************************
	* @brief echo command, see command line spec document for details
	*************************************************************/
	bool ParseEcho(int argc, char** argv);
	/*************************************************************
	* @brief Simply concatenate all arguments, adding them to the result.
	*************************************************************/
	bool DoEcho(int argc, char** argv);

	/*************************************************************
	* @brief excise command, see command line spec document for details
	*************************************************************/
	bool ParseExcise(int argc, char** argv);
	/*************************************************************
	* @brief See CommandData.h for the list of flags for the options
	*		 parameter.  If there are specific productions to excise,
	*		 they are passed as an array of character strings with their
	*		 number indicated by production count.
	*************************************************************/
	bool DoExcise(const unsigned short options, int productionCount = 0, char** productions = 0);

	/*************************************************************
	* @brief init-soar command, see command line spec document for details
	*************************************************************/
	bool ParseInitSoar(int argc, char** argv);
	/*************************************************************
	* @brief Reinitializes the current agent.  No arguments necessary.
	*		 The agent pointer member must be valid.
	*************************************************************/
	bool DoInitSoar();

	/*************************************************************
	* @brief learn command, see command line spec document for details
	*************************************************************/
	bool ParseLearn(int argc, char** argv);
	/*************************************************************
	* @brief See CommandData.h for the list of flags used in the options
	*		 parameter.  Passing no options simply prints current learn
	*		 settings.
	*************************************************************/
	bool DoLearn(const unsigned short options = 0);

	/*************************************************************
	* @brief ls/dir command, see command line spec document for details
	*************************************************************/
	bool ParseLS(int argc, char** argv);
	/*************************************************************
	* @brief Lists current working directory, no arguments necessary.
	*************************************************************/
	bool DoLS();

	/*************************************************************
	* @brief multi-attributes command, see command line spec document
	*		 for details.
	*************************************************************/
	bool ParseMultiAttributes(int argc, char** argv);
	/*************************************************************
	* @brief Two optional arguments, attribute and n.  If no arguments,
	*		 prints current settings.
	*************************************************************/
	bool DoMultiAttributes(const char* attribute = 0, int n = 0);

	/*************************************************************
	* @brief popd command, see command line spec document for details
	*************************************************************/
	bool ParsePopD(int argc, char** argv);
	/*************************************************************
	* @brief No arguments, pops a directory off the directory stack
	*		 and changes to it.
	*************************************************************/
	bool DoPopD();

	/*************************************************************
	* @brief print command, see command line spec document for details
	*************************************************************/
	bool ParsePrint(int argc, char** argv);
	/*************************************************************
	* @brief 
	*************************************************************/
	bool DoPrint(const unsigned short options, int depth, const char* pArg);

	/*************************************************************
	* @brief 
	*************************************************************/
	bool ParsePushD(int argc, char** argv);
	/*************************************************************
	* @brief 
	*************************************************************/
	bool DoPushD(const char* directory);

	/*************************************************************
	* @brief pwd command, see command line spec document for details
	*************************************************************/
	bool ParsePWD(int argc, char** argv);
	/*************************************************************
	* @brief 
	*************************************************************/
	bool DoPWD();

	/*************************************************************
	* @brief exit/quit command, see command line spec document for details
	*************************************************************/
	bool ParseQuit(int argc, char** argv);
	/*************************************************************
	* @brief 
	*************************************************************/
	bool DoQuit();

	/*************************************************************
	* @brief run command, see command line spec document for details
	*************************************************************/
	bool ParseRun(int argc, char** argv);
	/*************************************************************
	* @brief 
	*************************************************************/
	bool DoRun(const unsigned short options, int count);

	/*************************************************************
	* @brief source command, see command line spec document for details
	*************************************************************/
	bool ParseSource(int argc, char** argv);
	/*************************************************************
	* @brief 
	*************************************************************/
	bool DoSource(const char* filename);

	/*************************************************************
	* @brief sp command, see command line spec document for details
	*************************************************************/
	bool ParseSP(int argc, char** argv);
	/*************************************************************
	* @brief 
	*************************************************************/
	bool DoSP(const char* production);

	/*************************************************************
	* @brief stop-soar command, see command line spec document for details
	*************************************************************/
	bool ParseStopSoar(int argc, char** argv);
	/*************************************************************
	* @brief 
	*************************************************************/
	bool DoStopSoar(bool self, char const* reasonForStopping);

	/*************************************************************
	* @brief 
	*************************************************************/
	bool ParseTime(int argc, char** argv);
	/*************************************************************
	* @brief 
	*************************************************************/
	bool DoTime(int argc, char** argv);

	/*************************************************************
	* @brief watch command, see command line spec document for details
	*************************************************************/
	bool ParseWatch(int argc, char** argv);
	/*************************************************************
	* @brief 
	*************************************************************/
	bool DoWatch();

	/*************************************************************
	* @brief watch-wmes command, see command line spec document for details
	*************************************************************/
	bool ParseWatchWMEs(int argc, char** argv);
	/*************************************************************
	* @brief 
	*************************************************************/
	bool DoWatchWMEs();

protected:

	// A class containing the constants used by CommandLineInterface
	class CLIConstants
	{
	public:
		static char const* kCLISyntaxError;

		static char const* kCLIAddWME;
		static char const* kCLICD;
		static char const* kCLIDir;
		static char const* kCLIEcho;
		static char const* kCLIExcise;
		static char const* kCLIExit;
		static char const* kCLIInitSoar;
		static char const* kCLILearn;
		static char const* kCLILS;
		static char const* kCLIMultiAttributes;
		static char const* kCLIPopD;
		static char const* kCLIPrint;
		static char const* kCLIPushD;
		static char const* kCLIPWD;
		static char const* kCLIQuit;
		static char const* kCLIRun;
		static char const* kCLISource;
		static char const* kCLISP;
		static char const* kCLIStopSoar;
		static char const* kCLITime;
		static char const* kCLIWatch;
		static char const* kCLIWatchWMEs;

		static char const* kCLIAddWMEUsage;
		static char const* kCLICDUsage;
		static char const* kCLIEchoUsage;
		static char const* kCLIExciseUsage;
		static char const* kCLIInitSoarUsage;
		static char const* kCLILearnUsage;
		static char const* kCLILSUsage;
		static char const* kCLIMultiAttributesUsage;
		static char const* kCLIPopDUsage;
		static char const* kCLIPrintUsage;
		static char const* kCLIPushDUsage;
		static char const* kCLIPWDUsage;
		static char const* kCLIQuitUsage;
		static char const* kCLIRunUsage;
		static char const* kCLISourceUsage;
		static char const* kCLISPUsage;
		static char const* kCLIStopSoarUsage;
		static char const* kCLITimeUsage;
		static char const* kCLIWatchUsage;
		static char const* kCLIWatchWMEsUsage;

	};

	// Print handler for gSKI print callbacks
	class PrintHandler : public gSKI::IPrintListener
	{
	public:
		PrintHandler() {}

		void SetCLI(CommandLineInterface* pCLI) { m_pCLI = pCLI; }
		virtual void HandleEvent(egSKIEventId, gSKI::IAgent*, const char* msg) {
			// Simply append to message result
			m_pCLI->AppendToResult(msg);
		}
	protected:
		CommandLineInterface*	m_pCLI;	// pointer to command line interface
	};

	friend class PrintHandler;	// Allows calling of AppendToResult
	friend class sml::KernelSML;		// Allows calling of SetKernel

	/*************************************************************
	* @brief 
	*************************************************************/
	void SetKernel(gSKI::IKernel* pKernel);

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
	bool DoCommandInternal(const char* commandLine);

	/*************************************************************
	* @brief A utility function, splits the command line into argument
	*		 tokens and stores them in the argumentVector string.
	*************************************************************/
	int Tokenize(const char* commandLine, std::vector<std::string>& argumentVector);

	/*************************************************************
	* @brief A utility function used in the constructor to build the mapping
	*		 of command names to function pointers.
	*************************************************************/
	void BuildCommandMap();

	/*************************************************************
	* @brief Standard parsing of -h and --help flags.  Returns
	*		 true if the flag is present.
	*************************************************************/
	bool CheckForHelp(int argc, char** argv);

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
	void SourceError(int errorLine, const char* filename);

	/*************************************************************
	* @brief 
	*************************************************************/
	bool IsInteger(const std::string& s);

	GetOpt			m_GetOpt;			// Pointer to GetOpt utility class
	CommandMap		m_CommandMap;		// Mapping of command names to function pointers
	gSKI::IKernel*	m_pKernel;			// Pointer to the current gSKI kernel
	gSKI::IAgent*	m_pAgent;			// Pointer to the gSKI agent the command is valid for
	std::string		m_Result;			// String output from the command
	gSKI::Error*	m_pError;			// gSKI error output from calls made to process the command
	std::string		m_HomeDirectory;	// The initial working directory, server side
	PrintHandler	m_PrintHandler;		// The print callback handler, used for catching kernel/gSKI output
	bool			m_QuitCalled;		// True after DoQuit is called
	StringStack		m_DirectoryStack;	// Directory stack for pushd/popd
	bool			m_SourceError;		// Used to control debug printing for source command errors
};

} // namespace cli

#endif //COMMAND_LINE_INTERFACE_H