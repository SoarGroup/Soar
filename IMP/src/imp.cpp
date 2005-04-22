
#include <iostream>
#include <string>
#include "ilspec.h"

using namespace std;

int main(int argc, char* argv[])
{

	string inFileName;
	string ilSuffix = "il";
	string dmSuffix = "dm";
	bool parsed = 0;
	int foob;

	//create an ILspec
	InputLinkSpec ilspec;

	//should have at least 1 argument - the filename for the input link spec	
	if(argc != 2) 
	{
		cout << "Usage: " <<argc<< " <filename>" << endl;
		cin>>foob;
		exit(1);
	}
	else
		inFileName = argv[1];


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
		cin>>foob;
		exit(1);
	}

	


	cin>>foob;
	return 0;
}

