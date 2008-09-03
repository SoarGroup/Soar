#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_Constants.h"
#include "sml_StringOps.h"
#include "sml_Names.h"

#include "IgSKI_Agent.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseOSupportMode(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	
	if (argv.size() > 2) return m_Error.SetError(CLIError::kTooManyArgs);

	int mode = -1;
	if (argv.size() == 2) {
		if (!isdigit(argv[1][0])) return m_Error.SetError(CLIError::kIntegerOutOfRange);
		mode = atoi(argv[1].c_str());
		if (mode < 0 || mode > 4) return m_Error.SetError(CLIError::kIntegerOutOfRange);
	}

	return DoOSupportMode(pAgent, mode);
}

bool CommandLineInterface::DoOSupportMode(gSKI::IAgent* pAgent, int mode) {

	if (mode < 0) {
		egSKIOSupportMode m = pAgent->GetOSupportMode();
		char buf[kMinBufferSize];
		Int2String((int)m, buf, kMinBufferSize);
		if (m_RawOutput) {
			AppendToResult(buf);
		} else {
			AppendArgTag(sml_Names::kParamValue, sml_Names::kTypeInt, buf);
		}
		return true;
	}

	return m_Error.SetError(CLIError::kOptionNotImplemented);
}

