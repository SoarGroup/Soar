#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#ifdef WIN32
#include <windows.h>
#else // WIN32
#include <sys/time.h>
#endif // WIN32

#include <time.h>

#include "cli_Constants.h"

#include "sml_Names.h"

#include "IgSKI_Agent.h"

#ifdef _MSC_VER
#define snprintf _snprintf 
#endif // _MSC_VER

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseTime(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	// There must at least be a command
	if (argv.size() < 2) {
		return HandleSyntaxError(Constants::kCLITime, Constants::kCLITooFewArgs);
	}

	std::vector<std::string>::iterator iter = argv.begin();
	argv.erase(iter);

	return DoTime(pAgent, argv);
}

bool CommandLineInterface::DoTime(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {

	// Look at clock
#ifdef WIN32
	DWORD start = GetTickCount();
#else // WIN32
	struct timeval start;
	if (gettimeofday(&start, 0) != 0) {
		return HandleError("failed to get time: " + std::string(strerror(errno)));
	}
#endif // WIN32

	// Execute command
	bool ret = DoCommandInternal(pAgent, argv);

	// Look at clock again, evaluate elapsed time in seconds
#ifdef WIN32
	DWORD elapsedx = GetTickCount() - start;
	float elapsed = elapsedx / 1000.0f;
#else // WIN32
	struct timeval finish;
	if (gettimeofday(&finish, 0) != 0) {
		return HandleError("failed to get time: " + std::string(strerror(errno)));
	}
	double elapsed = (finish.tv_sec + (finish.tv_usec / 1000000.0)) - (start.tv_sec + (start.tv_usec / 1000000.0));
#endif

	// Print elapsed time and return
	char buf[32];
	memset(buf, 0, 32);
	snprintf(buf, 31, "%f", elapsed);

	if (m_RawOutput) {
		AppendToResult("\n(");
		AppendToResult(buf);
		AppendToResult("s) real");
	} else {
		AppendArgTagFast(sml_Names::kParamSeconds, sml_Names::kTypeDouble, buf);
	}
	return ret;
}

