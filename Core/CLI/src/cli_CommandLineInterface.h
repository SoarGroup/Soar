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

#include <vector>
#include <string>
#include <stack>
#include <bitset>
#include <map>
#include <list>
#include <sstream>
#include <cstdlib>

#include "sml_KernelCallback.h"
#include "sml_Events.h"
#include "kernel.h"
#include "cli_Cli.h"
#include "cli_Parser.h"

#include "Export.h"

namespace soarxml
{
    class ElementXML ;
    class XMLTrace;
}

namespace sml 
{
    class KernelSML;
    class Connection ;
    class AgentSML;
}

namespace cli 
{
    // Define the stack for pushd/popd
    typedef std::stack<std::string> StringStack;

    // Define the list for structured responses
    typedef std::list<soarxml::ElementXML*> ElementXMLList;
    typedef ElementXMLList::iterator        ElementXMLListIter;

    // for nested command calls
    struct CallData 
    {
        CallData(sml::AgentSML* pAgent, bool rawOutput) : pAgent(pAgent), rawOutput(rawOutput) {}

        sml::AgentSML* pAgent;
        bool rawOutput;
    };

    class CommandLineInterface : public sml::KernelCallback, public cli::Cli
    {
    public:

        EXPORT CommandLineInterface();
        EXPORT virtual ~CommandLineInterface();

        /*************************************************************
        * @brief Set the kernel this command line module is interfacing with.
        *         Also has the side effect of setting the home directory to
        *         the location of SoarKernelSML, because the kernel is required
        *         to get that directory.
        * @param pKernelSML The pointer to the KernelSML object, optional, used to disable print callbacks
        *************************************************************/
        EXPORT void SetKernel(sml::KernelSML* pKernelSML = 0);

        /*************************************************************
        * @brief Process a command.  Give it a command line and it will parse
        *         and execute the command using system calls.
        * @param pConnection The connection, for communication to the client
        * @param pAgent The pointer to the agent interface
        * @param pCommandLine The command line string, arguments separated by spaces
        * @param echoResults If true send a copy of the result to the echo event
        * @param rawOutput If false, return structured output
        * @param pResponse Pointer to XML response object
        *************************************************************/
        EXPORT bool DoCommand(sml::Connection* pConnection, sml::AgentSML* pAgent, const char* pCommandLine, bool echoResults, bool rawOutput, soarxml::ElementXML* pResponse);

        /*************************************************************
        * @brief Returns true if the given command should always be echoed (to any listeners)
        *        The current implementation doesn't support aliases or short forms of the commands.
        * @param pCommandLine    The command line being tested
        *************************************************************/
        EXPORT bool ShouldEchoCommand(char const* pCommandLine) ;

        /*************************************************************
        * @brief Methods to create an XML element by starting a tag, adding attributes and
        *         closing the tag.
        *         These tags are automatically collected into the result of the current command.
        *    
        * NOTE: The attribute names must be compile time constants -- i.e. they remain in scope
        *        at all times (so we don't have to copy them).
        *************************************************************/
        void XMLBeginTag(char const* pTagName) ;
        void XMLAddAttribute(char const* pAttribute, char const* pValue) ;
        void XMLEndTag(char const* pTagName) ;
        bool XMLMoveCurrentToParent() ;
        bool XMLMoveCurrentToChild(int index) ;
        bool XMLMoveCurrentToLastChild() ;

