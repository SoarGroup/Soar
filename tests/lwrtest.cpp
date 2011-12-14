#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <utility>
#include <iterator>
#include <sstream>
#include "lwr.h"
#include "common.h"

using namespace std;

void parsefloats(const string &s, vector<float> &v) {
	stringstream ss(s);
	float f;
	int c = 1;
	while (!ss.eof()) {
		if (!(ss >> f)) {
			if (ss.eof()) {
				break;
			}
			cerr << "error parsing field " << c << " in " << ss.str() << endl;
			exit(1);
		}

		v.push_back(f);
		c++;
	}
}

void parseline(const string &line, floatvec &x, floatvec &y, float &dt) {
	vector<string> parts;
	vector<float> x1, y1;
	
	split(line, ";", parts);
	assert(parts.size() == 3);
	parsefloats(parts[0], x1);
	parsefloats(parts[1], y1);
	stringstream ss(parts[2]);
	if (!(ss >> dt)) {
		cerr << "error parsing " << parts[2] << endl;
		exit(1);
	}
	x = x1;
	y = y1;
}

int main(int argc, char *argv[]) {
	string line;
	int nnbrs;
	lwr *m = NULL;
	vector<double> diffs;
	
	if (argc != 5) {
		cerr << "usage: lwrtest <method> <num neighbors> <training file> <test file>" << endl;
		exit(1);
	}
	
	stringstream ss(argv[2]);
	if (!(ss >> nnbrs)) {
		cerr << "nnbrs parse error" << endl;
		exit(1);
	}
	
	ifstream trainf(argv[3]);
	if (!trainf) {
		cerr << "couldn't open " << argv[3] << endl;
		exit(1);
	}
	
	while (getline(trainf, line)) {
		floatvec x, y;
		float dt;
		parseline(line, x, y, dt);
		if (!m) {
			m = new lwr(x.size(), y.size(), nnbrs);
		}
		for(int i = 0; i < y.size(); ++i) {
			y[i] = (y[i] - x[i]) / dt;
		}
		m->add(x, y);
	}
	
	trainf.close();
	
	ifstream testf(argv[4]);
	if (!testf) {
		cerr << "couldn't open " << argv[4] << endl;
		exit(1);
	}
	
	while (getline(testf, line)) {
		floatvec x, y, dy, py;
		float dt;
		parseline(line, x, y, dt);
		dy.resize(y.size());
		py.resize(y.size());
		m->predict(x, dy, argv[1][0], false);
		for(int i = 0; i < py.size(); ++i) {
			py[i] = x[i] + dy[i] * dt;
		}
		cout << "predict: " << py << endl;
		cout << "actual:  " << y << endl;
		diffs.push_back(sqrt(py.distsq(y)));
	}
	
	if (diffs.size() == 0) {
		cout << "No predictions" << endl;
		return 0;
	}
	
	double mean = 0.0, std = 0.0;
	for (int i = 0; i < diffs.size(); ++i) {
		mean += diffs[i];
	}
	mean /= diffs.size();
	
	for (int i = 0; i < diffs.size(); ++i) {
		std += pow(diffs[i] - mean, 2);
	}
	std = sqrt(std / diffs.size());
	
	cout << "MEAN: " << mean << endl;
	cout << "STD:  " << std << endl;
	return 0;
}
