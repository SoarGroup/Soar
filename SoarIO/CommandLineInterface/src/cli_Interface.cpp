#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

using namespace cli;

EXPORT bool CommandLineInterface::DoAddWME(sml::Connection* pConnection, sml::ElementXML* pResponse, gSKI::IAgent* pAgent, const std::string& id, const std::string& attribute, const std::string& value, bool acceptable) {
	bool ret = DoAddWME(pAgent, id, attribute, value, acceptable);
	GetLastResultSML(pConnection, pResponse);
	return ret;
}

EXPORT bool CommandLineInterface::DoAlias(sml::Connection* pConnection, sml::ElementXML* pResponse, const std::string* pCommand, const std::vector<std::string>* pSubstitution) {
	bool ret = DoAlias(pCommand, pSubstitution);
	GetLastResultSML(pConnection, pResponse);
	return ret;
}

EXPORT bool CommandLineInterface::DoCD(sml::Connection* pConnection, sml::ElementXML* pResponse, const std::string* pDirectory) {
	bool ret = DoCD(pDirectory);
	GetLastResultSML(pConnection, pResponse);
	return ret;
}

EXPORT bool CommandLineInterface::DoChunkNameFormat(sml::Connection* pConnection, sml::ElementXML* pResponse, gSKI::IAgent* pAgent, const bool* pLongFormat, const int* pCount, const std::string* pPrefix) {
	bool ret = DoChunkNameFormat(pAgent, pLongFormat, pCount, pPrefix);
	GetLastResultSML(pConnection, pResponse);
	return ret;
}

EXPORT bool CommandLineInterface::DoDefaultWMEDepth(sml::Connection* pConnection, sml::ElementXML* pResponse, gSKI::IAgent* pAgent, const int* pDepth) {
	bool ret = DoDefaultWMEDepth(pAgent, pDepth);
	GetLastResultSML(pConnection, pResponse);
	return ret;
}

EXPORT bool CommandLineInterface::DoDirs(sml::Connection* pConnection, sml::ElementXML* pResponse) {
	bool ret = DoDirs();
	GetLastResultSML(pConnection, pResponse);
	return ret;
}

EXPORT bool CommandLineInterface::DoEcho(sml::Connection* pConnection, sml::ElementXML* pResponse, const std::vector<std::string>& argv) {
	bool ret = DoEcho(argv);
	GetLastResultSML(pConnection, pResponse);
	return ret;
}

EXPORT bool CommandLineInterface::DoExcise(sml::Connection* pConnection, sml::ElementXML* pResponse, gSKI::IAgent* pAgent, const ExciseBitset& options, const std::string* pProduction) {
	bool ret = DoExcise(pAgent, options, pProduction);
	GetLastResultSML(pConnection, pResponse);
	return ret;
}

EXPORT bool CommandLineInterface::DoExplainBacktraces(sml::Connection* pConnection, sml::ElementXML* pResponse, gSKI::IAgent* pAgent, const std::string* pProduction, const int condition) {
	bool ret = DoExplainBacktraces(pAgent, pProduction, condition);
	GetLastResultSML(pConnection, pResponse);
	return ret;
}

EXPORT bool CommandLineInterface::DoFiringCounts(sml::Connection* pConnection, sml::ElementXML* pResponse, gSKI::IAgent* pAgent, const int numberToList, const std::string* pProduction) {
	bool ret = DoFiringCounts(pAgent, numberToList, pProduction);
	GetLastResultSML(pConnection, pResponse);
	return ret;
}

EXPORT bool CommandLineInterface::DoGDSPrint(sml::Connection* pConnection, sml::ElementXML* pResponse, gSKI::IAgent* pAgent) {
	bool ret = DoGDSPrint(pAgent);
	GetLastResultSML(pConnection, pResponse);
	return ret;
}

EXPORT bool CommandLineInterface::DoHelp(sml::Connection* pConnection, sml::ElementXML* pResponse, const std::string* pCommand) {
	bool ret = DoHelp(pCommand);
	GetLastResultSML(pConnection, pResponse);
	return ret;
}

EXPORT bool CommandLineInterface::DoHelpEx(sml::Connection* pConnection, sml::ElementXML* pResponse, const std::string& command) {
	bool ret = DoHelpEx(command);
	GetLastResultSML(pConnection, pResponse);
	return ret;
}

