#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_Constants.h"

#ifdef _MSC_VER
#define snprintf _snprintf 
#endif // _MSC_VER

using namespace cli;

bool CommandLineInterface::ParseVersion(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	unused(pAgent);
	unused(argv);

	return DoVersion();
}

bool CommandLineInterface::DoVersion() {

	char buf[32];
	snprintf(buf, 31, "%d.%d", m_KernelVersion.major, m_KernelVersion.minor);
	buf[31] = 0;
	AppendToResult(buf);
	return true;
}

