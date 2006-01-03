/*****************************************************
*  Taylor Lafrinere
*  
*  Soar Quick Link
*
*  Process.h
*
*  This idea of a process comes from Soar's QuickLink.
*  A process is a ordered list of QuickLink commands that
*  can be used as scripts for QuickLink or other programs.
*
*  Start Date: 08.24.2005
*
*****************************************************/

#ifndef PROCESS_H
#define PROCESS_H 

#include <string>
#include <vector>
#include <fstream>

class QLInterface;

using namespace std;

class Process 
{
public:

	//Constructor
	Process(  );

	//Accessors
	bool SaveProcess( vector<string> vecIn , string fileName );

	string LoadProcess( QLInterface* QLAPI , ifstream* inFile );  //May be source of load error
	//controls loading of process and structures


private:

	//Member data
	vector<string> ProcVec;
	string file;
	ifstream loader;
	ofstream saver;

	void MakeStringUpperCase( string& tooSmall );
};
#endif