#include <iostream>
#include <iomanip>
#include <sstream>
#include <cassert>
#include <vector>
#include <map>
#include "common.h"
#include "foil.h"
#include "serialize.h"
#include "linear.h"
#include "mat.h"
#include "lda.h"

using namespace std;

void read_data(const char *path, mat &X, vector<int> &classes);

int main(int argc, char *argv[]) {
	string line;
	vector<string> fields;
	vector<int> train_classes, test_classes;
	mat Xtrain, Xtest;
	
	if (argc != 3) {
		cerr << "specify files" << endl;
		exit(1);
	}
	
	read_data(argv[1], Xtrain, train_classes);
	read_data(argv[2], Xtest, test_classes);
	
	LDA lda;
	lda.learn(Xtrain, train_classes);
	
	rvec p;
	
	cout << "Projected Training: " << endl;
	for (int i = 0; i < Xtrain.rows(); ++i) {
		if (lda.project(Xtrain.row(i), p)) {
			output_rvec(cout, p, " ") << " " << train_classes[i] << endl;;
		} else {
			cout << "projection failed" << endl;
		}
	}
	
	int correct = 0;
	cout << endl << endl << "Projected Testing: " << endl;
	for (int i = 0; i < Xtest.rows(); ++i) {
		int pred = lda.classify(Xtest.row(i));
		if (lda.project(Xtest.row(i), p)) {
			output_rvec(cout, p, " ") << " " << pred << " " << test_classes[i] << endl;
		} else {
			cout << "projection failed" << endl;
		}
		if (pred == test_classes[i])
			++correct;
	}
	
	cout << correct << " out of " << Xtest.rows() << endl;
}

void read_data(const char *path, mat &X, vector<int> &classes) {
	string line;
	vector<string> fields;
	vector<vector<double> > data;
	double x;
	
	ifstream input(path);
	
	assert(input);
	
	while(getline(input, line)) {
		fields.clear();
		split(line, " ", fields);
		if (fields.empty()) {
			continue;
		}
		
		grow(data);
		for (int i = 0; i < fields.size(); ++i) {
			if (!parse_double(fields[i], x)) {
				cerr << "non number " << fields[i] << endl;;
				exit(1);
			}
			data.back().push_back(x);
		}
	}
	X.resize(data.size(), data[0].size() - 1);
	for (int i = 0; i < data.size(); ++i) {
		assert(data[i].size() - 1 == X.cols());
		for (int j = 0; j < data[i].size() - 1; ++j) {
			X(i, j) = data[i][j];
		}
		classes.push_back(static_cast<int>(data[i].back()));
	}
}
