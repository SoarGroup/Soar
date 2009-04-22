/////////////////////////////////////////////////////////////////
// CommandLineInterface class file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
// This is the main header for the command line interface module.
//
/////////////////////////////////////////////////////////////////

#ifndef COMMAND_LINE_INTERFACE_H
#define COMMAND_LINE_INTERFACE_H

// STL includes
#include <vector>
#include <string>
#include <stack>
#include <bitset>
#include <map>
#include <list>
#include <sstream>
#include <cstdlib>

// Local includes
#include "sml_KernelCallback.h"
#include "sml_Events.h"

#include "cli_CommandData.h"
#include "cli_Aliases.h"
#include "kernel.h"

// For test
//#define WIN_STATIC_LINK

// get definition of EXPORT
#include "Export.h"

// Forward Declarations
typedef struct agent_struct agent;
typedef struct production_struct production;

namespace soarxml
{
	class ElementXML ;
	class XMLTrace;
}

namespace sml {
	class KernelSML;
	class Connection ;
	class AgentSML;
}

namespace cli {

// Forward declarations
class CommandLineInterface;
class GetOpt;
}
typedef int ErrorCode;

namespace cli {
using ::ErrorCode;

// Define the CommandFunction which we'll call to process commands
typedef bool (CommandLineInterface::*CommandFunction)(std::vector<std::string>& argv);

// Used to store a map from command name to function handler for that command
typedef std::map<std::string, CommandFunction>	CommandMap;
typedef CommandMap::iterator					CommandMapIter;
typedef CommandMap::const_iterator				CommandMapConstIter;

// Used to decide if a given command should always be echoed
typedef std::map<std::string, bool>				EchoMap ;

// Define the stack for pushd/popd
typedef std::stack<std::string> StringStack;

// Define the list for structured responses
typedef std::list<soarxml::ElementXML*> ElementXMLList;
typedef ElementXMLList::iterator ElementXMLListIter;

// Define bitsets for various commands
typedef std::bitset<EPMEM_NUM_OPTIONS> EpMemBitset;
typedef std::bitset<EXCISE_NUM_OPTIONS> ExciseBitset;
typedef std::bitset<INDIFFERENT_NUM_OPTIONS> IndifferentBitset;
typedef std::bitset<LEARN_NUM_OPTIONS> LearnBitset;
typedef std::bitset<MEMORIES_NUM_OPTIONS> MemoriesBitset;
typedef std::bitset<PRINT_NUM_OPTIONS> PrintBitset;
typedef std::bitset<PRODUCTION_FIND_NUM_OPTIONS> ProductionFindBitset;
typedef std::bitset<RL_NUM_OPTIONS> RLBitset;
typedef std::bitset<RUN_NUM_OPTIONS> RunBitset;
typedef std::bitset<SMEM_NUM_OPTIONS> SMemBitset;
typedef std::bitset<STATS_NUM_OPTIONS> StatsBitset;
typedef std::bitset<WATCH_NUM_OPTIONS> WatchBitset;
typedef std::bitset<WATCH_WMES_TYPE_NUM_OPTIONS> WatchWMEsTypeBitset;
typedef std::bitset<WMA_NUM_OPTIONS> WMABitset;

// For option parsing
enum eOptionArgument {
	OPTARG_NONE,
	OPTARG_REQUIRED,
	OPTARG_OPTIONAL,
};

struct Options {
	int shortOpt;
	const char* longOpt;
	eOptionArgument argument;
};

// for nested command calls
struct CallData {
	CallData(sml::AgentSML* pAgent, bool rawOutput) : pAgent(pAgent), rawOutput(rawOutput) {}

	sml::AgentSML* pAgent;
	bool rawOutput;
};

CommandLineInterface* GetCLI() ;

class CommandLineInterface : public sml::KernelCallback {
public:

	EXPORT CommandLineInterface();
	EXPORT virtual ~CommandLineInterface();

	/*************************************************************
	* @brief Set the kernel this command line module is interfacing with.
	*		 Also has the side effect of setting the home directory to
	*		 the location of SoarKernelSML, because the kernel is required
	*		 to get that directory.
	* @param pKernelSML The pointer to the KernelSML object, optional, used to disable print callbacks
	*************************************************************/
	EXPORT void SetKernel(sml::KernelSML* pKernelSML = 0);

