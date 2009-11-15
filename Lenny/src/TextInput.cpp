#include <iostream>
#include <list>
#include <vector>
#include <sstream>
#include <fstream>

#include "TextInput.h"

using namespace std;
using namespace EpmemNS;

const int BUFSIZE = 1024;

enum State { ADD, DEL };

/*
 * Set begin to the index of the beginning of the first token after
 * position i, and return the length of the token. If there are no
 * tokens, return 0.
 */
size_t gettoken(const string &s, size_t i, size_t& begin) {
	size_t j, k;
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

Symbol* getSymbol(string t, SymbolFactory *symfac) {
	int intval;
	double floatval;
	
	if (t[0] == '"' && t[t.length()-1] == '"') {
		return symfac->GetStringSymbol(t.substr(1, t.length() - 2).c_str());
	}
	
	if (t[0] >= 'A' && t[0] <= 'Z') {
		stringstream ss(t.substr(1, t.length() - 1));
		int num;
		ss >> num;
		if (ss) {
			return symfac->GetIdentifierSymbol(t[0], num);
		}
	}

	if (t[0] == '@' && t[1] >= 'A' && t[1] <= 'Z') {
		stringstream ss(t.substr(2, t.length() - 2));
		int num;
		ss >> num;
		if (ss) {
			return symfac->GetIdentifierSymbol(t[1], num);
		}
	}
	
	stringstream ss(t);
	ss >> intval;
	if (ss) return symfac->GetIntegerSymbol(intval);
	ss.clear();
	ss >> floatval;
	if (ss) return symfac->GetFloatSymbol(floatval);
	
	return NULL;
}

WME* makeWME(SymbolFactory *symfac, string ids, string attrs, string vals, WMEUID uid) {
	Symbol *id;
	Symbol *attr;
	Symbol *val;
	
	id = getSymbol(ids, symfac);
	attr = getSymbol(attrs, symfac);
	val = getSymbol(vals, symfac);
	if (!id || !attr || !val) return NULL;
	if (id->GetType() != Symbol::IdSym) return NULL;
	
	return new WME(static_cast<EpmemNS::IdentifierSymbol*>(id), attr, val, uid);
}

int ReadEpisodes(istream &input, Epmem &epmem) {
	WMEList addlist;
	DelList dellist;
	
	char buf[BUFSIZE];
	string line;
	int linecount = 0;
	vector<string> t;
	WME *w;

	State s = ADD;
	
	stringstream ss;

	SymbolFactory *symfac = epmem.GetSymbolFactory();
	
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
			epmem.AddEpisode(addlist, dellist);
			addlist.clear();
			dellist.clear();
		} else if (line == "+") {
			s = ADD;
		} else if (line == "-") {
			s = DEL;
		} else {
			if (s == ADD) {
				t.clear();
				tokenize(line, t);
				
				if (t.size() != 4) return linecount;
				
				long uid; ss.clear(); ss.str(t[3]); ss >> uid;
				if (!ss) return linecount;
				
				w = makeWME(symfac, t[0], t[1], t[2], uid);
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

int ReadCue(istream &input, SymbolFactory *symfac, WMEList &cue) {
	char buf[BUFSIZE];
	string line;
	vector<string> t;
	int linecount;
	WME *w;
	
	while (input.good()) {
		input.getline(buf, BUFSIZE);
		linecount++;
		line = buf;
		// strip whitespace and parentheses
		line.erase(0, line.find_first_not_of(" \t("));
		line.erase(line.find_last_not_of(" \t)") + 1);
		
		if (line.size() == 0) {
			continue;
		}
		t.clear();
		tokenize(line, t);
		
		if (t.size() != 3) return linecount;
		
		w = makeWME(symfac, t[0], t[1], t[2], 0); // the WMEUID shouldn't matter
		if (!w) return linecount;
		
		cue.push_back(w);
	}
	
	return 0;
}