#include "cli_CommandLineInterface.h"

#ifdef WIN32
#include <windows.h>
#endif // WIN32

#include "sml_StringOps.h"
#include "sml_Names.h"

#include "IgSKI_Agent.h"

using namespace cli;
using namespace sml;

// ____                    _____ _
//|  _ \ __ _ _ __ ___  __|_   _(_)_ __ ___   ___
//| |_) / _` | '__/ __|/ _ \| | | | '_ ` _ \ / _ \
//|  __/ (_| | |  \__ \  __/| | | | | | | | |  __/
//|_|   \__,_|_|  |___/\___||_| |_|_| |_| |_|\___|
//
bool CommandLineInterface::ParseTime(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	// There must at least be a command
	if (argv.size() < 2) {
		return HandleSyntaxError(Constants::kCLITime, Constants::kCLITooFewArgs);
	}

	std::vector<std::string>::iterator iter = argv.begin();
	argv.erase(iter);

	return DoTime(pAgent, argv);
}

// ____      _____ _
//|  _ \  __|_   _(_)_ __ ___   ___
//| | | |/ _ \| | | | '_ ` _ \ / _ \
//| |_| | (_) | | | | | | | | |  __/
//|____/ \___/|_| |_|_| |_| |_|\___|
//
bool CommandLineInterface::DoTime(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {

#ifdef WIN32
	// Look at clock
	DWORD start = GetTickCount();

	// Execute command
	bool ret = DoCommandInternal(pAgent, argv);

	// Look at clock again, subtracting first value
	DWORD elapsed = GetTickCount() - start;

	// calculate elapsed in seconds
	float seconds = elapsed / 1000.0f;

	char buf[32];
	memset(buf, 0, 32);
	Double2String(seconds, buf, 31);

	if (m_RawOutput) {
		// Print time elapsed and return
		m_Result += "\n(";
		m_Result += buf;
		m_Result += "s) real";
	} else {
		AppendArgTagFast(sml_Names::kParamSeconds, sml_Names::kTypeDouble, buf);
	}
	return ret;

#else
	m_Result += "TODO: time on non-windows platform";
	return true;
#endif
}

