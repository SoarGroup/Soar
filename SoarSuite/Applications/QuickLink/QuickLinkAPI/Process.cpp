/*****************************************************
*  Taylor Lafrinere
*  
*  Soar Quick Link
*
*  Process.cpp
*
*  This idea of a process comes from Soar's QuickLink.
*  A process is a ordered list of QuickLink commands that
*  can be used as scripts for QuickLink or other programs.
*
*  Start Date: 08.24.2005
*
*****************************************************/

#include "Process.h"
#include "QLInterface.h"

using namespace std;

Process::Process()
{  
}

bool
Process::SaveProcess( vector<string> vecIn , string fileName )
{
	ProcVec = vecIn;
	file = fileName;

	saver.open(file.c_str());  //Opens file stream

	for(unsigned int i = 0; i < ProcVec.size(); i++) //Goes through entire vector
	{
		saver << ProcVec[i];  //write this string to file
		if (i != ProcVec.size()-1) //as long as this is not the last line...
			saver << endl;              //...print a new line
	}

	saver.close(); //so the file can be read in at any point
	saver.clear(); //just in case of errors

	return !saver.bad();
}

string
Process::LoadProcess( QLInterface* QLAPI , ifstream* inFile )
{
	string first, second, third, fourth, fifth, notCaps;
	*inFile >> first;
	notCaps = first;
	MakeStringUpperCase( first );
	while( true )
	{
		if( !( *inFile ))
		{   inFile->clear();
			return "file ended";
		}
		else if( first == "CLEAR" )
			QLAPI->ClearILink();
		else if( first == "SAVE" || first == "S" )
		{	*inFile >> second;
			QLAPI->SaveILink( second );
		}
		else if( first == "ADD" || first == "A" )
		{	*inFile >> second >> third >> fourth;
			QLAPI->CreateWME( second , third , fourth );
		}
		else if( first == "CHANGE" || first == "C" )
		{	*inFile >> second >> third >> fourth >> fifth;
			QLAPI->ChangeWME( second , third , fourth , fifth );
		}
		else if( first == "DELETE" || first == "D" )
		{	*inFile >> second >> third >> fourth;
			QLAPI->DeleteWME( second , third , fourth );
		}
		else if( first == "LOAD" || first == "L" )
		{	*inFile >> second;
			string catcher = QLAPI->LoadFile( second );
			if( catcher == "PAUSE" )
				return "PAUSE";
		}
		else if( first == "GO" || first == "G" )
			QLAPI->RunSoarTilOutput();
		else if( first == "RUNCYCLE" || "RC" )
		{	*inFile >> second;
			QLAPI->RunSoarForNCycles( atoi( second.c_str() ) );
		}
		else if( first == "COMMIT" )
			QLAPI->CommitChanges();
		else if( first == "PAUSE" )
			return "PAUSE";
		else if( first[0] == '#' )
		{	char elget;
			inFile->get( elget );
			while (elget != '\n' && elget != EOF && (*inFile))
				inFile->get(elget);
		}
		else
		{	string strcmd = notCaps;
			while(inFile->peek() != '\n' && inFile->peek() != EOF)
			{
				string tmp;
				*inFile >> tmp;
				if(strcmd == "")
					strcmd = tmp;
				else
					strcmd += (" " + tmp);
			}
			QLAPI->ExecuteCommandLine( strcmd );
		}
	}
}

void
Process::MakeStringUpperCase( string& tooSmall )
{
	for( unsigned int ii = 0 ; ii < tooSmall.size() ; ii++ )
		tooSmall[ii]=toupper( tooSmall[ii] );
	return;
}







