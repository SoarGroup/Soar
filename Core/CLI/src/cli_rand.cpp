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

bool CommandLineInterface::DoRand( bool integer, std::string* bound ) {
    if ( integer ) {
        uint32_t out;

        if ( bound ) {
            uint32_t n(0);
            if ( !from_string( n, *bound ) ) {
                return SetError( "Integer expected." );
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
                return SetError( "Real number expected." );
            }
            out = SoarRand( n );
        } else {
            out = SoarRand();
        }

        m_Result << out;
    }
    return true;
}

