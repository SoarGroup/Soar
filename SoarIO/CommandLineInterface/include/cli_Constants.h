#ifndef CLI_CONSTANTS_H
#define CLI_CONSTANTS_H

#include <string>
#ifdef _MSC_VER
#pragma warning (disable : 4702)  // warning C4702: unreachable code, need to disable for VS.NET 2003 due to STL "bug" in certain cases
#endif
#include <map>
#include <list>
#ifdef _MSC_VER
#pragma warning (default : 4702)
#endif

namespace cli {

	typedef std::map<std::string, std::string> UsageMap;
	typedef std::map<std::string, std::string>::iterator UsageMapIter;
	typedef std::map<std::string, std::string>::const_iterator UsageMapConstIter;

	// A class containing the constants used by CommandLineInterface
	class Constants
	{
	public:

		Constants();
		
		std::list<std::string> GetCommandList();
		bool GetUsageFor(const std::string& command, std::string& output);
		bool GetExtendedUsageFor(const std::string& command, std::string& output);
		bool IsUsageFileAvailable();

		static char const* kCLIAddWME;
		static char const* kCLIAlias;
		static char const* kCLICD;
		static char const* kCLIDefaultWMEDepth;
		static char const* kCLIDirs;
		static char const* kCLIEcho;
		static char const* kCLIExcise;
		static char const* kCLIFiringCounts;
		static char const* kCLIGDSPrint;
		static char const* kCLIHelp;
		static char const* kCLIHelpEx;
		static char const* kCLIHome;
		static char const* kCLIIndifferentSelection;
		static char const* kCLIInitSoar;
		static char const* kCLILearn;
		static char const* kCLILog;
		static char const* kCLILS;
		static char const* kCLIMatches;
		static char const* kCLIMaxChunks;
		static char const* kCLIMaxElaborations;
		static char const* kCLIMaxNilOutputCycles;
		static char const* kCLIMemories;
		static char const* kCLIMultiAttributes;
		static char const* kCLINumericIndifferentMode;
		static char const* kCLIOSupportMode;
		static char const* kCLIPopD;
		static char const* kCLIPreferences;
		static char const* kCLIPrint;
		static char const* kCLIProductionFind;
		static char const* kCLIPushD;
		static char const* kCLIPWatch;
		static char const* kCLIPWD;
		static char const* kCLIQuit;
		static char const* kCLIRemoveWME;
		static char const* kCLIReteNet;
		static char const* kCLIRun;
		static char const* kCLISoar8;
		static char const* kCLISource;
		static char const* kCLISP;
		static char const* kCLIStats;
		static char const* kCLIStopSoar;
		static char const* kCLITime;
		static char const* kCLITimers;
		static char const* kCLIVerbose;
		static char const* kCLIVersion;
		static char const* kCLIWaitSNC;
		static char const* kCLIWarnings;
		static char const* kCLIWatch;

	private:
		void LoadUsage(std::ifstream& usageFile);
		std::string GetUsage(std::ifstream& usageFile);
		std::string GetExtendedUsage(std::ifstream& usageFile);

		bool m_UsageFileAvailable;
		UsageMap m_UsageMap;
		UsageMap m_ExtendedUsageMap;
	};

} // namespace cli

#endif // CLI_CONSTANTS_H
