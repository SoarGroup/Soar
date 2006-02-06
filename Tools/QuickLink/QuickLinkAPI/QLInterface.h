/*****************************************************
*  Author: Taylor Lafrinere
*  
*  Soar QuickLink
*
*  QLInterface.h
*
*  Use this functions in this file to interface with 
*  QuickLink
*
*  Start Date: 08.24.2005
*
*****************************************************/

#ifndef QLINTERFACE_H
#define QLINTERFACE_H

#include <fstream>
#include <process.h>
#include <windows.h>
#include <cctype>
#include "sml_Client.h"
#include "QLMemory.h"
#include "QLDirtyWork.h"

class Process;

using namespace std;

class QLInterface 
{
public:

	/***Constructors***/

	QLInterface();

	/***Interface Functions***
	*
	*   Use these functions as an interface into QuickLink
	*
	***/

	bool ClearILink();
	/***ClearILink()***
	*   $brief: Clears the current input-link structure.
	*           Has no affect on memory that is stored.
	*
	*	$return: ClearILink will return true if the clear worked
	*            and false if it failed.
	***/

	bool SaveILink( string fileName );
	/***SaveILink()***
	*   $brief: Saves what is currently on the input-link 
	*           to the fileName provided.
	*
	*	$params: fileName: The file to which the structure will
	*					   be saved.
	*
	*	$return: SaveILink will return true if the save worked
	*            and false if it failed.
	***/

	string CreateWME( string parent , string attribute , string value );
	/***CreateWME()***
	*   $brief: Adds the Identifier triple provided to the Input-Link
	*
	*	$params: parent: The parent of the wme, the input-link is IL.
	*			 attribute: The name of the identifier.
	*			 value: A unique Id for the Identifier or value for an element.  
	*
	*	$example: identifier : (parent ^attribute /value)
	*			  non-id     : (parent ^attribute value)
	*
	*	$return: Possible return messages include:
	*			 "WME added successfully."
	*			 "ERROR: WME already exists."
	*			 "ERROR: Parent name not found."
	***/

	string ChangeWME( string parent , string attribute , string oldValue , string newValue );
	/***ChangeWME()***
	*   $brief: Changes the old value to the new value of the
	*			specified WME.
	*
	*	$params: parent: The parent of the wme to be changed.
	*			 attribute: The attribute of the wme to be changed.
	*			 oldValue: The value of the wme that will be replaced
	*					   by newValue.
	*			 newValue: The value that will replace oldValue
	*
	*	$return: Possible return messages include:
	*			 "WME changed successfully."
	*			 "ERROR: Either the parent, name or value specified does not exist."
	***/

	string DeleteWME( string parent , string attribute , string value );
	/***DeleteWME()***
	*   $brief: Deletes the specified identifier
	*
	*	$params: parent: The parent of the wme to be deleted.
	*			 attribute: The attribute of the wme to be deleted.
	*			 value: The value or uniqid of the wme to be deleted. 
	*
	*	$example: identifier : (parent ^attribute /value)
	*			  non-id     : (parent ^attribute value)
	*
	*	$return: Possible return messages include:
	*			 "WME deleted successfully."
	*			 "ERROR: Either the parent, name or value specified does not exist."
	***/

	bool Quit();
	/***Quit()***
	*   $brief: Shuts down all processes and clears memory
	*
	*	$return: Quit will return true if everything exited
	*            properly
	***/

	bool ClearProcessMemory();
	/***ClearProcessMemory()***
	*   $brief: Clears what is stored in process memory
	*
	*	$return: ClearProcessMemory will return true if the clear worked
	*            and false if it failed.
	***/

	string LoadFile( string fileName );
	/***LoadFile()***
	*   $brief: This function is used to load single-structure files
	*			and process files.  It will have the parser read from
	*			the given file
	*
	*	$params: fileName: The file from which to be read.
	*
	*	$return: Possible return messages include:
	*			 "File loaded successfully."
	*			 "File paused."  //indicates a paused process, use NextStepInProcess() to continue
	***/

	string NextStepInProcess();
	/***NextStepInProcess()***
	*   $brief: Runs Soar and loads the next step in the process.
	*
	*	$return: Possible return messages include:
	*			 "There are no processes running, CONTINUE is an invalid command."
	*			 "File paused."
	*			 "File loaded successfully."
	***/

