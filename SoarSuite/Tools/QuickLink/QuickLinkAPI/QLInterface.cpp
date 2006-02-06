/*****************************************************
*  Author: Taylor Lafrinere
*  
*  Soar QuickLink
*
*  QLInterface.cpp
*
*  Use this functions in this file to interface with 
*  QuickLink
*
*  Start Date: 08.24.2005
*
*****************************************************/

#include "QLInterface.h"


QLInterface::QLInterface()
{
	_API_BAD_FILE = "ERROR: File Failed to open.";
}

bool
QLInterface::ClearILink()
{
	return Memory->ClearAll();
}

bool
QLInterface::SaveILink( string fileName )
{
	ofstream tempFile;
	tempFile.open(fileName.c_str());
	bool toReturn = DirtyWork->SaveInput( tempFile );
	tempFile.close();
	tempFile.clear();
	return toReturn;
}

string
QLInterface::CreateWME( string parent , string attribute , string value )
{
	MakeStringUpperCase( parent );
	attribute = attribute.substr(1, attribute.size() - 1 ); //gets rid of ^
	if( value[0] == '/' )  //identifier
	{
		value = value.substr( 1 , value.size() - 1 );  //gets rid of /
		MakeStringUpperCase( value );
		return Memory->CreateId( parent , attribute , value );
	}
	else //value based
		return Memory->DetermineValueAndCreate( parent , attribute , value );
}

string
QLInterface::ChangeWME( string parent , string attribute , string oldValue , string newValue )
{
	MakeStringUpperCase( parent );
	return Memory->DetermineValueAndChange( parent , attribute , oldValue , newValue);
}

string
QLInterface::DeleteWME( string parent , string attribute , string value )
{
	MakeStringUpperCase( parent );
	attribute = attribute.substr(1, attribute.size() - 1 ); //gets rid of ^
	if( value[0] == '/' )  //identifier
	{
		value = value.substr( 1 , value.size() - 1 );  //gets rid of /
			MakeStringUpperCase( value );
		return Memory->DeleteId( parent , attribute , value );
	}
	else //value based
		return Memory->DetermineValueAndDelete( parent , attribute , value );
}

bool
QLInterface::Quit()
{
	return Memory->PurgeMemory();
}

bool
QLInterface::ClearProcessMemory()
{
	return Memory->ClearProcessMem();
}

string
QLInterface::LoadFile( string fileName )
{
	fileStack.resize(fileStack.size()+1);  //increase the size of the file stack
	fileStack[fileStack.size()-1] = new ifstream; //create new ifstream and have end of stack point to it
	(*(fileStack[fileStack.size()-1])).open(fileName.c_str()); //open the stream
	if(!(*(fileStack[fileStack.size()-1]))) //error testing
		return _API_BAD_FILE;

	return FileControl();	
}

string
QLInterface::NextStepInProcess()
{
	if( fileStack.size() == 0 )
		return "There are no processes running, CONTINUE is an invalid command.";
	else
	{
		return FileControl();
	}
}

bool
QLInterface::EndCurrentProcess()
{
	for( unsigned int i = fileStack.size() ; i > 0 ; i-- )
	{
		fileStack[fileStack.size()-1]->close(); //close the file stream
		delete fileStack[fileStack.size()-1];
		fileStack.pop_back();  //gets file stream that was just closed off of stack
	}

	return true;
}

bool	
QLInterface::RunSoarTilOutput()
{
	pAgent->RunSelfTilOutput( 15 );
	return true;
}

bool
QLInterface::RunSoarForNCycles( int n )
{
	pAgent->RunSelf( n );
	return true;
}

