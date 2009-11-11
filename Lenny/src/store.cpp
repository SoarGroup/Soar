#include <iostream>
#include <list>
#include <vector>
#include <sstream>
#include <fstream>

#include "SoarEpmem.h"

using namespace std;
using namespace EpmemNS;

const int BUFSIZE = 1024;

enum State { ADD, DEL };

static SoarEpmem *epmem;

/*
 * Set begin to the index of the beginning of the first token after
 * position i, and return the length of the token. If there are no
 * tokens, return 0.
 */
int gettoken(const string &s, size_t i, size_t& begin) {
	int j, k;
	j = s.find_first_not_of(" \t", i); // skip initial whitespace
	
	if (j == string::npos) {
		return 0;
	}
	
	if (s[j] == '"') {
		begin = j;
		k = s.find("\"", j+1); // find end of quoted string
		if (k != string::npos) {
			k++;
		}
	} else {
		begin = j;
		k = s.find_first_of(" \t", j);
	}
	
	if (k == string::npos) {
		return s.length() - j;
	} else {
		return k - j;
	}
}

void tokenize(const string &s, vector<string> &tokens) {
	size_t b = 0, len = 0;
	while (true) {
		len = gettoken(s, b + len, b);
		if (len == 0) {
			return;
		}
		tokens.push_back(s.substr(b, len));
	}
}

Symbol* getSymbol(string t) {
	int intval;
	double floatval;
	
	if (t[0] == '"' && t[t.length()-1] == '"') {
		return epmem->GetSymbolFactory()->GetStringSymbol(t.substr(1, t.length() - 2).c_str());
	}
	
	if (t[0] >= 'A' && t[0] <= 'Z') {
		stringstream ss(t.substr(1, t.length() - 1));
		int num;
		ss >> num;
		if (ss) {
			return epmem->GetSymbolFactory()->GetIdentifierSymbol(t[0], num);
		}
	}

	if (t[0] == '@' && t[1] >= 'A' && t[1] <= 'Z') {
		stringstream ss(t.substr(2, t.length() - 2));
		int num;
		ss >> num;
		if (ss) {
			return epmem->GetSymbolFactory()->GetIdentifierSymbol(t[1], num);
		}
	}
	
	stringstream ss(t);
	ss >> intval;
	if (ss) return epmem->GetSymbolFactory()->GetIntegerSymbol(intval);
	ss.clear();
	ss >> floatval;
	if (ss) return epmem->GetSymbolFactory()->GetFloatSymbol(floatval);
	
	return NULL;
}

WME* makeWME(string ids, string attrs, string vals, WMEUID uid) {
	Symbol *id;
	Symbol *attr;
	Symbol *val;
	
	id = getSymbol(ids);
	attr = getSymbol(attrs);
	val = getSymbol(vals);
	if (!id || !attr || !val) return NULL;
	if (id->GetType() != Symbol::IdSym) return NULL;
	
	return new WME(static_cast<EpmemNS::IdentifierSymbol*>(id), attr, val, uid);
}

int readInput(istream& input) {
	char buf[BUFSIZE];
	string line;
	int linecount = 0;

	WMEList addlist;
	DelList dellist;
	State s = ADD;
	
	stringstream ss;
	
	while (input.good()) {
		input.getline(buf, BUFSIZE);
		linecount++;
		line = buf;
		// strip whitespace
		line.erase(0, line.find_first_not_of(" \t"));
		line.erase(line.find_last_not_of(" \t") + 1);
		
		if (line.size() == 0) {
			continue;
		} else if (line == "#") {
			epmem->AddEpisode(addlist, dellist);
			addlist.clear();
			dellist.clear();
		} else if (line == "+") {
			s = ADD;
		} else if (line == "-") {
			s = DEL;
		} else {
			if (s == ADD) {
				vector<string> t;
				tokenize(line, t);
				
				if (t.size() != 4) return linecount;
				
				long uid; ss.clear(); ss.str(t[3]); ss >> uid;
				if (!ss) return linecount;
				
				WME* w = makeWME(t[0], t[1], t[2], uid);
				if (!w) return linecount;
				
				addlist.push_back(w);
			} else {
				long uid; ss.clear(); ss.str(line); ss >> uid;
				if (!ss) return linecount;
				dellist.push_back(uid);
			}
		}
	}
	return 0;
}

