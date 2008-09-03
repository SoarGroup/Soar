/////////////////////////////////////////////////////////////////
// version command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_Constants.h"
#include "sml_StringOps.h"
#include "sml_Names.h"

using namespace cli;
using namespace sml;

const char* kTimestamp = __TIMESTAMP__;

bool CommandLineInterface::ParseVersion(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	unused(pAgent);
	unused(argv);

	return DoVersion();
}

bool CommandLineInterface::DoVersion() {

	char buf[kMinBufferSize];

	if (m_RawOutput) {
		m_Result << m_KernelVersion.major << '.' << m_KernelVersion.minor << '.' << m_KernelVersion.micro << '\n';
		m_Result << "Build date: " << kTimestamp;

	} else {
		AppendArgTagFast(sml_Names::kParamVersionMajor, sml_Names::kTypeInt, Int2String(m_KernelVersion.major, buf, kMinBufferSize));
		AppendArgTagFast(sml_Names::kParamVersionMinor, sml_Names::kTypeInt, Int2String(m_KernelVersion.minor, buf, kMinBufferSize));
		AppendArgTagFast(sml_Names::kParamVersionMicro, sml_Names::kTypeInt, Int2String(m_KernelVersion.micro, buf, kMinBufferSize));
		AppendArgTagFast(sml_Names::kParamBuildDate, sml_Names::kTypeString, kTimestamp);
	}
	return true;
}

