/////////////////////////////////////////////////////////////////
// rete-net command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2004
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"
#include "cli_CLIError.h"

#include "sml_KernelSML.h"

#include "rete.h"

using namespace cli;

bool CommandLineInterface::ParseReteNet(std::vector<std::string>& argv) {
	Options optionsData[] = {
		{'l', "load",		OPTARG_REQUIRED},
		{'r', "restore",	OPTARG_REQUIRED},
		{'s', "save",		OPTARG_REQUIRED},
		{0, 0, OPTARG_NONE}
	};

	bool save = false;
	bool load = false;
	std::string filename;

	for (;;) {
		if (!ProcessOptions(argv, optionsData)) return false;
		if (m_Option == -1) break;

		switch (m_Option) {
			case 'l':
			case 'r':
				load = true;
				save = false;
				filename = m_OptionArgument;
				break;
			case 's':
				save = true;
				load = false;
				filename = m_OptionArgument;
				break;
			default:
				return SetError(CLIError::kGetOptError);
		}
	}

	// Must have a save or load operation
	if (!save && !load) return SetError(CLIError::kMustSaveOrLoad);
	if (m_NonOptionArguments) return SetError(CLIError::kTooManyArgs);

	return DoReteNet(save, filename);
}

bool CommandLineInterface::DoReteNet(bool save, std::string filename) {
	if (!filename.size()) return SetError(CLIError::kMissingFilenameArg);

	StripQuotes(filename);

	if ( save ) 
	{
		FILE* file = fopen( filename.c_str(), "wb" );

		if( file == 0 )
		{
			return SetError( CLIError::kOpenFileFail );
		}

		if ( ! save_rete_net( m_pAgentSoar, file, TRUE ) )
		{
			// TODO: additional error information
			return SetError( CLIError::kReteSaveOperationFail );
		}

		fclose( file );

	} else {
		FILE* file = fopen( filename.c_str(), "rb" );

		if(file == 0)
		{
			return SetError( CLIError::kOpenFileFail );
		}

		if ( ! load_rete_net( m_pAgentSoar, file ) )
		{
			// TODO: additional error information
			return SetError( CLIError::kReteLoadOperationFail );
		}

		fclose( file );
	}

	return true;
}

