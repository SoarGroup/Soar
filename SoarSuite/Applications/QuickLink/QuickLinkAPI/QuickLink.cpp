/*****************************************************
 *  Taylor Lafrinere
 *  
 *  Soar Quick Link
 *
 *  QuickLink.cpp
 *
 *  The purpose of this application is to be able to 
 *  control the input-link on soar from a command line
 *  and to read its output.
 *
 *  Start Date: 05.17.2005
 *
 *****************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

// Use Visual C++'s memory checking functionality
#ifdef _MSC_VER
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif // _MSC_VER

#include <process.h>
#include <windows.h>

#include "sml_Client.h"
#include "QuickLink.h"
#include "Reader.h"
#include <string>
#include <istream>
#include <iostream>
#include <vector>
#include <cctype>
#include <fstream>


using namespace std;


#if defined _WIN64 || defined _WIN32
 
void
QuickLink::OSFinder()
{
	OS = "windows";
}

#else

void 
QuickLink::OSFinder()
{
	OS = "not windows";
}
#endif


void MyStartSystemEventHandler(sml::smlSystemEventId id, void* pUserData, sml::Kernel* pKernel);

QuickLink::QuickLink()
{
	//OSFinder(); //needed for spawn debugger

	QLInterface Q;
	QLAPI = &Q;

	cout << "Would you like to create a new kernel or establish a remote connection \n(NEW or REMOTE): " ;
	cin >> connectionType;
	cout << endl << endl;
	makeUpper(connectionType);

	while(connectionType != "NEW" && connectionType != "REMOTE")
	{
		cout << "Please enter either NEW or REMOTE: ";
		cin >> connectionType;
		cout << endl << endl;
	}
	
	if(connectionType == "NEW")
		QLAPI->CreateNewKernel();
	else 
	{
		string agentName;
		cout << "What is the name of the agent you would like to connect to: ";
		cin >> agentName;
		while( QLAPI->EstablishRemoteConnection( agentName ) != "Connection Established.")
		{
			cout << "Make sure a Kernel exists on the default port." << endl;
			cout << "What is the name of the agent you would like to connect to: ";
			cin >> agentName;
		}
		


	//*******INITIALIZE VARIABLES******
	
	garbage = ""; parent = ""; 	path = ""; command = "";
	pInputLink = pAgent->GetInputLink();
	Fvalue = 0; Ivalue = 0; counter = 1;
	
	pOnce = true, printStep = false, Icycle = true, printTree = false, shouldPrintWM = true;
	loadingStep = false, StuffToSave = false, loadingProcess = false, endprocnow = false;
	resetProcStat = false, readFromCmd = true, enterOutputStage = false, printWM_runp = false;

	pKernel->RegisterForSystemEvent( sml::smlEVENT_SYSTEM_START , MyStartSystemEventHandler , this );
}

void
QuickLink::Run()
{
	string callParserRet = "";

	while(true)
	{
		while(!enterOutputStage)
		{
			Icycle = true;
			if(readFromCmd)
			{
				callParserRet = CallParser(&cin);
				if(callParserRet == "***QUITRETURN***")
					break;
			}
			else //read from a file
			{
				int tmp2 = fileStack.size();
				callParserRet = CallParser(fileStack[fileStack.size()-1]); //send in most recently opened filestream
				if(callParserRet == "***QUITRETURN***")
					break;
			}
		}

		if(callParserRet == "***QUITRETURN***")
			break;

		OutputCycle();
		pKernel->CheckForIncomingCommands();
		enterOutputStage = false;
	}
}

void
MyStartSystemEventHandler(sml::smlSystemEventId id, void* pUserData, sml::Kernel* pKernel)
{
	QuickLink* QL = (QuickLink*)pUserData;
	QL->pAgent->Commit();
}


string
QuickLink::CallParser(istream* in)
{
	Reader GetInfo(this);
	Icycle = true;
	//******INPUT******
	if(*in == cin)
		cout << endl << "******INPUT****** " << endl << endl;

	while (Icycle)
	{
		if(userInput)
			in = &cin;
		if(shouldPrintWM)
			PrintWorkingMem();
		else
			shouldPrintWM = true;
		if(*in == cin)
			cout << "> ";
		*in >> actualSize;
		first = actualSize;
		makeUpper(first);		

		toStore = GetInfo.ReadMe(in);	
		if(toStore == "***QUITRETURN***")
			return "***QUITRETURN***";
		if(toStore != "***VOIDRETURN***")
			commandStore.push_back(toStore);
	}
	//pAgent->Commit();
	if(printWM_runp)
	{
		PrintWorkingMem();
		printWM_runp = false;
	}
	return "";
}

void
QuickLink::PrintWorkingMem()
{
	if(!loadingStep)
	{
		cout << endl << "******CURRENT INPUT-LINK STRUCTURE******" << endl << endl;
		if (printStep)
		{
			cout << "******Step " << counter << " of Process " << processExt << "******" << endl << endl;
		}
		//Resize all to size of respective element number
		IDprint.resize(IDs.size());
		FEprint.resize(FEs.size());
		IEprint.resize(IEs.size());
		SEprint.resize(SEs.size());

		//set all to false
		for(unsigned int i = 0; i < IDs.size(); i++)
			IDprint[i] = false;
		for(unsigned int i = 0; i < IEs.size(); i++)
			IEprint[i] = false;
		for(unsigned int i = 0; i < SEs.size(); i++)
			SEprint[i] = false;
		for(unsigned int i = 0; i < FEs.size(); i++)
			FEprint[i] = false;
		if(printTree)
		{
			cout << "^input-link [IL]" << endl;
			WMrecurse("IL","  ",true);
		}
		else
			printSoarInForm();	
		cout << endl;
	}
	
	
	
}

void 
QuickLink::WMrecurse(string father, string indent, bool flag)
{
	unsigned int i;
	for(i = 0; i < IDs.size(); i++)
	{
		if(IDparent[i] == father && IDprint[i] == false)
		{
			cout << indent << "^" << IDnames[i] << " [" << IDsoar[i] <<"] " << endl;
			IDprint[i] = true;
            WMrecurse(IDsoar[i], indent + "  ",true);
		}
		if(flag)  
		{
			for(unsigned int s = 0; s < SEs.size(); s++)
			{
				if(SEparent[s] == father && SEprint[s] == false)
				{
					cout << indent << "^" << SEnames[s] << "  " << SEvalue[s] << endl;
					SEprint[s] = true;
				}
			}
			for(unsigned int f = 0; f < FEs.size(); f++)
			{
				if(FEparent[f] == father && FEprint[f] == false)
				{
					cout << indent << "^" << FEnames[f] << "  " << FEvalue[f] << endl;
					FEprint[f] = true;
				}
			}
			for(unsigned int n = 0; n < IEs.size(); n++)
			{
				if(IEparent[n] == father && IEprint[n] == false)
				{
					cout << indent << "^" << IEnames[n] << "  " << IEvalue[n] << endl;
					IEprint[n] = true;
				}
			}
			flag = false;
		}		
	}
	if(i == 0)
	{
		for(unsigned int s = 0; s < SEs.size(); s++)
		{
			if(SEparent[s] == father)
			{
				cout << indent << "^" << SEnames[s] << "  " << SEvalue[s] << endl;
			}
		}
		for(unsigned int f = 0; f < FEs.size(); f++)
		{
			if(FEparent[f] == father)
			{
				cout << indent << "^" << FEnames[f] << "  " << FEvalue[f] << endl;
			}
		}
		for(unsigned int n = 0; n < IEs.size(); n++)
		{
			if(IEparent[n] == father)
			{
				cout << indent << "^" << IEnames[n] << "  " << IEvalue[n] << endl;
			}
		}
		flag = false;
	}
}

void 
QuickLink::printSoarInForm()
{	
	vector<spaceControl> toPrint;  //queue of identifiers to print
	spaceControl tmp;  //controls indent level
	tmp.iden = "IL";
	tmp.indent = "";
	toPrint.push_back(tmp);
	for(unsigned int j = 0; j < toPrint.size(); j++)
	{
		if(IDs.size() > 0 || FEs.size() >0 || SEs.size() > 0 || IEs.size() >0)
			cout << toPrint[j].indent << "(" << toPrint[j].iden;
		else  //nothing in input-link structure
			cout << "(IL)" << endl << endl;
		int length = (3 + toPrint[j].indent.size() + toPrint[j].iden.size());  //controls word wrapping
		for(unsigned int i = 0; i < IDs.size(); i++)
		{
			if(IDparent[i] == toPrint[j].iden)
			{
				int tmp = (length + IDnames[i].size() + IDsoar[i].size() +3);
				if(tmp < 78)//controls word wrapping
				{	cout << " ^" << IDnames[i] << " " << IDsoar[i];
					length = tmp;
				}
				else
				{	cout << endl << toPrint[j].indent << "    " << " ^" << IDnames[i] << " " << IDsoar[i];
					length = (4 + toPrint[j].indent.size());  //controls word wrapping
				}
				spaceControl tmp2;
				string tmp5 = IDsoar[i];
				tmp2.iden = IDsoar[i];
				tmp2.indent = ("  " + toPrint[j].indent);
				if(IDprint[i] == false)
				{
					toPrint.push_back(tmp2);
					IDprint[i] = true;
				}
			}
		}
		for(unsigned int i = 0; i < Shared.size(); i++)
			if(SharedParent[i] == toPrint[j].iden)
			{
				int tmp = (length + SharedNames[i].size() + SharedValue[i].size() +3);
				if(tmp < 78)  //controls word wrapping
				{	cout << " ^" << SharedNames[i] << " " << SharedValue[i];
				length = tmp;  //controls word wrapping 
				}
				else
				{	cout << endl << toPrint[j].indent << "    " << " ^" << SharedNames[i] << " " << SharedValue[i];
				length = (4 + toPrint[j].indent.size());  //controls word wrapping
				}
			}
		for(unsigned int i = 0; i < SEs.size(); i++)
			if(SEparent[i] == toPrint[j].iden)
			{
				int tmp = (length + SEnames[i].size() + SEvalue[i].size() +3);
				if(tmp < 78)  //controls word wrapping
				{	cout << " ^" << SEnames[i] << " " << SEvalue[i];
					length = tmp;  //controls word wrapping 
				}
				else
				{	cout << endl << toPrint[j].indent << "    " << " ^" << SEnames[i] << " " << SEvalue[i];
					length = (4 + toPrint[j].indent.size());  //controls word wrapping
				}
			}
		for(unsigned int i = 0; i < IEs.size(); i++)
			if(IEparent[i] == toPrint[j].iden)
			{
				int tmp = (length + IEnames[i].size() + 8);
				if(tmp < 78)  //controls word wrapping
				{	cout << " ^" << IEnames[i] << " " << IEvalue[i];
					length = tmp;  //controls word wrapping
				}
				else
				{	cout << endl << toPrint[j].indent << "    " << " ^" << IEnames[i] << " " << IEvalue[i];
					length = (4 + toPrint[j].indent.size());  //controls word wrapping
				}
			}
		for(unsigned int i = 0; i < FEs.size(); i++)
			if(FEparent[i] == toPrint[j].iden)
			{
				int tmp = (length + FEnames[i].length() + 9);
				if(tmp < 78)  //controls word wrapping
				{	cout << " ^" << FEnames[i] << " " << FEvalue[i]; 
					length = tmp;  //controls word wrapping
				}
				else
				{	cout << endl << toPrint[j].indent << "    " << " ^" << FEnames[i] << " " << FEvalue[i];
					length = (4 + toPrint[j].indent.size());  //controls word wrapping
				}
			}
		if(IDs.size() > 0 || FEs.size() >0 || SEs.size() > 0 || IEs.size() >0)
			cout << ")" << endl;
		
	}
}

void
QuickLink::makeUpper(string & tosmall)
{
	for(unsigned int ii = 0; ii < tosmall.size(); ii++)
		tosmall[ii]=toupper(tosmall[ii]);
	return;
}

void
QuickLink::OutputCycle()
{
	storeO.resize(0);
	
	//******GET OUTPUT******	

	int numberCommands = pAgent->GetNumberOutputLinkChanges() ;
	int numberCommands2 = pAgent->GetNumberCommands();
	for(int i = 0; i < numberCommands; i++)
	{
		if(pAgent->IsOutputLinkChangeAdd(i))
		{
			triple trip;
			sml::WMElement* tmp =pAgent->GetOutputLinkChange(i) ;
			trip.name = tmp->GetIdentifierName() ;
			trip.att = tmp->GetAttribute();
			trip.val = tmp->GetValueAsString();
			trip.printed = false;
			storeO.push_back(trip);
		}		
	}

	printOutput();
	
	for(int i = 0; i< numberCommands2; i++)  //add's status complete
	{
		sml::Identifier* tmp2 = pAgent->GetCommand(i);
		tmp2->AddStatusComplete();
		pAgent->Commit();
	}
	pKernel->CheckForIncomingCommands();

	//pretty sure these next 3 lines are not needed, not sure though
	numberCommands = pAgent->GetNumberOutputLinkChanges() ;
	numberCommands2 = pAgent->GetNumberCommands();
	
	pAgent->ClearOutputLinkChanges();	

	return;
}

bool
QuickLink::displayTrips(string lookfor, string indent)  //output tree printer
{
    bool toReturn = true;
	for(unsigned int i = 0; i < storeO.size(); i++)
	{
		if(storeO[i].name == lookfor)
		{
			cout << endl << indent << "^" << storeO[i].att;
			if(displayTrips(storeO[i].val, indent + "  "))
			{
				cout << "  " << storeO[i].val ;
			}
			toReturn = false;
		}
	}
	return toReturn;
}

void
QuickLink::printSoarOutForm()
{
	for(unsigned int i = 0; i < storeO.size(); i++)
	{
		if(storeO[i].printed == false)
		{
			string indent = "";
			string iden;
			iden = storeO[i].name;
			for(unsigned int s = 0; s < SC.size(); s++)
				if(iden == SC[s].iden)
					indent = ("  " + SC[s].indent);
			spaceControl tmp;
			tmp.iden = storeO[i].val;
			tmp.indent = indent;
			SC.push_back(tmp);

			cout << indent << "(" << iden << " ^" << storeO[i].att << " " << storeO[i].val;
			storeO[i].printed = true;
			int returner = 1;  //controls word wrapping
			for(unsigned int j = 0; j < storeO.size(); j++)
			{
				if(storeO[j].name == iden && !storeO[j].printed)
				{
					if(returner > 3)  //controls word wrapping
					{
						cout << endl << indent;
						returner = 0;  //controls word wrapping
					}
					else
					{
						returner++;
						cout << " ";
					}
					cout << "^" << storeO[j].att << " " << storeO[j].val << " ";
					storeO[j].printed = true;
					spaceControl tmp2;
					tmp2.iden = storeO[j].val;
					tmp2.indent = indent;
					SC.push_back(tmp2);
				}
			}
			cout << ")" << endl;
		}
	}
	for(unsigned int i = 0; i < storeO.size(); i++)  //resets all printed flags to false
		storeO[i].printed = false;
}



void 
QuickLink::printOutput()
{
	cout << endl << endl << "******OUTPUT******" << endl;
	if(printTree)  //tree-form
	{
		cout << endl << "^output-link";
		bool useless = displayTrips("I3","  ");
	}
	else
		printSoarOutForm();  //soar-form
	
	cout << endl << endl << "******END OF OUTPUT******" << endl << endl;
	//WhenReady();
}

void
QuickLink::promptToSave()
{
	if(commandStore.size() != 0 && StuffToSave)
	{
		string toSave;
		cout << endl << "Would you like to save the steps stored in memory as a process before" 
			<< endl << "they are destroyed (y/n)?: ";
		cin >> toSave;
		makeUpper(toSave);
		while(toSave != "YES" && toSave != "NO" && toSave != "Y" && toSave != "N")
		{
			cout << endl << endl << "Please enter YES or NO: ";
			cin >> toSave;
			makeUpper(toSave);
		}
		if (toSave == "YES" || toSave == "Y")
		{
			cout << endl << "Please enter the name of the file to save your changes to: ";
			saveProcChanges();
		}
	}
}

void
QuickLink::WhenReady()
{
	cout << "Press any non white-space key + enter to continue: ";
	cout << endl;
	cin>> garbage;
	cout << endl << endl;
}

void
QuickLink::locFinder(istream* in)
{
	string fileName;
	string tmp;
	*in >> fileName;
	while(cin.peek() != '\n')
	{
		*in >> tmp;
		if(fileName != "")
			tmp = " " + tmp;
		fileName += tmp;
	}
	
	string location = "";//QuickLink\\ProcessesAndStructures\\";

	loc = location + fileName;	
	if(loc[loc.size()-4] != '.')
		loc += ".txt";

}

int main ()
{
	// When we have a memory leak, set this variable to
	// the allocation number (e.g. 122) and then we'll break
	// when that allocation occurs.
	//_crtBreakAlloc = 425 ;

	{ // create local scope to allow for local memory cleanup before we check at end
		QuickLink QL;
		Reader R(&QL);
		QL.Run();
	} // end local scope

#ifdef _MSC_VER
	printf("\nNow checking memory.  Any leaks will appear below.\nNothing indicates no leaks detected.\n") ;
	printf("\nIf no leaks appear here, but some appear in the output\nwindow in the debugger, they have been leaked from a DLL.\nWhich is reporting when it's unloaded.\n\n") ;

	// Set the memory checking output to go to Visual Studio's debug window (so we have a copy to keep)
	// and to stdout so we can see it immediately.
	_CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG );
	_CrtSetReportFile( _CRT_WARN, _CRTDBG_FILE_STDOUT );

	// Now check for memory leaks.
	// This will only detect leaks in objects that we allocate within this executable and static libs.
	// If we allocate something in a DLL then this call won't see it because it works by overriding the
	// local implementation of malloc.
	_CrtDumpMemoryLeaks();
#endif // _MSC_VER

	return 0;
}


