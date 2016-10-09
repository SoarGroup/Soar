#ifndef CLI_CLI_H
#define CLI_CLI_H

#include "cli_Cli_enums.h"

#include "sml_Events.h"

#include <bitset>
#include <stdint.h>
#include <vector>

typedef uint64_t epmem_time_id;
namespace soar_module
{
    class named_object;
}

namespace cli
{
    class Cli
    {
        public:
            virtual ~Cli() {}

            virtual bool SetError(const std::string& error) = 0;
            virtual bool AppendError(const std::string& error) = 0;

            virtual bool DoAlias(std::vector< std::string >* argv = 0, bool doRemove = 0) = 0;
            virtual bool DoChunk(const std::string* pAttr = 0, const std::string* pVal = 0) = 0;
            virtual bool DoCD(const std::string* pDirectory = 0) = 0;
            virtual bool DoCLog(const eLogMode mode = LOG_QUERY, const std::string* pFilename = 0, const std::string* pToAdd = 0, bool silent = false) = 0;
            virtual bool DoDebug(std::vector< std::string >* argv = 0) = 0;
            virtual bool DoDecide(std::vector<std::string>& argv, const std::string& pCmd) = 0;
            virtual bool DoDirs() = 0;
            virtual bool DoEcho(const std::vector<std::string>& argv, bool echoNewline) = 0;
            virtual bool DoEpMem(const char pOp = 0, const std::string* pAttr = 0, const std::string* pVal = 0, const epmem_time_id memory_id = 0) = 0;
            virtual bool DoExplain(const std::string* pArg = 0, const std::string* pArg2 = 0) = 0;
            virtual bool DoGP(const std::string& productionString) = 0;
            virtual bool DoHelp(const std::vector<std::string>& argv) = 0;
            virtual bool DoLearn(const LearnBitset& options) = 0;
            virtual bool DoLoad(std::vector<std::string>& argv, const std::string& pCmd) = 0;
            virtual bool DoLS() = 0;
            virtual bool DoOutput(std::vector<std::string>& argv, const std::string* pArg1 = 0, const std::string* pArg2 = 0) = 0;
            virtual bool DoPopD() = 0;
            virtual bool DoPreferences(const ePreferencesDetail detail, const bool object, const std::string* pId = 0, const std::string* pAttribute = 0) = 0;
            virtual bool DoPrint(PrintBitset options, int depth, const std::string* pArg = 0) = 0;
            virtual bool DoProduction(std::vector<std::string>& argv, const std::string& pCmd) = 0;
            virtual bool DoPushD(const std::string& directory) = 0;
            virtual bool DoPWD() = 0;
            virtual bool DoRL(const char pOp = 0, const std::string* pAttr = 0, const std::string* pVal = 0) = 0;
            virtual bool DoRun(const RunBitset& options, int count = 0, eRunInterleaveMode interleave = RUN_INTERLEAVE_DEFAULT) = 0;
            virtual bool DoSVS(const std::vector<std::string>& args) = 0;
            virtual bool DoSMem(const char pOp = 0, const std::string* pArg1 = 0, const std::string* pArg2 = 0, const std::string* pArg3 = 0) = 0;
            virtual bool DoSave(std::vector<std::string>& argv, const std::string& pCmd) = 0;
            virtual bool DoSoar(const char pOp = 0, const std::string* pArg1 = 0, const std::string* pArg2 = 0, const std::string* pArg3 = 0) = 0;
            virtual bool DoSP(const std::string& production) = 0;
            virtual bool DoStats(const StatsBitset& options, int sort = 0) = 0;
            virtual bool DoTclCommand(const std::string& pMessage) = 0;
            virtual bool DoTime(std::vector<std::string>& argv) = 0;
            virtual bool DoTrace(const WatchBitset& options, const WatchBitset& settings, const int wmeSetting, const int learnSetting) = 0;
            virtual bool DoTraceBackwardsCompatible(std::vector< std::string >& argv, bool fromTraceLevel = false) = 0;
            virtual bool DoVisualize(const std::string* pArg = 0, const std::string* pArg2 = 0, const std::string* pArg3 = 0) = 0;
            virtual bool DoWM(std::vector<std::string>& argv, const std::string& pCmd) = 0;

            /*************************************************************
            * @brief Prints message via either m_RawOutput or AppendArgTagFast
            *************************************************************/
            virtual void PrintCLIMessage(std::ostringstream* printString, bool add_raw_lf = true) = 0;
            virtual void PrintCLIMessage(std::string* printString, bool add_raw_lf = true) = 0;
            virtual void PrintCLIMessage(const char* printString, bool add_raw_lf = true) = 0;
            virtual void PrintCLIMessage_Justify(const char* prefixString, const char* printString, int column_width, bool add_raw_lf = true) = 0;
            virtual void PrintCLIMessage_Item(const char* prefixString, soar_module::named_object* printObject, int column_width, bool add_raw_lf = true) = 0;
            virtual void PrintCLIMessage_Header(const char* headerString, int column_width, bool add_raw_lf = true) = 0;
            virtual void PrintCLIMessage_Section(const char* headerString, int column_width, bool add_raw_lf = true) = 0;
    };
}

#endif // CLI_CLI_H