        virtual bool DoAddWME(const std::string& id, std::string attribute, const std::string& value, bool acceptable);
        virtual bool DoAlias(std::vector< std::string >* argv = 0);
        virtual bool DoAllocate(const std::string& pool, int blocks);
        virtual bool DoCaptureInput(eCaptureInputMode mode, bool autoflush = false, std::string* pathname = 0);
        virtual bool DoCD(const std::string* pDirectory = 0);
        virtual bool DoChunkNameFormat(const bool* pLongFormat = 0, const int64_t* pCount = 0, const std::string* pPrefix = 0);
        virtual bool DoCLog(const eLogMode mode = LOG_QUERY, const std::string* pFilename = 0, const std::string* pToAdd = 0, bool silent = false);
        virtual bool DoCommandToFile(const eLogMode mode, const std::string& filename, std::vector< std::string >& argv);
        virtual bool DoDefaultWMEDepth(const int* pDepth);
        virtual bool DoDirs();
        virtual bool DoEcho(const std::vector<std::string>& argv, bool echoNewline);
        virtual bool DoEchoCommands(bool onlyGetValue, bool echoCommands);
        virtual bool DoEditProduction(std::string productionName);
        virtual bool DoEpMem(const char pOp = 0, const std::string *pAttr = 0, const std::string *pVal = 0, const epmem_time_id memory_id = 0);
        virtual bool DoExcise(const ExciseBitset& options, const std::string* pProduction = 0);
        virtual bool DoExplainBacktraces(const std::string* pProduction = 0, const int condition = 0);
        virtual bool DoFiringCounts(const int numberToList = -1, const std::string* pProduction = 0);
        virtual bool DoGDSPrint();
        virtual bool DoGP(const std::string& productionString);
        virtual bool DoGPMax(const int& maximum);
        virtual bool DoHelp(const std::vector<std::string> &argv);
        virtual bool DoIndifferentSelection( const char pOp = 0, const std::string* p1 = 0, const std::string* p2 = 0, const std::string* p3 = 0 );
        virtual bool DoInitSoar();
        virtual bool DoInternalSymbols();
        virtual bool DoLearn(const LearnBitset& options);
        virtual bool DoLoadLibrary(const std::string& libraryCommand);
        virtual bool DoLS();
        virtual bool DoMatches(const eMatchesMode mode, const eWMEDetail detail = WME_DETAIL_NONE, const std::string* pProduction = 0);
        virtual bool DoMaxChunks(const int n = 0);
        virtual bool DoMaxDCTime(const int n = 0);
        virtual bool DoMaxElaborations(const int n = 0);
        virtual bool DoMaxGoalDepth(const int n = 0);
        virtual bool DoMaxMemoryUsage(const int n = 0);
        virtual bool DoMaxNilOutputCycles(const int n = 0);
        virtual bool DoMemories(const MemoriesBitset options, int n = 0, const std::string* pProduction = 0);
        virtual bool DoMultiAttributes(const std::string* pAttribute = 0, int n = 0);
        virtual bool DoNumericIndifferentMode(bool query, const ni_mode mode);
        virtual bool DoOSupportMode(int mode = -1);
        virtual bool DoPopD();
        virtual bool DoPort();
        virtual bool DoPredict();
        virtual bool DoPreferences(const ePreferencesDetail detail, const bool object, const std::string* pId = 0, const std::string* pAttribute = 0);
        virtual bool DoPrint(PrintBitset options, int depth, const std::string* pArg = 0);
        virtual bool DoProductionFind(const ProductionFindBitset& options, const std::string& pattern);
        virtual bool DoPushD(const std::string& directory);
        virtual bool DoPWatch(bool query = true, const std::string* pProduction = 0, bool setting = false);
        virtual bool DoPWD();
        virtual bool DoRand( bool integer, std::string* bound );
        virtual bool DoRemoveWME(uint64_t timetag);
        virtual bool DoReplayInput(eReplayInputMode mode, std::string* pathname);
        virtual bool DoReteNet(bool save, std::string filename);
        virtual bool DoRL( const char pOp = 0, const std::string *pAttr = 0, const std::string *pVal = 0 );
        virtual bool DoRun(const RunBitset& options, int count = 0, eRunInterleaveMode interleave = RUN_INTERLEAVE_DEFAULT);
        virtual bool DoSaveBacktraces(bool* pSetting = 0);
        virtual bool DoSelect(const std::string* pOp = 0);
        virtual bool DoSetLibraryLocation(std::string* pLocation = 0);
        virtual bool DoSetStopPhase(bool setPhase, bool before, sml::smlPhase phase);
        virtual bool DoSMem(const char pOp = 0, const std::string *pAttr = 0, const std::string *pVal = 0);
        virtual bool DoSoarNews();
        virtual bool DoSource(std::string filename, SourceBitset* pOptions = 0);
        virtual bool DoSP(const std::string& production);
        virtual bool DoSRand(uint32_t* pSeed = 0);
        virtual bool DoStats(const StatsBitset& options, int sort = 0);
        virtual bool DoStopSoar(bool self, const std::string* reasonForStopping = 0);
        virtual bool DoTime(std::vector<std::string>& argv);
        virtual bool DoTimers(bool* pSetting = 0);
        virtual bool DoUnalias(std::vector<std::string>& argv);
        virtual bool DoVerbose(bool* pSetting = 0);
        virtual bool DoVersion();
        virtual bool DoWaitSNC(bool* pSetting = 0);
        virtual bool DoWarnings(bool* pSetting = 0);
        virtual bool DoWatch(const WatchBitset& options, const WatchBitset& settings, const int wmeSetting, const int learnSetting);
        virtual bool DoWatchWMEs(const eWatchWMEsMode mode, WatchWMEsTypeBitset type, const std::string* pIdString = 0, const std::string* pAttributeString = 0, const std::string* pValueString = 0);
        virtual bool DoWMA( const char pOp = 0, const std::string *pAttr = 0, const std::string *pVal = 0 );

