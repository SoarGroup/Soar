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
	* @brief Process a command.  Give it a command line and it will parse
	*		 and execute the command using gSKI or system calls.
	* @param pConnection The connection, for communication to the client
	* @param pAgent The pointer to the gSKI agent interface
	* @param pCommandLine The command line string, arguments separated by spaces
	* @param pResponse Pointer to XML response object
	* @param rawOutput Set to true for human-readable raw output string, set to false for XML structured output
	* @param pError Pointer to the client-owned gSKI error object
	*************************************************************/
	EXPORT bool DoCommand(sml::Connection* pConnection, gSKI::IAgent* pAgent, const char* pCommandLine, sml::ElementXML* pResponse, bool rawOutput, gSKI::Error* pError);

	/*************************************************************
	* @brief Takes a command line and expands any aliases and returns
	*		 the result.  The command is NOT executed.
	* @param pConnection The connection, for communication to the client
	* @param pCommandLine The command line string, arguments separated by spaces
	* @param pResponse Pointer to XML response object
	* @param pError Pointer to the client-owned gSKI error object
	*************************************************************/
	EXPORT bool ExpandCommand(sml::Connection* pConnection, const char* pCommandLine, sml::ElementXML* pResponse, gSKI::Error* pError);

	/*************************************************************
	* @brief add-wme command
	* @param pAgent The pointer to the gSKI agent interface
	* @param id Id string for the new wme
	* @param attribute Attribute string for the new wme
	* @param value Value string for the new wme
	* @param acceptable True to give wme acceptable preference
	*************************************************************/
	EXPORT bool DoAddWME(gSKI::IAgent* pAgent, const std::string& id, const std::string& attribute, const std::string& value, bool acceptable);

	/*************************************************************
	* @brief alias command, see home command
	* @param command The alias to enable or disable, pass 0 to list aliases
	* @param pSubstitution Pass a pointer to a vector strings to enable a new alias, pass 0 to disable a current alias
	*************************************************************/
	EXPORT bool DoAlias(const std::string* pCommand = 0, const std::vector<std::string>* pSubstitution = 0);

	/*************************************************************
	* @brief cd command
	* @param pDirectory Pointer to the directory to pass in to.  
	*        Pass null to return to the initial (home) directory. 
	*************************************************************/
	EXPORT bool DoCD(const std::string* pDirectory = 0);

	/*************************************************************
	* @brief chunk-name-format command
	* @param pAgent The pointer to the gSKI agent interface
	* @param pLongFormat Pointer to the new format type, true for long format, false for short format, 0 (null) for query or no change
	* @param pCount Pointer to the new counter, non negative integer, 0 (null) for query
	* @param pPrefix Pointer to the new prefix, must not contain '*' character, null for query
	*************************************************************/
	EXPORT bool DoChunkNameFormat(gSKI::IAgent* pAgent, const bool* pLongFormat = 0, const int* pCount = 0, const std::string* pPrefix = 0);

	/*************************************************************
	* @brief default-wme-depth command
	* @param pAgent The pointer to the gSKI agent interface
	* @param pDepth The pointer to the new wme depth, a positive integer.  Pass 0 (null) pointer for query.
	*************************************************************/
	EXPORT bool DoDefaultWMEDepth(gSKI::IAgent* pAgent, const int* pDepth);

	/*************************************************************
	* @brief dirs command
	*************************************************************/
	EXPORT bool DoDirs();

	/*************************************************************
	* @brief echo command
	* @param argv The args to echo
	*************************************************************/
	EXPORT bool DoEcho(const std::vector<std::string>& argv);

	/*************************************************************
	* @brief excise command
	* @param pAgent The pointer to the gSKI agent interface
	* @param options The various options set on the command line, see cli_CommandData.h
	* @param pProduction A production to excise, optional
	*************************************************************/
	EXPORT bool DoExcise(gSKI::IAgent* pAgent, const ExciseBitset& options, const std::string* pProduction = 0);

	/*************************************************************
	* @brief explain-backtraces command
	* @param pAgent The pointer to the gSKI agent interface
	* @param pProduction Pointer to involved production. Pass 0 (null) for query
	* @param condition A number representing the condition number to explain, 0 for production name, -1 for full, 
	*        this argument ignored if pProduction is 0 (null)
	*************************************************************/
	EXPORT bool DoExplainBacktraces(gSKI::IAgent* pAgent, const std::string* pProduction = 0, const int condition = 0);

	/*************************************************************
	* @brief firing-counts command
	* @param pAgent The pointer to the gSKI agent interface
	* @param numberToList The number of top-firing productions to list.  Use 0 to list those that haven't fired. -1 lists all
	* @param pProduction The specific production to list, pass 0 (null) to list multiple productions
	*************************************************************/
	EXPORT bool DoFiringCounts(gSKI::IAgent* pAgent, const int numberToList = -1, const std::string* pProduction = 0);

	/*************************************************************
	* @brief gds-print command
	* @param pAgent The pointer to the gSKI agent interface
	*************************************************************/
	EXPORT bool DoGDSPrint(gSKI::IAgent* pAgent);

	/*************************************************************
	* @brief help command
	* @param pCommand The command to get help on, pass 0 (null) for a list of commands
	*************************************************************/
	EXPORT bool DoHelp(const std::string* pCommand = 0);

	/*************************************************************
	* @brief helpex command
	* @param command The command to get extended help on
	*************************************************************/
	EXPORT bool DoHelpEx(const std::string& command);

	/*************************************************************
	* @brief home command, loads aliases
	* @param pDirectory The directory to change the cli's initial (home) directory to, pass 0 (null) for current directory
	*************************************************************/
	EXPORT bool DoHome(const std::string* pDirectory = 0);

	/*************************************************************
	* @brief indifferent-selection command
	* @param pAgent The pointer to the gSKI agent interface
	* @param mode What mode to set indifferent selection to, or query.  See eIndifferentMode
	*************************************************************/
	EXPORT bool DoIndifferentSelection(gSKI::IAgent* pAgent, eIndifferentMode mode);

	/*************************************************************
	* @brief init-soar command
	* @param pAgent The pointer to the gSKI agent interface
	*************************************************************/
	EXPORT bool DoInitSoar(gSKI::IAgent* pAgent);

	/*************************************************************
	* @brief internal-symbols command
	* @param pAgent The pointer to the gSKI agent interface
	*************************************************************/
	EXPORT bool DoInternalSymbols(gSKI::IAgent* pAgent);

	/*************************************************************
	* @brief learn command
	* @param pAgent The pointer to the gSKI agent interface
	* @param options The various options set on the command line, see cli_CommandData.h
	*************************************************************/
	EXPORT bool DoLearn(gSKI::IAgent* pAgent, const LearnBitset& options);

	/*************************************************************
	* @brief log command
	* @param pAgent The pointer to the gSKI agent interface
	* @param mode The mode for the log command, see cli_CommandData.h
	* @param pFilename The log filename, pass 0 (null) if not applicable to mode
	* @param pToAdd The string to add to the log, pass 0 (null) if not applicable to mode
	*************************************************************/
	EXPORT bool DoLog(gSKI::IAgent* pAgent, const eLogMode mode = LOG_QUERY, const std::string* pFilename = 0, const std::string* pToAdd = 0);

	/*************************************************************
	* @brief ls command
	*************************************************************/
	EXPORT bool DoLS();

	/*************************************************************
	* @brief matches command
	* @param pAgent The pointer to the gSKI agent interface
	* @param mode The mode for the command, see cli_CommandData.h
	* @param detail The WME detail, see cli_CommandData.h
	* @param pProduction The production, pass 0 (null) if not applicable to mode
	*************************************************************/
	EXPORT bool DoMatches(gSKI::IAgent* pAgent, const eMatchesMode mode, const eWMEDetail detail = WME_DETAIL_NONE, const std::string* pProduction = 0);

	/*************************************************************
	* @brief max-chunks command
	* @param pAgent The pointer to the gSKI agent interface
	* @param n The new max chunks value, use 0 to query
	*************************************************************/
	EXPORT bool DoMaxChunks(gSKI::IAgent* pAgent, const int n = 0);

	/*************************************************************
	* @brief max-elaborations command
	* @param pAgent The pointer to the gSKI agent interface
	* @param n The new max elaborations value, use 0 to query
	*************************************************************/
	EXPORT bool DoMaxElaborations(gSKI::IAgent* pAgent, const int n = 0);

	/*************************************************************
	* @brief max-nil-output-cycles command
	* @param pAgent The pointer to the gSKI agent interface
	* @param n The new max nil output cycles value, use 0 to query
	*************************************************************/
	EXPORT bool DoMaxNilOutputCycles(gSKI::IAgent* pAgent, const int n);

	/*************************************************************
	* @brief memories command
	* @param pAgent The pointer to the gSKI agent interface
	* @param options Options for the memories flag, see cli_CommandData.h
	* @param n number of productions to print sorted by most memory use, use 0 for all
	* @param pProduction specific production to print, ignored if any options are set, pass 0 (null) if not applicable
	*************************************************************/
	EXPORT bool DoMemories(gSKI::IAgent* pAgent, const MemoriesBitset options, int n = 0, const std::string* pProduction = 0);

	/*************************************************************
	* @brief multi-attributes command
	* @param pAgent The pointer to the gSKI agent interface
	* @param pAttribute The attribute, pass 0 (null) for query
	* @param n The count, pass 0 (null) for query if pAttribute is also null, otherwise this will default to 10
	*************************************************************/
	EXPORT bool DoMultiAttributes(gSKI::IAgent* pAgent, const std::string* pAttribute = 0, int n = 0);

	/*************************************************************
	* @brief numeric-indifferent mode command
	* @param pAgent The pointer to the gSKI agent interface
	* @param mode The mode for this command, see cli_CommandData.h
	*************************************************************/
	EXPORT bool DoNumericIndifferentMode(gSKI::IAgent* pAgent, const eNumericIndifferentMode mode);

	/*************************************************************
	* @brief o-support-mode command
	* @param pAgent The pointer to the gSKI agent interface
	* @param mode The new o-support mode.  Use -1 to query.
	*************************************************************/
	EXPORT bool DoOSupportMode(gSKI::IAgent* pAgent, int mode = -1);

	/*************************************************************
	* @brief popd command
	*************************************************************/
	EXPORT bool DoPopD();

	/*************************************************************
	* @brief preferences command
	* @param pAgent The pointer to the gSKI agent interface
	* @param detail The preferences detail level, see cli_CommandData.h
	* @param pId An existing soar identifier or 0 (null)
	* @param pAttribute An existing soar attribute of the specified identifier or 0 (null)
	*************************************************************/
	EXPORT bool DoPreferences(gSKI::IAgent* pAgent, const ePreferencesDetail detail, const std::string* pId = 0, const std::string* pAttribute = 0);

	/*************************************************************
	* @brief print command
	* @param pAgent The pointer to the gSKI agent interface
	* @param options The options to the print command, see cli_CommandData.h
	* @param depth WME depth
	* @param pArg The identifier/timetag/pattern/production name to print, or 0 (null) if not applicable
	*************************************************************/
	EXPORT bool DoPrint(gSKI::IAgent* pAgent, const PrintBitset& options, int depth, const std::string* pArg = 0);

	/*************************************************************
	* @brief production-find command
	* @param pAgent The pointer to the gSKI agent interface
	* @param options The options to the command, see cli_CommandData.h
	* @param pattern Any pattern that can appear in productions.
	*************************************************************/
	EXPORT bool DoProductionFind(gSKI::IAgent* pAgent, const ProductionFindBitset& options, const std::string& pattern);

	/*************************************************************
	* @brief pushd command
	* @param directory The directory to change to
	*************************************************************/
	EXPORT bool DoPushD(const std::string& directory);

	/*************************************************************
	* @brief pwatch command
	* @param pAgent The pointer to the gSKI agent interface
	* @param query Pass true to query, all other args ignored
	* @param pProduction The production to watch or stop watching, pass 0 (null) to disable watching of all productions (setting ignored)
	* @param setting True to watch the pProduction, false to stop watching it
	*************************************************************/
	EXPORT bool DoPWatch(gSKI::IAgent* pAgent, bool query = true, const std::string* pProduction = 0, bool setting = false);

	/*************************************************************
	* @brief pwd command
	*************************************************************/
	EXPORT bool DoPWD();

	/*************************************************************
	* @brief quit command
	*************************************************************/
	EXPORT bool DoQuit();

	/*************************************************************
	* @brief remove-wme command
	* @param pAgent The pointer to the gSKI agent interface
	* @param timetag The timetag of the wme to remove
	*************************************************************/
	EXPORT bool DoRemoveWME(gSKI::IAgent*, int timetag);

	/*************************************************************
	* @brief rete-net command
	* @param pAgent The pointer to the gSKI agent interface
	* @param save true to save, false to load
	* @param filename the rete-net file
	*************************************************************/
	EXPORT bool DoReteNet(gSKI::IAgent* pAgent, bool save, const std::string& filename);

	/*************************************************************
	* @brief run command
	* @param pAgent The pointer to the gSKI agent interface
	* @param options Options for the run command, see cli_CommandData.h
	* @param count The count, units or applicability depends on options
	*************************************************************/
	EXPORT bool DoRun(gSKI::IAgent* pAgent, const RunBitset& options, int count = 0);

	/*************************************************************
	* @brief save-backtraces command
	* @param pAgent The pointer to the gSKI agent interface
	* @param setting The new setting, pass 0 (null) for query
	*************************************************************/
	EXPORT bool DoSaveBacktraces(gSKI::IAgent* pAgent, bool* pSetting = 0);

	/*************************************************************
	* @brief soar8 command
	* @param pSoar8 True to enable Soar 8, false for Soar 7
	*************************************************************/
	EXPORT bool DoSoar8(bool* pSoar8);

	/*************************************************************
	* @brief soarnews command
	*************************************************************/
	EXPORT bool DoSoarNews();

	/*************************************************************
	* @brief source command
	* @param pAgent The pointer to the gSKI agent interface
	* @param filename The file to source
	*************************************************************/
	EXPORT bool DoSource(gSKI::IAgent* pAgent, std::string filename);

	/*************************************************************
	* @brief sp command
	* @param pAgent The pointer to the gSKI agent interface
	* @param production The production to add to working memory
	*************************************************************/
	EXPORT bool DoSP(gSKI::IAgent* pAgent, const std::string& production);

	/*************************************************************
	* @brief stats command
	* @param pAgent The pointer to the gSKI agent interface
	* @param options The options for the stats command, see cli_CommandData.h
	*************************************************************/
	EXPORT bool DoStats(gSKI::IAgent* pAgent, const StatsBitset& options);

	/*************************************************************
	* @brief stop-soar command
	* @param pAgent The pointer to the gSKI agent interface
	* @param self Stop the only pAgent (false means stop all agents in kernel)
	* @param reasonForStopping optional reason for stopping
	*************************************************************/
	EXPORT bool DoStopSoar(gSKI::IAgent* pAgent, bool self, const std::string* reasonForStopping = 0);

	/*************************************************************
	* @brief time command
	* @param pAgent The pointer to the gSKI agent interface
	* @param argv The command line with the time arg removed
	*************************************************************/
	EXPORT bool DoTime(gSKI::IAgent* pAgent, std::vector<std::string>& argv);

	/*************************************************************
	* @brief timers command
	* @param pAgent The pointer to the gSKI agent interface
	* @param pSetting The timers setting, true to turn on, false to turn off, pass 0 (null) to query
	*************************************************************/
	EXPORT bool DoTimers(gSKI::IAgent* pAgent, bool* pSetting = 0);

	/*************************************************************
	* @brief verbose command
	* @param pAgent The pointer to the gSKI agent interface
	* @param pSetting The verbose setting, true to turn on, false to turn off, pass 0 (null) to query
	*************************************************************/
	EXPORT bool DoVerbose(gSKI::IAgent* pAgent, bool* pSetting = 0);

	/*************************************************************
	* @brief version command
	*************************************************************/
	EXPORT bool DoVersion();

	/*************************************************************
	* @brief waitsnc command
	* @param pAgent The pointer to the gSKI agent interface
	* @param pSetting The waitsnc setting, true to turn on, false to turn off, pass 0 (null) to query
	*************************************************************/
	EXPORT bool DoWaitSNC(gSKI::IAgent* pAgent, bool* pSetting = 0);

	/*************************************************************
	* @brief warnings command
	* @param pAgent The pointer to the gSKI agent interface
	* @param pSetting The warnings setting, true to turn on, false to turn off, pass 0 (null) to query
	*************************************************************/
	EXPORT bool DoWarnings(gSKI::IAgent* pAgent, bool* pSetting = 0);

	/*************************************************************
	* @brief watch command
	* @param pAgent The pointer to the gSKI agent interface
	* @param options Options for the watch command, see cli_CommandData.h
	* @param settings Settings for the watch command, if a flag (option) is set, its setting is set using this (true/on or false/off)
	* @param wmeSetting Setting for wme detail, not binary so it has its own arg
	* @param learnSetting Setting for learn level, not binary so it has its own arg
	*************************************************************/
	EXPORT bool DoWatch(gSKI::IAgent* pAgent, const WatchBitset& options, const WatchBitset& settings, const int wmeSetting, const int learnSetting);

	/*************************************************************
	* @brief watch-wmes command
	* @param pAgent The pointer to the gSKI agent interface
	*************************************************************/
	EXPORT bool DoWatchWMEs(gSKI::IAgent* pAgent, const eWatchWMEsMode mode, WatchWMEsTypeBitset type, const std::string* pIdString = 0, const std::string* pAttributeString = 0, const std::string* pValueString = 0);


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