	/*************************************************************
	* @brief Process a command.  Give it a command line and it will parse
	*		 and execute the command using system calls.
	* @param pConnection The connection, for communication to the client
	* @param pAgent The pointer to the agent interface
	* @param pCommandLine The command line string, arguments separated by spaces
	* @param echoResults If true send a copy of the result to the echo event
	* @param rawOutput If false, return structured output
	* @param pResponse Pointer to XML response object
	*************************************************************/
	EXPORT bool DoCommand(sml::Connection* pConnection, sml::AgentSML* pAgent, const char* pCommandLine, bool echoResults, bool rawOutput, soarxml::ElementXML* pResponse);

	/*************************************************************
	* @brief Takes a command line and expands any aliases and returns
	*		 the result.  The command is NOT executed.
	* @param pConnection The connection, for communication to the client
	* @param pCommandLine The command line string, arguments separated by spaces
	* @param pResponse Pointer to XML response object
	*************************************************************/
	EXPORT bool ExpandCommand(sml::Connection* pConnection, const char* pCommandLine, soarxml::ElementXML* pResponse);

	/*************************************************************
	* @brief Returns true if the given command should always be echoed (to any listeners)
	*        The current implementation doesn't support aliases or short forms of the commands.
	* @param pCommandLine	The command line being tested
	*************************************************************/
	EXPORT bool ShouldEchoCommand(char const* pCommandLine) ;

	/*************************************************************
	* @brief Takes a command line and expands any aliases and returns
	*		 the result.  The command is NOT executed.
	* @param pCommandLine The command line string, arguments separated by spaces
	* @param pExpandedLine The return value -- the expanded version of the command
	*************************************************************/
	bool ExpandCommandToString(const char* pCommandLine, std::string* pExpandedLine) ;

	/*************************************************************
	* @brief Methods to create an XML element by starting a tag, adding attributes and
	*		 closing the tag.
	*		 These tags are automatically collected into the result of the current command.
	*	
	* NOTE: The attribute names must be compile time constants -- i.e. they remain in scope
	*		at all times (so we don't have to copy them).
	*************************************************************/
	void XMLBeginTag(char const* pTagName) ;
	void XMLAddAttribute(char const* pAttribute, char const* pValue) ;
	void XMLEndTag(char const* pTagName) ;
	bool XMLMoveCurrentToParent() ;
	bool XMLMoveCurrentToChild(int index) ;
	bool XMLMoveCurrentToLastChild() ;

