#ifndef CLIPARSERTEST_H
#define CLIPARSERTEST_H

#include "cli_Commands.h"
#include "cli_Parser.h"

class CliAdapter : public cli::Cli
{
public:
    virtual ~CliAdapter() {}

    virtual bool SetError(const std::string& error) { return false; };
    virtual bool DoAddWME(const std::string& id, std::string attribute, const std::string& value, bool acceptable) { return false; }
    virtual bool DoAlias(std::vector< std::string >* argv = 0) { return false; }
    virtual bool DoAllocate(const std::string& pool, int blocks) { return false; }
    virtual bool DoCaptureInput(eCaptureInputMode mode, bool autoflush = false, std::string* pathname = 0) { return false; }
    virtual bool DoCD(const std::string* pDirectory = 0) { return false; }
    virtual bool DoChunkNameFormat(const bool* pLongFormat = 0, const int64_t* pCount = 0, const std::string* pPrefix = 0) { return false; }
    virtual bool DoCLog(const eLogMode mode = LOG_QUERY, const std::string* pFilename = 0, const std::string* pToAdd = 0, bool silent = false) { return false; }
    virtual bool DoCommandToFile(const eLogMode mode, const std::string& filename, std::vector< std::string >& argv) { return false; }
    virtual bool DoDefaultWMEDepth(const int* pDepth) { return false; }
    virtual bool DoDirs() { return false; }
    virtual bool DoEcho(const std::vector<std::string>& argv, bool echoNewline) { return false; }
    virtual bool DoEchoCommands(bool onlyGetValue, bool echoCommands) { return false; }
    virtual bool DoEditProduction(std::string productionName) { return false; }
    virtual bool DoEpMem(const char pOp = 0, const std::string *pAttr = 0, const std::string *pVal = 0, const epmem_time_id memory_id = 0) { return false; }
    virtual bool DoExcise(const ExciseBitset& options, const std::string* pProduction = 0) { return false; }
    virtual bool DoExplainBacktraces(const std::string* pProduction = 0, const int condition = 0) { return false; }
    virtual bool DoFiringCounts(const int numberToList = -1, const std::string* pProduction = 0) { return false; }
    virtual bool DoGDSPrint() { return false; }
    virtual bool DoGP(const std::string& productionString) { return false; }
    virtual bool DoGPMax(const int& maximum) { return false; }
    virtual bool DoHelp(const std::vector<std::string> &argv) { return false; }
    virtual bool DoIndifferentSelection( const char pOp = 0, const std::string* p1 = 0, const std::string* p2 = 0, const std::string* p3 = 0 ) { return false; }
    virtual bool DoInitSoar() { return false; }
    virtual bool DoInternalSymbols() { return false; }
    virtual bool DoLearn(const LearnBitset& options) { return false; }
    virtual bool DoLoadLibrary(const std::string& libraryCommand) { return false; }
    virtual bool DoLS() { return false; }
    virtual bool DoMatches(const eMatchesMode mode, const eWMEDetail detail = WME_DETAIL_NONE, const std::string* pProduction = 0) { return false; }
    virtual bool DoMaxChunks(const int n = 0) { return false; }
    virtual bool DoMaxDCTime(const int n = 0) { return false; }
    virtual bool DoMaxElaborations(const int n = 0) { return false; }
    virtual bool DoMaxGoalDepth(const int n = 0) { return false; }
    virtual bool DoMaxMemoryUsage(const int n = 0) { return false; }
    virtual bool DoMaxNilOutputCycles(const int n = 0) { return false; }
    virtual bool DoMemories(const MemoriesBitset options, int n = 0, const std::string* pProduction = 0) { return false; }
    virtual bool DoMultiAttributes(const std::string* pAttribute = 0, int n = 0) { return false; }
    virtual bool DoNumericIndifferentMode(bool query, const ni_mode mode) { return false; }
    virtual bool DoOSupportMode(int mode = -1) { return false; }
    virtual bool DoPopD() { return false; }
    virtual bool DoPort() { return false; }
    virtual bool DoPredict() { return false; }
    virtual bool DoPreferences(const ePreferencesDetail detail, const bool object, const std::string* pId = 0, const std::string* pAttribute = 0) { return false; }
    virtual bool DoPrint(PrintBitset options, int depth, const std::string* pArg = 0) { return false; }
    virtual bool DoProductionFind(const ProductionFindBitset& options, const std::string& pattern) { return false; }
    virtual bool DoPushD(const std::string& directory) { return false; }
    virtual bool DoPWatch(bool query = true, const std::string* pProduction = 0, bool setting = false) { return false; }
    virtual bool DoPWD() { return false; }
    virtual bool DoRand( bool integer, std::string* bound ) { return false; }
    virtual bool DoRemoveWME(uint64_t timetag) { return false; }
    virtual bool DoReplayInput(eReplayInputMode mode, std::string* pathname) { return false; }
    virtual bool DoReteNet(bool save, std::string filename) { return false; }
    virtual bool DoRL( const char pOp = 0, const std::string *pAttr = 0, const std::string *pVal = 0 ) { return false; }
    virtual bool DoRun(const RunBitset& options, int count = 0, eRunInterleaveMode interleave = RUN_INTERLEAVE_DEFAULT) { return false; }
    virtual bool DoSaveBacktraces(bool* pSetting = 0) { return false; }
    virtual bool DoSelect(const std::string* pOp = 0) { return false; }
    virtual bool DoSetLibraryLocation(std::string* pLocation = 0) { return false; } 
    virtual bool DoSetStopPhase(bool setPhase, bool before, sml::smlPhase phase) { return false; }
    virtual bool DoSMem(const char pOp = 0, const std::string *pAttr = 0, const std::string *pVal = 0) { return false; }
    virtual bool DoSoarNews() { return false; }
    virtual bool DoSource(std::string filename, SourceBitset* pOptions = 0) { return false; }
    virtual bool DoSP(const std::string& production) { return false; }
    virtual bool DoSRand(uint32_t* pSeed = 0) { return false; }
    virtual bool DoStats(const StatsBitset& options, int sort = 0) { return false; }
    virtual bool DoStopSoar(bool self, const std::string* reasonForStopping = 0) { return false; }
    virtual bool DoTime(std::vector<std::string>& argv) { return false; }
    virtual bool DoTimers(bool* pSetting = 0) { return false; }
    virtual bool DoUnalias(std::vector<std::string>& argv) { return false; }
    virtual bool DoVerbose(bool* pSetting = 0) { return false; }
    virtual bool DoVersion() { return false; }
    virtual bool DoWaitSNC(bool* pSetting = 0) { return false; }
    virtual bool DoWarnings(bool* pSetting = 0) { return false; }
    virtual bool DoWatch(const WatchBitset& options, const WatchBitset& settings, const int wmeSetting, const int learnSetting) { return false; }
    virtual bool DoWatchWMEs(const eWatchWMEsMode mode, WatchWMEsTypeBitset type, const std::string* pIdString = 0, const std::string* pAttributeString = 0, const std::string* pValueString = 0) { return false; }
    virtual bool DoWMA( const char pOp = 0, const std::string *pAttr = 0, const std::string *pVal = 0 ) { return false; }
};

class CliEcho : public CliAdapter
{
public:
    virtual ~CliEcho() {}

    void SetExpected(unsigned numArgs, bool newLine)
    {
        this->numArgs = numArgs;
        this->newLine = newLine;
    }

    virtual bool DoEcho(const std::vector<std::string>& argv, bool echoNewline) 
    { 
        return (argv.size() == numArgs) && (echoNewline == newLine);
    }
private:
    std::vector<std::string>::size_type numArgs;
    bool newLine;
};

class CliMaxDCTime : public CliAdapter
{
public:
    virtual ~CliMaxDCTime() {}

    void SetExpected(int n)
    {
        this->n = n;
    }

    virtual bool DoMaxDCTime(const int n) 
    { 
        return this->n == n;
    }
private:
    int n;
};


#endif // CLIPARSERTEST_H
