#ifndef CLI_TEST_H
#define CLI_TEST_H

#include <string>

// Forward Declarations
namespace sml {
	class Kernel;
};

class CommandProcessor {
public:
	CommandProcessor(sml::Kernel* pKernel);
	~CommandProcessor();

	void DisplayPrompt();

	// returns false on error or exit
	bool ProcessCharacter();

protected:

	void Backspace();

	// returns false on error or exit
	bool ProcessLine();

	sml::Kernel* pKernel;

	char		inputChar;		// character of input
	std::string commandLine;	// everything typed before \n \r
	std::string output;
	bool		previousResult;	// boolean result of last command
	bool		raw;			// true when output is 'raw' as opposed to 'structured'

	// History variables
	int			historyIndex;
	int			temporaryHistoryIndex;
	std::string* pHistory;

};

#endif // CLI_TEST_H
