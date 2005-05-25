#include <iostream>
#include <string>

#include "ilspec.h"
#include "ilobject.h"
#include "CodeGenerator.h"

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
//the user wants, like -java -cpp
int main(int argc, char* argv[])
{

	string inFileName;
	bool parsed = 0;
	ilObjVector_t	ilObjects;
	typedObjectsMap_t typedILObjects;

	//create an ILspec
	InputLinkSpec ilspec(ilObjects, typedILObjects);

	cout << "Hajimeteimas..." << endl;
	//cout << "Number of args is " << argc << endl;

	//should have at least 1 argument - the filename for the input link spec
	if(argc != 2) 
	{
		cout << "First arg is " << argv[1] << endl;
		cout << "Usage: " <<argc<< " <filename>" << endl;
		Pause();
		exit(1);
	}
	else
	{
		inFileName = argv[1];
		cout << "Filename set to >" << inFileName << "<" << endl;		
	}

	//determine file type
	if( static_cast<int>(inFileName.find(dmSuffix.c_str(), inFileName.length() - dmSuffix.length() ) != -1))
	{
		cout<<"Reading DM file..."<<endl;
		parsed = ilspec.ImportDM(inFileName);
	}
	else if( static_cast<int>(inFileName.find(ilSuffix.c_str(), inFileName.length() - ilSuffix.length() ) != -1))
	{
		cout<<"Reading IL file..."<<endl;
		parsed = ilspec.ImportIL(inFileName);
	}
	else
	{
		cout<<"Error: unknown file type"<<endl;
		parsed = 0;
	}

	if(!parsed)
	{
		cout<<"Error: reading file"<<endl;
		Pause();
		exit(1);
	}
	
	/*cout << "Back in the main loop, printing the stored objects now..." << endl;
	for(ilObjItr objItr = ilObjects.begin(); objItr != ilObjects.end(); ++objItr)
	{
		cout << *objItr;
	}*/
	
	typeMapItr_t typeItr = typedILObjects.begin();
	cout << "Number of simulation types specified: " << typedILObjects.size() << endl;
	for(; typeItr != typedILObjects.end(); ++typeItr)
	{
		cout << "Type " << typeItr->first << " has " << typeItr->second.size() << " objects." << endl;
	}
	cout << endl;
	
	//create the code generator - //TODO use a commandline arg to specify which 
	//type to make
	//TODO the name of the output file should be read in from command line
	string outFileName("testCPPFile.cpp");
	CPPGenerator generator(outFileName, ilObjects, typedILObjects);

	cout << "Pausing before exiting program" << endl;
	Pause();
	return 0;
}

