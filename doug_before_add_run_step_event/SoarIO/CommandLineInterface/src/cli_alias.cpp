#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_Constants.h"
#include "cli_GetOpt.h"

#include "sml_Names.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseAlias(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	unused(pAgent);

	static struct GetOpt::option longOptions[] = {
		{"disable",	1, 0, 'd'},
		{"off",		1, 0, 'd'},
		{0, 0, 0, 0}
	};

	GetOpt::optind = 0;
	GetOpt::opterr = 0;

	bool disable = false;
	std::string command;

	for (;;) {
		int option = m_pGetOpt->GetOpt_Long(argv, ":d:", longOptions, 0);
		if (option == -1) break;

		switch (option) {
			case 'd':
				disable = true;
				command = GetOpt::optarg;
				break;
			case ':':
				return m_Error.SetError(CLIError::kMissingOptionArg);
			case '?':
				return m_Error.SetError(CLIError::kUnrecognizedOption);
			default:
				return m_Error.SetError(CLIError::kGetOptError);
		}
	}

	// If disabling, no additional argument.
	if (disable) {
		if (argv.size() != (unsigned)GetOpt::optind) return m_Error.SetError(CLIError::kTooManyArgs);
		return DoAlias(disable, command, 0);
	}
	
	// If not disabling and no arguments, list aliases
	if ((argv.size() - GetOpt::optind) == 0) return DoAlias(disable, command, 0);

	// If not disabling and not listing, there must be at least two additional arguments
	if ((argv.size() - GetOpt::optind) < 2) return m_Error.SetError(CLIError::kTooFewArgs);		

	std::vector<std::string> substitution;
	std::vector<std::string>::iterator iter = argv.begin();

	command = *(++iter);
	while (++iter != argv.end()) {
		substitution.push_back(*iter);
	}

	return DoAlias(disable, command, &substitution);
}

bool CommandLineInterface::DoAlias(bool disable, const std::string& command, const std::vector<std::string>* pSubstitution) {
	if (disable) {
		if (!m_Aliases.RemoveAlias(command)) return m_Error.SetError(CLIError::kAliasNotFound);

	} else {
		if (!command.size()) {
			if (m_RawOutput) {
				// list aliases
				AppendToResult(m_Aliases.List());
				return true;
			} else {
				AliasMap::const_iterator citer = m_Aliases.GetAliasMapBegin();
				while (citer != m_Aliases.GetAliasMapEnd()) {
					AppendArgTag(sml_Names::kParamAlias, sml_Names::kTypeString, citer->first.c_str());

					std::string aliasedCommand;
					for (std::vector<std::string>::const_iterator iter = citer->second.begin(); iter != citer->second.end(); ++iter) {
						aliasedCommand += *iter;
						aliasedCommand += ' ';
					}
					aliasedCommand = aliasedCommand.substr(0, aliasedCommand.length() - 1);
					AppendArgTag(sml_Names::kParamAliasedCommand, sml_Names::kTypeString, aliasedCommand.c_str());
					++citer;
				}
				return true;
			}
		}

		if (!m_Aliases.NewAlias((*pSubstitution), command)) return m_Error.SetError(CLIError::kAliasExists);
	}
	return true;
}