	// The internal Parse functions follow
	// do not call these directly, these should only be called in DoCommandInternal
	bool ParseAddWME(std::vector<std::string>& argv);
	bool ParseAlias(std::vector<std::string>& argv);
	bool ParseCaptureInput(std::vector<std::string>& argv);
	bool ParseCD(std::vector<std::string>& argv);
	bool ParseChunkNameFormat(std::vector<std::string>& argv);
	bool ParseCLog(std::vector<std::string>& argv);
	bool ParseCommandToFile(std::vector<std::string>& argv);
	bool ParseDefaultWMEDepth(std::vector<std::string>& argv);
	bool ParseDirs(std::vector<std::string>& argv);
	bool ParseEcho(std::vector<std::string>& argv);
	bool ParseEchoCommands(std::vector<std::string>& argv) ;
	bool ParseEditProduction(std::vector<std::string>& argv);
	bool ParseEpMem(std::vector<std::string>& argv);
	bool ParseExcise(std::vector<std::string>& argv);
	bool ParseExplainBacktraces(std::vector<std::string>& argv);
	bool ParseFiringCounts(std::vector<std::string>& argv);
	bool ParseGDSPrint(std::vector<std::string>& argv);
	bool ParseGP(std::vector<std::string>& argv);
	bool ParseHelp(std::vector<std::string>& argv);
	bool ParseIndifferentSelection(std::vector<std::string>& argv);
	bool ParseInitSoar(std::vector<std::string>& argv);
	bool ParseInternalSymbols(std::vector<std::string>& argv);
	bool ParseLearn(std::vector<std::string>& argv);
	bool ParseLoadLibrary(std::vector<std::string>& argv);
	bool ParseLS(std::vector<std::string>& argv);
	bool ParseMatches(std::vector<std::string>& argv);
	bool ParseMaxChunks(std::vector<std::string>& argv);
	bool ParseMaxElaborations(std::vector<std::string>& argv);
	bool ParseMaxMemoryUsage(std::vector<std::string>& argv);
	bool ParseMaxNilOutputCycles(std::vector<std::string>& argv);
	bool ParseMemories(std::vector<std::string>& argv);
	bool ParseMultiAttributes(std::vector<std::string>& argv);
	bool ParseNumericIndifferentMode(std::vector<std::string>& argv);
	bool ParseOSupportMode(std::vector<std::string>& argv);
	bool ParsePopD(std::vector<std::string>& argv);
	bool ParsePredict(std::vector<std::string>& argv);
	bool ParsePreferences(std::vector<std::string>& argv);
	bool ParsePrint(std::vector<std::string>& argv);
	bool ParseProductionFind(std::vector<std::string>& argv);
	bool ParsePushD(std::vector<std::string>& argv);
	bool ParsePWatch(std::vector<std::string>& argv);
	bool ParsePWD(std::vector<std::string>& argv);
	bool ParseQuit(std::vector<std::string>& argv);
	bool ParseRand(std::vector<std::string>& argv);
	bool ParseRemoveWME(std::vector<std::string>& argv);
	bool ParseReplayInput(std::vector<std::string>& argv);
	bool ParseReteNet(std::vector<std::string>& argv);
	bool ParseRL(std::vector<std::string>& argv);
	bool ParseRun(std::vector<std::string>& argv);
	bool ParseSaveBacktraces(std::vector<std::string>& argv);
	bool ParseSelect(std::vector<std::string>& argv);
	bool ParseSetLibraryLocation(std::vector<std::string>& argv);
	bool ParseSMem(std::vector<std::string>& argv);
	bool ParseSoarNews(std::vector<std::string>& argv);
	bool ParseSource(std::vector<std::string>& argv);
	bool ParseSP(std::vector<std::string>& argv);
	bool ParseSRand(std::vector<std::string>& argv);
	bool ParseStats(std::vector<std::string>& argv);
	bool ParseSetStopPhase(std::vector<std::string>& argv);
	bool ParseStopSoar(std::vector<std::string>& argv);
	bool ParseTime(std::vector<std::string>& argv);
	bool ParseTimers(std::vector<std::string>& argv);
	bool ParseUnalias(std::vector<std::string>& argv);
	bool ParseVerbose(std::vector<std::string>& argv);
	bool ParseVersion(std::vector<std::string>& argv);
	bool ParseWaitSNC(std::vector<std::string>& argv);
	bool ParseWarnings(std::vector<std::string>& argv);
	bool ParseWatch(std::vector<std::string>& argv);
	bool ParseWatchWMEs(std::vector<std::string>& argv);
	bool ParseWMA(std::vector<std::string>& argv);

	/*************************************************************
	* @brief add-wme command
	* @param id Id string for the new wme
	* @param attribute Attribute string for the new wme
	* @param value Value string for the new wme
	* @param acceptable True to give wme acceptable preference
	*************************************************************/
	bool DoAddWME(const std::string& id, std::string attribute, const std::string& value, bool acceptable);

	/*************************************************************
	* @brief alias command
	* @param command The alias to enable or disable, pass 0 to list aliases
	* @param pSubstitution Pass a pointer to a vector strings to enable a new 
	*        alias, pass 0 to disable a current alias, pass empty vector to list
	*        command's (the parameter) alias
	*************************************************************/
	bool DoAlias(const std::string* pCommand = 0, const std::vector<std::string>* pSubstitution = 0);

	/*************************************************************
	* @brief capture-input command
	*************************************************************/
	bool DoCaptureInput(eCaptureInputMode mode, bool autoflush = false, std::string* pathname = 0);

	/*************************************************************
	* @brief cd command
	* @param pDirectory Pointer to the directory to pass in to. Pass null to return 
	*        to the initial (home) directory. 
	*************************************************************/
	bool DoCD(const std::string* pDirectory = 0);

	/*************************************************************
	* @brief chunk-name-format command
	* @param pLongFormat Pointer to the new format type, true for long format, false 
	*        for short format, 0 (null) for query or no change
	* @param pCount Pointer to the new counter, non negative integer, 0 (null) for query
	* @param pPrefix Pointer to the new prefix, must not contain '*' character, 
	*        null for query
	*************************************************************/
	bool DoChunkNameFormat(const bool* pLongFormat = 0, const int* pCount = 0, const std::string* pPrefix = 0);

