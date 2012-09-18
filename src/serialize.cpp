#include "serialize.h"
#include "common.h"

using namespace std;

void serialize(const_mat_view m, ostream &os) {
	os << "begin_mat " << m.rows() << " " << m.cols() << endl;
	os << m << endl << "end_mat" << endl;
}

void unserialize(mat &m, istream &is) {
	string token;
	int nrows, ncols;
	
	is >> token;
	assert(token == "begin_mat");
	is >> token;
	if (!parse_int(token, nrows)) assert(false);
	is >> token;
	if (!parse_int(token, ncols)) assert(false);
	
	m.resize(nrows, ncols);
	for (int i = 0; i < nrows; ++i) {
		for (int j = 0; j < ncols; ++j) {
			is >> token;
			if (!parse_double(token, m(i, j))) assert(false);
		}
	}
	is >> token;
	assert(token == "end_mat");
}

void serialize(const imat &m, ostream &os) {
	os << "begin_mat " << m.rows() << " " << m.cols() << endl;
	os << m << endl << "end_mat" << endl;
}

void unserialize(imat &m, istream &is) {
	string token;
	int nrows, ncols;
	
	is >> token;
	assert(token == "begin_mat");
	is >> token;
	if (!parse_int(token, nrows)) assert(false);
	is >> token;
	if (!parse_int(token, ncols)) assert(false);
	
	m.resize(nrows, ncols);
	for (int i = 0; i < nrows; ++i) {
		for (int j = 0; j < ncols; ++j) {
			is >> token;
			if (!parse_int(token, m(i, j))) assert(false);
		}
	}
	is >> token;
	assert(token == "end_mat");
}

void serialize(const rvec &v, ostream &os) {
	os << "begin_vec " << v.size() << endl;
	os << v << endl;
	os << "end_vec" << endl;
}

void unserialize(rvec &v, istream &is) {
	string token;
	int n;
	
	is >> token;
	assert(token == "begin_vec");
	is >> token;
	if (!parse_int(token, n)) assert(false);
	
	v.resize(n);
	for (int i = 0; i < n; ++i) {
		is >> token;
		if (!parse_double(token, v(i))) assert(false);
	}
	is >> token;
	assert(token == "end_vec");
}

void serialize(const cvec &v, ostream &os) {
	::serialize(rvec(v.transpose()), os);
}

void unserialize(cvec &v, istream &is) {
	rvec v1;
	::unserialize(v1, is);
	v = v1.transpose();
}
