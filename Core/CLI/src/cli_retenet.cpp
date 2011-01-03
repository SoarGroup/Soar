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

#include "sml_KernelSML.h"

#include "rete.h"

using namespace cli;

bool CommandLineInterface::DoReteNet(bool save, std::string filename) {
	if (!filename.size()) return SetError("Missing file name.");

    agent* agnt = m_pAgentSML->GetSoarAgent();
	if ( save ) 
	{
		FILE* file = fopen( filename.c_str(), "wb" );

		if( file == 0 )
		{
			return SetError( "Open file failed." );
		}

		if ( ! save_rete_net( agnt, file, TRUE ) )
		{
			// TODO: additional error information
			return SetError( "Rete save operation failed." );
		}

		fclose( file );

	} else {
		FILE* file = fopen( filename.c_str(), "rb" );

		if(file == 0)
		{
			return SetError(  "Open file failed." );
		}

		if ( ! load_rete_net( agnt, file ) )
		{
			// TODO: additional error information
			return SetError( "Rete load operation failed." );
		}

		fclose( file );
	}

	return true;
}

