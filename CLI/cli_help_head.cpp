#include <map>
#include <portability.h>
#include "cli_CommandLineInterface.h"

using namespace cli;
using namespace sml;

std::map<std::string, const char*> docstrings;

void initdocstrings();

bool CommandLineInterface::DoHelp(const std::vector<std::string> &argv) {
	std::map<std::string, const char*>::iterator i;
	
	if (docstrings.size() == 0) {
		initdocstrings();
	}
	
	if (argv.size() == 1) {
		m_Result << "Available commands:" << std::endl << std::endl;
		for (i = docstrings.begin(); i != docstrings.end(); ++i) {
			m_Result << i->first << std::endl;
		}
	} else {
		if ((i = docstrings.find(argv[1])) == docstrings.end()) {
			m_Result << "No such command" << std::endl;
			return false;
		}
		m_Result << i->second;
	}
	return true;
}

void initdocstrings() {
