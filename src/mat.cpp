#include <iostream>
#include "mat.h"
#include "common.h"
#include "params.h"
#include "serialize.h"

using namespace std;
dyn_mat::dyn_mat() : buf(0, 0), r(0), c(0), released(false) {}

dyn_mat::dyn_mat(int nrows, int ncols) 
: buf(nrows, ncols), r(nrows), c(ncols), released(false) {}

dyn_mat::dyn_mat(int nrows, int ncols, int init_row_capacity, int init_col_capacity) 
: buf(init_row_capacity, init_col_capacity), r(nrows), c(ncols), released(false) {}

dyn_mat::dyn_mat(const_mat_view m)
: r(m.rows()), c(m.cols()), released(false)
{
	buf = m;
}

dyn_mat::dyn_mat(const dyn_mat &other) 
: buf(other.buf), r(other.r), c(other.c), released(false) 
{
	assert(!other.released);
}

void dyn_mat::resize(int nrows, int ncols) {
	assert(!released);
	r = nrows;
	c = ncols;
	if (r > buf.rows() && c > buf.cols()) {
		buf.conservativeResize(r, c);
	} else if (r > buf.rows()) {
		buf.conservativeResize(r, Eigen::NoChange);
	} else if (c > buf.cols()) {
		buf.conservativeResize(Eigen::NoChange, c);
	}
}

void dyn_mat::append_row() {
	assert(!released);
	if (r >= buf.rows()) {
		buf.conservativeResize(r == 0 ? 1 : r * 2, Eigen::NoChange);
	}
	++r;
}

void dyn_mat::append_row(const rvec &row) {
	assert(!released && row.size() == c);
	append_row();
	buf.block(r - 1, 0, 1, c) = row;
}

void dyn_mat::insert_row(int i) {
	assert(!released && 0 <= i && i <= r);
	if (r >= buf.rows()) {
		buf.conservativeResize(r == 0 ? 1 : r * 2, Eigen::NoChange);
	}
	for (int j = r; j > i; --j) {
		buf.block(j, 0, 1, c) = buf.block(j - 1, 0, 1, c);
	}
	++r;
}

void dyn_mat::insert_row(int i, const rvec &row) {
	assert(!released && row.size() == c);
	insert_row(i);
	buf.block(i, 0, 1, c) = row;
}

void dyn_mat::remove_row(int i) {
	assert(!released && 0 <= i && i < r);
	for (int j = i + 1; j < r; ++j) {
		buf.block(j - 1, 0, 1, c) = buf.block(j, 0, 1, c);
	}
	--r;
}

void dyn_mat::append_col() {
	assert(!released);
	if (c >= buf.cols()) {
		buf.conservativeResize(Eigen::NoChange, c == 0 ? 1 : c * 2);
	}
	++c;
}

void dyn_mat::append_col(const cvec &col) {
	assert(!released && col.size() == r);
	append_col();
	buf.block(0, c - 1, r, 1) = col;
}

void dyn_mat::insert_col(int i) {
	assert(!released && 0 <= i && i <= c);
	if (c >= buf.cols()) {
		buf.conservativeResize(Eigen::NoChange, c == 0 ? 1 : c * 2);
	}
	for (int j = c; j > i; --j) {
		buf.block(0, j, r, 1) = buf.block(0, j - 1, r, 1);
	}
	++c;
}

void dyn_mat::insert_col(int i, const cvec &col) {
	assert(!released && col.size() == r);
	insert_col(i);
	buf.block(0, i, r, 1) = col;
}

void dyn_mat::remove_col(int i) {
	assert(!released && 0 <= i && i < c);
	for (int j = i + 1; j < c; ++j) {
		buf.block(0, j - 1, r, 1) = buf.block(0, j, r, 1);
	}
	--c;
}

void dyn_mat::serialize(ostream &os) const {
	assert(!released);
	::serialize(const_mat_view(buf.topLeftCorner(r, c)), os);
}

void dyn_mat::unserialize(istream &is) {
	assert(!released);
	::unserialize(buf, is);
	r = buf.rows();
	c = buf.cols();
}

ostream& output_rvec(ostream &os, const rvec &v, const string &sep) {
	int n = v.size();
	if (n == 0) return os;
	
	for (int i = 0; i < n - 1; ++i) {
		os << v(i) << sep;
	}
	os << v(n - 1);
	return os;
}

ostream& output_cvec(ostream &os, const cvec &v, const string &sep) {
	int n = v.size();
	if (n == 0) return os;
	
	for (int i = 0; i < n - 1; ++i) {
		os << v(i) << sep;
	}
	os << v(n - 1);
	return os;
}

