/////////////////////////////////////////////////////////////////
// alias command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"

#include "sml_Names.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseAlias(gSKI::Agent* pAgent, std::vector<std::string>& argv) {
	unused(pAgent);

	Options optionsData[] = {
		{'d', "disable",	1},
		{'d', "off",		1},
		{0, 0, 0}
	};

	bool disable = false;
	std::string command;

	for (;;) {
		if (!ProcessOptions(argv, optionsData)) return false;
		if (m_Option == -1) break;

		switch (m_Option) {
			case 'd':
				disable = true;
				command = m_OptionArgument;
				break;
			default:
				return SetError(CLIError::kGetOptError);
		}
	}

	// If disabling, no additional argument.
	if (disable) {
		if (m_NonOptionArguments) {
			SetErrorDetail("When disabling, no additional arguments are required.");
			return SetError(CLIError::kTooManyArgs);
		}
		return DoAlias(&command);
	}
	
	// If not disabling and no arguments, list aliases
	if (m_NonOptionArguments == 0) return DoAlias();

	std::vector<std::string> substitution;
	std::vector<std::string>::iterator iter = argv.begin();

	command = *(++iter);
	while (++iter != argv.end()) {
		substitution.push_back(*iter);
	}

	return DoAlias(&command, &substitution);
}

bool CommandLineInterface::DoAlias(const std::string* pCommand, const std::vector<std::string>* pSubstitution) {

	if (!pCommand && !pSubstitution) {
		// list aliases
		if (m_RawOutput) {
			std::string result = m_Aliases.List();
			if (!result.size()) {
				SetErrorDetail("No aliases in alias database.");
				return SetError(CLIError::kAliasNotFound);
			}
			m_Result << result;
			return true;

		} else {
			AliasMap::const_iterator citer = m_Aliases.GetAliasMapBegin();
			while (citer != m_Aliases.GetAliasMapEnd()) {
				AppendArgTagFast(sml_Names::kParamAlias, sml_Names::kTypeString, citer->first.c_str());

				std::string aliasedCommand;
				for (std::vector<std::string>::const_iterator iter = citer->second.begin(); iter != citer->second.end(); ++iter) {
					aliasedCommand += *iter;
					aliasedCommand += ' ';
				}
				aliasedCommand = aliasedCommand.substr(0, aliasedCommand.length() - 1);
				AppendArgTagFast(sml_Names::kParamAliasedCommand, sml_Names::kTypeString, aliasedCommand.c_str());
				++citer;
			}
			return true;
		}
	}

	// command needs to have a size
	if (!pCommand || !pCommand->size()) {
		SetErrorDetail("No alias parameter received.");
		return SetError(CLIError::kAliasNotFound);
	}

	if (!pSubstitution) {
		// no substitution, remove
		if (!m_Aliases.RemoveAlias(*pCommand)) {
			SetErrorDetail("Didn't find '" + *pCommand + "' in alias database.");
			return SetError(CLIError::kAliasNotFound);
		}
		return true;
	} 

	// if substitution is empty, list only that alias
	if (!pSubstitution->size()) {
		if (m_RawOutput) {
			std::string result = m_Aliases.List(pCommand);
			if (!result.size()) {
				SetErrorDetail("Didn't find '" + *pCommand + "' in alias database.");
				return SetError(CLIError::kAliasNotFound);
			}
			m_Result << result;
		} else {
			AliasMap::const_iterator citer = m_Aliases.GetAliasMapBegin();
			while (citer != m_Aliases.GetAliasMapEnd()) {
				if (citer->first == *pCommand) {
					AppendArgTagFast(sml_Names::kParamAlias, sml_Names::kTypeString, citer->first.c_str());

					std::string aliasedCommand;
					for (std::vector<std::string>::const_iterator iter = citer->second.begin(); iter != citer->second.end(); ++iter) {
						aliasedCommand += *iter;
						aliasedCommand += ' ';
					}
					aliasedCommand = aliasedCommand.substr(0, aliasedCommand.length() - 1);
					AppendArgTagFast(sml_Names::kParamAliasedCommand, sml_Names::kTypeString, aliasedCommand.c_str());
					break;
				}
				++citer;
			}
			if (citer == m_Aliases.GetAliasMapEnd()) {
				SetErrorDetail("Didn't find '" + *pCommand + "' in alias database.");
				return SetError(CLIError::kAliasNotFound);
			}
		}
		return true;
	}

	// new alias
	if (!m_Aliases.NewAlias((*pSubstitution), *pCommand)) return SetError(CLIError::kAliasExists);
	return true;
}

