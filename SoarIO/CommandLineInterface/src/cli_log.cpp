#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "cli_CommandLineInterface.h"

#include <fstream>

#include "cli_GetOpt.h"
#include "cli_Constants.h"

#include "sml_Names.h"

#include "IgSKI_Agent.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseLog(gSKI::IAgent* pAgent, std::vector<std::string>& argv) {
	static struct GetOpt::option longOptions[] = {
		{"append",	0, 0, 'a'},
		{"close",	0, 0, 'c'},
		{"disable",	0, 0, 'c'},
		{"off",		0, 0, 'c'},
		{0, 0, 0, 0}
	};

	GetOpt::optind = 0;
	GetOpt::opterr = 0;

	int option;
	bool append = false;
	bool close = false;

	for (;;) {
		option = m_pGetOpt->GetOpt_Long(argv, "ac", longOptions, 0);
		if (option == -1) {
			break;
		}

		switch (option) {
			case 'a':
				append = true;
				break;
			case 'c':
				close = true;
				break;
			case '?':
				return HandleSyntaxError(Constants::kCLILog, Constants::kCLIUnrecognizedOption);
			default:
				return HandleGetOptError((char)option);
		}
	}

	// Only one non-option arg allowed, filename
	if ((unsigned)GetOpt::optind == argv.size() - 1) {

		// But not with the close option
		if (close) {
			return HandleSyntaxError(Constants::kCLILog, Constants::kCLITooManyArgs);
		}

		return DoLog(pAgent, argv[GetOpt::optind].c_str(), append);

	} else if ((unsigned)GetOpt::optind < argv.size()) {
		return HandleSyntaxError(Constants::kCLILog, Constants::kCLITooManyArgs);
	}

	// No appending without a filename, and if we got here, there is no filename
	if (append) {
		return HandleSyntaxError(Constants::kCLILog, "Append requires a filename.");
	}

	return DoLog(pAgent, 0, close);
}

bool CommandLineInterface::DoLog(gSKI::IAgent* pAgent, const char* pFilename, bool option) {
	if (!RequireAgent(pAgent)) return false;

	// Presence of a filename means open, absence means close or query
	if (pFilename) {
		// Open a file
		if (m_pLogFile) {
			// Unless one is already opened
			return HandleError("Close log file '" + m_LogFilename + "' first.");
		}

		// Create the stream
		if (option) {
			// Option flag means append in presence of filename
			m_pLogFile = new std::ofstream(pFilename, std::ios_base::app);
		} else {
			m_pLogFile = new std::ofstream(pFilename);
		}

		if (!m_pLogFile) {
			// Creation and opening was not successful
			return HandleError("Failed to open file for logging.");
		}

		// Logging opened, add listener and save filename since we can't get it from ofstream
		m_LogFilename = pFilename;

	} else if (option) {		
		// In absence of filename, option true means close
		if (m_pLogFile) {
			// Remove the listener and close the file
			delete m_pLogFile;
			m_pLogFile = 0;

			// Forget the filename
			m_LogFilename.clear();
		}
	}

	// Query at end of successful command, or by default
	if (m_RawOutput) {
		AppendToResult(m_pLogFile ? "Log file '" + m_LogFilename + "' opened." : "Log file closed.");

	} else {
		const char* setting = m_pLogFile ? sml_Names::kTrue : sml_Names::kFalse;
		AppendArgTagFast(sml_Names::kParamLogSetting, sml_Names::kTypeBoolean, setting);

		if (m_LogFilename.size()) {
			AppendArgTagFast(sml_Names::kParamFilename, sml_Names::kTypeString, m_LogFilename.c_str());
		}
	}

	return true;
}

