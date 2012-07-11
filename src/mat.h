#ifndef MAT_H
#define MAT_H

#include <iostream>
#include <vector>

/*
 By default Eigen will try to align all fixed size vectors to 128-bit
 boundaries to enable SIMD instructions on hardware such as SSE. However,
 this requires that you modify every class that has such vectors as
 members so that they are correctly allocated. This seems like more
 trouble than it's worth at the moment, so I'm disabling it.
*/
#define EIGEN_DONT_ALIGN
#include <Eigen/Dense>
//#include <Eigen/src/plugins/BlockMethods.h>
#include <Eigen/StdVector>

typedef Eigen::RowVector3d vec3;
typedef std::vector<vec3> ptlist;

typedef Eigen::RowVectorXd rvec;
typedef Eigen::VectorXd cvec;
typedef Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> mat;
typedef Eigen::Matrix<int, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> imat;

/*
 Don't try to understand what these mean, just know that you
 can treat them like regular matrices.
*/

typedef Eigen::Block<      mat, Eigen::Dynamic, Eigen::Dynamic, true,  true> mat_iblock; // i means inner_panel template arg = true
typedef Eigen::Block<      mat, Eigen::Dynamic, Eigen::Dynamic, false, true> mat_block;
typedef Eigen::Block<const mat, Eigen::Dynamic, Eigen::Dynamic, true,  true> const_mat_iblock;
typedef Eigen::Block<const mat, Eigen::Dynamic, Eigen::Dynamic, false, true> const_mat_block;

typedef Eigen::Stride<Eigen::Dynamic, 1> mat_stride;
typedef Eigen::Map<mat, Eigen::Unaligned, mat_stride> mat_map;
typedef Eigen::Map<const mat, Eigen::Unaligned, mat_stride> const_mat_map;

/*
 If you define a function argument as "const mat &" and a block or a map
 is passed into it, Eigen will implicitly create a copy of the matrix
 underlying the block or map.  Using mat_views avoids this problem,
 because they have the appropriate constructors for handling any type
 of object.
*/
class mat_view : public mat_map {
public:
	mat_view(mat &m)               : mat_map(m.data(), m.rows(), m.cols(), mat_stride(m.rowStride(), 1)) {}
	mat_view(mat &m, int r, int c) : mat_map(m.data(), r, c, mat_stride(m.rowStride(), 1)) {}
	mat_view(const mat_view &m)    : mat_map(m) {}
};

class const_mat_view : public const_mat_map {
public:
	const_mat_view(const mat &m)               : const_mat_map(m.data(), m.rows(), m.cols(), mat_stride(m.rowStride(), 1)) {}
	const_mat_view(const rvec &v)              : const_mat_map(v.data(), 1, v.size(), mat_stride(1, 1)) {}
	const_mat_view(const cvec &v)              : const_mat_map(v.data(), v.size(), 1, mat_stride(1, 1)) {}
	const_mat_view(const mat &m, int r, int c) : const_mat_map(m.data(), r, c, mat_stride(m.rowStride(), 1)) {}
	const_mat_view(const mat_block &b)         : const_mat_map(b.data(), b.rows(), b.cols(), mat_stride(b.rowStride(), 1)) {}
	const_mat_view(const mat_iblock &b)        : const_mat_map(b.data(), b.rows(), b.cols(), mat_stride(b.rowStride(), 1)) {}
	const_mat_view(const const_mat_block &b)   : const_mat_map(b.data(), b.rows(), b.cols(), mat_stride(b.rowStride(), 1)) {}
	const_mat_view(const const_mat_iblock &b)  : const_mat_map(b.data(), b.rows(), b.cols(), mat_stride(b.rowStride(), 1)) {}
	const_mat_view(const const_mat_map &m)     : const_mat_map(m) {}
	const_mat_view(const mat_map &m)           : const_mat_map(m.data(), m.rows(), m.cols(), mat_stride(m.rowStride(), 1)) {}
};


/*
 A matrix that can be efficiently dynamically resized. Uses a doubling
 memory allocation policy.
*/
class dyn_mat {
public:
	dyn_mat();
	dyn_mat(int nrows, int ncols);
	dyn_mat(int nrows, int ncols, int init_row_capacity, int init_col_capacity);
	dyn_mat(const dyn_mat &other);
	
	void resize(int nrows, int ncols);
	void append_row();
	void append_row(const rvec &row);
	void insert_row(int i);
	void insert_row(int i, const rvec &row);
	void remove_row(int i);
	void append_col();
	void append_col(const cvec &col);
	void insert_col(int i);
	void insert_col(int i, const cvec &col);
	void remove_col(int i);
	
	void save(std::ostream &os) const;
	void load(std::istream &is);
	
	double &operator()(int i, int j) {
		assert(0 <= i && i < r && 0 <= j && j < c);
		return buf(i, j);
	}
	
	double operator()(int i, int j) const {
		assert(0 <= i && i < r && 0 <= j && j < c);
		return buf(i, j);
	}
	
	mat::RowXpr row(int i) {
		assert(0 <= i && i < r);
		return buf.row(i);
	}
	
	mat::ConstRowXpr row(int i) const {
		assert(0 <= i && i < r);
		return buf.row(i);
	}
	
	mat::ColXpr col(int j) {
		assert(0 <= j && j < c);
		return buf.col(j);
	}
	
	mat::ConstColXpr col(int j) const {
		assert(0 <= j && j < c);
		return buf.col(j);
	}
	
	inline mat_view get() {
		return mat_view(buf, r, c);
	}
	
	inline const_mat_view get() const {
		return const_mat_view(buf, r, c);
	}
	
	inline int rows() const {
		return r;
	}
	
	inline int cols() const {
		return c;
	}
	
public:
	mat buf;
	int r, c;
};

void save_mat (std::ostream &os, const_mat_view m);
void load_mat (std::istream &is, mat &m);
void save_imat(std::ostream &os, const imat &m);
void load_imat(std::istream &is, imat &m);
void save_rvec(std::ostream &os, const rvec &v);
void load_rvec(std::istream &is, rvec &v);
void save_cvec(std::ostream &os, const cvec &v);
void load_cvec(std::istream &is, cvec &v);

std::ostream& output_rvec(std::ostream &os, const rvec &v, const std::string &sep = " ");
std::ostream& output_cvec(std::ostream &os, const cvec &v, const std::string &sep = " ");
std::ostream& output_mat(std::ostream &os, const const_mat_view m);

bool is_normal(const_mat_view m);

/*
 Return indices of columns that have significantly different values,
 meaning the maximum absolute value of the column is greater than
 SAME_THRESH times the minimum absolute value.
*
void get_nonstatic_cols(const_mat_view X, int ncols, std::vector<int> &nonstatic_cols);

/*
 Remove the static columns from the first 'cols' columns of X. This
 will not resize the matrix.
*/
void del_static_cols(mat_view X, int cols, std::vector<int> &nonstatic);

void pick_cols(const_mat_view X, const std::vector<int> &cols, mat &result);
void pick_rows(const_mat_view X, const std::vector<int> &rows, mat &result);
void pick_cols(mat_view X, const std::vector<int> &cols);
void pick_cols(mat_view X, const std::vector<int> &rows);

#endif
