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
#include <bitset>
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
#include "../../gSKI/src/gSKI_Error.h"	// this is rediculous
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

// Define bitsets for various commands
typedef std::bitset<EXCISE_NUM_OPTIONS> ExciseBitset;
typedef std::bitset<LEARN_NUM_OPTIONS> LearnBitset;
typedef std::bitset<MEMORIES_NUM_OPTIONS> MemoriesBitset;
typedef std::bitset<PRINT_NUM_OPTIONS> PrintBitset;
typedef std::bitset<PRODUCTION_FIND_NUM_OPTIONS> ProductionFindBitset;
typedef std::bitset<RUN_NUM_OPTIONS> RunBitset;
typedef std::bitset<STATS_NUM_OPTIONS> StatsBitset;
typedef std::bitset<WATCH_NUM_OPTIONS> WatchBitset;
typedef std::bitset<WATCH_WMES_TYPE_NUM_OPTIONS> WatchWMEsTypeBitset;

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
	* @brief Set the output style to raw or structured.
	* @param rawOutput Set true for raw (string) output, false for structured output
	*************************************************************/
	EXPORT void SetRawOutput(bool rawOutput) { m_RawOutput = rawOutput; }

	/*************************************************************
	* @brief Process a command.  Give it a command line and it will parse
	*		 and execute the command using gSKI or system calls.
	* @param pConnection The connection, for communication to the client
	* @param pAgent The pointer to the gSKI agent interface
	* @param pCommandLine The command line string, arguments separated by spaces
	* @param pResponse Pointer to XML response object
	*************************************************************/
	EXPORT bool DoCommand(sml::Connection* pConnection, gSKI::IAgent* pAgent, const char* pCommandLine, sml::ElementXML* pResponse);

	/*************************************************************
	* @brief Takes a command line and expands any aliases and returns
	*		 the result.  The command is NOT executed.
	* @param pConnection The connection, for communication to the client
	* @param pCommandLine The command line string, arguments separated by spaces
	* @param pResponse Pointer to XML response object
	*************************************************************/
	EXPORT bool ExpandCommand(sml::Connection* pConnection, const char* pCommandLine, sml::ElementXML* pResponse);

	/*************************************************************
	* @brief add-wme command
	* @param pConnection Pointer to connection
	* @param pResponse Pointer to XML response
	* @param pAgent The pointer to the gSKI agent interface
	* @param id Id string for the new wme
	* @param attribute Attribute string for the new wme
	* @param value Value string for the new wme
	* @param acceptable True to give wme acceptable preference
	*************************************************************/
	EXPORT bool DoAddWME(sml::Connection* pConnection, sml::ElementXML* pResponse, gSKI::IAgent* pAgent, const std::string& id, const std::string& attribute, const std::string& value, bool acceptable);

	/*************************************************************
	* @brief alias command, see also home command
	* @param pConnection Pointer to connection
	* @param pResponse Pointer to XML response
	* @param command The alias to enable or disable, pass 0 to list aliases
	* @param pSubstitution Pass a pointer to a vector strings to enable a new 
	*        alias, pass 0 to disable a current alias
	*************************************************************/
	EXPORT bool DoAlias(sml::Connection* pConnection, sml::ElementXML* pResponse, const std::string* pCommand = 0, const std::vector<std::string>* pSubstitution = 0);

	/*************************************************************
	* @brief cd command
	* @param pConnection Pointer to connection
	* @param pResponse Pointer to XML response
	* @param pDirectory Pointer to the directory to pass in to. Pass null to return 
	*        to the initial (home) directory. 
	*************************************************************/
	EXPORT bool DoCD(sml::Connection* pConnection, sml::ElementXML* pResponse, const std::string* pDirectory = 0);

	/*************************************************************
	* @brief chunk-name-format command
	* @param pConnection Pointer to connection
	* @param pResponse Pointer to XML response
	* @param pAgent The pointer to the gSKI agent interface
	* @param pLongFormat Pointer to the new format type, true for long format, false 
	*        for short format, 0 (null) for query or no change
	* @param pCount Pointer to the new counter, non negative integer, 0 (null) for query
	* @param pPrefix Pointer to the new prefix, must not contain '*' character, 
	*        null for query
	*************************************************************/
	EXPORT bool DoChunkNameFormat(sml::Connection* pConnection, sml::ElementXML* pResponse, gSKI::IAgent* pAgent, const bool* pLongFormat = 0, const int* pCount = 0, const std::string* pPrefix = 0);

	/*************************************************************
	* @brief default-wme-depth command
	* @param pConnection Pointer to connection
	* @param pResponse Pointer to XML response
	* @param pAgent The pointer to the gSKI agent interface
	* @param pDepth The pointer to the new wme depth, a positive integer.  
	*        Pass 0 (null) pointer for query.
	*************************************************************/
	EXPORT bool DoDefaultWMEDepth(sml::Connection* pConnection, sml::ElementXML* pResponse, gSKI::IAgent* pAgent, const int* pDepth);

	/*************************************************************
	* @brief dirs command
	* @param pConnection Pointer to connection
	* @param pResponse Pointer to XML response
	*************************************************************/
	EXPORT bool DoDirs(sml::Connection* pConnection, sml::ElementXML* pResponse);

	/*************************************************************
	* @brief echo command
	* @param pConnection Pointer to connection
	* @param pResponse Pointer to XML response
	* @param argv The args to echo
	*************************************************************/
	EXPORT bool DoEcho(sml::Connection* pConnection, sml::ElementXML* pResponse, const std::vector<std::string>& argv);

	/*************************************************************
	* @brief excise command
	* @param pConnection Pointer to connection
	* @param pResponse Pointer to XML response
	* @param pAgent The pointer to the gSKI agent interface
	* @param options The various options set on the command line, see 
	*        cli_CommandData.h
	* @param pProduction A production to excise, optional
	*************************************************************/
	EXPORT bool DoExcise(sml::Connection* pConnection, sml::ElementXML* pResponse, gSKI::IAgent* pAgent, const ExciseBitset& options, const std::string* pProduction = 0);

	/*************************************************************
	* @brief explain-backtraces command
	* @param pConnection Pointer to connection
	* @param pResponse Pointer to XML response
	* @param pAgent The pointer to the gSKI agent interface
	* @param pProduction Pointer to involved production. Pass 0 (null) for 
	*        query
	* @param condition A number representing the condition number to explain, 
	*        0 for production name, -1 for full, 
	*        this argument ignored if pProduction is 0 (null)
	*************************************************************/
	EXPORT bool DoExplainBacktraces(sml::Connection* pConnection, sml::ElementXML* pResponse, gSKI::IAgent* pAgent, const std::string* pProduction = 0, const int condition = 0);

	/*************************************************************
	* @brief firing-counts command
	* @param pConnection Pointer to connection
	* @param pResponse Pointer to XML response
	* @param pAgent The pointer to the gSKI agent interface
	* @param numberToList The number of top-firing productions to list.  
	*        Use 0 to list those that haven't fired. -1 lists all
	* @param pProduction The specific production to list, pass 0 (null) to list 
	*        multiple productions
	*************************************************************/
	EXPORT bool DoFiringCounts(sml::Connection* pConnection, sml::ElementXML* pResponse, gSKI::IAgent* pAgent, const int numberToList = -1, const std::string* pProduction = 0);

	/*************************************************************
	* @brief gds-print command
	* @param pConnection Pointer to connection
	* @param pResponse Pointer to XML response
	* @param pAgent The pointer to the gSKI agent interface
	*************************************************************/
	EXPORT bool DoGDSPrint(sml::Connection* pConnection, sml::ElementXML* pResponse, gSKI::IAgent* pAgent);

	/*************************************************************
	* @brief help command
	* @param pConnection Pointer to connection
	* @param pResponse Pointer to XML response
	* @param pCommand The command to get help on, pass 0 (null) for a list of commands
	*************************************************************/
	EXPORT bool DoHelp(sml::Connection* pConnection, sml::ElementXML* pResponse, const std::string* pCommand = 0);

	/*************************************************************
	* @brief helpex command
	* @param pConnection Pointer to connection
	* @param pResponse Pointer to XML response
	* @param command The command to get extended help on
	*************************************************************/
	EXPORT bool DoHelpEx(sml::Connection* pConnection, sml::ElementXML* pResponse, const std::string& command);

	/*************************************************************
	* @brief home command, loads aliases
	* @param pConnection Pointer to connection
	* @param pResponse Pointer to XML response
	* @param pDirectory The directory to change the cli's initial (home) directory 
	*        to, pass 0 (null) for current directory
	*************************************************************/
	EXPORT bool DoHome(sml::Connection* pConnection, sml::ElementXML* pResponse, const std::string* pDirectory = 0);

	/*************************************************************
	* @brief indifferent-selection command
	* @param pConnection Pointer to connection
	* @param pResponse Pointer to XML response
	* @param pAgent The pointer to the gSKI agent interface
	* @param mode What mode to set indifferent selection to, or query.  
	*        See eIndifferentMode
	*************************************************************/
	EXPORT bool DoIndifferentSelection(sml::Connection* pConnection, sml::ElementXML* pResponse, gSKI::IAgent* pAgent, eIndifferentMode mode);

	/*************************************************************
	* @brief init-soar command
	* @param pConnection Pointer to connection
	* @param pResponse Pointer to XML response
	* @param pAgent The pointer to the gSKI agent interface
	*************************************************************/
	EXPORT bool DoInitSoar(sml::Connection* pConnection, sml::ElementXML* pResponse, gSKI::IAgent* pAgent);

	/*************************************************************
	* @brief internal-symbols command
	* @param pConnection Pointer to connection
	* @param pResponse Pointer to XML response
	* @param pAgent The pointer to the gSKI agent interface
	*************************************************************/
	EXPORT bool DoInternalSymbols(sml::Connection* pConnection, sml::ElementXML* pResponse, gSKI::IAgent* pAgent);

	/*************************************************************
	* @brief learn command
	* @param pConnection Pointer to connection
	* @param pResponse Pointer to XML response
	* @param pAgent The pointer to the gSKI agent interface
	* @param options The various options set on the command line, 
	*        see cli_CommandData.h
	*************************************************************/
	EXPORT bool DoLearn(sml::Connection* pConnection, sml::ElementXML* pResponse, gSKI::IAgent* pAgent, const LearnBitset& options);

	/*************************************************************
	* @brief log command
	* @param pConnection Pointer to connection
	* @param pResponse Pointer to XML response
	* @param pAgent The pointer to the gSKI agent interface
	* @param mode The mode for the log command, see cli_CommandData.h
	* @param pFilename The log filename, pass 0 (null) if not applicable to mode
	* @param pToAdd The string to add to the log, pass 0 (null) if not applicable to mode
	*************************************************************/
	EXPORT bool DoLog(sml::Connection* pConnection, sml::ElementXML* pResponse, gSKI::IAgent* pAgent, const eLogMode mode = LOG_QUERY, const std::string* pFilename = 0, const std::string* pToAdd = 0);

	/*************************************************************
	* @brief ls command
	* @param pConnection Pointer to connection
	* @param pResponse Pointer to XML response
	*************************************************************/
	EXPORT bool DoLS(sml::Connection* pConnection, sml::ElementXML* pResponse);

	/*************************************************************
	* @brief matches command
	* @param pConnection Pointer to connection
	* @param pResponse Pointer to XML response
	* @param pAgent The pointer to the gSKI agent interface
	* @param mode The mode for the command, see cli_CommandData.h
	* @param detail The WME detail, see cli_CommandData.h
	* @param pProduction The production, pass 0 (null) if not applicable to mode
	*************************************************************/
	EXPORT bool DoMatches(sml::Connection* pConnection, sml::ElementXML* pResponse, gSKI::IAgent* pAgent, const eMatchesMode mode, const eWMEDetail detail = WME_DETAIL_NONE, const std::string* pProduction = 0);

	/*************************************************************
	* @brief max-chunks command
	* @param pConnection Pointer to connection
	* @param pResponse Pointer to XML response
	* @param pAgent The pointer to the gSKI agent interface
	* @param n The new max chunks value, use 0 to query
	*************************************************************/
	EXPORT bool DoMaxChunks(sml::Connection* pConnection, sml::ElementXML* pResponse, gSKI::IAgent* pAgent, const int n = 0);

	/*************************************************************
	* @brief max-elaborations command
	* @param pConnection Pointer to connection
	* @param pResponse Pointer to XML response
	* @param pAgent The pointer to the gSKI agent interface
	* @param n The new max elaborations value, use 0 to query
	*************************************************************/
	EXPORT bool DoMaxElaborations(sml::Connection* pConnection, sml::ElementXML* pResponse, gSKI::IAgent* pAgent, const int n = 0);

	/*************************************************************
	* @brief max-nil-output-cycles command
	* @param pConnection Pointer to connection
	* @param pResponse Pointer to XML response
	* @param pAgent The pointer to the gSKI agent interface
	* @param n The new max nil output cycles value, use 0 to query
	*************************************************************/
	EXPORT bool DoMaxNilOutputCycles(sml::Connection* pConnection, sml::ElementXML* pResponse, gSKI::IAgent* pAgent, const int n);

	/*************************************************************
	* @brief memories command
	* @param pConnection Pointer to connection
	* @param pResponse Pointer to XML response
	* @param pAgent The pointer to the gSKI agent interface
	* @param options Options for the memories flag, see cli_CommandData.h
	* @param n number of productions to print sorted by most memory use, use 0 for all
	* @param pProduction specific production to print, ignored if any 
	*        options are set, pass 0 (null) if not applicable
	*************************************************************/
	EXPORT bool DoMemories(sml::Connection* pConnection, sml::ElementXML* pResponse, gSKI::IAgent* pAgent, const MemoriesBitset options, int n = 0, const std::string* pProduction = 0);

	/*************************************************************
	* @brief multi-attributes command
	* @param pConnection Pointer to connection
	* @param pResponse Pointer to XML response
	* @param pAgent The pointer to the gSKI agent interface
	* @param pAttribute The attribute, pass 0 (null) for query
	* @param n The count, pass 0 (null) for query if pAttribute is also null, 
	*        otherwise this will default to 10
	*************************************************************/
	EXPORT bool DoMultiAttributes(sml::Connection* pConnection, sml::ElementXML* pResponse, gSKI::IAgent* pAgent, const std::string* pAttribute = 0, int n = 0);

	/*************************************************************
	* @brief numeric-indifferent mode command
	* @param pConnection Pointer to connection
	* @param pResponse Pointer to XML response
	* @param pAgent The pointer to the gSKI agent interface
	* @param mode The mode for this command, see cli_CommandData.h
	*************************************************************/
	EXPORT bool DoNumericIndifferentMode(sml::Connection* pConnection, sml::ElementXML* pResponse, gSKI::IAgent* pAgent, const eNumericIndifferentMode mode);

	/*************************************************************
	* @brief o-support-mode command
	* @param pConnection Pointer to connection
	* @param pResponse Pointer to XML response
	* @param pAgent The pointer to the gSKI agent interface
	* @param mode The new o-support mode.  Use -1 to query.
	*************************************************************/
	EXPORT bool DoOSupportMode(sml::Connection* pConnection, sml::ElementXML* pResponse, gSKI::IAgent* pAgent, int mode = -1);

	/*************************************************************
	* @brief popd command
	* @param pConnection Pointer to connection
	* @param pResponse Pointer to XML response
	*************************************************************/
	EXPORT bool DoPopD(sml::Connection* pConnection, sml::ElementXML* pResponse);

	/*************************************************************
	* @brief preferences command
	* @param pConnection Pointer to connection
	* @param pResponse Pointer to XML response
	* @param pAgent The pointer to the gSKI agent interface
	* @param detail The preferences detail level, see cli_CommandData.h
	* @param pId An existing soar identifier or 0 (null)
	* @param pAttribute An existing soar attribute of the specified identifier or 0 (null)
	*************************************************************/
	EXPORT bool DoPreferences(sml::Connection* pConnection, sml::ElementXML* pResponse, gSKI::IAgent* pAgent, const ePreferencesDetail detail, const std::string* pId = 0, const std::string* pAttribute = 0);

	/*************************************************************
	* @brief print command
	* @param pConnection Pointer to connection
	* @param pResponse Pointer to XML response
	* @param pAgent The pointer to the gSKI agent interface
	* @param options The options to the print command, see cli_CommandData.h
	* @param depth WME depth
	* @param pArg The identifier/timetag/pattern/production name to print, 
	*        or 0 (null) if not applicable
	*************************************************************/
	EXPORT bool DoPrint(sml::Connection* pConnection, sml::ElementXML* pResponse, gSKI::IAgent* pAgent, PrintBitset options, int depth, const std::string* pArg = 0);

	/*************************************************************
	* @brief production-find command
	* @param pConnection Pointer to connection
	* @param pResponse Pointer to XML response
	* @param pAgent The pointer to the gSKI agent interface
	* @param options The options to the command, see cli_CommandData.h
	* @param pattern Any pattern that can appear in productions.
	*************************************************************/
	EXPORT bool DoProductionFind(sml::Connection* pConnection, sml::ElementXML* pResponse, gSKI::IAgent* pAgent, const ProductionFindBitset& options, const std::string& pattern);

	/*************************************************************
	* @brief pushd command
	* @param pConnection Pointer to connection
	* @param pResponse Pointer to XML response
	* @param directory The directory to change to
	*************************************************************/
	EXPORT bool DoPushD(sml::Connection* pConnection, sml::ElementXML* pResponse, const std::string& directory);

	/*************************************************************
	* @brief pwatch command
	* @param pConnection Pointer to connection
	* @param pResponse Pointer to XML response
	* @param pAgent The pointer to the gSKI agent interface
	* @param query Pass true to query, all other args ignored
	* @param pProduction The production to watch or stop watching, pass 0 (null) 
	*        to disable watching of all productions (setting ignored)
	* @param setting True to watch the pProduction, false to stop watching it
	*************************************************************/
	EXPORT bool DoPWatch(sml::Connection* pConnection, sml::ElementXML* pResponse, gSKI::IAgent* pAgent, bool query = true, const std::string* pProduction = 0, bool setting = false);

	/*************************************************************
	* @brief pwd command
	* @param pConnection Pointer to connection
	* @param pResponse Pointer to XML response
	*************************************************************/
	EXPORT bool DoPWD(sml::Connection* pConnection, sml::ElementXML* pResponse);

	/*************************************************************
	* @brief quit command
	* @param pConnection Pointer to connection
	* @param pResponse Pointer to XML response
	*************************************************************/
	EXPORT bool DoQuit(sml::Connection* pConnection, sml::ElementXML* pResponse);

	/*************************************************************
	* @brief remove-wme command
	* @param pConnection Pointer to connection
	* @param pResponse Pointer to XML response
	* @param pAgent The pointer to the gSKI agent interface
	* @param timetag The timetag of the wme to remove
	*************************************************************/
	EXPORT bool DoRemoveWME(sml::Connection* pConnection, sml::ElementXML* pResponse, gSKI::IAgent*, int timetag);

	/*************************************************************
	* @brief rete-net command
	* @param pConnection Pointer to connection
	* @param pResponse Pointer to XML response
	* @param pAgent The pointer to the gSKI agent interface
	* @param save true to save, false to load
	* @param filename the rete-net file
	*************************************************************/
	EXPORT bool DoReteNet(sml::Connection* pConnection, sml::ElementXML* pResponse, gSKI::IAgent* pAgent, bool save, const std::string& filename);

	/*************************************************************
	* @brief run command
	* @param pConnection Pointer to connection
	* @param pResponse Pointer to XML response
	* @param pAgent The pointer to the gSKI agent interface
	* @param options Options for the run command, see cli_CommandData.h
	* @param count The count, units or applicability depends on options
	*************************************************************/
	EXPORT bool DoRun(sml::Connection* pConnection, sml::ElementXML* pResponse, gSKI::IAgent* pAgent, const RunBitset& options, int count = 0);

	/*************************************************************
	* @brief save-backtraces command
	* @param pConnection Pointer to connection
	* @param pResponse Pointer to XML response
	* @param pAgent The pointer to the gSKI agent interface
	* @param setting The new setting, pass 0 (null) for query
	*************************************************************/
	EXPORT bool DoSaveBacktraces(sml::Connection* pConnection, sml::ElementXML* pResponse, gSKI::IAgent* pAgent, bool* pSetting = 0);

	/*************************************************************
	* @brief soar8 command
	* @param pConnection Pointer to connection
	* @param pResponse Pointer to XML response
	* @param pSoar8 True to enable Soar 8, false for Soar 7
	*************************************************************/
	EXPORT bool DoSoar8(sml::Connection* pConnection, sml::ElementXML* pResponse, bool* pSoar8);

	/*************************************************************
	* @brief soarnews command
	* @param pConnection Pointer to connection
	* @param pResponse Pointer to XML response
	*************************************************************/
	EXPORT bool DoSoarNews(sml::Connection* pConnection, sml::ElementXML* pResponse);

	/*************************************************************
	* @brief source command
	* @param pConnection Pointer to connection
	* @param pResponse Pointer to XML response
	* @param pAgent The pointer to the gSKI agent interface
	* @param filename The file to source
	*************************************************************/
	EXPORT bool DoSource(sml::Connection* pConnection, sml::ElementXML* pResponse, gSKI::IAgent* pAgent, std::string filename);

	/*************************************************************
	* @brief sp command
	* @param pConnection Pointer to connection
	* @param pResponse Pointer to XML response
	* @param pAgent The pointer to the gSKI agent interface
	* @param production The production to add to working memory
	*************************************************************/
	EXPORT bool DoSP(sml::Connection* pConnection, sml::ElementXML* pResponse, gSKI::IAgent* pAgent, const std::string& production);

	/*************************************************************
	* @brief stats command
	* @param pConnection Pointer to connection
	* @param pResponse Pointer to XML response
	* @param pAgent The pointer to the gSKI agent interface
	* @param options The options for the stats command, see cli_CommandData.h
	*************************************************************/
	EXPORT bool DoStats(sml::Connection* pConnection, sml::ElementXML* pResponse, gSKI::IAgent* pAgent, const StatsBitset& options);

	/*************************************************************
	* @brief stop-soar command
	* @param pConnection Pointer to connection
	* @param pResponse Pointer to XML response
	* @param pAgent The pointer to the gSKI agent interface
	* @param self Stop the only pAgent (false means stop all agents in kernel)
	* @param reasonForStopping optional reason for stopping
	*************************************************************/
	EXPORT bool DoStopSoar(sml::Connection* pConnection, sml::ElementXML* pResponse, gSKI::IAgent* pAgent, bool self, const std::string* reasonForStopping = 0);

	/*************************************************************
	* @brief time command
	* @param pConnection Pointer to connection
	* @param pResponse Pointer to XML response
	* @param pAgent The pointer to the gSKI agent interface
	* @param argv The command line with the time arg removed
	*************************************************************/
	EXPORT bool DoTime(sml::Connection* pConnection, sml::ElementXML* pResponse, gSKI::IAgent* pAgent, std::vector<std::string>& argv);

	/*************************************************************
	* @brief timers command
	* @param pConnection Pointer to connection
	* @param pResponse Pointer to XML response
	* @param pAgent The pointer to the gSKI agent interface
	* @param pSetting The timers setting, true to turn on, false to turn off, 
	*        pass 0 (null) to query
	*************************************************************/
	EXPORT bool DoTimers(sml::Connection* pConnection, sml::ElementXML* pResponse, gSKI::IAgent* pAgent, bool* pSetting = 0);

	/*************************************************************
	* @brief verbose command
	* @param pConnection Pointer to connection
	* @param pResponse Pointer to XML response
	* @param pAgent The pointer to the gSKI agent interface
	* @param pSetting The verbose setting, true to turn on, false to turn off, 
	*        pass 0 (null) to query
	*************************************************************/
	EXPORT bool DoVerbose(sml::Connection* pConnection, sml::ElementXML* pResponse, gSKI::IAgent* pAgent, bool* pSetting = 0);

	/*************************************************************
	* @brief version command
	* @param pConnection Pointer to connection
	* @param pResponse Pointer to XML response
	*************************************************************/
	EXPORT bool DoVersion(sml::Connection* pConnection, sml::ElementXML* pResponse);

	/*************************************************************
	* @brief waitsnc command
	* @param pConnection Pointer to connection
	* @param pResponse Pointer to XML response
	* @param pAgent The pointer to the gSKI agent interface
	* @param pSetting The waitsnc setting, true to turn on, false to turn off, 
	*        pass 0 (null) to query
	*************************************************************/
	EXPORT bool DoWaitSNC(sml::Connection* pConnection, sml::ElementXML* pResponse, gSKI::IAgent* pAgent, bool* pSetting = 0);

	/*************************************************************
	* @brief warnings command
	* @param pConnection Pointer to connection
	* @param pResponse Pointer to XML response
	* @param pAgent The pointer to the gSKI agent interface
	* @param pSetting The warnings setting, true to turn on, false to turn off, 
	*        pass 0 (null) to query
	*************************************************************/
	EXPORT bool DoWarnings(sml::Connection* pConnection, sml::ElementXML* pResponse, gSKI::IAgent* pAgent, bool* pSetting = 0);

	/*************************************************************
	* @brief watch command
	* @param pConnection Pointer to connection
	* @param pResponse Pointer to XML response
	* @param pAgent The pointer to the gSKI agent interface
	* @param options Options for the watch command, see cli_CommandData.h
	* @param settings Settings for the watch command, if a flag (option) is set, its 
	*        setting is set using this (true/on or false/off)
	* @param wmeSetting Setting for wme detail, not binary so it has its own arg
	* @param learnSetting Setting for learn level, not binary so it has its own arg
	*************************************************************/
	EXPORT bool DoWatch(sml::Connection* pConnection, sml::ElementXML* pResponse, gSKI::IAgent* pAgent, const WatchBitset& options, const WatchBitset& settings, const int wmeSetting, const int learnSetting);

	/*************************************************************
	* @brief watch-wmes command
	* @param pConnection Pointer to connection
	* @param pResponse Pointer to XML response
	* @param pAgent The pointer to the gSKI agent interface
	*************************************************************/
	EXPORT bool DoWatchWMEs(sml::Connection* pConnection, sml::ElementXML* pResponse, gSKI::IAgent* pAgent, const eWatchWMEsMode mode, WatchWMEsTypeBitset type, const std::string* pIdString = 0, const std::string* pAttributeString = 0, const std::string* pValueString = 0);


