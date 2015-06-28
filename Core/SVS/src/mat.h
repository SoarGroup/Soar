#ifndef MAT_H
#define MAT_H

#include <iostream>
#include <vector>
#include "serializable.h"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wshorten-64-to-32"
#elif _MSC_VER
#pragma warning(push, 0)
#endif

#include <Eigen/Dense>
#include <Eigen/StdVector>

#ifdef __clang__
#pragma clang diagnostic pop
#elif _MSC_VER
#pragma warning(pop)
#endif

typedef Eigen::Vector3d vec3;
typedef Eigen::Vector4d vec4;
typedef std::vector<vec3> ptlist;

typedef Eigen::RowVectorXd rvec;
typedef Eigen::VectorXd cvec;
typedef Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> mat;

/*
 Don't try to understand what these mean, just know that you
 can treat them like regular matrices.
*/

typedef Eigen::Block<      mat, Eigen::Dynamic, Eigen::Dynamic, true> mat_iblock; // i means inner_panel template arg = true
typedef Eigen::Block<      mat, Eigen::Dynamic, Eigen::Dynamic, false> mat_block;
typedef Eigen::Block<      mat, 1,              Eigen::Dynamic, false> row_block;
typedef Eigen::Block<      mat, Eigen::Dynamic, 1,              false> col_block;
typedef Eigen::Block<const mat, Eigen::Dynamic, Eigen::Dynamic, true> const_mat_iblock;
typedef Eigen::Block<const mat, Eigen::Dynamic, Eigen::Dynamic, false> const_mat_block;
typedef Eigen::Block<const mat, 1,              Eigen::Dynamic, false> const_row_block;
typedef Eigen::Block<const mat, Eigen::Dynamic, 1,              false> const_col_block;

typedef Eigen::Stride<Eigen::Dynamic, 1> mat_stride;
typedef Eigen::Map<mat, Eigen::Unaligned, mat_stride> mat_map;
typedef Eigen::Map<const mat, Eigen::Unaligned, mat_stride> const_mat_map;

typedef Eigen::Block<      mat_map, Eigen::Dynamic, Eigen::Dynamic, false> map_block;
typedef Eigen::Block<const_mat_map, Eigen::Dynamic, Eigen::Dynamic, true> const_map_iblock;
typedef Eigen::Block<const_mat_map, Eigen::Dynamic, Eigen::Dynamic, false> const_map_block;
typedef Eigen::Block<const_mat_map, Eigen::Dynamic, 1,              false> const_map_col_block;
typedef Eigen::Block<const_mat_map, 1,              Eigen::Dynamic, false> const_map_row_block;

/*
 If you define a function argument as "const mat &" and a block or a
 map is passed into it, the compiler will implicitly create a copy of
 the matrix underlying the block or map. Using mat_views avoids this
 problem, because they have the appropriate constructors for handling
 any type of object.
*/
class mat_view : public mat_map
{
    public:
        mat_view(mat& m)               : mat_map(m.data(), m.rows(), m.cols(), mat_stride(m.rowStride(), 1)) {}
        mat_view(mat& m, size_t r, size_t c) : mat_map(m.data(), r, c, mat_stride(m.rowStride(), 1)) {}
        mat_view(const mat_view& m)    : mat_map(m) {}
};

class const_mat_view : public const_mat_map
{
    public:
        const_mat_view(const const_mat_view& m)      : const_mat_map(m) {}
        
        // standard types
        const_mat_view(const mat& m)                 : const_mat_map(m.data(), m.rows(), m.cols(), mat_stride(m.rowStride(), 1)) {}
        const_mat_view(const rvec& v)                : const_mat_map(v.data(), 1, v.size(), mat_stride(1, 1)) {}
        const_mat_view(const cvec& v)                : const_mat_map(v.data(), v.size(), 1, mat_stride(1, 1)) {}
        const_mat_view(const mat& m, size_t r, size_t c)   : const_mat_map(m.data(), r, c, mat_stride(m.rowStride(), 1)) {}
        
        // for things like m.block
        const_mat_view(const mat_block& b)           : const_mat_map(b.data(), b.rows(), b.cols(), mat_stride(b.rowStride(), 1)) {}
        const_mat_view(const mat_iblock& b)          : const_mat_map(b.data(), b.rows(), b.cols(), mat_stride(b.rowStride(), 1)) {}
        const_mat_view(const const_mat_block& b)     : const_mat_map(b.data(), b.rows(), b.cols(), mat_stride(b.rowStride(), 1)) {}
        const_mat_view(const const_mat_iblock& b)    : const_mat_map(b.data(), b.rows(), b.cols(), mat_stride(b.rowStride(), 1)) {}
        
        // for things like m.row and m.col
        const_mat_view(const row_block& b)           : const_mat_map(b.data(), 1,        b.cols(), mat_stride(b.rowStride(), 1)) {}
        const_mat_view(const const_row_block& b)     : const_mat_map(b.data(), 1,        b.cols(), mat_stride(b.rowStride(), 1)) {}
        const_mat_view(const col_block& b)           : const_mat_map(b.data(), b.rows(), 1,        mat_stride(b.rowStride(), 1)) {}
        const_mat_view(const const_col_block& b)     : const_mat_map(b.data(), b.rows(), 1,        mat_stride(b.rowStride(), 1)) {}
        
