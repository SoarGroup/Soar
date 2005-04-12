
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

	//create an ILspec
	InputLinkSpec il;

	//should have at least 1 argument - the filename for the input link spec	
	if(argc != 2) 
	{
		cout << "Usage: " <<argc<< " <filename>" << endl;
		exit(1);
	}
	else
		inFileName = argv[1];

	if( (int)inFileName.find(dmSuffix.c_str(), inFileName.length() - dmSuffix.length() ) != -1)
	{
		//this is a dm file
		cout<<"Reading DM file..."<<endl;
		parsed = il.ImportDM(inFileName);
	}
	else if( (int)inFileName.find(ilSuffix.c_str(), inFileName.length() - ilSuffix.length() ) != -1)
	{
		//this is a il file
		cout<<"Reading IL file..."<<endl;
		parsed = il.ImportIL(inFileName);
	}
	else
	{
		//who knows what this is. 
		cout<<"Error: unknown file type"<<endl;
		parsed = 0;
	}

	if(!parsed)
	{
		cout<<"Error: reading file"<<endl;
		exit(1);
	}

	


	int foob;
	cin>>foob;
	//inputFile.close();
	return 0;
}

