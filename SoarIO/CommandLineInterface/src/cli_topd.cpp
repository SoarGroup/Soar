#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_Constants.h"

#include "sml_Names.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseDirs(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	unused(pAgent);
	unused(argv);

	return DoDirs();
}

bool CommandLineInterface::DoDirs() {
	
	StringStack tempStack;

	std::string cwd;
	GetCurrentWorkingDirectory(cwd);

	if (m_RawOutput) {
		AppendToResult(cwd);
	} else {
		AppendArgTag(sml_Names::kParamDirectory, sml_Names::kTypeString, cwd.c_str());
	}

	while (m_DirectoryStack.size()) {

		if (m_RawOutput) {
			AppendToResult(' ');
			AppendToResult(m_DirectoryStack.top());
		} else {
			AppendArgTag(sml_Names::kParamDirectory, sml_Names::kTypeString, m_DirectoryStack.top().c_str());
		}

		tempStack.push(m_DirectoryStack.top());
		m_DirectoryStack.pop();
	}

	while (tempStack.size()) {
		m_DirectoryStack.push(tempStack.top());
		tempStack.pop();
	}
	return true;
}