        const_mat_view(const const_mat_map& m)       : const_mat_map(m) {}
        const_mat_view(const mat_map& m)             : const_mat_map(m.data(), m.rows(), m.cols(), mat_stride(m.rowStride(), 1)) {}
        const_mat_view(const map_block& b)           : const_mat_map(b.data(), b.rows(), b.cols(), mat_stride(b.rowStride(), 1)) {}
        const_mat_view(const const_map_block& b)     : const_mat_map(b.data(), b.rows(), b.cols(), mat_stride(b.rowStride(), 1)) {}
        const_mat_view(const const_map_col_block& b) : const_mat_map(b.data(), b.rows(), 1,        mat_stride(b.rowStride(), 1)) {}
        const_mat_view(const const_map_row_block& b) : const_mat_map(b.data(), 1,        b.cols(), mat_stride(b.rowStride(), 1)) {}
};


/*
 A matrix that can be efficiently dynamically resized. Uses a doubling
 memory allocation policy.
*/
class dyn_mat : public serializable
{
    public:
        dyn_mat();
        dyn_mat(size_t nrows, size_t ncols);
        dyn_mat(size_t nrows, size_t ncols, size_t init_row_capacity, size_t init_col_capacity);
        dyn_mat(const dyn_mat& other);
        dyn_mat(const_mat_view m);
        
        void resize(size_t nrows, size_t ncols);
        void append_row();
        void append_row(const rvec& row);
        void insert_row(size_t i);
        void insert_row(size_t i, const rvec& row);
        void remove_row(size_t i);
        void append_col();
        void append_col(const cvec& col);
        void insert_col(size_t i);
        void insert_col(size_t i, const cvec& col);
        void remove_col(size_t i);
        
        void serialize(std::ostream& os) const;
        void unserialize(std::istream& is);
        
        double& operator()(size_t i, size_t j)
        {
            assert(!released && 0 <= i && i < r && 0 <= j && j < c);
            return buf(i, j);
        }
        
        double operator()(size_t i, size_t j) const
        {
            assert(!released && 0 <= i && i < r && 0 <= j && j < c);
            return buf(i, j);
        }
        
        mat::RowXpr row(size_t i)
        {
            assert(!released && 0 <= i && i < r);
            return buf.row(i);
        }
        
        mat::ConstRowXpr row(size_t i) const
        {
            assert(!released && 0 <= i && i < r);
            return buf.row(i);
        }
        
        mat::ColXpr col(size_t j)
        {
            assert(!released && 0 <= j && j < c);
            return buf.col(j);
        }
        
        mat::ConstColXpr col(size_t j) const
        {
            assert(!released && 0 <= j && j < c);
            return buf.col(j);
        }
        
        mat_view get()
        {
            assert(!released);
            return mat_view(buf, r, c);
        }
        
        const_mat_view get() const
        {
            assert(!released);
            return const_mat_view(buf, r, c);
        }
        
        inline size_t rows() const
        {
            assert(!released);
            return r;
        }
        
        inline size_t cols() const
        {
            assert(!released);
            return c;
        }
        
        /*
         The dyn_mat should no longer be used after the internal matrix is
         released. This function is useful for avoiding redundant copying.
        */
        mat& release()
        {
            assert(!released);
            released = true;
            buf.conservativeResize(r, c);
            return buf;
        }
        
    public:
        mat buf;
        size_t r, c;
        bool released;
};

std::ostream& output_rvec(std::ostream& os, const rvec& v, const std::string& sep = " ");
std::ostream& output_cvec(std::ostream& os, const cvec& v, const std::string& sep = " ");
std::ostream& output_mat(std::ostream& os, const const_mat_view m);

bool normal(const_mat_view m);
bool uniform(const_mat_view X);
void randomize_vec(rvec& v, const rvec& min, const rvec& max);
void randomize_vec(vec3& v, const vec3& min, const vec3& max);

/*
 Return indices of columns that have significantly different values,
 meaning the maximum absolute value of the column is greater than
 SAME_THRESH times the minimum absolute value.
*/
void get_nonuniform_cols(const_mat_view X, int ncols, std::vector<int>& cols);

/*
 Remove the static columns from the first 'ncols' columns of X. This
 will not resize the matrix. Upon completion, the cols vector will
 contain the original column indexes that were not removed.
*/
void del_uniform_cols(mat_view X, int ncols, std::vector<int>& cols);

void pick_cols(const_mat_view X, const std::vector<int>& cols, mat& result);
void pick_rows(const_mat_view X, const std::vector<int>& rows, mat& result);
void pick_cols(mat_view X, const std::vector<int>& cols);
void pick_rows(mat_view X, const std::vector<int>& rows);

/*
 Calculate the maximum difference between points in two point clouds in
 the direction of u.

  a         b
  .<-- d -->.        returns a positive d
 --------------> u

  b         a
  .<-- d -->.        returns a negative d
 --------------> u
*/
double dir_separation(const ptlist& a, const ptlist& b, const vec3& u);