	/*************************************************************
	* @brief clog command
	* @param mode The mode for the log command, see cli_CommandData.h
	* @param pFilename The log filename, pass 0 (null) if not applicable to mode
	* @param pToAdd The string to add to the log, pass 0 (null) if not applicable to mode
	* @param silent Supress query messages (log file open/closed).
	*************************************************************/
	bool DoCLog(const eLogMode mode = LOG_QUERY, const std::string* pFilename = 0, const std::string* pToAdd = 0, bool silent = false);

	/*************************************************************
	* @brief default-wme-depth command
	* @param pDepth The pointer to the new wme depth, a positive integer.  
	*        Pass 0 (null) pointer for query.
	*************************************************************/
	bool DoDefaultWMEDepth(const int* pDepth);

	/*************************************************************
	* @brief dirs command
	*************************************************************/
	bool DoDirs();

	/*************************************************************
	* @brief echo command
	* @param argv The args to echo
	* @param echoNewline True means a newline character will be appended to string
	*************************************************************/
	bool DoEcho(const std::vector<std::string>& argv, bool echoNewline);

	/*************************************************************
	* @brief echo-commands command
	* @param onlyGetValue
	* @param echoCommands
	*************************************************************/
	bool DoEchoCommands(bool onlyGetValue, bool echoCommands);

	/*************************************************************
	* @brief edit-production command
	* @param productionName Production to edit
	*************************************************************/
	bool DoEditProduction(std::string productionName);
	
	/*************************************************************
	* @brief epmem command
	* @param pOp the epmem switch to implement, pass 0 (null) for full parameter configuration
	* @param pAttr the attribute to get/set/stats, pass 0 (null) only if no pOp (all config) or stats (full stats)
	* @param pVal the value to set, pass 0 (null) only if no pOp (all config), get, or stats
	*************************************************************/
	bool DoEpMem(const char pOp = 0, const std::string *pAttr = 0, const std::string *pVal = 0);

	/*************************************************************
	* @brief excise command
	* @param options The various options set on the command line, see 
	*        cli_CommandData.h
	* @param pProduction A production to excise, optional
	*************************************************************/
	bool DoExcise(const ExciseBitset& options, const std::string* pProduction = 0);

	/*************************************************************
	* @brief explain-backtraces command
	* @param pProduction Pointer to involved production. Pass 0 (null) for 
	*        query
	* @param condition A number representing the condition number to explain, 
	*        0 for production name, -1 for full, 
	*        this argument ignored if pProduction is 0 (null)
	*************************************************************/
	bool DoExplainBacktraces(const std::string* pProduction = 0, const int condition = 0);

	/*************************************************************
	* @brief firing-counts command
	* @param numberToList The number of top-firing productions to list.  
	*        Use 0 to list those that haven't fired. -1 lists all
	* @param pProduction The specific production to list, pass 0 (null) to list 
	*        multiple productions
	*************************************************************/
	bool DoFiringCounts(const int numberToList = -1, const std::string* pProduction = 0);

	/*************************************************************
	* @brief gds-print command
	*************************************************************/
	bool DoGDSPrint();

	/*************************************************************
	* @brief gp command
	* @param productionString The general soar production to generate more productions to load to memory
	*************************************************************/
	bool DoGP(const std::string& productionString);

	/*************************************************************
	* @brief help command
	* @param pCommand The command to get help on, pass 0 (null) for a list of commands
	*************************************************************/
	bool DoHelp(const std::string* pCommand = 0);

	/*************************************************************
	* @brief indifferent-selection command
	* @param pOp The operation to perform, pass 0 if unnecssary
	* @param p1 First parameter, pass 0 (null) if unnecessary
	* @param p2 Second parameter, pass 0 (null) if unnecessary
	* @param p3 Third parameter, pass 0 (null) if unnecessary
	*************************************************************/
	bool DoIndifferentSelection( const char pOp = 0, const std::string* p1 = 0, const std::string* p2 = 0, const std::string* p3 = 0 );

	/*************************************************************
	* @brief init-soar command
	*************************************************************/
	bool DoInitSoar();

