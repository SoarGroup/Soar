#pragma once

#include <map>
#include <vector>
#include <string>

#include "IgSKI_Agent.h"
#include "IgSKI_Kernel.h"

#include "sml_ElementXML.h"
#include "sml_TagResult.h"
#include "sml_TagError.h"

#include "commanddata.h"
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
typedef bool (CommandLineInterface::*CommandFunction)(int argc, char**& argv, std::string* result);

// Used to store a map from command name to function handler for that command
typedef std::map<char const*, CommandFunction, strCompareCommand>	CommandMap;
typedef CommandMap::iterator										         CommandMapIter;
typedef CommandMap::const_iterator									      CommandMapConstIter;

class CommandLineInterface
{
public:
	CommandLineInterface(void);
	~CommandLineInterface(void);

   bool DoCommand(gSKI::IAgent* pAgent, gSKI::IKernel* pKernel, const char* pCommandLine, sml::ElementXML* pResponse);
   bool DoCommandInternal(const char* commandLine, std::string* result);

	bool QuitCalled() { return m_QuitCalled; }

	//bool Parse(int argc, char**& argv, std::string* result = 0);
	//bool Do(std::string* result = 0);

	bool ParseAddWME(int argc, char**& argv, std::string* result = 0);
	bool DoAddWME(std::string* result = 0);

	bool ParseCD(int argc, char**& argv, std::string* result = 0);
	bool DoCD(const char* directory = 0, std::string* result = 0);

	bool ParseEcho(int argc, char**& argv, std::string* result = 0);
	bool DoEcho(std::string* result = 0);

	bool ParseExcise(int argc, char**& argv, std::string* result = 0);
	bool DoExcise(const unsigned short options, int productionCount, char**& productions, std::string* result = 0);

	bool ParseInitSoar(int argc, char**& argv, std::string* result = 0);
	bool DoInitSoar(std::string* result = 0);

	bool ParseLearn(int argc, char**& argv, std::string* result = 0);
	bool DoLearn(const unsigned short options = 0, std::string* result = 0);

	bool ParseNewAgent(int argc, char**& argv, std::string* result = 0);
	bool DoNewAgent(char const* agentName, std::string* result = 0);

	bool ParseQuit(int argc, char**& argv, std::string* result = 0);
	bool DoQuit(std::string* result = 0);

	bool ParseRun(int argc, char**& argv, std::string* result = 0);
	bool DoRun(const unsigned short options = 0, std::string* result = 0);

	bool ParseSource(int argc, char**& argv, std::string* result = 0);
	bool DoSource(const char* filename, std::string* result = 0);

   bool ParseSP(int argc, char**& argv, std::string* result = 0);
   bool DoSP(const char* production, std::string* result = 0);

	bool ParseStopSoar(int argc, char**& argv, std::string* result = 0);
	bool DoStopSoar(bool self, char const* reasonForStopping, std::string* result = 0);

	bool ParseWatch(int argc, char**& argv, std::string* result = 0);
	bool DoWatch(std::string* result = 0);

	bool ParseWatchWMEs(int argc, char**& argv, std::string* result = 0);
	bool DoWatchWMEs(std::string* result = 0);

protected:

	int Tokenize(const char* commandLine, std::vector<std::string>& argumentVector, std::string* result = 0);
	void BuildCommandMap();

	CommandMap	m_CommandMap;
	bool		   m_QuitCalled;

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