#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_Constants.h"

using namespace cli;

bool CommandLineInterface::ParseDirs(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	unused(pAgent);
	unused(argv);

	return DoDirs();
}

bool CommandLineInterface::DoDirs() {
	
	StringStack tempStack;

	std::string cwd;
	GetCurrentWorkingDirectory(cwd);
	AppendToResult(cwd);

	while (m_DirectoryStack.size()) {
		AppendToResult(' ');
		AppendToResult(m_DirectoryStack.top());
		tempStack.push(m_DirectoryStack.top());
		m_DirectoryStack.pop();
	}

	while (tempStack.size()) {
		m_DirectoryStack.push(tempStack.top());
		tempStack.pop();
	}

	return true;
}

