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
void run_print(int first, int argc, char *argv[]);
void run_test_set(int first, int argc, char *argv[]);
void run_cross_validation(int first, int argc, char *argv[]);

bool input_serialized = false;

int main(int argc, char *argv[]) {
	int i = 1;
	
	if (argc < 2) {
		cerr << "specify operation: print, test_set, cv" << endl;
		exit(1);
	}
	
	while (i < argc && argv[i][0] == '-') {
		switch (argv[i][1]) {
			case 's':
				input_serialized = true;
				break;
			default:
				cerr << "unrecognized option " << argv[i] << endl;
				exit(1);
				break;
		}
		++i;
	}
	
	if (strcmp(argv[i], "print") == 0) {
		run_print(++i, argc, argv);
	} else if (strcmp(argv[i], "test_set") == 0) {
		run_test_set(++i, argc, argv);
	} else if (strcmp(argv[i], "cv") == 0) {
		run_cross_validation(++i, argc, argv);
	} else {
		cerr << "no such operation" << endl;
		return 1;
	}
	
	return 0;
}

void read_data(const char *path, mat &X, vector<int> &classes) {
	string line;
	vector<string> fields;
	vector<vector<double> > data;
	double x;
	
	ifstream input(path);
	
	assert(input);
	
	if (input_serialized) {
		unserialize(X, input);
		unserialize(classes, input);
		return;
	}
	
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

void run_print(int first, int argc, char *argv[]) {
	mat data;
	vector<int> classes;
	LDA lda;
	
	if (first >= argc) {
		cerr << "specify training file" << endl;
		exit(1);
	}
	
	read_data(argv[first], data, classes);
	lda.learn(data, classes);
	lda.inspect(cout);
}

void run_test_set(int first, int argc, char *argv[]) {
	string line;
	vector<string> fields;
	vector<int> train_classes, test_classes;
	mat Xtrain, Xtest;
	
	if (first + 1 >= argc) {
		cerr << "specify training and test files" << endl;
		exit(1);
	}
	
	read_data(argv[first], Xtrain, train_classes);
	read_data(argv[first+1], Xtest, test_classes);
	
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

void run_cross_validation(int first, int argc, char *argv[]) {
	mat data, train;
	vector<int> classes, train_classes, reorder;
	int n, k, chunksize, extra, start, ndata, ncols, correct;
	
	if (first >= argc) {
		cerr << "specify training file" << endl;
		exit(1);
	}
	
	read_data(argv[first], data, classes);
	ndata = data.rows();
	ncols = data.cols();
	
	if (first + 1 < argc) {
		if (!parse_int(argv[first+1], n)) {
			cerr << "invalid n" << endl;
			exit(1);
		}
	} else {
		n = 10;
	}
	
	reorder.resize(ndata);
	for (int i = 0, iend = ndata; i < iend; ++i) {
		reorder[i] = i;
	}
	random_shuffle(reorder.begin(), reorder.end());
	chunksize = data.rows() / n;
	extra = data.rows() - chunksize * n;
	correct = 0;
	start = 0;
	for (int i = 0; i < n; ++i) {
		if (i < extra) {
			k = chunksize + 1;
		} else {
			k = chunksize;
		}
		train.resize(ndata - k, ncols);
		train_classes.resize(ndata - k);
		
		for (int j = 0; j < ndata - k; ++j) {
			if (j < start) {
				train.row(j) = data.row(reorder[j]);
				train_classes[j] = classes[reorder[j]];
			} else {
				train.row(j) = data.row(reorder[j + k]);
				train_classes[j] = classes[reorder[j + k]];
			}
		}

		LDA lda;
		lda.learn(train, train_classes);
		
		for (int j = 0; j < k; ++j) {
			int a = lda.classify(data.row(reorder[start + j]));
			if (a == classes[reorder[start + j]]) {
				correct++;
			}
		}
		start += k;
	}
	
	cout << correct << " correct out of " << ndata << endl;
}
