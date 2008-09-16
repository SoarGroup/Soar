/////////////////////////////////////////////////////////////////
// version command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "sml_Utils.h"
#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"
#include "sml_StringOps.h"
#include "sml_Names.h"
#include "sml_KernelSML.h"

#include "soarversion.h"

#include "agent.h"

using namespace cli;
using namespace sml;

const char* kTimestamp = __TIME__;
const char* kDatestamp = __DATE__;

bool CommandLineInterface::ParseVersion(std::vector<std::string>&) {
	return DoVersion();
}

bool CommandLineInterface::DoVersion() {

	char buf[kMinBufferSize];

	std::ostringstream timedatestamp;
	timedatestamp << kDatestamp << " " << kTimestamp;
	std::string sTimeDateStamp = timedatestamp.str();

	if (m_RawOutput) {
		m_Result << sml_Names::kSoarVersionValue << "\n";
		m_Result << "Build date: " << sTimeDateStamp.c_str() << " " ;

	} else {
		AppendArgTagFast(sml_Names::kParamVersionMajor, sml_Names::kTypeInt, Int2String(MAJOR_VERSION_NUMBER, buf, kMinBufferSize));
		AppendArgTagFast(sml_Names::kParamVersionMinor, sml_Names::kTypeInt, Int2String(MINOR_VERSION_NUMBER, buf, kMinBufferSize));
		AppendArgTagFast(sml_Names::kParamVersionMicro, sml_Names::kTypeInt, Int2String(MICRO_VERSION_NUMBER, buf, kMinBufferSize));
		AppendArgTag(sml_Names::kParamBuildDate, sml_Names::kTypeString, sTimeDateStamp.c_str());
	}
	return true;
}

