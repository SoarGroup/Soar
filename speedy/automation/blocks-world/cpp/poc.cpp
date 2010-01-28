
#include <iostream>
#include <string>
#include <stdlib.h>
#include <iomanip>
#include <sstream>
#include <cstdio>

#include "sml_Client.h"
#include "sml_Names.h"

using namespace std;
using namespace sml;

// Conversion of value to string
template<class T> std::string& toString( const T& x, std::string& dest )
{
	static std::ostringstream o;
	
	// get value into stream
	o << std::setprecision( 16 ) << x;
	
	dest.assign( o.str() );
	o.str("");
	return dest;
}

void usage( char* progName )
{
	cout << progName << " " << "<soar dir> [key1=val1 key2=val2 ...]" << endl;
}

int main( int argc, char* argv[] )
{
	// need reporting utility
	if ( argc < 2 )
	{
		usage( argv[0] );
		return 0;
	}

	// do stuff
	{
		// create kernel
		Kernel* pKernel = Kernel::CreateKernelInNewThread();
		if ( pKernel->HadError() )
		{
			cout << pKernel->GetLastErrorDescription() << endl;
			return 0;
		}
		
		// create agent
		Agent* pAgent = pKernel->CreateAgent( "headless" );
		{
			if ( pKernel->HadError() )
			{
				cout << pKernel->GetLastErrorDescription() << endl;
				return 0;
			}
		}
		
		// source rules, set stuff
		{
			{
				std::string sourcePath;
				toString( argv[1], sourcePath );
				sourcePath.append( "/SoarLibrary/Demos/blocks-world/blocks-world.soar" );

				pAgent->LoadProductions( sourcePath.c_str() );
			}
			
			// no monitors, success
			pAgent->ExecuteCommandLine( "excise blocks-world*elaborate*state*success" );
			pAgent->ExecuteCommandLine( "excise blocks-world*monitor*world-state" );
			pAgent->ExecuteCommandLine( "excise blocks-world*monitor*operator-application*move-block" );
			
			// watch 0, seed, timers
			pAgent->ExecuteCommandLine( "watch 0" );
			pAgent->ExecuteCommandLine( "srand 55512" );
			pAgent->ExecuteCommandLine( "timers --off" );
		}

		// perform the task
		for ( int i=0; i<10; i++ )
		{
			// run X decisions
			{
				std::string execCmd( "d " );
				std::string numDecisions;
				toString( 10000, numDecisions );
				execCmd.append( numDecisions );

				pAgent->ExecuteCommandLine( execCmd.c_str() );
			}
			
			// report
			{
				string pidString;
				int pid = getpid();
				toString( pid, pidString );

				string cmd( "php report.php " );
				cmd.append( pidString );

				system( cmd.c_str() );
			}
			
			// soar stats
			{
				ClientAnalyzedXML response;
				pAgent->ExecuteCommandLineXML( "stats", &response );
				cout << " decisions=" << response.GetArgInt( sml_Names::kParamStatsCycleCountDecision, 0 );
			}

			// output any extra params
			for ( int j=2; j<argc; j++ )
			{
				cout << " " << argv[j];
			}

			cout << std::endl;
		}
		
		// clean-up
		{
			pKernel->Shutdown();
			delete pKernel;
		}
	}
	
	return 0;
}
