#include "Console.h"
#include "SoarPlayerClient.h"

#include <string>
#include <iostream>
#include "ConfigFile.h"

using std::cerr;
using std::string;
using std::endl;

int main( int argc, char** argv )
{
	if ( argc < 2 )
	{
		cerr << "Please specify configuration file name on command line." << endl;
		return 1;
	}
	
	try
	{
		ConfigFile config( argv[1] );
		SoarPlayerClient client( config );
		Console console( client, config.read( "host", std::string ("localhost" ) ), config.read( "sedan_port", 6664 ) );
		return console.run();
	}
	catch ( ConfigFile::file_not_found e )
	{
		cerr << "Could not find configuration file: " << e.filename << endl;
		return 1;
	}
	catch ( ConfigFile::key_not_found e )
	{
		cerr << "Could not find configuration key: " << e.key << endl;
		return 1;
	}
	catch ( SoarPlayerClient::soar_error e )
	{
		cerr << "Soar error: " << e.what() << endl;
	}
	catch ( ... )
	{
		cerr << "Terminating on failure." << endl;
	}
	return 1;
}

