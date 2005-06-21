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
//the user wants, like -java -cpp
int main(int argc, char* argv[])
{

	string inFileName;
	string outFileName("IMPDefaultOut.cpp");
	bool parsed = false;
	ilObjVector_t	ilObjects;
	typedObjectsMap_t typedILObjects;

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
		parsed = false;
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
		cout << "Type " << typeItr->first << " has " << typeItr->second.size() << " object(s)." << endl;
	}
	cout << endl;
	
	//create the code generator - //TODO use a commandline arg to specify which 
	//type to make
	CPPGenerator generator(outFileName, ilObjects, typedILObjects);

	cout << "Pausing before exiting program.  Press non-whitespace key + <enter> to exit" << endl;
	Pause();
	return 0;
}