	/*************************************************************
	* @brief internal-symbols command
	*************************************************************/
	bool DoInternalSymbols();

	/*************************************************************
	* @brief learn command
	* @param options The various options set on the command line, 
	*        see cli_CommandData.h
	*************************************************************/
	bool DoLearn(const LearnBitset& options);

	/*************************************************************
	* @brief load-library command
	* @param libraryCommand The name of the library to load 
	* WITHOUT the .so/.dll/etc plus its arguments.
	*************************************************************/
	bool DoLoadLibrary(const std::string& libraryCommand);

	/*************************************************************
	* @brief ls command
	*************************************************************/
	bool DoLS();

	/*************************************************************
	* @brief matches command
	* @param mode The mode for the command, see cli_CommandData.h
	* @param detail The WME detail, see cli_CommandData.h
	* @param pProduction The production, pass 0 (null) if not applicable to mode
	*************************************************************/
	bool DoMatches(const eMatchesMode mode, const eWMEDetail detail = WME_DETAIL_NONE, const std::string* pProduction = 0);

	/*************************************************************
	* @brief max-chunks command
	* @param n The new max chunks value, use 0 to query
	*************************************************************/
	bool DoMaxChunks(const int n = 0);

	/*************************************************************
	* @brief max-elaborations command
	* @param n The new max elaborations value, use 0 to query
	*************************************************************/
	bool DoMaxElaborations(const int n = 0);

	/*************************************************************
	* @brief max-memory-usage command
	* @param n The new memory usage value, in bytes
	*************************************************************/
	bool DoMaxMemoryUsage(const int n = 0);

	/*************************************************************
	* @brief max-nil-output-cycles command
	* @param n The new max nil output cycles value, use 0 to query
	*************************************************************/
	bool DoMaxNilOutputCycles(const int n);

	/*************************************************************
	* @brief memories command
	* @param options Options for the memories flag, see cli_CommandData.h
	* @param n number of productions to print sorted by most memory use, use 0 for all
	* @param pProduction specific production to print, ignored if any 
	*        options are set, pass 0 (null) if not applicable
	*************************************************************/
	bool DoMemories(const MemoriesBitset options, int n = 0, const std::string* pProduction = 0);

	/*************************************************************
	* @brief multi-attributes command
	* @param pAttribute The attribute, pass 0 (null) for query
	* @param n The count, pass 0 (null) for query if pAttribute is also null, 
	*        otherwise this will default to 10
	*************************************************************/
	bool DoMultiAttributes(const std::string* pAttribute = 0, int n = 0);

	/*************************************************************
	* @brief numeric-indifferent mode command
	* @param query true to query
	* @param mode The new mode, ignored on query
	*************************************************************/
	bool DoNumericIndifferentMode(bool query, const ni_mode mode);

	/*************************************************************
	* @brief o-support-mode command
	* @param mode The new o-support mode.  Use -1 to query.
	*************************************************************/
	bool DoOSupportMode(int mode = -1);

	/*************************************************************
	* @brief popd command
	*************************************************************/
	bool DoPopD();

	/*************************************************************
	* @brief predict command
	*************************************************************/
	bool DoPredict();

	/*************************************************************
	* @brief preferences command
	* @param detail The preferences detail level, see cli_CommandData.h
	* @param pId An existing soar identifier or 0 (null)
	* @param pAttribute An existing soar attribute of the specified identifier or 0 (null)
	*************************************************************/
	bool DoPreferences(const ePreferencesDetail detail, const bool object, const std::string* pId = 0, const std::string* pAttribute = 0);

	/*************************************************************
	* @brief print command
	* @param options The options to the print command, see cli_CommandData.h
	* @param depth WME depth
	* @param pArg The identifier/timetag/pattern/production name to print, 
	*        or 0 (null) if not applicable
	*************************************************************/
	bool DoPrint(PrintBitset options, int depth, const std::string* pArg = 0);

	/*************************************************************
	* @brief production-find command
	* @param options The options to the command, see cli_CommandData.h
	* @param pattern Any pattern that can appear in productions.
	*************************************************************/
	bool DoProductionFind(const ProductionFindBitset& options, const std::string& pattern);

	/*************************************************************
	* @brief pushd command
	* @param directory The directory to change to
	*************************************************************/
	bool DoPushD(const std::string& directory);

