#include <iostream>
#include <fstream>

#include "TextInput.h"
#include "SoarEpmem.h"

using namespace EpmemNS;
using namespace std;

int main(int argc, char *argv[]) {
	SoarEpmem epmem;
	WMEList cue;
	SymbolFactory *symfac = epmem.GetSymbolFactory();
	int errorline;
	
	errorline = ReadCue(cin, symfac, cue);
	if (errorline > 0) {
		cerr << "Error in cue line " << errorline << endl;
		return 1;
	}
	
	for (WMEList::iterator i = cue.begin(); i != cue.end(); ++i) {
		cout << (*i)->GetString() << endl;
	}
	
	if (argc <= 1) {
		cerr << "Specify an episodes file" << endl;
		return 1;
	}
	
	ifstream epin(argv[1], fstream::in);
	errorline = ReadEpisodes(epin, epmem);
	if (errorline > 0) {
		cerr << "Error in " << argv[1] << ":" << errorline << endl;
		return 1;
	}
	cout << "read " << epmem.GetNumEpisodes() << " episodes" << endl;
	
	epmem.Query(cue);
	
	return 0;
}