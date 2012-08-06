#include <iostream>
#include "mat.h"
#include "common.h"
#include "params.h"

using namespace std;
dyn_mat::dyn_mat() : buf(0, 0), r(0), c(0) {}

dyn_mat::dyn_mat(int nrows, int ncols) 
: buf(nrows, ncols), r(nrows), c(ncols) {}

dyn_mat::dyn_mat(int nrows, int ncols, int init_row_capacity, int init_col_capacity) 
: buf(init_row_capacity, init_col_capacity), r(nrows), c(ncols) {}

dyn_mat::dyn_mat(const dyn_mat &other) 
: buf(other.buf), r(other.r), c(other.c) {}

void dyn_mat::resize(int nrows, int ncols) {
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
	if (r >= buf.rows()) {
		buf.conservativeResize(r == 0 ? 1 : r * 2, Eigen::NoChange);
	}
	++r;
}

void dyn_mat::append_row(const rvec &row) {
	assert(row.size() == c);
	append_row();
	buf.block(r - 1, 0, 1, c) = row;
}

void dyn_mat::insert_row(int i) {
	assert(0 <= i && i <= r);
	if (r >= buf.rows()) {
		buf.conservativeResize(r == 0 ? 1 : r * 2, Eigen::NoChange);
	}
	for (int j = r; j > i; --j) {
		buf.block(j, 0, 1, c) = buf.block(j - 1, 0, 1, c);
	}
	++r;
}

void dyn_mat::insert_row(int i, const rvec &row) {
	assert(row.size() == c);
	insert_row(i);
	buf.block(i, 0, 1, c) = row;
}

void dyn_mat::remove_row(int i) {
	assert(0 <= i && i < r);
	for (int j = i + 1; j < r; ++j) {
		buf.block(j - 1, 0, 1, c) = buf.block(j, 0, 1, c);
	}
	--r;
}

void dyn_mat::append_col() {
	if (c >= buf.cols()) {
		buf.conservativeResize(Eigen::NoChange, c == 0 ? 1 : c * 2);
	}
	++c;
}

void dyn_mat::append_col(const cvec &col) {
	assert(col.size() == r);
	append_col();
	buf.block(0, c - 1, r, 1) = col;
}

void dyn_mat::insert_col(int i) {
	assert(0 <= i && i <= c);
	if (c >= buf.cols()) {
		buf.conservativeResize(Eigen::NoChange, c == 0 ? 1 : c * 2);
	}
	for (int j = c; j > i; --j) {
		buf.block(0, j, r, 1) = buf.block(0, j - 1, r, 1);
	}
	++c;
}

void dyn_mat::insert_col(int i, const cvec &col) {
	assert(col.size() == r);
	insert_col(i);
	buf.block(0, i, r, 1) = col;
}

void dyn_mat::remove_col(int i) {
	assert(0 <= i && i < c);
	for (int j = i + 1; j < c; ++j) {
		buf.block(0, j - 1, r, 1) = buf.block(0, j, r, 1);
	}
	--c;
}

void dyn_mat::save(ostream &os) const {
	save_mat(os, buf.topLeftCorner(r, c));
}

void dyn_mat::load(istream &is) {
	load_mat(is, buf);
	r = buf.rows();
	c = buf.cols();
}

void save_mat(ostream &os, const_mat_view m) {
	os << "begin_mat " << m.rows() << " " << m.cols() << endl;
	os << m << endl << "end_mat" << endl;
}

void load_mat(istream &is, mat &m) {
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

void save_imat(ostream &os, const imat &m) {
	os << "begin_mat " << m.rows() << " " << m.cols() << endl;
	os << m << endl << "end_mat" << endl;
}

void load_imat(istream &is, imat &m) {
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

void save_rvec(ostream &os, const rvec &v) {
	os << "begin_vec " << v.size() << endl;
	os << v << endl;
	os << "end_vec" << endl;
}

void load_rvec(istream &is, rvec &v) {
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

void save_cvec(ostream &os, const cvec &v) {
	save_rvec(os, v.transpose());
}

void load_cvec(istream &is, cvec &v) {
	rvec v1;
	load_rvec(is, v1);
	v = v1.transpose();
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

bool is_normal(const_mat_view m) {
	for (int i = 0; i < m.rows(); ++i) {
		for (int j = 0; j < m.cols(); ++j) {
			if (isnan(m(i, j)) || isinf(m(i, j))) {
				return false;
			}
		}
	}
	return true;
}

void get_nonstatic_cols(const_mat_view X, int ncols, vector<int> &nonstatic_cols) {
	for (int i = 0; i < ncols; ++i) {
		cvec c = X.col(i).array().abs();
		if (c.maxCoeff() > c.minCoeff() * SAME_THRESH) {
			nonstatic_cols.push_back(i);
		}
	}
}

void del_static_cols(mat_view X, int ncols, vector<int> &nonstatic_cols) {
	get_nonstatic_cols(X, ncols, nonstatic_cols);
	pick_cols(X, nonstatic_cols);
}

void pick_cols(const_mat_view X, const vector<int> &cols, mat &result) {
	result.resize(X.rows(), cols.size());
	for (int i = 0; i < cols.size(); ++i) {
		result.col(i) = X.col(cols[i]);
	}
}

void pick_rows(const_mat_view X, const vector<int> &rows, mat &result) {
	result.resize(rows.size(), X.cols());
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
			X.row(i) = X.row(rows[i]);
		}
	}
}
