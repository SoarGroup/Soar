/////////////////////////////////////////////////////////////////
// time command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "cli_CommandLineInterface.h"

#include <time.h>

#include "cli_Commands.h"
#include "cli_CLIError.h"

#include "sml_Names.h"
#include "misc.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseTime(std::vector<std::string>& argv) {
	// There must at least be a command
	if (argv.size() < 2) {
		SetErrorDetail("Please supply a command to time.");
		return SetError(CLIError::kTooFewArgs);
	}

	std::vector<std::string>::iterator iter = argv.begin();
	argv.erase(iter);

	return DoTime(argv);
}

bool CommandLineInterface::DoTime(std::vector<std::string>& argv) {

	// Look at clock
#ifdef WIN32
	DWORD realStart = GetTickCount();
#else // WIN32
	struct timeval realStart;
	if (gettimeofday(&realStart, 0) != 0) {
		return SetError(CLIError::kgettimeofdayFail);
	}
#endif // WIN32

	// Look at clock for process time
	clock_t procStart;
	procStart = clock();

	// Execute command
	bool ret = DoCommandInternal(argv);

	// Look at clock for process time
	clock_t procFinish;
	procFinish = clock();

	// Look at clock again, evaluate elapsed time in seconds
#ifdef WIN32
	DWORD elapsedx = GetTickCount() - realStart;
	float realElapsed = elapsedx / 1000.0f;
#else // WIN32
	struct timeval realFinish;
	if (gettimeofday(&realFinish, 0) != 0) {
		return SetError(CLIError::kgettimeofdayFail);
	}
	double realElapsed = (realFinish.tv_sec + (realFinish.tv_usec / 1000000.0)) 
		- (realStart.tv_sec + (realStart.tv_usec / 1000000.0));
#endif

	double procElapsed = (procFinish - procStart) / static_cast<double>(CLOCKS_PER_SEC);

	// Print elapsed time and return
	if (m_RawOutput) {
		m_Result << "\n(" << procElapsed << "s) proc" << "\n(" << realElapsed << "s) real";
	} else {
		std::string temp;
		AppendArgTagFast(sml_Names::kParamRealSeconds, sml_Names::kTypeDouble, to_string(realElapsed, temp));
		AppendArgTagFast(sml_Names::kParamProcSeconds, sml_Names::kTypeDouble, to_string(procElapsed, temp));
	}
	return ret;
}

