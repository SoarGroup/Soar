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

#include "portability.h"

#include "cli_enums.h"
#include "cli_Parser.h"

#include "sml_KernelCallback.h"
#include "sml_Events.h"

#include "Export.h"

#include <bitset>
#include <cstdlib>
#include <list>
#include <map>
#include <sstream>
#include <stdint.h>
#include <stack>
#include <string>
#include <vector>

typedef uint64_t epmem_time_id;

namespace soar_module
{
    class named_object;
}

namespace soarxml
{
    class ElementXML;
    class XMLTrace;
}

namespace sml
{
    class KernelSML;
    class Connection;
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

    class CommandLineInterface : public sml::KernelCallback
    {
            friend class cli::ParserCommand;

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

            bool DoAlias(std::vector< std::string >* argv = 0, bool doRemove = false);
            bool DoCD(const std::string* pDirectory = 0);
            bool DoChunk(const std::string* pAttr = 0, const std::string* pVal = 0);
            bool DoDebug(std::vector< std::string >* argv = 0);
            bool DoDecide(std::vector<std::string>& argv, const std::string& pCmd);
            bool DoDirs();
            bool DoEcho(const std::vector<std::string>& argv, bool echoNewline);
            bool DoEpMem(const char pOp = 0, const std::string* pAttr = 0, const std::string* pVal = 0, const epmem_time_id memory_id = 0);
            bool DoExplain(const std::string* pArg = 0, const std::string* pArg2 = 0);
            bool DoGP(const std::string& productionString);
            bool DoHelp(const std::vector<std::string>& argv);
            bool DoLearn(const LearnBitset& options);
            bool DoLoad(std::vector<std::string>& argv, const std::string& pCmd);
            bool DoLS();
            bool DoOutput(std::vector<std::string>& argv, const std::string* pArg1 = 0, const std::string* pArg2 = 0);
            bool DoPopD();
            bool DoPreferences(const ePreferencesDetail detail, const bool object, const std::string* pId = 0, const std::string* pAttribute = 0);
            bool DoPrint(PrintBitset options, int depth, const std::string* pArg = 0);
            bool DoProduction(std::vector<std::string>& argv, const std::string& pCmd);
            bool DoPushD(const std::string& directory);
            bool DoPWD();
            bool DoRL(const char pOp = 0, const std::string* pAttr = 0, const std::string* pVal = 0);
            bool DoRun(const RunBitset& options, int count = 0, eRunInterleaveMode interleave = RUN_INTERLEAVE_DEFAULT);
            bool DoSave(std::vector<std::string>& argv, const std::string& pCmd);
            bool DoSMem(const char pOp = 0, const std::string* pArg1 = 0, const std::string* pArg2 = 0, const std::string* pArg3 = 0);
            bool DoSoar(const char pOp = 0, const std::string* pArg1 = 0, const std::string* pArg2 = 0, const std::string* pArg3 = 0);
            bool DoSP(const std::string& production);
            bool DoSRand(uint32_t* pSeed = 0);
            bool DoStats(const StatsBitset& options, int sort = 0);
            bool DoSVS(const std::vector<std::string>& args);
            bool DoTrace(const WatchBitset& options, const WatchBitset& settings, const int wmeSetting, const int learnSetting, bool fromWatch);
            bool DoVisualize(const std::string* pArg = 0, const std::string* pArg2 = 0, const std::string* pArg3 = 0);
            bool DoWM(std::vector<std::string>& argv, const std::string& pCmd);

            EXPORT std::string ExpandCommand(const char* pCommand);

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
            void AppendArgTag(const char* pParam, const char* pType, const char* pValue);
            void AppendArgTag(const char* pParam, const char* pType, const std::string& value);
            void AppendArgTagFast(const char* pParam, const char* pType, const char* pValue);
            void AppendArgTagFast(const char* pParam, const char* pType, const std::string& value);
            void PrependArgTag(const char* pParam, const char* pType, const char* pValue);
            void PrependArgTag(const char* pParam, const char* pType, const std::string& value);
            void PrependArgTagFast(const char* pParam, const char* pType, const char* pValue);
            void PrependArgTagFast(const char* pParam, const char* pType, const std::string& value);

            bool SetError(const std::string& error);
            bool AppendError(const std::string& error);

            /*************************************************************
            * @brief Prints message via either m_RawOutput or AppendArgTagFast
            *************************************************************/
            void PrintCLIMessage(std::ostringstream* printString, bool add_raw_lf = true);
            void PrintCLIMessage(std::string* printString, bool add_raw_lf = true);
            void PrintCLIMessage(const char* printString, bool add_raw_lf = true);
            void PrintCLIMessage_Justify(const char* prefixString, const char* printString, int column_width, const char* commentString = 0);
            void PrintCLIMessage_Item(const char* prefixString, soar_module::named_object* printObject, int column_width, bool add_raw_lf = true);
            void PrintCLIMessage_Header(const char* headerString, int column_width, bool add_raw_lf = true);
            void PrintCLIMessage_Section(const char* headerString, int column_width, bool add_raw_lf = true);

            // utility for kernel SML
            bool IsLogOpen();

            bool GetCurrentWorkingDirectory(std::string& directory);

        private:

            /* Previous top-level commands that are now sub-commands */
            bool DoAddWME(const std::string& id, std::string attribute, const std::string& value, bool acceptable);
            bool DoAllocate(const std::string& pool, int blocks);
            bool DoCaptureInput(eCaptureInputMode mode, bool autoflush = false, std::string* pathname = 0);
            bool DoCLog(const eLogMode mode = LOG_QUERY, const std::string* pFilename = 0, const std::string* pToAdd = 0, bool silent = false);
            bool DoCommandToFile(const eLogMode mode, const std::string& filename, std::vector< std::string >& argv);
            bool DoExcise(const ExciseBitset& options, const std::string* pProduction = 0);
            bool DoFiringCounts(PrintBitset options, const int numberToList = -1, const std::string* pProduction = 0);
            bool DoIndifferentSelection(const char pOp = 0, const std::string* p1 = 0, const std::string* p2 = 0, const std::string* p3 = 0);
            bool DoLoadLibrary(const std::string& libraryCommand);
            bool DoMatches(const eMatchesMode mode, const eWMEDetail detail = WME_DETAIL_NONE, const std::string* pProduction = 0);
            bool DoMemories(const MemoriesBitset options, int n = 0, const std::string* pProduction = 0);
            bool DoMultiAttributes(const std::string* pAttribute = 0, int n = 0);
            bool DoNumericIndifferentMode(bool query, bool usesAvgNIM);
            bool DoPbreak(const char& mode, const std::string& production);
            bool DoPredict();
            bool DoProductionFind(const ProductionFindBitset& options, const std::string& pattern);
            bool DoPWatch(bool query = true, const std::string* pProduction = 0, bool setting = false);
            bool DoRemoveWME(uint64_t timetag);
            bool DoReplayInput(eReplayInputMode mode, std::string* pathname);
            bool DoReteNet(bool save, std::string filename);
            bool DoSelect(const std::string* pOp = 0);
            bool DoSource(std::string filename, SourceBitset* pOptions = 0);
            bool DoTime(std::vector<std::string>& argv);
            bool DoWatchWMEs(const eWatchWMEsMode mode, WatchWMEsTypeBitset type, const std::string* pIdString = 0, const std::string* pAttributeString = 0, const std::string* pValueString = 0);
            bool DoWMA(const char pOp = 0, const std::string* pAttr = 0, const std::string* pVal = 0);


            bool ParseAllocate(std::vector< std::string >& argv);
            bool ParseClog(std::vector< std::string >& argv);
            bool ParseCTF(std::vector< std::string >& argv);
            bool ParseExcise(std::vector< std::string >& argv);
            bool ParseFC(std::vector< std::string >& argv);
            bool ParseMemories(std::vector< std::string >& argv);
            bool ParseMatches(std::vector< std::string >& argv);
            bool ParseMultiAttributes(std::vector< std::string >& argv);
            bool ParsePBreak(std::vector< std::string >& argv);
            bool ParsePFind(std::vector< std::string >& argv);
            bool ParsePWatch(std::vector< std::string >& argv);
            bool ParseReplayInput(std::vector< std::string >& argv);
            bool ParseSource(std::vector< std::string >& argv);
            bool ParseReteLoad(std::vector< std::string >& argv);
            bool ParseLoadLibrary(std::vector< std::string >& argv);
            bool ParseCaptureInput(std::vector< std::string >& argv);
            bool ParseReteSave(std::vector< std::string >& argv);
            bool ParseIndifferentSelection(std::vector< std::string >& argv);
            bool ParseNumericIndifferentMode(std::vector< std::string >& argv);
            bool ParsePredict(std::vector< std::string >& argv);
            bool ParseSelect(std::vector< std::string >& argv);
            bool ParseSRand(std::vector< std::string >& argv);
            bool ParseWMEAdd(std::vector< std::string >& argv);
            bool ParseWMERemove(std::vector< std::string >& argv);
            bool ParseWMEWatch(std::vector< std::string >& argv);
            bool ParseWMA(std::vector< std::string >& argv);

            void Run_DC(agent* thisAgent, int run_count);
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

            /*************************************************************
            * @brief clears m_XMLResult
            *************************************************************/
            void XMLResultToResponse(char const* pCommandName) ;

            void GetSystemStats(); // for stats
            void GetMemoryStats(); // for stats
            void GetMaxStats(); // for stats
            void GetReteStats(); // for stats
            void GetAgentStats(); // for stats

            bool Evaluate(const char* pInput); // source, formerly StreamSource

            // These help manage nested CLI calls
            void PushCall(CallData callData);
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

/*
* This procedure parses a string to determine if it is a
*      lexeme for an identifier or context variable.
*
*      Many interface routines take identifiers as arguments.
*      These ids can be given as normal ids, or as special variables
*      such as <s> for the current state, etc.  This routine reads
*      (without consuming it) an identifier or context variable,
*      and returns a pointer (Symbol *) to the id.  (In the case of
*      context variables, the instantiated variable is returned.  If
*      any error occurs (e.g., no such id, no instantiation of the
*      variable), an error message is printed and NIL is returned.
*
* Results:
* Pointer to a symbol for the variable or NIL.
*
* Side effects:
* None.
*
===============================
*/
namespace soar {
    class Lexeme;
}
extern bool read_id_or_context_var_from_string(agent* thisAgent, const char* the_lexeme, Symbol** result_id);
extern Symbol* read_identifier_or_context_variable(agent* thisAgent, soar::Lexeme* lexeme);

#endif //COMMAND_LINE_INTERFACE_H
