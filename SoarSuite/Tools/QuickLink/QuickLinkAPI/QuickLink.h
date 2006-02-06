/*****************************************************
*  Taylor Lafrinere
*  
*  Soar Quick Link
*
*  QuickLink.h
*
*  The purpose of this application is to be able to 
*  control the input-link on soar from a command line
*  and to read its output to another window.
*
*  Start Date: 05.17.2005
*
*****************************************************/

#ifndef QUICKLINK_H	
#define QUICKLINK_H

#include <vector>
#include <fstream>
#include <string>
#include "sml_Client.h"
#include "Listener.h"
#include "Process.h"
//#include "Reader.h"

using namespace std;

class QuickLink
{
	friend class Reader;
public:

	//******CONSTRUCTORS******

	QuickLink();
	//default constructor

	//******PUBLIC METHODS******

	void Run();
	//Runs the program

	void Run2();

	sml::Agent* pAgent;

private:

	//******MEMBER STORAGE******  //QLMemory

	vector<sml::Identifier*> IDs;
	vector<string> IDnames;
	vector<string> IDparent;
	vector<string> IDsoar;
	vector<bool> IDprint;
	vector<sml::StringElement*> SEs;
	vector<string> SEnames;
	vector<string> SEparent;
	vector<string> SEvalue;
	vector<bool> SEprint;
	vector<sml::Identifier*> Shared;
	vector<string> SharedNames;
	vector<string> SharedParent;
	vector<string> SharedValue;
	vector<string> SharedPrint;
	vector<sml::IntElement*> IEs;
	vector<string> IEnames;
	vector<string> IEparent;
	vector<int> IEvalue;
	vector<bool> IEprint;
	vector<sml::FloatElement*> FEs;
	vector<string> FEnames;
	vector<string> FEparent;
	vector<float> FEvalue;
	vector<bool> FEprint;

	//******OUTPUT STORAGE******  //QLMemory

	struct triple
	{
		string name;
		string att;
		string val;
		bool printed;
	};

	vector<triple> storeO;

	//*******SOAR-FORM OUTPUT SPACING TRICK******

	struct spaceControl
	{
		string indent;
		string iden;
	};

	vector<spaceControl> SC;


	//******FILE STACK*******

	vector<ifstream*> fileStack;

	//******ALTERING PROCESS FILES******

	vector<string> commandStore;

	//******FOR MEMORY LEAKS******
	vector<sml::WMElement*> toBeCleansed;

	//*******MEMBER VARIABLES******

	QLInterface* QLAPI;
	sml::Kernel* pKernel;

	sml::Identifier* pInputLink;
	sml::Identifier* advHold;

	float Fvalue;
	int Ivalue;
	string command, title, garbage, parent, path, uniqid;
	string OldVal, NewVal, value, processExt, pCommand;
	string first, second, third, fourth, fifth, OS, loc, toStore, actualSize;
	ofstream outFile;
	bool toClose, pOnce, printStep, askToSave, Icycle, printTree, loadingStep, userInput, StuffToSave, loadingProcess;
	bool endprocnow, shouldPrintWM, resetProcStat, readFromCmd, enterOutputStage, printWM_runp;
	char endFind;
	int counter;
	string connectionType;

	


	
	//******MEMBER METHODS******

	void InputCycle();
	//Create and Alter working memory

	void OutputCycle();
	//Capture and print Output from Soar

	void PrintWorkingMem();
	//Prints the Working Memory in tree form

	void WMrecurse(string father, string indent, bool flag);
	//Recursively prints the WM tree

	void makeUpper(string& tosmall);
	//change a string to all lower case letters

	bool displayTrips(string lookfor, string indent);
	//prints output nicely

	void WhenReady();
	//asks user when ready

	void printOutput();
	//prints output

	void printSoarOutForm();
	//prints output in soar form

	void printSoarInForm();
	//prints input in soar form

	void locFinder(istream* in);
	//sets location for storage

	void OSFinder();

	string CallParser(istream* in);


};


#endif