	/*************************************************************
	* @brief pwatch command
	* @param query Pass true to query, all other args ignored
	* @param pProduction The production to watch or stop watching, pass 0 (null) 
	*        to disable watching of all productions (setting ignored)
	* @param setting True to watch the pProduction, false to stop watching it
	*************************************************************/
	bool DoPWatch(bool query = true, const std::string* pProduction = 0, bool setting = false);

	/*************************************************************
	* @brief pwd command
	*************************************************************/
	bool DoPWD();

	/*************************************************************
	* @brief quit command
	*************************************************************/
	bool DoQuit();

	/*************************************************************
	* @brief rand command
	*************************************************************/
	bool DoRand( bool integer, std::string* bound );

	/*************************************************************
	* @brief remove-wme command
	* @param timetag The timetag of the wme to remove
	*************************************************************/
	bool DoRemoveWME(unsigned long timetag);

	/*************************************************************
	* @brief replay-input command
	*************************************************************/
	bool DoReplayInput(eReplayInputMode mode, std::string* pathname);

	/*************************************************************
	* @brief rete-net command
	* @param save true to save, false to load
	* @param filename the rete-net file
	*************************************************************/
	bool DoReteNet(bool save, std::string filename);
	
	/*************************************************************
	* @brief rl command
	* @param pOp the rl switch to implement, pass 0 (null) for full parameter configuration
	* @param pAttr the attribute to get/set/stats, pass 0 (null) only if no pOp (all config) or stats (full stats)
	* @param pVal the value to set, pass 0 (null) only if no pOp (all config), get, or stats
	*************************************************************/
	bool DoRL( const char pOp = 0, const std::string *pAttr = 0, const std::string *pVal = 0 );

	/*************************************************************
	* @brief run command
	* @param options Options for the run command, see cli_CommandData.h
	* @param count The count, units or applicability depends on options
	* @param interleave Support for round robin execution across agents 
	*		 at a finer grain than the run-size parameter.
	*************************************************************/
	bool DoRun(const RunBitset& options, int count = 0, eRunInterleaveMode interleave = RUN_INTERLEAVE_DEFAULT);

	/*************************************************************
	* @brief save-backtraces command
	* @param setting The new setting, pass 0 (null) for query
	*************************************************************/
	bool DoSaveBacktraces(bool* pSetting = 0);
	
	/*************************************************************
	* @brief select command
	* @param pAgent The pointer to the gSKI agent interface
	* @param setting The new setting, pass 0 (null) for query
	*************************************************************/
	bool DoSelect(const std::string* pOp = 0);

	/*************************************************************
	* @brief set-library-location command
	* @param pLocation String of new location, pass null for query.
	*************************************************************/
	bool DoSetLibraryLocation(const std::string* pLocation = 0);

	/*************************************************************
	* @brief set-stop-phase command
	* @param setPhase
	* @param before
	* @param phase
	*************************************************************/
	bool DoSetStopPhase(bool setPhase, bool before, sml::smlPhase phase);

	/*************************************************************
<<<<<<< .working
	* @brief smem command
	* @param pOp the smem switch to implement, pass 0 (null) for full parameter configuration
	* @param pAttr the attribute to get/set/stats, pass 0 (null) only if no pOp (all config) or stats (full stats)
	* @param pVal the value to set, pass 0 (null) only if no pOp (all config), get, or stats
	*************************************************************/
	bool DoSMem(const char pOp = 0, const std::string *pAttr = 0, const std::string *pVal = 0);

	/*************************************************************
	* @brief soar8 command
	* @param pSoar8 True to enable Soar 8, false for Soar 7
	*************************************************************/
	bool DoSoar8(bool* pSoar8);

	/*************************************************************
=======
>>>>>>> .merge-right.r10413
	* @brief soarnews command
	*************************************************************/
	bool DoSoarNews();

	/*************************************************************
	* @brief source command
	* @param filename The file to source
	*************************************************************/
	bool DoSource(std::string filename);

	/*************************************************************
	* @brief sp command
	* @param production The production to add to working memory
	*************************************************************/
	bool DoSP(const std::string& production);

	/*************************************************************
	* @brief srand command
	* @param pSeed Number to seed the random number generator with, pass
	*		 null to seed randomly.
	*************************************************************/
	bool DoSRand(unsigned long int* pSeed = 0);

