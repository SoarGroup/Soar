/* Reads previous state vector on stdin and outputs next state vector on stdout */

#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <iterator>
#include <vector>
#include "linalg.h"
#include "splinter.h"

using namespace std;

void split(const string &s, const string &delim, vector<string> &out) {
	int i = s.find_first_not_of(delim), j;
	out.clear();
	while (i != string::npos) {
		j = s.find_first_of(delim, i);
		if (j == string::npos) {
			out.push_back(s.substr(i));
			return;
		} else {
			out.push_back(s.substr(i, j - i));
		}
		i = s.find_first_not_of(delim, j);
	}
}

void appendvec(vector<double> &v1, const vec3 &v2) {
	v1.push_back(v2.x);
	v1.push_back(v2.y);
	v1.push_back(v2.z);
}

int main(int argc, char *argv[]) {
	string line;
	vector<string> fields;
	vector<vector<double> > ys;
	double x[16];
	int lc = 0;
	
	while (getline(cin, line)) {
		++lc;
		split(line, " ", fields);
		if (fields.size() == 0) {
			continue;
		}
		if (fields.size() != 16) {
			cerr << "Incorrect number of fields, line " << lc << endl;
			return 1;
		}
		for(int i = 0; i < 16; ++i) {
			stringstream ss(fields[i]);
			if (!(ss >> x[i])) {
				cerr << "Invalid double, line " << lc << " field " << i << endl;
				return 1;
			}
		}
	
		vec3 pos(x);
		vec3 rot(x + 3);
		vec3 rotrate(x + 8);
		vec3 vel(x + 11);
		
		double lrps = x[6];
		double rrps = x[7];
		double lvolt = x[14];
		double rvolt = x[15];
		
		splinter_update(lvolt, rvolt, lrps, rrps, pos, vel, rot, rotrate);
		
		ys.push_back(vector<double>());
		vector<double> &y = ys.back();
		
		y.reserve(14);
		appendvec(y, pos);
		appendvec(y, rot);
		y.push_back(lrps);
		y.push_back(rrps);
		appendvec(y, rotrate);
		appendvec(y, vel);
	}
	
	vector<vector<double> >::iterator i;
	for(i = ys.begin(); i != ys.end(); ++i) {
		copy(i->begin(), i->end(), ostream_iterator<double>(cout, " "));
		cout << endl;
	}
	
	return 0;
}