int main(int argc, char* argv[]) {
	int errorline;
	
	epmem = new SoarEpmem();

	if (argc > 1) {
		ifstream fin(argv[1], fstream::in);
		errorline = readInput(fin);
	} else {
		errorline = readInput(cin);
	}
	
	if (errorline > 0) {
		cerr << "Error on input line " << errorline << endl;
		return 1;
	}
	
	WMEList cue;
	SymbolFactory *sf = epmem->GetSymbolFactory();
	
	IdentifierSymbol* sS1 = sf->GetIdentifierSymbol( 'S', 1 );
	StringSymbol* sBlock = sf->GetStringSymbol( "block" );
	StringSymbol* sId = sf->GetStringSymbol( "id" );
	StringSymbol* sOnTop = sf->GetStringSymbol( "on-top" );
	StringSymbol* sOnTable = sf->GetStringSymbol( "on-table" );
	StringSymbol* sTop = sf->GetStringSymbol( "top" );
	StringSymbol* sBottom = sf->GetStringSymbol( "bottom" );
	StringSymbol* sA = sf->GetStringSymbol( "A" );
	StringSymbol* sB = sf->GetStringSymbol( "B" );
	StringSymbol* sC = sf->GetStringSymbol( "C" );

	IdentifierSymbol* sB1 = sf->GetIdentifierSymbol( 'B', 1 );
	IdentifierSymbol* sB2 = sf->GetIdentifierSymbol( 'B', 2 );
	IdentifierSymbol* sB3 = sf->GetIdentifierSymbol( 'B', 3 );

	IdentifierSymbol* sO1 = sf->GetIdentifierSymbol( 'O', 1 );
	IdentifierSymbol* sO2 = sf->GetIdentifierSymbol( 'O', 2 );
	IdentifierSymbol* sO3 = sf->GetIdentifierSymbol( 'O', 3 );

	WME *wBlock1 = new WME( sS1, sBlock, sB1, 1 );
	WME *wBlock2 = new WME( sS1, sBlock, sB2, 2 );
	WME *wBlock3 = new WME( sS1, sBlock, sB3, 3 );

	WME *wOnTop1 = new WME( sS1, sOnTop, sO1, 4 );
	WME *wOnTop2 = new WME( sS1, sOnTop, sO2, 5 );
	WME *wOnTableB1 = new WME( sS1, sOnTable, sB1, 6 );

	WME *wBlock1Id = new WME( sB1, sId, sA, 7 );
	WME *wBlock2Id = new WME( sB2, sId, sB, 8 );
	WME *wBlock3Id = new WME( sB3, sId, sC, 9 );

	WME *wOnTop1Top = new WME( sO1, sTop, sB3, 10 );
	WME *wOnTop1Bottom = new WME( sO1, sBottom, sB2, 11 );

	WME *wOnTop2Top = new WME( sO2, sTop, sB2, 12 );
	WME *wOnTop2Bottom = new WME( sO2, sBottom, sB1, 13 );

	//

	WME *wOnTableB3 = new WME( sS1, sOnTable, sB3, 14 );

	//

	WME *wOnTop3 = new WME( sS1, sOnTop, sO3, 15 );
	WME *wOnTop3Top = new WME( sO3, sTop, sB2, 16 );
	WME *wOnTop3Bottom = new WME( sO3, sBottom, sB3, 17 );

	cue.push_back(wBlock1);
	cue.push_back(wBlock2);
	cue.push_back(wBlock3);
	cue.push_back(wBlock1Id);
	cue.push_back(wBlock2Id);
	cue.push_back(wBlock3Id);
	cue.push_back(wOnTop3);
	cue.push_back(wOnTop3Top);
	cue.push_back(wOnTop3Bottom);

	epmem->Query(cue);
	return 0;
}