EXPORT bool CommandLineInterface::DoHome(sml::Connection* pConnection, sml::ElementXML* pResponse, const std::string* pDirectory) {
	bool ret = DoHome(pDirectory);
	GetLastResultSML(pConnection, pResponse);
	return ret;
}

EXPORT bool CommandLineInterface::DoIndifferentSelection(sml::Connection* pConnection, sml::ElementXML* pResponse, gSKI::IAgent* pAgent, eIndifferentMode mode) {
	bool ret = DoIndifferentSelection(pAgent, mode);
	GetLastResultSML(pConnection, pResponse);
	return ret;
}

EXPORT bool CommandLineInterface::DoInitSoar(sml::Connection* pConnection, sml::ElementXML* pResponse, gSKI::IAgent* pAgent) {
	bool ret = DoInitSoar(pAgent);
	GetLastResultSML(pConnection, pResponse);
	return ret;
}

EXPORT bool CommandLineInterface::DoInternalSymbols(sml::Connection* pConnection, sml::ElementXML* pResponse, gSKI::IAgent* pAgent) {
	bool ret = DoInternalSymbols(pAgent);
	GetLastResultSML(pConnection, pResponse);
	return ret;
}

EXPORT bool CommandLineInterface::DoLearn(sml::Connection* pConnection, sml::ElementXML* pResponse, gSKI::IAgent* pAgent, const LearnBitset& options) {
	bool ret = DoLearn(pAgent, options);
	GetLastResultSML(pConnection, pResponse);
	return ret;
}

EXPORT bool CommandLineInterface::DoLog(sml::Connection* pConnection, sml::ElementXML* pResponse, gSKI::IAgent* pAgent, const eLogMode mode, const std::string* pFilename, const std::string* pToAdd) {
	bool ret = DoLog(pAgent, mode, pFilename, pToAdd);
	GetLastResultSML(pConnection, pResponse);
	return ret;
}

EXPORT bool CommandLineInterface::DoLS(sml::Connection* pConnection, sml::ElementXML* pResponse) {
	bool ret = DoLS();
	GetLastResultSML(pConnection, pResponse);
	return ret;
}

EXPORT bool CommandLineInterface::DoMatches(sml::Connection* pConnection, sml::ElementXML* pResponse, gSKI::IAgent* pAgent, const eMatchesMode mode, const eWMEDetail detail, const std::string* pProduction) {
	bool ret = DoMatches(pAgent, mode, detail, pProduction);
	GetLastResultSML(pConnection, pResponse);
	return ret;
}

EXPORT bool CommandLineInterface::DoMaxChunks(sml::Connection* pConnection, sml::ElementXML* pResponse, gSKI::IAgent* pAgent, const int n) {
	bool ret = DoMaxChunks(pAgent, n);
	GetLastResultSML(pConnection, pResponse);
	return ret;
}

EXPORT bool CommandLineInterface::DoMaxElaborations(sml::Connection* pConnection, sml::ElementXML* pResponse, gSKI::IAgent* pAgent, const int n) {
	bool ret = DoMaxElaborations(pAgent, n);
	GetLastResultSML(pConnection, pResponse);
	return ret;
}

EXPORT bool CommandLineInterface::DoMaxNilOutputCycles(sml::Connection* pConnection, sml::ElementXML* pResponse, gSKI::IAgent* pAgent, const int n) {
	bool ret = DoMaxNilOutputCycles(pAgent, n);
	GetLastResultSML(pConnection, pResponse);
	return ret;
}

EXPORT bool CommandLineInterface::DoMemories(sml::Connection* pConnection, sml::ElementXML* pResponse, gSKI::IAgent* pAgent, const MemoriesBitset options, int n, const std::string* pProduction) {
	bool ret = DoMemories(pAgent, options, n, pProduction);
	GetLastResultSML(pConnection, pResponse);
	return ret;
}

EXPORT bool CommandLineInterface::DoMultiAttributes(sml::Connection* pConnection, sml::ElementXML* pResponse, gSKI::IAgent* pAgent, const std::string* pAttribute, int n) {
	bool ret = DoMultiAttributes(pAgent, pAttribute, n);
	GetLastResultSML(pConnection, pResponse);
	return ret;
}