ostream& output_mat(ostream &os, const_mat_view m) {
	int r = m.rows(), c = m.cols();
	if (r == 0 || c == 0) return os;
	
	for (int i = 0; i < r; ++i) {
		for (int j = 0; j < c - 1; ++j) {
			os << m(i, j) << " ";
		}
		os << m(i, c - 1) << endl;
	}
	return os;
}

bool normal(const_mat_view m) {
	for (int i = 0; i < m.rows(); ++i) {
		for (int j = 0; j < m.cols(); ++j) {
			if (isnan(m(i, j)) || isinf(m(i, j))) {
				return false;
			}
		}
	}
	return true;
}

bool uniform(const_mat_view m) {
	if (m.rows() == 0 || m.cols() == 0) {
		return true;
	}
	return m.isConstant(m(0, 0), SAME_THRESH);
}

void get_nonuniform_cols(const_mat_view X, int ncols, vector<int> &cols) {
	for (int i = 0; i < ncols; ++i) {
		if (!uniform(X.col(i))) {
			cols.push_back(i);
		}
	}
}

void del_uniform_cols(mat_view X, int ncols, vector<int> &cols) {
	get_nonuniform_cols(X, ncols, cols);
	pick_cols(X, cols);
}

void pick_cols(const_mat_view X, const vector<int> &cols, mat &result) {
	result.resize(X.rows(), cols.size());
	for (int i = 0; i < cols.size(); ++i) {
		result.col(i) = X.col(cols[i]);
	}
}

void pick_rows(const_mat_view X, const vector<int> &rows, mat &result) {
	if (result.rows() < X.rows() || result.cols() != X.cols()) {
		result.resize(rows.size(), X.cols());
	}
	for(int i = 0; i < rows.size(); ++i) {
		result.row(i) = X.row(rows[i]);
	}
}

void pick_cols(mat_view X, const vector<int> &cols) {
	assert(X.cols() >= cols.size());
	bool need_copy = false;
	for (int i = 0; i < cols.size(); ++i) {
		if (cols[i] < i) {
			need_copy = true;
			break;
		}
	}
	if (need_copy) {
		mat c = X;
		for (int i = 0; i < cols.size(); ++i) {
			X.col(i) = c.col(cols[i]);
		}
	} else {
		for (int i = 0; i < cols.size(); ++i) {
			X.col(i) = X.col(cols[i]);
		}
	}
}

void pick_rows(mat_view X, const vector<int> &rows) {
	assert(X.rows() >= rows.size());
	bool need_copy = false;
	for (int i = 0; i < rows.size(); ++i) {
		if (rows[i] < i) {
			need_copy = true;
			break;
		}
	}
	if (need_copy) {
		mat c = X;
		for (int i = 0; i < rows.size(); ++i) {
			X.row(i) = c.row(rows[i]);
		}
	} else {
		for (int i = 0; i < rows.size(); ++i) {
			if (rows[i] > i) {
				X.row(i) = X.row(rows[i]);
			}
		}
	}
}

void randomize_vec(rvec &v, const rvec &min, const rvec &max) {
	//v = min.array() + (rvec::Random(v.size()).array() * (max - min).array());
	// apparently rvec::Random will generate numbers outside of [0, 1]
	for (int i = 0; i < v.size(); ++i) {
		v(i) = min(i) + (rand() / (double) RAND_MAX) * (max(i) - min(i));
	}
}

vec3 project(const vec3 &v, const vec3 &u) {
	double m = u.squaredNorm();
	if (m == 0.) {
		return vec3::Zero();
	}
	return u * (v.dot(u) / m);
}

double dir_separation(const ptlist &a, const ptlist &b, const vec3 &u) {
	int counter = 0;
	ptlist::const_iterator i;
	vec3 p;
	double x, min = numeric_limits<double>::max(), max = -numeric_limits<double>::max();
	for (i = a.begin(); i != a.end(); ++i) {
		p = project(*i, u);
		x = p[0] / u[0];
		if (x < min) {
			min = x;
		}
	}
	for (i = b.begin(); i != b.end(); ++i) {
		p = project(*i, u);
		x = p[0] / u[0];
		if (x > max) {
			max = x;
		}
	}
	
	return max - min;
}

ostream& operator<<(ostream &os, const bbox &b) {
	os << b.min[0] << " " << b.min[1] << " " << b.min[2] << " " << b.max[0] << " " << b.max[1] << " " << b.max[2];
	return os;
}

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