	/*************************************************************
	* @brief stats command
	* @param options The options for the stats command, see cli_CommandData.h
	*************************************************************/
	bool DoStats(const StatsBitset& options);

	/*************************************************************
	* @brief stop-soar command
	* @param self Stop the only pAgent (false means stop all agents in kernel)
	* @param reasonForStopping optional reason for stopping
	*************************************************************/
	bool DoStopSoar(bool self, const std::string* reasonForStopping = 0);

	/*************************************************************
	* @brief time command
	* @param argv The command line with the time arg removed
	*************************************************************/
	bool DoTime(std::vector<std::string>& argv);

	/*************************************************************
	* @brief timers command
	* @param pSetting The timers setting, true to turn on, false to turn off, 
	*        pass 0 (null) to query
	*************************************************************/
	bool DoTimers(bool* pSetting = 0);

	/*************************************************************
	* @brief verbose command
	* @param pSetting The verbose setting, true to turn on, false to turn off, 
	*        pass 0 (null) to query
	*************************************************************/
	bool DoVerbose(bool* pSetting = 0);

	/*************************************************************
	* @brief version command
	*************************************************************/
	bool DoVersion();

	/*************************************************************
	* @brief waitsnc command
	* @param pSetting The waitsnc setting, true to turn on, false to turn off, 
	*        pass 0 (null) to query
	*************************************************************/
	bool DoWaitSNC(bool* pSetting = 0);

	/*************************************************************
	* @brief warnings command
	* @param pSetting The warnings setting, true to turn on, false to turn off, 
	*        pass 0 (null) to query
	*************************************************************/
	bool DoWarnings(bool* pSetting = 0);

	/*************************************************************
	* @brief watch command
	* @param options Options for the watch command, see cli_CommandData.h
	* @param settings Settings for the watch command, if a flag (option) is set, its 
	*        setting is set using this (true/on or false/off)
	* @param wmeSetting Setting for wme detail, not binary so it has its own arg
	* @param learnSetting Setting for learn level, not binary so it has its own arg
	*************************************************************/
	bool DoWatch(const WatchBitset& options, const WatchBitset& settings, const int wmeSetting, const int learnSetting);

	/*************************************************************
	* @brief watch-wmes command
	*************************************************************/
	bool DoWatchWMEs(const eWatchWMEsMode mode, WatchWMEsTypeBitset type, const std::string* pIdString = 0, const std::string* pAttributeString = 0, const std::string* pValueString = 0);

	/*************************************************************
	* @brief wma command
	* @param pOp the wma switch to implement, pass 0 (null) for full parameter configuration
	* @param pAttr the attribute to get/set/stats, pass 0 (null) only if no pOp (all config) or stats (full stats)
	* @param pVal the value to set, pass 0 (null) only if no pOp (all config), get, or stats
	*************************************************************/
	bool DoWMA( const char pOp = 0, const std::string *pAttr = 0, const std::string *pVal = 0 );

protected:

	void GetLastResultSML(sml::Connection* pConnection, soarxml::ElementXML* pResponse);

	/*************************************************************
	* @brief Does the bulk of command parsing and chooses what function
	*		 to call to process the command.  DoCommand mainly does
	*		 SML stuff.
	*************************************************************/
	bool DoCommandInternal(const std::string& commandLine);
	bool DoCommandInternal(std::vector<std::string>& argv);
	bool PartialMatch(std::vector<std::string>& argv);

	void SetTrapPrintCallbacks(bool setting);

	virtual void OnKernelEvent(int eventID, sml::AgentSML* pAgentSML, void* pCallData) ;

	// Wrapped to handle errors more easily
	int CLITokenize(std::string cmdline, std::vector<std::string>& argumentVector);

	/*************************************************************
	* @brief Standard parsing of -h and --help flags.  Returns
	*		 true if the flag is present.
	*************************************************************/
	bool CheckForHelp(std::vector<std::string>& argv);

	/*************************************************************
	* @brief Add the contents of the helpFile file to m_Result.  
	*        Return true if successful, set error and return false if not.
	*************************************************************/
	bool GetHelpString(const std::string& helpFile);

	/*************************************************************
	* @brief 
	*************************************************************/
	bool GetCurrentWorkingDirectory(std::string& directory);

