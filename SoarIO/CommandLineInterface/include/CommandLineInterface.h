#pragma once

#include <map>
#include <vector>
#include <string>

#include "commanddata.h"

// Forward Declarations
namespace gSKI {
	class IAgent;
	class IKernel;
	struct Error;
}
namespace sml {
	class ElementXML;
}

namespace cli {

class CommandLineInterface;

// We need a comparator to make the map we're about to define work with char*
struct strCompareCommand
{
	bool operator()(const char* s1, const char* s2) const
	{
		return std::strcmp(s1, s2) < 0;
	}
};

// Define the CommandFunction which we'll call to process commands
typedef bool (CommandLineInterface::*CommandFunction)(int argc, char**& argv);

// Used to store a map from command name to function handler for that command
typedef std::map<char const*, CommandFunction, strCompareCommand>	CommandMap;
typedef CommandMap::iterator										         CommandMapIter;
typedef CommandMap::const_iterator									      CommandMapConstIter;

class CommandLineInterface
{
public:
	CommandLineInterface(void);
	~CommandLineInterface(void);

	bool DoCommand(gSKI::IAgent* pAgent, gSKI::IKernel* pKernel, const char* pCommandLine, sml::ElementXML* pResponse, gSKI::Error* pError);
	bool DoCommandInternal(const char* commandLine);

	bool QuitCalled() { return m_QuitCalled; }
	void GetLastResult(std::string* pResult);

	//bool Parse(int argc, char**& argv);
	//bool Do();

	bool ParseAddWME(int argc, char**& argv);
	bool DoAddWME();

	bool ParseCD(int argc, char**& argv);
	bool DoCD(const char* directory = 0);

	bool ParseEcho(int argc, char**& argv);
	bool DoEcho(int argc, char**& argv);

	bool ParseExcise(int argc, char**& argv);
	bool DoExcise(const unsigned short options, int productionCount, char**& productions);

	bool ParseInitSoar(int argc, char**& argv);
	bool DoInitSoar();

	bool ParseLearn(int argc, char**& argv);
	bool DoLearn(const unsigned short options = 0);

	bool ParseNewAgent(int argc, char**& argv);
	bool DoNewAgent(char const* agentName);

	bool ParseQuit(int argc, char**& argv);
	bool DoQuit();

	bool ParseRun(int argc, char**& argv);
	bool DoRun(const unsigned short options = 0);

	bool ParseSource(int argc, char**& argv);
	bool DoSource(const char* filename);

	bool ParseSP(int argc, char**& argv);
	bool DoSP(const char* production);

	bool ParseStopSoar(int argc, char**& argv);
	bool DoStopSoar(bool self, char const* reasonForStopping);

	bool ParseWatch(int argc, char**& argv);
	bool DoWatch();

	bool ParseWatchWMEs(int argc, char**& argv);
	bool DoWatchWMEs();

protected:

	int Tokenize(const char* commandLine, std::vector<std::string>& argumentVector);
	void BuildCommandMap();
	bool CheckForHelp(int argc, char**& argv);

	CommandMap	   m_CommandMap;
	bool		      m_QuitCalled;
	gSKI::IKernel* m_pKernel;
	gSKI::IAgent*  m_pAgent;
	std::string    m_Result;
	gSKI::Error*   m_pError;

};

class CLIConstants
{
public:
	static char const* kCLIAddWME;
	static char const* kCLICD;
	static char const* kCLIEcho;
	static char const* kCLIExcise;
	static char const* kCLIExit;
	static char const* kCLIInitSoar;
	static char const* kCLILearn;
	static char const* kCLINewAgent;
	static char const* kCLIQuit;
	static char const* kCLIRun;
	static char const* kCLISource;
	static char const* kCLISP;
	static char const* kCLIStopSoar;
	static char const* kCLIWatch;
	static char const* kCLIWatchWMEs;

	static char const* kCLIAddWMEUsage;
	static char const* kCLICDUsage;
	static char const* kCLIEchoUsage;
	static char const* kCLIExciseUsage;
	static char const* kCLIInitSoarUsage;
	static char const* kCLILearnUsage;
	static char const* kCLINewAgentUsage;
	static char const* kCLIQuitUsage;
	static char const* kCLIRunUsage;
	static char const* kCLISourceUsage;
	static char const* kCLISPUsage;
	static char const* kCLIStopSoarUsage;
	static char const* kCLIWatchUsage;
	static char const* kCLIWatchWMEsUsage;

};

} // namespace cli