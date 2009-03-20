/////////////////////////////////////////////////////////////////
// smem command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com,
// Date  : 2009
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "cli_CommandLineInterface.h"

#include "cli_Commands.h"
#include "cli_CLIError.h"

#include "sml_Names.h"

#include "semantic_memory.h"
#include "misc.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseSMem( std::vector<std::string>& argv )
{
	Options optionsData[] =
	{
		{'c', "close",		OPTARG_NONE},
		{'g', "get",		OPTARG_NONE},
		{'s', "set",		OPTARG_NONE},
		{'S', "stats",		OPTARG_NONE},
		{'t', "timers",		OPTARG_NONE},
		{0, 0, OPTARG_NONE} // null
	};
	SMemBitset options(0);

	for (;;)
	{
		if ( !ProcessOptions( argv, optionsData ) )
			return false;

		if (m_Option == -1) break;

		switch (m_Option)
		{
			case 'c':
				options.set( SMEM_CLOSE );
				break;

			case 'g':
				options.set( SMEM_GET );
				break;

			case 's':
				options.set( SMEM_SET );
				break;

			case 'S':
				options.set( SMEM_STAT );
				break;

			case 't':
				options.set( SMEM_TIMER );
				break;

			default:
				return SetError( CLIError::kGetOptError );
		}
	}
	
	// not sure why you'd get here
	return false;
}

bool CommandLineInterface::DoSMem( const char pOp, const std::string* pAttr, const std::string* pVal )
{
	return SetError( CLIError::kCommandNotImplemented );
}