	/*************************************************************
	* @brief 
	*************************************************************/
	int ParseLevelOptarg();
	int ParseLearningOptarg();
	bool CheckOptargRemoveOrZero();
	bool ProcessWatchLevelSettings(const int level, WatchBitset& options, WatchBitset& settings, int& wmeSetting, int& learnSetting);

	eRunInterleaveMode ParseRunInterleaveOptarg();

	/*************************************************************
	* @brief 
	*************************************************************/
	void HandleSourceError(int errorLine, const std::string* pFilename);

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

	/************************************************************* 	 
	* @brief Echo the given string through the smlEVENT_ECHO event
	*		 if the call requested that commands be echoed.
	*************************************************************/ 	 
	void EchoString(sml::Connection* pConnection, char const* pString);	

	/************************************************************* 	 
	* @brief Strip quotes off of a string.  Must start and end with
    *        a '"' character.
    * @return True if quotes were removed from the string.
	*************************************************************/ 	 
	bool StripQuotes(std::string& str); 	 

	bool SetError(cli::ErrorCode code);				// always returns false
	bool SetErrorDetail(const std::string detail);	// always returns false

	void XMLResultToResponse(char const* pCommandName) ; // clears m_XMLResult

	void LogQuery(); // for CLog command

	std::string GenerateErrorString();

	void GetSystemStats(); // for stats
	void GetMemoryStats(); // for stats
	void GetReteStats(); // for stats

	bool StreamSource( std::istream& soarStream, const std::string* pFilename );

	// These help manage nested CLI calls
	void PushCall( CallData callData );
	void PopCall();

	// For help system
	bool ListHelpTopics(const std::string& directory, std::list< std::string >& topics);

////////////////////////////////////////////
	// New options code

	void ResetOptions();
	bool ProcessOptions(std::vector<std::string>& argv, Options* options);
	void MoveBack(std::vector<std::string>& argv, int what, int howFar);
	bool HandleOptionArgument(std::vector<std::string>& argv, const char* option, eOptionArgument arg);

	int			m_Argument;
	int			m_Option;
	std::string m_OptionArgument;
	int			m_NonOptionArguments;

	eSourceMode m_SourceMode;
	int			m_NumProductionsSourced;
	int			m_NumProductionsExcised;
	int			m_NumProductionsIgnored;
	std::list< std::string > m_ExcisedDuringSource;
	bool		m_SourceVerbose;

////////////////////////////////////////////

	bool				m_Initialized;			// True if state has been cleared for a new command execution
	static std::ostringstream m_Result;			// Raw output from the command
	bool				m_RawOutput;			// True if we want string output.
	bool				m_SourceError;			// Used to control debug printing for source command errors
	std::string			m_SourceErrorDetail;	// Used for detailed source error output
	int					m_SourceDepth;			// Depth of source command calls.
	int					m_SourceDirDepth;		// Depth of directory stack since source command, used to return to the dir that source was issued in.
	cli::ErrorCode		m_LastError;			// Last error code (see cli_CLIError.h)
	std::string			m_LastErrorDetail;		// Additional detail concerning the last error
	bool				m_TrapPrintEvents;		// True when print events should be trapped
	bool				m_EchoResult;			// If true, copy result of command to echo event stream
	EchoMap				m_EchoMap;				// If command appears in this map, always echo it.
	bool				m_VarPrint;				// Used in print command to put <>'s around identifiers.

	soarxml::XMLTrace*	m_XMLResult;			// Used to collect up XML output from commands that directly support that.
	ElementXMLList		m_ResponseTags;			// List of tags for the response.

	Aliases				m_Aliases;				// Alias management object
	CommandMap			m_CommandMap;			// Mapping of command names to function pointers
	sml::KernelSML*		m_pKernelSML;
	sml::AgentSML*		m_pAgentSML;			// Agent we're currently working with
	std::stack< CallData > m_pCallDataStack;	// Call data we're currently working with
	agent*				m_pAgentSoar;			// Agent we're currently working with (soar kernel)
	std::string			m_LibraryDirectory;		// The library directory, server side, see help command
	StringStack			m_DirectoryStack;		// Directory stack for pushd/popd
	std::string			m_LogFilename;			// Used for logging to a file.
	std::ofstream*		m_pLogFile;				// The log file stream
};

} // namespace cli

#endif //COMMAND_LINE_INTERFACE_H