	bool EndCurrentProcess();
	/***EndCurrentProcess()***
	*   $brief: Stopes the current process; gives control back to
	*			user input
	*
	*	$return: EndCurrentProcess will return true if it worked
	*            and false if it failed.
	***/

	bool SaveProcess( string fileName );
	/***SaveProcess()***
	*   $brief: Saves everything currently in process memory
	*			to a process in the file specified
	*
	*	$params: fileName: The file to which the process is saved.
	*
	*	$return: SaveProcess will return true if the save worked
	*            and false if it failed.
	***/

	bool RunSoarTilOutput();
	/***RunSoarTilOutput()***
	*   $brief: Runs Soar until output is generated.  This is the 
	*			preferred method for running QuickLink
	*
	*	$params: pAgent: pointer to agent that will be running.
	*
	*	$return: RunSoarTilOutput will return true if it worked
	*            and false if it failed.
	***/

	bool RunSoarForNCycles( int n );
	/***RunSoarForNCycles()***
	*   $brief: Runs Soar for n decision cycles.  Consider using the 
	*			RunSoarTilOutput method instead of this one
	*
	*	$params: n: Number of decision cycles to run for.
	*			 pAgent: pointer to agent that will be running.
	*
	*	$return: RunSoarForNCycles will return true if it worked
	*            and false if it failed.
	***/

	string GetNewOutput();
	/***GetNewOutput()***
	*   $brief: Gets the output generated on pOutputLink.
	*
	*	$params: pOutputLink: a pointer to the output-link that
	*			 the information is found on
	*
	*	$return: It will return a string containing the output in the form:
	*			 "<parent1> ^<att1> <value1> \n <parent2> ^<att2> <value2> \n ..."
	***/

	string GetLastOutput();
	/***GetLastOutput()***
	*   $brief: Gets the last output that was generated
	*
	*	$return: the same string returned in GetNewOutput()
	***/

	string GetInput();
	/***GetInput()***
	*   $brief: Gets the current input-link structure.
	*
	*	$return: Returns a string containing input in the following form:
	*            "<parent1> ^<att1> <value1> \n <parent2> ^<att2> <value2> \n ..."
	***/

	bool CommitChanges();
	/***CommitChanges()***
	*   $brief: Sends changes made on the user side to the kernel.
	*			This is done automatically on a run
	*
	*	$return: CommitChanges will return true if it worked
	*            and false if it failed.
	***/

	bool SpawnDebugger();  //currently only available on windows
	/***SpawnDebugger()***
	*   $brief: Spawns and connects the Soar Debugger.
	*
	*	$return: SpawnDebugger will return true if it worked
	*            and false if it failed.
	***/

	bool RecordInputLink(); 
	/***RecordInputLink()***
	*   $brief: Spawns and connects the Soar Debugger.
	*
	*	$return: RecordInputLink will return true if it worked
	*            and false if it failed.
	***/

	string ExecuteCommandLine( string cmd );
	/***ExecuteCommandLine()***
	*   $brief: Sends command to Soar's command line.
	*
	*	$return: ExecuteCommandLine will return what is outputted
	*            from the command line.
	***/

	string CreateNewKernel();
	/***CreateNewKernel()***
	*   $brief: Creates a new kernel on the default port.
	*
	*	$return: CreateNewKernel will return an error message or
	*            "Kernel Created."
	***/

	string EstablishRemoteConnection( string agentName );
	/***EstRemoteConnection()***
	*   $brief: Creates a remote connection to a kernel on the default port
	*			and to the agent with agentName.
	*
	*	$params: agentName: The name of the agent to connect to.
	*
	*	$return: CreateNewKernel will return an error message or
	*            "Connection Established."
	***/

private:

	QLMemory* Memory;
	QLDirtyWork* DirtyWork;

	#include "Process.h"
	Process* ProcessManager;

	sml::Agent* pAgent;
	sml::Kernel* pKernel;
	sml::Identifier* pOutputLink;
	sml::Identifier* pInputLink;

	//******FILE STACK*******

	vector<ifstream*> fileStack;

	/***Constants***/

	string _API_BAD_FILE;

	/***Output Storage***/

	string OutputStorage;

	void MakeStringUpperCase( string& tooSmall );

	string FileControl();

	void InitializeVariables();


};
#endif