EXPORT bool CommandLineInterface::DoNumericIndifferentMode(sml::Connection* pConnection, sml::ElementXML* pResponse, gSKI::IAgent* pAgent, const eNumericIndifferentMode mode) {
	bool ret = DoNumericIndifferentMode(pAgent, mode);
	GetLastResultSML(pConnection, pResponse);
	return ret;
}

EXPORT bool CommandLineInterface::DoOSupportMode(sml::Connection* pConnection, sml::ElementXML* pResponse, gSKI::IAgent* pAgent, int mode) {
	bool ret = DoOSupportMode(pAgent, mode);
	GetLastResultSML(pConnection, pResponse);
	return ret;
}

EXPORT bool CommandLineInterface::DoPopD(sml::Connection* pConnection, sml::ElementXML* pResponse) {
	bool ret = DoPopD();
	GetLastResultSML(pConnection, pResponse);
	return ret;
}

EXPORT bool CommandLineInterface::DoPreferences(sml::Connection* pConnection, sml::ElementXML* pResponse, gSKI::IAgent* pAgent, const ePreferencesDetail detail, const std::string* pId, const std::string* pAttribute) {
	bool ret = DoPreferences(pAgent, detail, pId, pAttribute);
	GetLastResultSML(pConnection, pResponse);
	return ret;
}

EXPORT bool CommandLineInterface::DoPrint(sml::Connection* pConnection, sml::ElementXML* pResponse, gSKI::IAgent* pAgent, PrintBitset options, int depth, const std::string* pArg) {
	bool ret = DoPrint(pAgent, options, depth, pArg);
	GetLastResultSML(pConnection, pResponse);
	return ret;
}

EXPORT bool CommandLineInterface::DoProductionFind(sml::Connection* pConnection, sml::ElementXML* pResponse, gSKI::IAgent* pAgent, const ProductionFindBitset& options, const std::string& pattern) {
	bool ret = DoProductionFind(pAgent, options, pattern);
	GetLastResultSML(pConnection, pResponse);
	return ret;
}

EXPORT bool CommandLineInterface::DoPushD(sml::Connection* pConnection, sml::ElementXML* pResponse, const std::string& directory) {
	bool ret = DoPushD(directory);
	GetLastResultSML(pConnection, pResponse);
	return ret;
}

EXPORT bool CommandLineInterface::DoPWatch(sml::Connection* pConnection, sml::ElementXML* pResponse, gSKI::IAgent* pAgent, bool query, const std::string* pProduction, bool setting) {
	bool ret = DoPWatch(pAgent, query, pProduction, setting);
	GetLastResultSML(pConnection, pResponse);
	return ret;
}

EXPORT bool CommandLineInterface::DoPWD(sml::Connection* pConnection, sml::ElementXML* pResponse) {
	bool ret = DoPWD();
	GetLastResultSML(pConnection, pResponse);
	return ret;
}

EXPORT bool CommandLineInterface::DoQuit(sml::Connection* pConnection, sml::ElementXML* pResponse) {
	bool ret = DoQuit();
	GetLastResultSML(pConnection, pResponse);
	return ret;
}

EXPORT bool CommandLineInterface::DoRemoveWME(sml::Connection* pConnection, sml::ElementXML* pResponse, gSKI::IAgent* pAgent, int timetag) {
	bool ret = DoRemoveWME(pAgent, timetag);
	GetLastResultSML(pConnection, pResponse);
	return ret;
}

EXPORT bool CommandLineInterface::DoReteNet(sml::Connection* pConnection, sml::ElementXML* pResponse, gSKI::IAgent* pAgent, bool save, const std::string& filename) {
	bool ret = DoReteNet(pAgent, save, filename);
	GetLastResultSML(pConnection, pResponse);
	return ret;
}

EXPORT bool CommandLineInterface::DoRun(sml::Connection* pConnection, sml::ElementXML* pResponse, gSKI::IAgent* pAgent, const RunBitset& options, int count) {
	bool ret = DoRun(pAgent, options, count);
	GetLastResultSML(pConnection, pResponse);
	return ret;
}

EXPORT bool CommandLineInterface::DoSaveBacktraces(sml::Connection* pConnection, sml::ElementXML* pResponse, gSKI::IAgent* pAgent, bool* pSetting) {
	bool ret = DoSaveBacktraces(pAgent, pSetting);
	GetLastResultSML(pConnection, pResponse);
	return ret;
}