protected:

	void GetLastResultSML(sml::Connection* pConnection, sml::ElementXML* pResponse);

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

	// The internal Parse functions follow
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
	bool ParseVerbose(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	bool ParseVersion(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	bool ParseWaitSNC(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	bool ParseWarnings(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	bool ParseWatch(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	bool ParseWatchWMEs(gSKI::IAgent* pAgent, std::vector<std::string>& argv);

	// the internal Do functions follow
	bool DoAddWME(gSKI::IAgent* pAgent, const std::string& id, const std::string& attribute, const std::string& value, bool acceptable);
	bool DoAlias(const std::string* pCommand = 0, const std::vector<std::string>* pSubstitution = 0);
	bool DoCD(const std::string* pDirectory = 0);
	bool DoChunkNameFormat(gSKI::IAgent* pAgent, const bool* pLongFormat = 0, const int* pCount = 0, const std::string* pPrefix = 0);
	bool DoDefaultWMEDepth(gSKI::IAgent* pAgent, const int* pDepth);
	bool DoDirs();
	bool DoEcho(const std::vector<std::string>& argv);
	bool DoExcise(gSKI::IAgent* pAgent, const ExciseBitset& options, const std::string* pProduction = 0);
	bool DoExplainBacktraces(gSKI::IAgent* pAgent, const std::string* pProduction = 0, const int condition = 0);
	bool DoFiringCounts(gSKI::IAgent* pAgent, const int numberToList = -1, const std::string* pProduction = 0);
	bool DoGDSPrint(gSKI::IAgent* pAgent);
	bool DoHelp(const std::string* pCommand = 0);
	bool DoHelpEx(const std::string& command);
	bool DoHome(const std::string* pDirectory = 0);
	bool DoIndifferentSelection(gSKI::IAgent* pAgent, eIndifferentMode mode);
	bool DoInitSoar(gSKI::IAgent* pAgent);
	bool DoInternalSymbols(gSKI::IAgent* pAgent);
	bool DoLearn(gSKI::IAgent* pAgent, const LearnBitset& options);
	bool DoLog(gSKI::IAgent* pAgent, const eLogMode mode = LOG_QUERY, const std::string* pFilename = 0, const std::string* pToAdd = 0);
	bool DoLS();
	bool DoMatches(gSKI::IAgent* pAgent, const eMatchesMode mode, const eWMEDetail detail = WME_DETAIL_NONE, const std::string* pProduction = 0);
	bool DoMaxChunks(gSKI::IAgent* pAgent, const int n = 0);
	bool DoMaxElaborations(gSKI::IAgent* pAgent, const int n = 0);
	bool DoMaxNilOutputCycles(gSKI::IAgent* pAgent, const int n);
	bool DoMemories(gSKI::IAgent* pAgent, const MemoriesBitset options, int n = 0, const std::string* pProduction = 0);
	bool DoMultiAttributes(gSKI::IAgent* pAgent, const std::string* pAttribute = 0, int n = 0);
	bool DoNumericIndifferentMode(gSKI::IAgent* pAgent, const eNumericIndifferentMode mode);
	bool DoOSupportMode(gSKI::IAgent* pAgent, int mode = -1);
	bool DoPopD();
	bool DoPreferences(gSKI::IAgent* pAgent, const ePreferencesDetail detail, const std::string* pId = 0, const std::string* pAttribute = 0);
	bool DoPrint(gSKI::IAgent* pAgent, PrintBitset options, int depth, const std::string* pArg = 0);
	bool DoProductionFind(gSKI::IAgent* pAgent, const ProductionFindBitset& options, const std::string& pattern);
	bool DoPushD(const std::string& directory);
	bool DoPWatch(gSKI::IAgent* pAgent, bool query = true, const std::string* pProduction = 0, bool setting = false);
	bool DoPWD();
	bool DoQuit();
	bool DoRemoveWME(gSKI::IAgent*, int timetag);
	bool DoReteNet(gSKI::IAgent* pAgent, bool save, const std::string& filename);
	bool DoRun(gSKI::IAgent* pAgent, const RunBitset& options, int count = 0);
	bool DoSaveBacktraces(gSKI::IAgent* pAgent, bool* pSetting = 0);
	bool DoSoar8(bool* pSoar8);
	bool DoSoarNews();
	bool DoSource(gSKI::IAgent* pAgent, std::string filename);
	bool DoSP(gSKI::IAgent* pAgent, const std::string& production);
	bool DoStats(gSKI::IAgent* pAgent, const StatsBitset& options);
	bool DoStopSoar(gSKI::IAgent* pAgent, bool self, const std::string* reasonForStopping = 0);
	bool DoTime(gSKI::IAgent* pAgent, std::vector<std::string>& argv);
	bool DoTimers(gSKI::IAgent* pAgent, bool* pSetting = 0);
	bool DoVerbose(gSKI::IAgent* pAgent, bool* pSetting = 0);
	bool DoVersion();
	bool DoWaitSNC(gSKI::IAgent* pAgent, bool* pSetting = 0);
	bool DoWarnings(gSKI::IAgent* pAgent, bool* pSetting = 0);
	bool DoWatch(gSKI::IAgent* pAgent, const WatchBitset& options, const WatchBitset& settings, const int wmeSetting, const int learnSetting);
	bool DoWatchWMEs(gSKI::IAgent* pAgent, const eWatchWMEsMode mode, WatchWMEsTypeBitset type, const std::string* pIdString = 0, const std::string* pAttributeString = 0, const std::string* pValueString = 0);

	// Print callback events go here
	virtual void HandleEvent(egSKIPrintEventId, gSKI::IAgent*, const char* msg) {
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
	bool ProcessWatchLevelSettings(const int level, WatchBitset& options, WatchBitset& settings, int& wmeSetting, int& learnSetting);

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

	void ResultToArgTag(); // clears result

	bool				m_Initialized;			// True if state has been cleared for a new command execution
	static std::ostringstream m_Result;			// Raw output from the command
	bool				m_RawOutput;			// True if we want string output.
	ElementXMLList		m_ResponseTags;			// List of tags for the response.
	bool				m_SourceError;			// Used to control debug printing for source command errors
	std::string			m_SourceErrorDetail;	// Used for detailed source error output
	int					m_SourceDepth;			// Depth of source command calls.
	int					m_SourceDirDepth;		// Depth of directory stack since source command, used to return to the dir that source was issued in.
	cli::ErrorCode		m_LastError;			// Last error code (see cli_CLIError.h)
	std::string			m_LastErrorDetail;		// Additional detail concerning the last error
	gSKI::Error			m_gSKIError;			// gSKI error output from calls made to process the command

	Aliases				m_Aliases;				// Alias management object
	GetOpt*				m_pGetOpt;				// Pointer to GetOpt utility class
	CommandMap			m_CommandMap;			// Mapping of command names to function pointers
	gSKI::IKernel*		m_pKernel;				// Pointer to the current gSKI kernel
	sml::KernelSML*		m_pKernelSML;
	gSKI::Version		m_KernelVersion;		// Kernel version number
	std::string			m_HomeDirectory;		// The initial working directory, server side
	StringStack			m_DirectoryStack;		// Directory stack for pushd/popd
	std::string			m_LogFilename;			// Used for logging to a file.
	std::ofstream*		m_pLogFile;				// The log file stream
};

} // namespace cli

#endif //COMMAND_LINE_INTERFACE_H
