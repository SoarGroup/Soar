#ifndef CLI_CONSTANTS_H
#define CLI_CONSTANTS_H

#include <string>
#include <map>

namespace cli {

	typedef std::map<std::string, std::string> UsageMap;

	// A class containing the constants used by CommandLineInterface
	class Constants
	{
	public:

		Constants();
		~Constants();

		std::string GetUsageFor(const std::string& command);

		static char const* kCLISyntaxError;

		static char const* kCLIAddWME;
		static char const* kCLICD;
		static char const* kCLIDir;
		static char const* kCLIEcho;
		static char const* kCLIExcise;
		static char const* kCLIExit;
		static char const* kCLIInit;
		static char const* kCLIInitSoar;
		static char const* kCLILearn;
		static char const* kCLILog;
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

	private:
		void LoadUsage(std::ifstream& usageFile);
		std::string GetUsage(std::ifstream& usageFile);

		bool m_UsageFileAvailable;
		UsageMap m_UsageMap;
	};

} // namespace cli

#endif // CLI_CONSTANTS_H