EXPORT bool CommandLineInterface::DoSoar8(sml::Connection* pConnection, sml::ElementXML* pResponse, bool* pSoar8) {
	bool ret = DoSoar8(pSoar8);
	GetLastResultSML(pConnection, pResponse);
	return ret;
}

EXPORT bool CommandLineInterface::DoSoarNews(sml::Connection* pConnection, sml::ElementXML* pResponse) {
	bool ret = DoSoarNews();
	GetLastResultSML(pConnection, pResponse);
	return ret;
}

EXPORT bool CommandLineInterface::DoSource(sml::Connection* pConnection, sml::ElementXML* pResponse, gSKI::IAgent* pAgent, std::string filename) {
	bool ret = DoSource(pAgent, filename);
	GetLastResultSML(pConnection, pResponse);
	return ret;
}

EXPORT bool CommandLineInterface::DoSP(sml::Connection* pConnection, sml::ElementXML* pResponse, gSKI::IAgent* pAgent, const std::string& production) {
	bool ret = DoSP(pAgent, production);
	GetLastResultSML(pConnection, pResponse);
	return ret;
}

EXPORT bool CommandLineInterface::DoStats(sml::Connection* pConnection, sml::ElementXML* pResponse, gSKI::IAgent* pAgent, const StatsBitset& options) {
	bool ret = DoStats(pAgent, options);
	GetLastResultSML(pConnection, pResponse);
	return ret;
}

EXPORT bool CommandLineInterface::DoStopSoar(sml::Connection* pConnection, sml::ElementXML* pResponse, gSKI::IAgent* pAgent, bool self, const std::string* reasonForStopping) {
	bool ret = DoStopSoar(pAgent, self, reasonForStopping);
	GetLastResultSML(pConnection, pResponse);
	return ret;
}

EXPORT bool CommandLineInterface::DoTime(sml::Connection* pConnection, sml::ElementXML* pResponse, gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	bool ret = DoTime(pAgent, argv);
	GetLastResultSML(pConnection, pResponse);
	return ret;
}

EXPORT bool CommandLineInterface::DoTimers(sml::Connection* pConnection, sml::ElementXML* pResponse, gSKI::IAgent* pAgent, bool* pSetting) {
	bool ret = DoTimers(pAgent, pSetting);
	GetLastResultSML(pConnection, pResponse);
	return ret;
}

EXPORT bool CommandLineInterface::DoVerbose(sml::Connection* pConnection, sml::ElementXML* pResponse, gSKI::IAgent* pAgent, bool* pSetting) {
	bool ret = DoVerbose(pAgent, pSetting);
	GetLastResultSML(pConnection, pResponse);
	return ret;
}

EXPORT bool CommandLineInterface::DoVersion(sml::Connection* pConnection, sml::ElementXML* pResponse) {
	bool ret = DoVersion();
	GetLastResultSML(pConnection, pResponse);
	return ret;
}

EXPORT bool CommandLineInterface::DoWaitSNC(sml::Connection* pConnection, sml::ElementXML* pResponse, gSKI::IAgent* pAgent, bool* pSetting) {
	bool ret = DoWaitSNC(pAgent, pSetting);
	GetLastResultSML(pConnection, pResponse);
	return ret;
}

EXPORT bool CommandLineInterface::DoWarnings(sml::Connection* pConnection, sml::ElementXML* pResponse, gSKI::IAgent* pAgent, bool* pSetting) {
	bool ret = DoWarnings(pAgent, pSetting);
	GetLastResultSML(pConnection, pResponse);
	return ret;
}

EXPORT bool CommandLineInterface::DoWatch(sml::Connection* pConnection, sml::ElementXML* pResponse, gSKI::IAgent* pAgent, const WatchBitset& options, const WatchBitset& settings, const int wmeSetting, const int learnSetting) {
	bool ret = DoWatch(pAgent, options, settings, wmeSetting, learnSetting);
	GetLastResultSML(pConnection, pResponse);
	return ret;
}

EXPORT bool CommandLineInterface::DoWatchWMEs(sml::Connection* pConnection, sml::ElementXML* pResponse, gSKI::IAgent* pAgent, const eWatchWMEsMode mode, WatchWMEsTypeBitset type, const std::string* pIdString, const std::string* pAttributeString, const std::string* pValueString) {
	bool ret = DoWatchWMEs(pAgent, mode, type, pIdString, pAttributeString, pValueString);
	GetLastResultSML(pConnection, pResponse);
	return ret;
}
