/*****************************************************
*  Author: Taylor Lafrinere
*  
*  Soar SoarTextIO
*
*  SoarTextIO.h
*
*  The purpose of this application is to allow the user
*  to freely type random words and sentences and have them
*  appear on the input-link.
*
*  Start Date: 06.23.2005
*
*****************************************************/

#ifndef SOARTEXTIO_H
#define SOARTEXTIO_H

#include "sml_client.h"
#include "sml_ClientWMElement.h"
#include <vector>
#include <fstream>

using namespace std;

class SoarTextIO 
{
public:

	//******CONSTRUCTORS******
	SoarTextIO();
	~SoarTextIO();

	void RespondCycle();
	//gets output from agent

	//******SML VARIABLES******
	sml::Kernel* pKernel;
	sml::Agent* pAgent;
	sml::Identifier* pInputLink;
	sml::Identifier* pOutputLink;
	sml::Identifier* pTextInput;
	sml::Identifier* pTextOutput;
	sml::StringElement* pLastNewest;
	vector<sml::WMElement*> dontlose;
	vector<sml::Identifier*> LastSent;

	bool waitForOutput, m_StopNow, printNow, m_IsRunning, initiateRem, initiateRes, init_soar;
	static void RunForever( void* info );
	void runner();

	void init();
	//initializes things for SoarTextIO

	

private:

	//******SENTENCE STORAGE******
	vector<sml::Identifier*> sentStore;  //holds identifiers

	vector<string> memory;

	struct wordSet
	{
		string word;
		int word_num;
	};

	vector<wordSet> sent;

	struct triple
	{
		string name;
		string att;
		string val;
		bool printed;
	};

	vector<triple> storeO;

	vector<sml::Identifier*> NextWord;
	

		
	//******FUNCTIONAL VARIABLES******
	int sentenceNum;
	int wordNum;
	string checker, loc, word, forMem, top_level;
	ifstream inFile;
	bool loadPlease, subtractOne, ShouldPrintNow, PrintNothing, getnextline;
	string agentName;	

	//******MEMBER FUNCTIONS******
	

	void run();
	//controls outside input and output loop

	void createSentId();
	//Creates Id for every sentence

	void WriteCycle(istream* getFrom);
	//controls input

	void spawnRunner();
	//spawns the debugger

	void makeUpper(string& tosmall);
	//change a string to all upper case letters

	void saveMem();
	//saves memory to a file

	//void locFinder();
	//in charge of getting file names

	void loadMem();
	//loads txt files to WM

	void CreateWord();
	//creates word identifier, word string and word-num

	void WhenReady();
	//asks to hit key when ready

	void spawnDebugger();

	void PrintOutput();
	//prints output

	string FindNextParent(string name);

	void RemoteConnect();

	void ResetConnect();

	void KillKernel();

	void step();

	string GetRelevant( string toShorten );

	char checkNext(istream& getFrom);

	bool GetFileInput(string& word);

	void CloseFile();

	void CarryOutCommand(istream* getFrom);

	void GetNextLine();

	
};






#endif