
#include "Reader.h"


using namespace std;

Reader::Reader(QuickLink* bringIn)
{
	QL = bringIn;

	//******CONSTANTS FOR PARSER******
	_QUIT = "QUIT";	_CLEAR = "CLEAR";	_SAVE = "SAVE";	_SAVES = "S";	_LOAD = "LOAD";
	_LOADS = "L";	_ADD = "ADD";	_ADDS = "A";	_CHANGE = "CHANGE";	_CHANGES = "C";
	_DELETE = "DELETE";	_DELETES = "D";	_DONE = "DONE";	_NEWP = "CLEARP";	_NEWPS = "CP";
	_LOADP = "LOADP";	_LOADPS = "LP";	_SAVEP = "SAVEP";	_SAVEPS = "SP";	_ENDP = "ENDP";
	_ENDPS = "EP";	_RUNCS = "RC";	_RUNC = "RUNCYCLE";	_RUN = "GO";	_RUNS = "G";
	_LASTOS = "LO";	_LASTO = "LASTOUTPUT"; _CMDLIN = "CMDLIN"; _CL = "CL"; _SOAR_FORM = "SOAR-FORM";
	_SF = "SF"; _TREE_FORM = "TREE-FORM"; _TF = "TF";
}

string
Reader::ReadMe(istream* in)
{
	string toReturn;
	
	if(!(*in))  //EOF is reached in either a process or a structure
	{
		EndOfFile(in);
		toReturn = "***VOIDRETURN***";
		return toReturn;
	}
	else if (QL->first == _CLEAR)  //clear current input-link structure
	{
		QL->clearAll();
		if(in != &cin)
			QL->shouldPrintWM = false;
		toReturn = QL->first;
		return toReturn;
	}
	else if (QL->first == _SAVE || QL->first == _SAVES)  //save current input-link structure
	{
		QL->locFinder(in);  //gets location of file
		ofstream tempFile;
		tempFile.open(QL->loc.c_str());
		QL->saveInput(true,tempFile);
		tempFile.close();
		tempFile.clear();
		toReturn = "***VOIDRETURN***";
		return toReturn;
	}
	else if (QL->first == _ADD || QL->first == _ADDS)  //add something to il structure
	{
		*in >> QL->second >> QL->third >> QL->fourth;  //used to make parser changes easier
		QL->parent = QL->second;
		QL->makeUpper(QL->parent);
		QL->path = QL->third.substr(1, QL->third.size()-1); //gets rid of ^
		if(QL->fourth[0] == '/')  //IDENTIFER
		{				
			QL->uniqid = QL->fourth.substr(1, QL->fourth.size()-1); //gets rid of /
			QL->makeUpper(QL->uniqid);
			QL->createID();
		}
		else //VALUE_BASED
		{
			QL->value = QL->fourth;
			QL->advValue();
		}
		if(in != &cin)
			QL->shouldPrintWM = false;
		toReturn = QL->first + " " + QL->second + " " + QL->third + " " + QL->fourth;
		return toReturn;
	}
	else if (QL->first == _CHANGE || QL->first == _CHANGES) //change the value of something
	{
		*in >> QL->second >> QL->third >> QL->fourth >> QL->fifth;
		QL->parent = QL->second;
		QL->makeUpper(QL->parent);
		QL->path = QL->third.substr(1, QL->third.size()-1);  //gets rid of ^
		QL->OldVal = QL->fourth;
		QL->NewVal = QL->fifth;
		QL->advAlter();
		if(in != &cin)
			QL->shouldPrintWM = false;
		toReturn = QL->first + " " + QL->second + " " + QL->third + " " + QL->fourth + " " + QL->fifth;
		return toReturn;
	}
	else if (QL->first == _DELETE || QL->first == _DELETES) //delete an element
	{
		*in >> QL->second >> QL->third >> QL->fourth;
		QL->parent = QL->second;
		QL->makeUpper(QL->parent);
		QL->path = QL->third.substr(1,QL->third.size()-1);  //gets rid of ^
		if(QL->fourth[0] == '/') //IDENTIFIER
		{
			QL->uniqid = QL->fourth.substr(1,QL->fourth.size()-1);  //gets rid of /
			QL->makeUpper(QL->uniqid);
			QL->advDelInd();
		}
		else //VALUE_BASE
		{
			QL->value = QL->fourth;
			QL->advDelVal();
		}
		if(in != &cin)
			QL->shouldPrintWM = false;
		toReturn = QL->first + " " + QL->second + " " + QL->third + " " + QL->fourth;
		return toReturn;
	}
	else if (QL->first == _QUIT || QL->first == "EXIT")  //quit program
	{
		QL->PurgeAllVectors();
		QL->promptToSave();
		QL->pKernel->Shutdown();
		delete QL->pKernel;
		return "***QUITRETURN***";
	}
	else if (QL->first == _NEWP || QL->first == _NEWPS)  //clears process memory
	{
		QL->promptToSave();
		QL->commandStore.resize(0);
		QL->StuffToSave = false;
		QL->clearAll();
		if(in != &cin)
			QL->shouldPrintWM = false;
		return toReturn;
	}
	else if(QL->first == _LOAD || QL->first == _LOADS)
	{
		QL->shouldPrintWM = false; //we do not want to print WM everytime a new structure is added
		QL->locFinder(in);  //gets the location of the file
		
		QL->fileStack.resize(QL->fileStack.size()+1);  //increase the size of the file stack
		QL->fileStack[QL->fileStack.size()-1] = new ifstream; //create new ifstream and have end of stack point to it
		(*(QL->fileStack[QL->fileStack.size()-1])).open(QL->loc.c_str()); //open the stream
		if(!(*(QL->fileStack[QL->fileStack.size()-1]))) //error testing
			cout << "File " << QL->loc << " Failed to Open" << endl;
		
		QL->readFromCmd = false; //intiates CallParser(&inFile)
		QL->Icycle = false; //allows for a change of input source
		toReturn = "***VOIDRETURN***";
		return toReturn;
	}
	else if(QL->first == "CONTINUE" || QL->first == "CONT")
	{
		if(QL->fileStack.size() == 0)    //a process cannot be running
			cout << "There are no processes running, CONTINUE is an invalid command." << endl;
		else
		{
			QL->Icycle = false;
			QL->readFromCmd = false;
			QL->enterOutputStage = true;
			QL->pAgent->RunSelfTilOutput();
			QL->shouldPrintWM = false;
			if(in != &cin)
				QL->printWM_runp = true;
		}
		toReturn = "PAUSE";
		return toReturn;
	}
	else if (QL->first == _SAVEP || QL->first == _SAVEPS) //save process
	{
		QL->saveProcChanges();
		toReturn = "***VOIDRETURN***";
		return toReturn;
	}
	else if (QL->first == _ENDPS || QL->first == _ENDP) //end loaded process
	{
		EndOfFile(in);
		toReturn = "***VOIDRETURN***";
		return toReturn;
	}
	else if (QL->first == _RUNS || QL->first == _RUN)  //run soar til output
	{
		QL->counter++;
		QL->userInput = false;
		QL->StuffToSave = true;
		QL->SC.resize(0);   //clears output storage
		QL->Icycle = false;  //gets out of outer loop
		QL->pAgent->RunSelfTilOutput();
		QL->enterOutputStage = true;
		if(in == &cin)
			toReturn = "PAUSE";
		else
		{
			QL->PrintWorkingMem();
			QL->shouldPrintWM = false;
			toReturn = "Go";
		}
		return toReturn;
	}
	else if(QL->first == _RUNC || QL->first == _RUNCS)  //run soar for n cycles
	{
		QL->counter++;
		QL->userInput = false;
		QL->StuffToSave = true;
		QL->SC.resize(0); //clears output storage
		int amount;
		*in >> amount;
		if(amount > 15)
			amount = 15;
		QL->Icycle = false;
		QL->pAgent->RunSelf(amount);
		if(in == &cin)
            toReturn = "PAUSE";
		else
			toReturn = "Go";
		return toReturn;
	}
	else if(QL->first == _LASTO || QL->first == _LASTOS)  //re-print last output
	{
		cout << endl;
		QL->printOutput();
		QL->shouldPrintWM = false;
		toReturn = "***VOIDRETURN***";
		return toReturn;
	}
	else if(QL->first == "LASTINPUT" || QL->first == "LI")
	{
		//Nothing has to be done here because WM will be printed by default
		toReturn = "***VOIDRETURN***";
		return toReturn;
	}
	else if(QL->first == "COMMIT")
	{
		if(QL->pKernel->IsAgentValid(QL->pAgent))
			QL->pAgent->Commit();
		else
			cout << endl << "There is no agent loaded." << endl;
		toReturn = "Commit";
		return toReturn;
	}
	else if(QL->first == _CMDLIN || QL->first == _CL)  //execute command line command
	{
		char cmd[1000] ;  //used to get entire line including spaces
		in->getline(cmd,1000) ;
		string strcmd = cmd;
		cout << endl << QL->pKernel->ExecuteCommandLine(strcmd.c_str(), QL->pAgent->GetAgentName()) << endl;
		QL->shouldPrintWM = false;
		toReturn = "***VOIDRETURN***";
		return toReturn;
	}
	else if(QL->first == _TF || QL->first == _TREE_FORM)  //print structures in tree-form
	{
		cout << endl << "***Structures will now be printed in Tree-form***" << endl;
		QL->printTree = true;
		toReturn = QL->first;
		return toReturn;
	}
	else if (QL->first == _SF || QL->first == _SOAR_FORM)  //print structures in soar-form
	{
		cout << endl << "***Structures will now be printed in Soar-form***" << endl;
		QL->printTree = false;
		toReturn = QL->first;
		return toReturn;
	}
	else if(QL->first == "PAUSE")
	{
		QL->shouldPrintWM = true;  
		QL->Icycle = false;  //allows for a new input source
		QL->readFromCmd = true;  //new source is command line
		toReturn = "***VOIDRETURN***";
		return toReturn;
	}
	else if(QL->first[0] == '#')
	{
		char elget;
		in->get(elget);
		toReturn = QL->first;
		while (elget != '\n' && elget != EOF && (*in))
		{
			toReturn += elget;
			in->get(elget);
		}
		QL->shouldPrintWM = false;
		return toReturn;
	}
	else if(QL->first == "DEBUG")
	{
		QL->spawnDebug();
		toReturn = "***VOIDRETURN***";
		return toReturn;
	}
	else if(QL->first == "SWITCH")
	{	
		
		QL->clearAll();
		QL->PurgeAllVectors();

		QL->pKernel->Shutdown();
		QL->EstRemoteConnection();
		toReturn = "***VOIDRETURN***";
		return toReturn;
	}
	else 
	{
		string strcmd = QL->actualSize;
		QL->shouldPrintWM = false;
		while(in->peek() != '\n' && in->peek() != EOF)
		{
			string tmp;
			*in >> tmp;
			if(strcmd == "")
				strcmd = tmp;
			else
				strcmd += (" " + tmp);
		}
		cout << endl << QL->pKernel->ExecuteCommandLine(strcmd.c_str(), QL->pAgent->GetAgentName()) << endl;
		toReturn = "***VOIDRETURN***";
		return toReturn;
	}		

}

void
Reader::EndOfFile(istream* in)
{
	in->clear();
	if(in != &cin)          //as long as in != cin
	{
		QL->fileStack[QL->fileStack.size()-1]->close(); //close the file stream
		delete QL->fileStack[QL->fileStack.size()-1];
	}
	QL->fileStack.pop_back();  //gets file stream that was just closed off of stack
	QL->Icycle = false;     //allows for a new source of input
	if(QL->fileStack.size() > 0)   //there is a file open somewhere
	{
		QL->readFromCmd = false;  //read from the file
		QL->shouldPrintWM = false;
	}
	else
	{
		QL->readFromCmd = true; //read from cmdlin
		QL->shouldPrintWM = true;
	}
}