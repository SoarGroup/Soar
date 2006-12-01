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

#include "sml_Client.h"
#include "sml_ClientWMElement.h"
#include <vector>
#include <fstream>
#include <sstream>

using namespace std;

struct WMEpointer {
	sml::WMElement* holder;
};

class SoarTextIO 
{


private:

	int print_hack;
	//******SENTENCE STORAGE******
	vector<WMEpointer*> sentStore;  //holds identifiers

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

	vector<WMEpointer*> NextWord;

	

	//******Buffering******
	enum buf_command { DESTROY , CREATE , UPDATE };
	enum buf_type { INTEGER , STRING , FLOAT , ID };
	struct buffered_change_t {
		buffered_change_t(buf_command in_command, WMEpointer* in_dest, WMEpointer* in_parent, string in_att,
			string in_value, buf_type in_type)
			: command(in_command), destination(in_dest), parent(in_parent), attribute(in_att),
			value(in_value), type(in_type)
		{}

		void make_change(sml::Agent* pAgent)
		{
			switch (command) {
				case DESTROY : {
					pAgent->DestroyWME(destination->holder);
					break;
				}
				case CREATE : {
					switch (type) {
						case INTEGER : {
							if(destination)
								destination->holder = pAgent->CreateIntWME(parent->holder->ConvertToIdentifier(), attribute.c_str(), atoi(value.c_str()));
							else
								pAgent->CreateIntWME(parent->holder->ConvertToIdentifier(), attribute.c_str(), atoi(value.c_str()));
							break;
						}
						case STRING : {
							if(destination)
								destination->holder = pAgent->CreateStringWME(parent->holder->ConvertToIdentifier(), attribute.c_str(), value.c_str());
							else
								pAgent->CreateStringWME(parent->holder->ConvertToIdentifier(), attribute.c_str(), value.c_str());
							break;
						}
						case FLOAT : {
							if(destination)
								destination->holder = pAgent->CreateFloatWME(parent->holder->ConvertToIdentifier(), attribute.c_str(), atof(value.c_str()));
							else
								pAgent->CreateFloatWME(parent->holder->ConvertToIdentifier(), attribute.c_str(), atof(value.c_str()));
							break;
						}
						case ID : {
							if(destination)
								destination->holder = pAgent->CreateIdWME(parent->holder->ConvertToIdentifier(), attribute.c_str());
							else
								pAgent->CreateIdWME(parent->holder->ConvertToIdentifier(), attribute.c_str());
							break;
						}					
					}
					break;
				}
			}
		}

		buf_command command;
		WMEpointer* destination;
		WMEpointer* parent;
		string attribute;
		string value;
		buf_type type;

		
	};
	
		
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

	template <typename T>
	std::string string_make(T value)
	{
		std::ostringstream ss;
		ss << value;
		return ss.str();
	}

	buf_type decipher_type(const std::string& str);

	public:

		//******CONSTRUCTORS******
		SoarTextIO();
		~SoarTextIO();

		void RespondCycle();
		//gets output from agent

		void make_buffered_changes();

		//******SML VARIABLES******
		sml::Kernel* pKernel;
		sml::Agent* pAgent;
		WMEpointer pInputLink;
		sml::Identifier* pOutputLink;
		WMEpointer pTextInput;
		sml::Identifier* pTextOutput;
		sml::StringElement* pLastNewest;
		vector<WMEpointer*> dontlose;
		vector<WMEpointer*> LastSent;

		bool waitForOutput, m_StopNow, printNow, m_IsRunning, initiateRem, initiateRes, init_soar;
#ifdef _WIN32
		static void RunForever( void* info );
#else
		pthread_t newThread;
		int print_position;
#endif
		void runner();

		void init();
		//initializes things for SoarTextIO


		list<buffered_change_t> changes;

	
};






#endif
