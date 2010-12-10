/////////////////////////////////////////////////////////////////
// rand command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com
// Date  : 2009
//
/////////////////////////////////////////////////////////////////

#include <portability.h>

#include "sml_Utils.h"
#include "cli_CommandLineInterface.h"
#include "cli_Commands.h"

#include "sml_Names.h"

#include "soar_rand.h"
#include "misc.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::ParseRand(std::vector<std::string>& argv) {
	Options optionsData[] = {
		{'i', "integer", OPTARG_NONE},
		{0, 0, OPTARG_NONE}
	};

	bool integer(false);

	for (;;) {
		if (!ProcessOptions(argv, optionsData)) return false;
		if (m_Option == -1) break;

		switch (m_Option) {
			case 'i':
				integer = true;
				break;
			default:
				return SetError(kGetOptError);
		}
	}

	if ( m_NonOptionArguments > 1 ) {
		return SetError( kTooManyArgs );
	} else if ( m_NonOptionArguments == 1 ) {
		unsigned optind = m_Argument - m_NonOptionArguments;
		return DoRand( integer, &(argv[optind]) );
	}

	return DoRand( integer, 0 );
}

bool CommandLineInterface::DoRand( bool integer, std::string* bound ) {
	if ( integer ) {
		uint32_t out;

		if ( bound ) {
			uint32_t n(0);
			if ( !from_string( n, *bound ) ) {
				return SetError( kIntegerExpected );
			}
			out = SoarRandInt( n );
		} else {
			out = SoarRandInt();
		}

		m_Result << out;
	} else {
		double out;

		if ( bound ) {
			double n(0);
			if ( !from_string( n, *bound ) ) {
				return SetError( kRealExpected );
			}
			out = SoarRand( n );
		} else {
			out = SoarRand();
		}

		m_Result << out;
	}
	return true;
}

