#include "portability.h"

#include <cppunit/TestRunner.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/CompilerOutputter.h>

#include "simplelistener.h"

bool g_CancelPressed = false;

#if defined(_WIN32) || defined(_WINDOWS) || defined(__WIN32__)

BOOL WINAPI handle_ctrlc( DWORD dwCtrlType )
{
	if ( dwCtrlType == CTRL_C_EVENT )
	{
		g_CancelPressed = true;
		return TRUE;
	}

	return FALSE;
}

#endif

int main( int argc, char** argv )
{
#if defined(_WIN32) || defined(_WINDOWS) || defined(__WIN32__)
	SetConsoleCtrlHandler( handle_ctrlc, TRUE );
#endif

	bool pause = true;
	if ( argc >= 2 )
	{
		if ( std::string( argv[1] ) == "--listener" ) 
		{
			SimpleListener simpleListener( 3000, 12121 );
			return simpleListener.run();
		}
		if ( std::string( argv[1] ) == "--nopause" ) pause = false;
	}

	//--- Create the event manager and test controller
	CPPUNIT_NS::TestResult controller;

	//--- Add a listener that colllects test result
	CPPUNIT_NS::TestResultCollector result;
	controller.addListener( &result );        

	//--- Add a listener that print dots as test run.
	CPPUNIT_NS::BriefTestProgressListener progress;
	controller.addListener( &progress );      

	//--- Add the top suite to the test runner
	CPPUNIT_NS::TestRunner runner;
	runner.addTest( CPPUNIT_NS::TestFactoryRegistry::getRegistry().makeTest() );
	runner.run( controller );

	CPPUNIT_NS::CompilerOutputter outputter( &result, std::cerr );
	outputter.write();                      

	if ( pause )
	{
		std::cout << std::endl << "Press enter to exit." << std::endl;
		std::cin.get();
	}

	return result.wasSuccessful() ? 0 : 1;
}