void serialize(const_mat_view m, std::ostream& os);
void unserialize(mat& m,           std::istream& is);
void unserialize(rvec& v,          std::istream& is);
void unserialize(cvec& v,          std::istream& is);

class bbox
{
    public:
        bbox()
        {
            min_pt.setZero();
            max_pt.setZero();
        }
        
        /* bounding box around single point */
        bbox(const vec3& v)
        {
            min_pt = v;
            max_pt = v;
        }
        
        bbox(const ptlist& pts)
        {
            if (pts.size() == 0)
            {
                min_pt.setZero();
                max_pt.setZero();
            }
            else
            {
                min_pt = pts[0];
                max_pt = pts[0];
                
                for (size_t i = 1; i < pts.size(); ++i)
                {
                    include(pts[i]);
                }
            }
        }
        
        bbox(const vec3& min_pt, const vec3& max_pt)
            : min_pt(min_pt), max_pt(max_pt)
        {}
        
        void include(const vec3& v)
        {
            for (int d = 0; d < 3; ++d)
            {
                if (v[d] < min_pt[d])
                {
                    min_pt[d] = v[d];
                }
                if (v[d] > max_pt[d])
                {
                    max_pt[d] = v[d];
                }
            }
        }
        
        void include(const ptlist& pts)
        {
            ptlist::const_iterator i;
            for (i = pts.begin(); i != pts.end(); ++i)
            {
                include(*i);
            }
        }
        
        void include(const bbox& b)
        {
            include(b.min_pt);
            include(b.max_pt);
        }
        
        bool intersects(const bbox& b) const
        {
            int d;
            for (d = 0; d < 3; ++d)
            {
                if (max_pt[d] < b.min_pt[d] || min_pt[d] > b.max_pt[d])
                {
                    return false;
                }
            }
            return true;
        }
        
        bool contains(const bbox& b) const
        {
            int d;
            for (d = 0; d < 3; ++d)
            {
                if (max_pt[d] < b.max_pt[d] || min_pt[d] > b.min_pt[d])
                {
                    return false;
                }
            }
            return true;
        }
        
        void get_vals(vec3& min_out, vec3& max_out) const
        {
            min_out = min_pt;
            max_out = max_pt;
        }
        
        bool operator==(const bbox& b) const
        {
            return min_pt == b.min_pt && max_pt == b.max_pt;
        }
        
        bool operator!=(const bbox& b) const
        {
            return min_pt != b.min_pt || max_pt != b.max_pt;
        }
        
        bbox& operator=(const bbox& b)
        {
            min_pt = b.min_pt;
            max_pt = b.max_pt;
            return *this;
        }
        
        void reset()
        {
            min_pt.setZero();
            max_pt.setZero();
        }
        
        vec3 get_centroid() const
        {
            return (max_pt + min_pt) / 2.0;
        }
        
        const vec3& get_min() const
        {
            return min_pt;
        }
        
        const vec3& get_max() const
        {
            return max_pt;
        }
        
        void get_points(ptlist& p) const
        {
            p.push_back(vec3(min_pt[0], min_pt[1], min_pt[2]));
            p.push_back(vec3(min_pt[0], min_pt[1], max_pt[2]));
            p.push_back(vec3(min_pt[0], max_pt[1], min_pt[2]));
            p.push_back(vec3(min_pt[0], max_pt[1], max_pt[2]));
            p.push_back(vec3(max_pt[0], min_pt[1], min_pt[2]));
            p.push_back(vec3(max_pt[0], min_pt[1], max_pt[2]));
            p.push_back(vec3(max_pt[0], max_pt[1], min_pt[2]));
            p.push_back(vec3(max_pt[0], max_pt[1], max_pt[2]));
        }
        
        vec3 get_random_point() const
        {
            vec3 randPt;
            randomize_vec(randPt, min_pt, max_pt);
            return randPt;
        }
        
        double get_volume() const
        {
            return (max_pt[0] - min_pt[0]) * (max_pt[1] - min_pt[1]) * (max_pt[2] - min_pt[2]);
        }
        
        
        friend std::ostream& operator<<(std::ostream& os, const bbox& b);
        
    private:
        vec3 min_pt;
        vec3 max_pt;
};

std::ostream& operator<<(std::ostream& os, const bbox& b);

class transform3
{
    public:
        transform3();
        transform3(const transform3& t);
        transform3(char type, const vec3& v);
        transform3(const vec3& p, const vec3& r, const vec3& s);
        void to_prs(vec3& p, vec4& r, vec3& s) const;
        
        vec3 operator()(const vec3& v) const
        {
            return trans * v;
        }
        
        transform3 operator*(const transform3& t) const
        {
            transform3 r;
            r.trans = trans * t.trans;
            return r;
        }
        
        void get_matrix(mat& m) const
        {
            m = trans.matrix();
        }
        
    private:
        Eigen::Transform<double, 3, Eigen::Affine> trans;
};

#endif
