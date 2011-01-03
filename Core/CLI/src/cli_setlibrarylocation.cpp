/////////////////////////////////////////////////////////////////
// set-library-location command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
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

bool CommandLineInterface::DoSetLibraryLocation(std::string* pLocation) {

	if (!pLocation || !pLocation->size()) {
		if (m_RawOutput) {
			m_Result << "Current library location: " << m_pKernelSML->GetLibraryLocation();
		} else {
			AppendArgTagFast(sml_Names::kParamDirectory, sml_Names::kTypeString, m_pKernelSML->GetLibraryLocation());
		}
	} else {
		m_pKernelSML->SetLibraryLocation(*pLocation);
	}
	return true;
}

