#include "Console.h"
#include "SoarPlayerClient.h"

int main( int argc, char** argv )
{
    if ( argc < 2 )
    {
    	std::cerr << "please specify productions on command line" << std::endl;
    	return 1;
    }
    
    try
    {
        SoarPlayerClient client( argv[1] );
        Console console( client );

        return console.run();
    }
    catch ( ... )
    {
        std::cerr << "terminating on failure" << std::endl;
    }
    return 1;
}

