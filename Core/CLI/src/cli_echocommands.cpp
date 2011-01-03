/////////////////////////////////////////////////////////////////
// echo command file.
//
// Author: Douglas Pearson, doug@threepenny.net
// Date  : 2005
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "sml_Utils.h"
#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"

#include "sml_Names.h"
#include "sml_KernelSML.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::DoEchoCommands(bool onlyGetValue, bool echoCommands) {

	// We only set the value if asked, but we always report the current setting.
	if (!onlyGetValue)
	{
		m_pKernelSML->SetEchoCommands(echoCommands) ;
	}

	echoCommands = m_pKernelSML->GetEchoCommands() ;

	if (m_RawOutput) {
		m_Result << (echoCommands ? "Echoing commands." : "Not echoing commands.") ;
	} else {
		AppendArgTagFast(sml_Names::kParamValue, sml_Names::kTypeString, echoCommands ? sml_Names::kTrue : sml_Names::kFalse);
	}

	return true;
}

