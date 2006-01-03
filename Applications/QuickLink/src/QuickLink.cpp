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

#include "sml_Client.h"
#include "QuickLink.h"
#include "Reader.h"
#include <string>
#include <istream>
#include <iostream>
#include <vector>
#include <cctype>
#include <fstream>
#include <process.h>
#include <windows.h>

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

QuickLink::QuickLink(int argc, char* argv[])
{
	//OSFinder(); //needed for spawn debugger

	if(argc == 1)
		CreateNewKernel();
	else 
	{
		char* r = "-r";
		char* remote = "-remote";
		char* l = "-l";
		char* local = "-local";

		if(!strcmp(argv[1], r) || !strcmp(argv[1], remote))
			EstRemoteConnection();
		else if(!strcmp(argv[1], l) || !strcmp(argv[1], local))
			CreateNewKernel();
		else
		{
			cout << "Error: usage: legal commands are -l -local -r -remote.\n Example: QuickLink -remote" << endl;
			exit(1);
		}
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
QuickLink::CreateNewKernel()
{
	//Create an instance of Soar kernel
		pKernel = sml::Kernel::CreateKernelInNewThread("SoarKernelSML");


	//Error checking to see if kernel loaded correctly
	if (pKernel->HadError())
	{
		cout << pKernel->GetLastErrorDescription() << endl;
		return ;
	}

	//Create a Soar agent named "QuickLink"
	pAgent = pKernel->CreateAgent("QuickLink");

	//Make sure agent was successfully created
	if (pKernel->HadError())
	{
		cout << pKernel->GetLastErrorDescription() << endl;
		return ;
	}
}

void
QuickLink::EstRemoteConnection()
{
	pKernel = sml::Kernel::CreateRemoteConnection( true , NULL );
	
	//Error checking to see if kernel loaded correctly
	while (pKernel->HadError())
	{
		string junk;
		cout << pKernel->GetLastErrorDescription() << endl;
		cout << "Make sure a Kernel exists on the default port." << endl;
		cout << "When you are ready press any non-white space key + enter: ";
		cin >> junk;
		pKernel = sml::Kernel::CreateRemoteConnection( true , NULL );
	}

	string agentName;

	int numAgents = pKernel->GetNumberAgents();
	if(numAgents == 1)
		pAgent = pKernel->GetAgentByIndex(0);
	else
	{
		cout << "\nAvailable agents are:\n\n";
		for(int i = 0; i < numAgents; i++)
			cout << (pKernel->GetAgentByIndex(i))->GetAgentName() << endl;

		
		cout << "\nWhat is the name of the agent you would like to connect to: ";
		cin >> agentName;

		pAgent = pKernel->GetAgent(agentName.c_str());
	}	

	//Make sure agent was successfully created
	while (pKernel->HadError())
	{
		cout << pKernel->GetLastErrorDescription() << endl;
		cout << "Please enter a valid agent name: ";
		cin >> agentName;
		pAgent = pKernel->GetAgent(agentName.c_str());
	}
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
QuickLink::advValue()  //figure out what type the value is
{
	bool mFloat = false;
	for(unsigned int i = 0; i < value.size(); i++)
	{
		if (value[i] == '.')  //if it has a period it might be a float, not and int
			mFloat = true;
	}
	if(mFloat && (isdigit(value[0]) || isdigit(value[1]))) //value is float
	{
		command = "FLOAT";  //flag for createEL
		Fvalue = static_cast<float>(atof(value.c_str()));
		bool tester = true;
		for(unsigned int i = 0; i < FEs.size() && tester; i++)  //checks to see if element already exists
		{
			if(FEparent[i] == parent && FEnames[i] == path && FEvalue[i] == Fvalue)
				tester = false;
		}
		if(tester)	
		{
			
			createEL();	
		}				
	}
	else if(isdigit(value[0])) //value is int
	{
		command = "INT";  //flag for createEL
		Ivalue = atoi(value.c_str());
		bool tester = true;
		for(unsigned int i = 0; i < IEs.size() && tester; i++)  //checks to see if element already exists
		{
			if(IEparent[i] == parent && IEnames[i] == path && IEvalue[i] == Ivalue)
				tester = false;
		}
		if(tester)
		{
			
			createEL();
		}
	}
	else //value is string
	{
		bool tester = true;
		for(unsigned int i = 0; i < SEs.size() && tester; i++)  //checks to see if element already exists
		{
			if(SEparent[i] == parent && SEnames[i] == path && SEvalue[i] == value)
				tester = false;
		}
		if(tester)	
		{
			command = "STRING";  //flag for createEL
			createEL();
		}		
	}
}

void
QuickLink::createID()
{
	int sharedId = -1;
	bool tester = true;
	for(unsigned int i = 0; i < IDs.size() && tester; i++)  //checks to see if element already exists
	{
		if(IDparent[i] == parent && IDnames[i] == path && IDsoar[i] == uniqid)
			tester = false;
		if(IDsoar[i] == uniqid)
			sharedId = i;
	}
	if(tester)	
	{
		if(parent == "IL")
		{
			if(sharedId != -1)  //If this needs to be a sharedIdWme
			{
				sml::Identifier* temp1;
				temp1 = pAgent->CreateSharedIdWME(pInputLink, path.c_str(), IDs[sharedId]);
				SharedParent.push_back(parent);  
				SharedNames.push_back(path);	 
				SharedValue.push_back(uniqid);
				Shared.push_back(temp1);
			}
			else
			{
				sml::Identifier* temp1;
				temp1 = pAgent->CreateIdWME(pInputLink, path.c_str());
				IDnames.push_back(path);
				IDparent.push_back(parent);
				IDsoar.push_back(uniqid);
				IDs.push_back(temp1);
			}
			
		}
		else
		{
			int iI = -1;
			for(unsigned int i = 0; i < IDs.size(); i++)  //find the parent
			{
				if(IDsoar[i] == parent)
					iI = i;
			}
			if(iI == -1)  //parent doesn't exist
			{
				cout << endl << "ERROR: Parent name not found!" << endl << endl;
				WhenReady();
			}
			else
			{
				if(sharedId != -1)  //If this needs to be a sharedIdWme
				{
					sml::Identifier* temp1;
					temp1 = pAgent->CreateSharedIdWME(IDs[iI], path.c_str(), IDs[sharedId]);
					SharedParent.push_back(parent);  
					SharedNames.push_back(path);	 
					SharedValue.push_back(uniqid);
					Shared.push_back(temp1);
				}
				else
				{
					sml::Identifier* temp1;
					temp1 = pAgent->CreateIdWME(IDs[iI], path.c_str());
					IDnames.push_back(path);
					IDparent.push_back(parent);
					IDsoar.push_back(uniqid);
					IDs.push_back(temp1);
				}
				
			}					
		}
	}
}

void
QuickLink::createEL()
{
	int pI = -1;

	for(unsigned int i = 0; i < IDs.size(); i++)  //find parent
	{
		if(IDsoar[i] == parent)
			pI = i;
	}
	if(pI == -1 && parent != "IL") //parent doesn't exist
	{
		cout << endl << "ERROR: Parent name not found!" << endl << endl;
		WhenReady();
	}
	else
	{
		if (command == "STRING")
		{
			SEnames.push_back(path);
			SEvalue.push_back(value);
			if(parent == "IL")  //create on input-link
			{
				SEparent.push_back(parent);
				sml::StringElement* temp2 = pAgent->CreateStringWME(pInputLink, path.c_str(), value.c_str());
				SEs.push_back(temp2);
			}
			else
			{
				SEparent.push_back(parent);
				sml::StringElement* temp2 = pAgent->CreateStringWME(IDs[pI], path.c_str(), value.c_str());
				SEs.push_back(temp2);
			}

		}
		else if (command == "INT")
		{
            IEnames.push_back(path);
			IEvalue.push_back(Ivalue);
			if(parent == "IL")
			{
				IEparent.push_back(parent);
				sml::IntElement* temp3 = pAgent->CreateIntWME(pInputLink, path.c_str(), Ivalue);
				IEs.push_back(temp3);
			}
			else
			{
				IEparent.push_back(parent);
				sml::IntElement* temp3 = pAgent->CreateIntWME(IDs[pI], path.c_str(), Ivalue);
				IEs.push_back(temp3);
			}						
		}
		else
		{
			FEnames.push_back(path);
			FEvalue.push_back(Fvalue);
			if(parent == "IL")
			{
				FEparent.push_back(parent);
				sml::FloatElement* temp4 = pAgent->CreateFloatWME(pInputLink, path.c_str(), Fvalue);
				FEs.push_back(temp4);
			}
			else
			{
				FEparent.push_back(parent);
				sml::FloatElement* temp4 = pAgent->CreateFloatWME(IDs[pI], path.c_str(), Fvalue);
				FEs.push_back(temp4);
			}	

		}
	}
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
QuickLink::delSE(int index)
{
	if(first != "CLEAR" && first != "CLEARP" && first != "CP") //Implemented for Bob, clear should not erase lower-level structures
		pAgent->DestroyWME(SEs[index]);
	int last = SEs.size() - 1;  //the delete is made by swapping the element to be deleted with the last and using pop_back
	SEs[index] = SEs[last];
	SEnames[index] = SEnames[last];
	SEparent[index] = SEparent[last];
	SEvalue[index] = SEvalue[last];
	SEs.pop_back();
	SEnames.pop_back();
	SEparent.pop_back();
	SEvalue.pop_back();
}

void
QuickLink::delIE(int index)
{
	if(first != "CLEAR" && first != "CLEARP" && first != "CP") //Implemented for Bob, clear should not erase lower-level structures
		pAgent->DestroyWME(IEs[index]);
	int last = IEs.size() - 1;  //the delete is made by swapping the element to be deleted with the last and using pop_back
	IEs[index] = IEs[last];
	IEnames[index] = IEnames[last];
	IEparent[index] = IEparent[last];
	IEvalue[index] = IEvalue[last];
	IEs.pop_back();
	IEnames.pop_back();
	IEparent.pop_back();
	IEvalue.pop_back();
}

void
QuickLink::delFE(int index)
{
	if(first != "CLEAR" && first != "CLEARP" && first != "CP") //Implemented for Bob, clear should not erase lower-level structures
		pAgent->DestroyWME(FEs[index]);
	int last = FEs.size() - 1;  //the delete is made by swapping the element to be deleted with the last and using pop_back
	FEs[index] = FEs[last];
	FEnames[index] = FEnames[last];
	FEparent[index] = FEparent[last];
	FEvalue[index] = FEvalue[last];
	FEs.pop_back();
	FEnames.pop_back();
	FEparent.pop_back();
	FEvalue.pop_back();
}

void
QuickLink::delShared(int index)
{
	if(first != "CLEAR" && first != "CLEARP" && first != "CP") //Implemented for Bob, clear should not erase lower-level structures
		pAgent->DestroyWME(Shared[index]);
	int last = Shared.size() - 1;  //the delete is made by swapping the element to be deleted with the last and using pop_back
	Shared[index] = Shared[last];
	SharedParent[index] = SharedParent[last];
	SharedNames[index] = SharedNames[last];
	SharedValue[index] = SharedValue[last];
	Shared.pop_back();
	SharedParent.pop_back();
	SharedNames.pop_back();
	SharedValue.pop_back();
}

void
QuickLink::delID(int index)
{
	if(first != "CLEAR" && first != "CLEARP" && first != "CP") //Implemented for Bob, clear should not erase lower-level structures
		pAgent->DestroyWME(IDs[index]);
	int last = IDs.size() - 1;  //the delete is made by swapping the element to be deleted with the last and using pop_back
	IDs[index] = IDs[last];
	IDnames[index] = IDnames[last];
	IDparent[index] = IDparent[last];
	IDsoar[index] = IDsoar[last];
	IDs.pop_back();
	IDnames.pop_back();
	IDparent.pop_back();
	IDsoar.pop_back();
}

void
QuickLink::deleteChilds(string father, string always)
{
	int ind = -1;
	
	for(unsigned int j = 0; j < IDs.size(); j++)  //Goes through all Identifiers
	{
		if(IDparent[j] == father) //Finds ones who have this father
		{
			ind = j;
			deleteChilds(IDsoar[ind],always);
			if(first == "CLEAR") //Implemented for Bob, clear should not erase lower-level structures
			{
				if(IDparent[j] == always)
					pAgent->DestroyWME(IDs[ind]);
				else
				{
					//delete IDs[ind];
				}
			}
			delID(ind);
			j--;  //needed because of way delete is made
		}
	}	
	ind = -1;
	for(unsigned int j = 0; j < IEs.size(); j++) //Goes through all Int Elements 
	{
		if(IEparent[j] == father)  //Finds ones who have this father
		{
			ind = j;
			if(first == "CLEAR") //Implemented for Bob, clear should not erase lower-level structures
			{
				if(IEparent[j] == always)
					pAgent->DestroyWME(IEs[ind]);
				else
				{
					//delete IEs[ind];
				}
			}
			delIE(ind);
			j--;  //needed because of way delete is made
		}
	}
	ind = -1;
	for(unsigned int j = 0; j < FEs.size(); j++)  //Goes through all Float Elements
	{
		if(FEparent[j] == father ) //Finds ones who have this father
		{
			ind = j;
			if(first == "CLEAR") //Implemented for Bob, clear should not erase lower-level structures
			{
				if(FEparent[j] == always)
					pAgent->DestroyWME(FEs[ind]);
				else
				{
					//delete FEs[ind];
				}
			}
			delFE(ind);
			j--;  //needed because of way delete is made
		}
	}
	ind = -1;
	for(unsigned int j = 0; j < SEs.size(); j++)  //Goes through all String Elements
	{
		if(SEparent[j] == father ) //Finds ones who have this father
		{
			ind = j;
			if(first == "CLEAR") //Implemented for Bob, clear should not erase lower-level structures
			{
				if(SEparent[j] == always)
					pAgent->DestroyWME(SEs[ind]);
				else
				{
					//delete SEs[ind];
				}
			}
			delSE(ind);
			j--;  //needed because of way delete is made
		}
	}
	ind = -1;
	for(unsigned int j = 0; j < Shared.size(); j++)  //Goes through all Shared Elements
	{
		if(SharedParent[j] == father || SharedValue[j] == father ) //Finds ones who have this father
		{
			ind = j;
			if(first == "CLEAR") //Implemented for Bob, clear should not erase lower-level structures
			{
				if(SharedParent[j] == always)
					pAgent->DestroyWME(Shared[ind]);
				else
				{
					//delete Shared[ind];
				}
			}
			delShared(ind);
			j--;  //needed because of way delete is made
		}
	}
	
	ind = -1;


}
	

void
QuickLink::saveProcChanges()
{
	locFinder(&cin);
	cout << endl;
	ofstream aFile;
	aFile.open(loc.c_str());
	
	for(unsigned int i = 0; i < commandStore.size(); i++)
	{
		aFile << commandStore[i];
		if (i != commandStore.size()-1)
			aFile << endl;
	}

	aFile.close();
	aFile.clear();
	cout << endl << endl << "***Your process has been saved***" << endl;
	//WhenReady();
}

void 
QuickLink::saveInput(bool toClose, ofstream& oFile)
{
	if(!oFile)
	{
		cout << endl << "***ERROR: File failed to open***" << endl << endl;
		WhenReady();
	}
	else //write to file
	{	
		for(unsigned int i =0; i< IDs.size(); i++)
		{
			oFile << "add " << IDparent[i] << " ^" << IDnames[i] << " /" << IDsoar[i] << endl;
		}
		for(unsigned int i =0; i< Shared.size(); i++)
		{
			oFile << "add " << SharedParent[i] << " ^" << SharedNames[i] << " /" << SharedValue[i] << endl;
		}
		for(unsigned int i =0; i< IEs.size(); i++)
		{
			oFile << "add " << IEparent[i] << " ^" << IEnames[i] << " " << IEvalue[i] << endl;
		}
		for(unsigned int i =0; i< FEs.size(); i++)
		{
			oFile << "add " << FEparent[i] << " ^" << FEnames[i] << " " << FEvalue[i] << endl;
		}
		for(unsigned int i =0; i< SEs.size(); i++)
		{
			oFile << "add " << SEparent[i] << " ^" << SEnames[i] << " " << SEvalue[i] << endl;
		}

		if (toClose)  //only closes structure files, not process files
		{
			oFile.close();
			oFile.clear();
			cout << "***YOUR FILE HAS BEEN SAVED***" << endl << endl;
			//WhenReady();
		}
	}
}

void
QuickLink::advDelInd()  //deletes identifiers
{
	int ind = -1;
	for (unsigned int i = 0; i < IDs.size(); i ++)  //find element
	{
		if(IDparent[i] == parent && IDnames[i] == path && IDsoar[i] == uniqid)
			ind = i;
	}
	if (ind == -1)  //doesn't exist
	{
		cout << "ERROR: Either the parent, name or value specified does not exist!" << endl << endl;
		WhenReady();
	}
	else
	{
		deleteChilds(IDsoar[ind], IDsoar[ind]);
		delID(ind);
		cout << endl << "***WME HAS BEEN DELETED***" << endl << endl;
		//WhenReady();
	}			
}

void
QuickLink::advDelVal()  //deletes value based elements
{
	int ind = -1;
	bool mFloat = false;
	for(unsigned int i = 0; i < value.size(); i++)
	{
		if (value[i] == '.')
			mFloat = true;
	}
	if(mFloat && (isdigit(value[0]) || isdigit(value[1]))) //value is float
	{
		Fvalue = static_cast<float>(atof(value.c_str()));
		for(unsigned int i = 0; i < FEs.size(); i++)
		{
			if(FEparent[i] == parent && FEnames[i] == path && FEvalue[i] == Fvalue)
				ind = i;
		}
		if (ind == -1)
		{
			cout << "ERROR: Either the parent, name or value specified does not exist!" << endl << endl;
			WhenReady();
		}
		else
		{
			delFE(ind);
			cout << endl << "***WME HAS BEEN DELETED***" << endl << endl;
			//WhenReady();
		}

	}
	else if(isdigit(value[0])) //value is int
	{
		Ivalue = static_cast<int>(atoi(value.c_str()));
		for(unsigned int i = 0; i < IEs.size(); i++)
		{
			if(IEparent[i] == parent && IEnames[i] == path && IEvalue[i] == Ivalue)
				ind = i;
		}
		if (ind == -1)
		{
			cout << "ERROR: Either the parent, name or value specified does not exist!" << endl << endl;
			WhenReady();
		}
		else
		{
			delIE(ind);
			cout << endl << "***WME HAS BEEN DELETED***" << endl << endl;
			//WhenReady();
		}

	}
	else //value is string
	{
		for(unsigned int i = 0; i < SEs.size(); i++)
		{
			if(SEparent[i] == parent && SEnames[i] == path && SEvalue[i] == value)
				ind = i;
		}
		if (ind == -1)
		{
			cout << "ERROR: Either the parent, name or value specified does not exist!" << endl << endl;
			WhenReady();
		}
		else
		{
			delSE(ind);
			cout << endl << "***WME HAS BEEN DELETED***" << endl << endl;
			//WhenReady();
		}

	}
}

void
QuickLink::advAlter()
{
	int index = -1;
	bool mFloat = false;
	for(unsigned int i = 0; i < NewVal.size(); i++)
	{
		if (NewVal[i] == '.')
			mFloat = true;
	}
	if(mFloat && (isdigit(NewVal[0]) || isdigit(NewVal[1]))) //value is float
	{
		float OFvalue = static_cast<float>(atof(OldVal.c_str()));
		for(unsigned int i = 0; i < FEs.size(); i++)
		{
			if(FEparent[i] == parent && FEnames[i] == path && FEvalue[i] == OFvalue)
				index = i;
		}
		if (index == -1)
		{
			cout << "ERROR: Either the parent, name or value specified does not exist!" << endl << endl;
			WhenReady();
		}
		else
		{
			Fvalue = static_cast<float>(atof(NewVal.c_str()));
			pAgent->Update(FEs[index],Fvalue);
			FEvalue[index] = Fvalue;
		}					
	}
	else if(isdigit(NewVal[0])) //value is int
	{
		int OIvalue = atoi(OldVal.c_str());
		for(unsigned int i = 0; i < IEs.size(); i++)
		{
			if(IEparent[i] == parent && IEnames[i] == path && IEvalue[i] == OIvalue)
				index = i;
		}
		if (index == -1)
		{
			cout << "ERROR: Either the parent, name or value specified does not exist!" << endl << endl;
			WhenReady();
		}
		else
		{
			Ivalue = atoi(NewVal.c_str());
			pAgent->Update(IEs[index],Ivalue);
			IEvalue[index] = Ivalue;
		}					
	}
	else //value is string
	{
		for(unsigned int i = 0; i < SEs.size(); i++)
		{
			if(SEparent[i] == parent && SEnames[i] == path && SEvalue[i] == OldVal)
				index = i;
		}
		if (index == -1)
		{
			cout << "ERROR: Either the parent, name or value specified does not exist!" << endl << endl;
			WhenReady();
		}
		else
		{
			pAgent->Update(SEs[index],NewVal.c_str());
			SEvalue[index] = NewVal;
		}					
	}
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
QuickLink::clearAll()
{
	//commandStore.resize(0);
	//first = "";
	deleteChilds("IL", "IL");
	if(IDs.size() > 0 || FEs.size() > 0 || SEs.size() > 0 || IEs.size() > 0) //used to guarantee clear
		deleteChilds("IL", "IL");
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

void
QuickLink::spawnDebug()
{
	// spawn the debugger asynchronously
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

//	Sleep(3500);

	pKernel->GetAllConnectionInfo();
	char const * java_debugger = "java-debugger";
	char const * ready = "ready";
	
	//printf("%s", status);
	while(1)
	{
		Sleep(100);
		pKernel->GetAllConnectionInfo();
		char const * status = pKernel->GetAgentStatus(java_debugger);
		if(status && !strcmp(status,ready)) break;
	}
}

void
QuickLink::PurgeAllVectors()
{
	deleteChilds("IL", "IL");
/*	for(unsigned int i = 0; i < toBeCleansed.size(); i++)
		pAgent->DestroyWME(toBeCleansed[i]);*/
	IDnames.resize(0);
	IDparent.resize(0);
	IDsoar.resize(0);
	IDprint.resize(0);
	SEnames.resize(0);
	SEparent.resize(0);
	SEvalue.resize(0);
	SEprint.resize(0);
	SharedNames.resize(0);
	SharedParent.resize(0);
	SharedValue.resize(0);
	SharedPrint.resize(0);
	IEnames.resize(0);
	IEparent.resize(0);
	IEvalue.resize(0);
	IEprint.resize(0);
	FEnames.resize(0);
	FEparent.resize(0);
	FEvalue.resize(0);
	FEprint.resize(0);
	storeO.resize(0);
	fileStack.resize(0);
	SC.resize(0);
	commandStore.resize(0);

}

int main (int argc, char* argv[])
{
	// When we have a memory leak, set this variable to
	// the allocation number (e.g. 122) and then we'll break
	// when that allocation occurs.
	//_crtBreakAlloc = 449 ;

	{ // create local scope to allow for local memory cleanup before we check at end
		QuickLink QL(argc, argv);
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