        // utility for kernel SML
        bool IsLogOpen();

        bool GetCurrentWorkingDirectory(std::string& directory);

        virtual bool SetError(const std::string& error);
        virtual bool AppendError(const std::string& error);

        void AppendArgTag(const char* pParam, const char* pType, const char* pValue);
        void AppendArgTag(const char* pParam, const char* pType, const std::string& value);

        void AppendArgTagFast(const char* pParam, const char* pType, const char* pValue);
        void AppendArgTagFast(const char* pParam, const char* pType, const std::string& value);

        void PrependArgTag(const char* pParam, const char* pType, const char* pValue);
        void PrependArgTag(const char* pParam, const char* pType, const std::string& value);

        void PrependArgTagFast(const char* pParam, const char* pType, const char* pValue);
        void PrependArgTagFast(const char* pParam, const char* pType, const std::string& value);

    protected:

        void GetLastResultSML(sml::Connection* pConnection, soarxml::ElementXML* pResponse, bool echoResults);

        void SetTrapPrintCallbacks(bool setting);

        virtual void OnKernelEvent(int eventID, sml::AgentSML* pAgentSML, void* pCallData) ;

        /*************************************************************
        * @brief Standard parsing of -h and --help flags.  Returns
        *         true if the flag is present.
        *************************************************************/
        bool CheckForHelp(std::vector<std::string>& argv);

        /*************************************************************
        * @brief Add the contents of the helpFile file to m_Result.  
        *        Return true if successful, set error and return false if not.
        *************************************************************/
        bool GetHelpString(const std::string& helpFile);

        /*************************************************************      
        * @brief This is a utility function used by DoLS      
        *************************************************************/      
        void PrintFilename(const std::string& name, bool isDirectory);      

        void XMLResultToResponse(char const* pCommandName) ; // clears m_XMLResult

        void GetSystemStats(); // for stats
        void GetMemoryStats(); // for stats
        void GetMaxStats(); // for stats
        void GetReteStats(); // for stats

        bool Evaluate(const char* pInput); // source, formerly StreamSource

        // These help manage nested CLI calls
        void PushCall( CallData callData );
        void PopCall();

        // For help system
        bool ListHelpTopics(const std::string& directory, std::list< std::string >& topics);

        // stats, allocate
        void GetMemoryPoolStatistics();

        void PrintSourceSummary(int sourced, const std::list< std::string >& excised, int ignored);
        bool Source(const char* input, bool printFileStack = false);

        std::ostringstream      m_Result;                     // Raw output from the command
        bool                    m_RawOutput;                  // True if we want string output.
        std::string             m_LastError;                  // Last error
        bool                    m_TrapPrintEvents;            // True when print events should be trapped
        bool                    m_VarPrint;                   // Used in print command to put <>'s around identifiers.
        size_t                  m_GPMax;                      // Max number of productions to allow gp to produce
        soarxml::XMLTrace*      m_XMLResult;                  // Used to collect up XML output from commands that directly support that.
        ElementXMLList          m_ResponseTags;               // List of tags for the response.
        sml::KernelSML*         m_pKernelSML;
        sml::AgentSML*          m_pAgentSML;                  // Agent we're currently working with
        std::stack<CallData>    m_CallDataStack;              // Call data we're currently working with
        StringStack             m_DirectoryStack;             // Directory stack for pushd/popd
        std::string             m_LogFilename;                // Used for logging to a file.
        std::ofstream*          m_pLogFile;                   // The log file stream
        SourceBitset*           m_pSourceOptions;
        std::stack<std::string> m_SourceFileStack;            // Stack of source calls, if zero then command line
        int                     m_NumProductionsSourced;
        std::list<std::string>  m_ExcisedDuringSource;
        int                     m_NumProductionsIgnored;
        int                     m_NumTotalProductionsSourced;
        std::list<std::string>  m_TotalExcisedDuringSource;
        int                     m_NumTotalProductionsIgnored;
        cli::Parser             m_Parser;
    };
} // namespace cli

#endif //COMMAND_LINE_INTERFACE_H
