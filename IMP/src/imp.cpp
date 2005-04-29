
#include <iostream>
#include <string>
#include "ilspec.h"

using namespace std;

void pause()
{
	cout << "Pausing now...." << endl;
	string foo;
	cin >> foo;
}
 
int main(int argc, char* argv[])
{

	string inFileName;
	const string ilSuffix = "il";
	const string dmSuffix = "dm";
	bool parsed = 0;
	string foos;

	//create an ILspec
	InputLinkSpec ilspec;
	
	cout << "Hajimeteimas..." << endl;
	cout << "Number of args is " << argc << endl;
	//pause();

	//should have at least 1 argument - the filename for the input link spec	
	if(argc != 2) 
	{
		cout << "First arg is " << argv[1] << endl;
		cout << "Usage: " <<argc<< " <filename>" << endl;
		pause();
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
		//pause();
		parsed = ilspec.ImportDM(inFileName);
	}
	else if( static_cast<int>(inFileName.find(ilSuffix.c_str(), inFileName.length() - ilSuffix.length() ) != -1))
	{
		cout<<"Reading IL file...and pausing"<<endl;
		//pause();	
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
		pause();
		exit(1);
	}

	cout << "Pausing before exiting program" << endl;
	pause();
	return 0;
}

