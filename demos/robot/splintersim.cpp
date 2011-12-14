#include <iostream>
#include <sstream>
#include "linalg.h"
#include "splinter.h"

using namespace std;

int main(int argc, char *argv[]) {
	string line, outdim;
	double lrps = 0.0, rrps = 0.0, lvolt = 0.0, rvolt = 0.0;
	vec3 pos, vel, rot, rotrate;
	
	cout << "a splinter world v -1 -1 0 1 -1 0 1 1 0 -1 1 0" << endl << "***" << endl;
	
	while (true) {
		if (!getline(cin, line)) {
			return 0;
		}
		line.erase(line.find_last_not_of(" \t\n")+1);
		if (line.size() == 0) {
			continue;
		}
		if (line == "***") {
			splinter_update(lvolt, rvolt, lrps, rrps, pos, vel, rot, rotrate);
			cerr << "OUT: " << lvolt << " " << rvolt << endl;
			cerr << pos << " , " << rot << " , " << lrps << " , " << rrps << " , " << rotrate << " , " << vel << endl;
			cout << "c splinter p " << pos << " r " << rot << endl;
			cout << "p left_rads_per_sec " << lrps << endl;
			cout << "p right_rads_per_sec " << rrps << endl;
			for (int i = 0; i < 3; ++i) {
				cout << "p vel_" << i << " " << vel[i] << endl;
				cout << "p rotation_rate_" << i << " " << rotrate[i] << endl;
			}
			cout << "***" << endl;
			continue;
		}
		
		stringstream ss(line);
		ss >> outdim;
		if (outdim == "left") {
			ss >> lvolt;
		} else if (outdim == "right") {
			ss >> rvolt;
		}
	}
}
