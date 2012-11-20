#include "serialize.h"
#include "common.h"

using namespace std;

void serialize(const_mat_view m, ostream &os) {
	os << "begin_mat " << m.rows() << " " << m.cols() << endl;
	ios_base::fmtflags flags = os.flags();
	os << scientific;
	int p = os.precision();
	os.precision(PRECISION);
	for (int i = 0; i < m.rows(); ++i) {
		for (int j = 0; j < m.cols(); ++j) {
			os << m(i, j) << " ";
		}
		os << endl;
	}
	os << "end_mat" << endl;
	os.precision(p);
	os.flags(flags);
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

void unserialize(rvec &v, istream &is) {
	mat m;
	unserialize(m, is);
	assert(m.rows() == 1);
	v = m.row(0);
}

void unserialize(cvec &v, istream &is) {
	mat m;
	unserialize(m, is);
	assert(m.cols() == 1);
	v = m.col(0);
}
