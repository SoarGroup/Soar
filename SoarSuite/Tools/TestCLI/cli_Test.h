#ifndef CLI_TEST_H
#define CLI_TEST_H

#include <string>

#include "thread_Thread.h"

// Forward Declarations
namespace sml {
	class Kernel;
};

class InputThread : public soar_thread::Thread {
public:
	InputThread();
	~InputThread();

	void Run();

protected:

	char m_C;
};

class CommandProcessor {
public:
	CommandProcessor(sml::Kernel* pKernel);
	~CommandProcessor();

	void DisplayPrompt(bool previousResult);

	// returns false on error or exit
	bool ProcessCharacter(int c);

	bool IsRaw() { return raw; }

protected:

	void Backspace();

	// returns false on error or exit
	bool ProcessLine(std::string& commandLine);

	sml::Kernel* pKernel;

	std::string line;			// everything typed before \n \r
	bool		raw;			// true when output is 'raw' as opposed to 'structured'
	bool		meta;			// windows meta key

	// History variables
	int			historyIndex;
	int			temporaryHistoryIndex;
	std::string* pHistory;

};

char getKey(bool block);

#endif // CLI_TEST_H