string
QLInterface::GetNewOutput()
{
	OutputStorage = "";
	int numberCommands = pAgent->GetNumberOutputLinkChanges() ;
	int numberCommands2 = pAgent->GetNumberCommands();
	for(int i = 0; i < numberCommands; i++)
	{
		if(pAgent->IsOutputLinkChangeAdd(i))
		{
			sml::WMElement* tmp = pAgent->GetOutputLinkChange(i) ;
			string iden = tmp->GetIdentifierName();
			string att = tmp->GetAttribute();
			string val = tmp->GetValueAsString();
			OutputStorage += iden + " ^" + att + " " + val;
		}		
	}

	for(int i = 0; i< numberCommands2; i++)  //add's status complete
	{
		sml::Identifier* tmp2 = pAgent->GetCommand(i);
		tmp2->AddStatusComplete();
	}

	return OutputStorage;
}

string 
QLInterface::GetLastOutput()
{
	return OutputStorage;
}

string
QLInterface::GetInput()
{
	return Memory->GetInput();
}

bool
QLInterface::CommitChanges()
{
	pAgent->Commit();
	return true;
}

string
QLInterface::CreateNewKernel()
{
	//Create an instance of Soar kernel
	pKernel = sml::Kernel::CreateKernelInNewThread("SoarKernelSML");


	//Error checking to see if kernel loaded correctly
	if (pKernel->HadError())
		return pKernel->GetLastErrorDescription();

	//Create a Soar agent named "QuickLink"
	pAgent = pKernel->CreateAgent("QuickLink");

	//Make sure agent was successfully created
	if (pKernel->HadError())
		return pKernel->GetLastErrorDescription();

	return "Kernel Created.";
}

string
QLInterface::EstablishRemoteConnection( string agentName )
{
	pKernel = sml::Kernel::CreateRemoteConnection( true , NULL );

	//Error checking to see if kernel loaded correctly
	if (pKernel->HadError())
		return pKernel->GetLastErrorDescription();

	pAgent = pKernel->GetAgent(agentName.c_str());

	//Make sure agent was successfully created
	if (pKernel->HadError())
		return pKernel->GetLastErrorDescription();
	
	return "Connection Established.";
}

string 
QLInterface::ExecuteCommandLine( string cmd )
{
	return pAgent->ExecuteCommandLine( cmd.c_str() );
}

void
QLInterface::InitializeVariables()
{
	pInputLink = pAgent->GetInputLink();
	pOutputLink = pAgent->GetOutputLink();
	QLMemory NewMem( pKernel , pAgent , pInputLink , pOutputLink );
	Memory = &NewMem;
	QLDirtyWork NewDirtyWork( Memory );
	DirtyWork = &NewDirtyWork;
	Process Proc;
	ProcessManager = &Proc;
}

bool
QLInterface::SpawnDebugger()
{
/*	// spawn the debugger asynchronously
	int ret = _spawnlp(_P_NOWAIT, "javaw.exe", "javaw.exe", "-jar", "SoarJavaDebugger.jar", "-remote", NULL);
	if(ret == -1) {
		switch (errno) {
			case E2BIG:
				cout << "arg list too long";
				break;
			case EINVAL:
				cout << "illegal mode";
				break;
			case ENOENT:
				cout << "file/path not found";
				break;
			case ENOEXEC:
				cout << "specified file not an executable";
				break;
			case ENOMEM:
				cout << "not enough memory";
				break;
			default:
				cout << ret;
		}
	}
	Sleep(3500);*/
	return false;
}















string
QLInterface::FileControl()
{
	string toReturn = "file ended";  //flag used to enter while loop

	while( toReturn == "file ended" && fileStack.size() > 0 )
	{
		ProcessManager->LoadProcess( this , fileStack[fileStack.size() - 1] );

		if( toReturn == "file ended" )
		{
			fileStack[fileStack.size()-1]->close(); //close the file stream
			delete fileStack[fileStack.size()-1];
			fileStack.pop_back();  //gets file stream that was just closed off of stack
		}
	}

	if( toReturn == "file ended" )
		return "File loaded successfully.";
	else 
		return "File paused.";
}

void
QLInterface::MakeStringUpperCase( string& tooSmall )
{
	for( unsigned int ii = 0 ; ii < tooSmall.size() ; ii++ )
		tooSmall[ii]=toupper( tooSmall[ii] );
	return;
}
