//#include "QuickLink.h"
#include <string>
#include <istream>
#include <iostream>
#include "QuickLink.h"

//using namespace std;

//class QuickLink;
using namespace std;

class Reader
{
public:
	
	Reader(QuickLink* bringIn);

	string ReadMe(istream* in);

	void EndOfFile(istream* in);
private:

	QuickLink* QL;
	//PARSER CONSTANTS
	string _QUIT ;
	string _CLEAR ;
	string _SAVE ;
	string _SAVES ;
	string _LOAD ;
	string _LOADS ;
	string _ADD ;
	string _ADDS ;
	string _CHANGE ;
	string _CHANGES ;
	string _DELETE ;
	string _DELETES, loc ;
	string _DONE, _SOAR_FORM, _SF, _TREE_FORM, _TF ;
	string _NEWP, _NEWPS, _SAVEP, _SAVEPS, _LOADP, _LOADPS, _ENDP, _ENDPS;
	string _RUN, _RUNS, _RUNC, _RUNCS, _LASTO, _LASTOS, _CMDLIN, _CL;

	

};