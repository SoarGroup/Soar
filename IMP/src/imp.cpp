//This is the main file for the IMP project
//=========================================

#include <iostream>
#include <string>

#include "InputLinkObject.h"
#include "InputLinkSpec.h"
#include "CPPGenerator.h"

using namespace std;

const string ilSuffix = "il";
const string dmSuffix = "dm";

void Pause()
{
	cout << "Pausing now...." << endl;
	string foo;
	cin >> foo;
}
//TODO eventually, we'll want to read in args for what kinds of code output
//the user wants, like -java and -cpp
int main(int argc, char* argv[])
{

	string inFileName;
	string outFileName("IMPDefaultOut.cpp");
	bool parsed = false;
	ilObjVector_t	ilObjects; //container of digested WME descriptions
	typedObjectsMap_t typedILObjects;//container of digested WME descriptions that
	//represent complex classes/structure in the simulation environment

	//create an ILspec
	InputLinkSpec ilspec(ilObjects, typedILObjects);

	cout << "IMP Beginning..." << endl;
	//cout << "Number of args is " << argc << endl;

	//should have at least 1 argument - the filename for the input link spec
	if(argc < 2) 
	{
		cout << "First arg is " << argv[1] << endl;
		cout << "Usage: " <<argc<< " <filename>" << endl;
		Pause();
		exit(1);
	}
	else
	{
		inFileName = argv[1];
		cout << "Input file is:>" << inFileName << "<" << endl;		
	}
	
	//No output filename specified
	if(argc < 3)
		cout << "No output filename specified, using default...." << endl;
	else
	{
		outFileName = argv[2];
		cout << "Output file is:>" << outFileName << "<" << endl;
	}

	//determine file type
	//If this is a datamap file...
	if( static_cast<int>(inFileName.find(dmSuffix.c_str(), inFileName.length() - dmSuffix.length() ) != -1))
	{
		cout<<"Reading DM file..."<<endl;
		parsed = ilspec.ImportDM(inFileName);
	}
	//If this is an input link specification text file...
	else if( static_cast<int>(inFileName.find(ilSuffix.c_str(), inFileName.length() - ilSuffix.length() ) != -1))
	{
		cout<<"Reading IL file..."<<endl;
		parsed = ilspec.ImportIL(inFileName);
	}
	else
	{
		cout<<"Error: unknown file type"<<endl;
		parsed = false;
	}

	if(!parsed)
	{
		cout<<"Error: reading file"<<endl;
		Pause();
		exit(1);
	}
	
	//For debugging - print out all of the untyped object that were specified
	/*cout << "Back in the main loop, printing the stored objects now..." << endl;
	for(ilObjItr objItr = ilObjects.begin(); objItr != ilObjects.end(); ++objItr)
	{
		cout << *objItr;
	}*/
	
	/*typeMapItr_t typeItr = typedILObjects.begin();
	cout << "Number of simulation types specified: " << typedILObjects.size() << endl;
	for(; typeItr != typedILObjects.end(); ++typeItr)
	{
		cout << "Type " << typeItr->first << " has " << typeItr->second.size() << " object(s)." << endl;
	}*/
	cout << endl;
	
	//Now the containers are full of digested WME descriptions.  Generate code from them	
	//create the code generator
	//TODO use a command line arg to specify which flavor of SML to generate
	CPPGenerator generator(outFileName, ilObjects, typedILObjects);

	cout << "Pausing before exiting program.  Press non-whitespace key + <enter> to exit" << endl;
	Pause();
	return 0;
}

