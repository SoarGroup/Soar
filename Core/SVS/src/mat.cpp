#include <iostream>
#include "mat.h"
#include "common.h"
#include "params.h"
#include "serialize.h"

using namespace std;
dyn_mat::dyn_mat() : buf(0, 0), r(0), c(0), released(false) {}

dyn_mat::dyn_mat(size_t nrows, size_t ncols)
    : buf(nrows, ncols), r(nrows), c(ncols), released(false) {}

dyn_mat::dyn_mat(size_t nrows, size_t ncols, size_t init_row_capacity, size_t init_col_capacity)
    : buf(init_row_capacity, init_col_capacity), r(nrows), c(ncols), released(false) {}

dyn_mat::dyn_mat(const_mat_view m)
    : r(m.rows()), c(m.cols()), released(false)
{
    buf = m;
}

dyn_mat::dyn_mat(const dyn_mat& other)
    : buf(other.buf), r(other.r), c(other.c), released(false)
{
    assert(!other.released);
}

void dyn_mat::resize(size_t nrows, size_t ncols)
{
    assert(!released);
    r = nrows;
    c = ncols;
    if (r > static_cast<size_t>(buf.rows()) && c > static_cast<size_t>(buf.cols()))
    {
        buf.conservativeResize(r, c);
    }
    else if (r > static_cast<size_t>(buf.rows()))
    {
        buf.conservativeResize(r, Eigen::NoChange);
    }
    else if (c > static_cast<size_t>(buf.cols()))
    {
        buf.conservativeResize(Eigen::NoChange, c);
    }
}

void dyn_mat::append_row()
{
    assert(!released);
    if (r >= static_cast<size_t>(buf.rows()))
    {
        buf.conservativeResize(r == 0 ? 1 : r * 2, Eigen::NoChange);
    }
    ++r;
}

void dyn_mat::append_row(const rvec& row)
{
    assert(!released && row.size() == c);
    append_row();
    buf.block(r - 1, 0, 1, c) = row;
}

void dyn_mat::insert_row(size_t i)
{
    assert(!released && i <= r);
    if (r >= static_cast<size_t>(buf.rows()))
    {
        buf.conservativeResize(r == 0 ? 1 : r * 2, Eigen::NoChange);
    }
    for (int j = static_cast<int>(r); j > static_cast<int>(i); --j)
    {
        buf.block(j, 0, 1, c) = buf.block(j - 1, 0, 1, c);
    }
    ++r;
}

void dyn_mat::insert_row(size_t i, const rvec& row)
{
    assert(!released && row.size() == c);
    insert_row(i);
    buf.block(i, 0, 1, c) = row;
}

void dyn_mat::remove_row(size_t i)
{
    assert(!released && i < r);
    for (size_t j = i + 1; j < r; ++j)
    {
        buf.block(j - 1, 0, 1, c) = buf.block(j, 0, 1, c);
    }
    --r;
}

void dyn_mat::append_col()
{
    assert(!released);
    if (c >= static_cast<size_t>(buf.cols()))
    {
        buf.conservativeResize(Eigen::NoChange, c == 0 ? 1 : c * 2);
    }
    ++c;
}

void dyn_mat::append_col(const cvec& col)
{
    assert(!released && col.size() == r);
    append_col();
    buf.block(0, c - 1, r, 1) = col;
}

void dyn_mat::insert_col(size_t i)
{
    assert(!released && i <= c);
    if (c >= static_cast<size_t>(buf.cols()))
    {
        buf.conservativeResize(Eigen::NoChange, c == 0 ? 1 : c * 2);
    }
    for (int j = static_cast<int>(c); j > static_cast<int>(i); --j)
    {
        buf.block(0, j, r, 1) = buf.block(0, j - 1, r, 1);
    }
    ++c;
}

void dyn_mat::insert_col(size_t i, const cvec& col)
{
    assert(!released && col.size() == r);
    insert_col(i);
    buf.block(0, i, r, 1) = col;
}

void dyn_mat::remove_col(size_t i)
{
    assert(!released && i < c);
    for (size_t j = i + 1; j < c; ++j)
    {
        buf.block(0, j - 1, r, 1) = buf.block(0, j, r, 1);
    }
    --c;
}

void dyn_mat::serialize(ostream& os) const
{
    assert(!released);
    ::serialize(const_mat_view(buf.topLeftCorner(r, c)), os);
}

void dyn_mat::unserialize(istream& is)
{
    assert(!released);
    ::unserialize(buf, is);
    r = buf.rows();
    c = buf.cols();
}

ostream& output_rvec(ostream& os, const rvec& v, const string& sep)
{
    size_t n = v.size();
    if (n == 0)
    {
        return os;
    }
    
    for (size_t i = 0; i < n - 1; ++i)
    {
        os << v(i) << sep;
    }
    os << v(n - 1);
    return os;
}

ostream& output_cvec(ostream& os, const cvec& v, const string& sep)
{
    size_t n = v.size();
    if (n == 0)
    {
        return os;
    }
    
    for (size_t i = 0; i < n - 1; ++i)
    {
        os << v(i) << sep;
    }
    os << v(n - 1);
    return os;
}

ostream& output_mat(ostream& os, const_mat_view m)
{
    size_t r = m.rows(), c = m.cols();
    if (r == 0 || c == 0)
    {
        return os;
    }
    
    for (size_t i = 0; i < r; ++i)
    {
        for (size_t j = 0; j < c - 1; ++j)
        {
            os << m(i, j) << " ";
        }
        os << m(i, c - 1) << endl;
    }
    return os;
}

bool normal(const_mat_view m)
{
    for (int i = 0; i < m.rows(); ++i)
    {
        for (int j = 0; j < m.cols(); ++j)
        {
            if (is_nan(m(i, j)) || is_inf(m(i, j)))
            {
                return false;
            }
        }
    }
    return true;
}

bool uniform(const_mat_view m)
{
    if (m.rows() == 0 || m.cols() == 0)
    {
        return true;
    }
    return m.isConstant(m(0, 0), SAME_THRESH);
}

void get_nonuniform_cols(const_mat_view X, int ncols, vector<int>& cols)
{
    for (int i = 0; i < ncols; ++i)
    {
        if (!uniform(X.col(i)))
        {
            cols.push_back(i);
        }
    }
}

void del_uniform_cols(mat_view X, int ncols, vector<int>& cols)
{
    get_nonuniform_cols(X, ncols, cols);
    pick_cols(X, cols);
}

void pick_cols(const_mat_view X, const vector<int>& cols, mat& result)
{
    result.resize(X.rows(), cols.size());
    for (size_t i = 0; i < cols.size(); ++i)
    {
        result.col(i) = X.col(cols[i]);
    }
}

void pick_rows(const_mat_view X, const vector<int>& rows, mat& result)
{
    if (result.rows() < X.rows() || result.cols() != X.cols())
    {
        result.resize(rows.size(), X.cols());
    }
    for (size_t i = 0; i < rows.size(); ++i)
    {
        result.row(i) = X.row(rows[i]);
    }
}

void pick_cols(mat_view X, const vector<int>& cols)
{
    assert(static_cast<size_t>(X.cols()) >= cols.size());
    bool need_copy = false;
    for (size_t i = 0; i < cols.size(); ++i)
    {
        if (cols[i] < static_cast<int>(i))
        {
            need_copy = true;
            break;
        }
    }
    if (need_copy)
    {
        mat c = X;
        for (size_t i = 0; i < cols.size(); ++i)
        {
            X.col(i) = c.col(cols[i]);
        }
    }
    else
    {
        for (size_t i = 0; i < cols.size(); ++i)
        {
            X.col(i) = X.col(cols[i]);
        }
    }
}

void pick_rows(mat_view X, const vector<int>& rows)
{
    assert(static_cast<size_t>(X.rows()) >= rows.size());
    bool need_copy = false;
    for (size_t i = 0; i < rows.size(); ++i)
    {
        if (rows[i] < static_cast<int>(i))
        {
            need_copy = true;
            break;
        }
    }
    if (need_copy)
    {
        mat c = X;
        for (size_t i = 0; i < rows.size(); ++i)
        {
            X.row(i) = c.row(rows[i]);
        }
    }
    else
    {
        for (size_t i = 0; i < rows.size(); ++i)
        {
            if (rows[i] > static_cast<int>(i))
            {
                X.row(i) = X.row(rows[i]);
            }
        }
    }
}

void randomize_vec(rvec& v, const rvec& min, const rvec& max)
{
    //v = min.array() + (rvec::Random(v.size()).array() * (max - min).array());
    // apparently rvec::Random will generate numbers outside of [0, 1]
    for (int i = 0; i < v.size(); ++i)
    {
        v(i) = min(i) + (rand() / (double) RAND_MAX) * (max(i) - min(i));
    }
}

void randomize_vec(vec3& v, const vec3& min, const vec3& max)
{
    //v = min.array() + (rvec::Random(v.size()).array() * (max - min).array());
    // apparently rvec::Random will generate numbers outside of [0, 1]
    for (int i = 0; i < 3; ++i)
    {
        v[i] = min[i] + (rand() / (double) RAND_MAX) * (max[i] - min[i]);
    }
}

vec3 project(const vec3& v, const vec3& u)
{
    double m = u.squaredNorm();
    if (m == 0.)
    {
        return vec3::Zero();
    }
    return u * (v.dot(u) / m);
}

double dir_separation(const ptlist& a, const ptlist& b, const vec3& u)
{
    vec3 p;
    double x, minx = 0.0, maxx = 0.0;
    for (size_t i = 0, iend = a.size(); i < iend; ++i)
    {
        p = project(a[i], u);
        x = p(0) / u(0);
        if (i == 0 || x < minx)
        {
            minx = x;
        }
    }
    for (size_t i = 0, iend = b.size(); i < iend; ++i)
    {
        p = project(b[i], u);
        x = p(0) / u(0);
        if (i == 0 || x > maxx)
        {
            maxx = x;
        }
    }
    
    return maxx - minx;
}

ostream& operator<<(ostream& os, const bbox& b)
{
    os << b.min_pt[0] << " " << b.min_pt[1] << " " << b.min_pt[2] << " "
       << b.max_pt[0] << " " << b.max_pt[1] << " " << b.max_pt[2];
    return os;
}

void serialize(const_mat_view m, ostream& os)
{
    serializer sr(os);
    sr << string("MAT") << static_cast<int>(m.rows()) << static_cast<int>(m.cols()) << '\n';
    for (int i = 0; i < m.rows(); ++i)
    {
        for (int j = 0; j < m.cols(); ++j)
        {
            sr << m(i, j);
        }
        sr << '\n';
    }
}

void unserialize(mat& m, istream& is)
{
    string label;
    int nrows, ncols;
    unserializer unsr(is);
    
    unsr >> label >> nrows >> ncols;
    assert(label == "MAT");
    
    m.resize(nrows, ncols);
    for (int i = 0; i < nrows; ++i)
    {
        for (int j = 0; j < ncols; ++j)
        {
            unsr >> m(i, j);
        }
    }
}

void unserialize(rvec& v, istream& is)
{
    mat m;
    unserialize(m, is);
    assert(m.rows() == 1);
    v = m.row(0);
}

void unserialize(cvec& v, istream& is)
{
    mat m;
    unserialize(m, is);
    assert(m.cols() == 1);
    v = m.col(0);
}

transform3::transform3() {}

transform3::transform3(const transform3& t) : trans(t.trans) {}

transform3::transform3(char type, const vec3& v)
{
    switch (type)
    {
        case 'p':
            trans = Eigen::Translation<double, 3>(v);
            break;
        case 'r':
            trans = Eigen::AngleAxisd(v(2), Eigen::Vector3d::UnitZ()) *
                    Eigen::AngleAxisd(v(1), Eigen::Vector3d::UnitY()) *
                    Eigen::AngleAxisd(v(0), Eigen::Vector3d::UnitX()) ;
            break;
        case 's':
            trans = Eigen::Scaling(v);
            break;
        default:
            assert(false);
    }
}

transform3::transform3(const vec3& p, const vec3& r, const vec3& s)
{
    trans = Eigen::Translation<double, 3>(p) *
            Eigen::AngleAxisd(r(2), Eigen::Vector3d::UnitZ()) *
            Eigen::AngleAxisd(r(1), Eigen::Vector3d::UnitY()) *
            Eigen::AngleAxisd(r(0), Eigen::Vector3d::UnitX()) *
            Eigen::Scaling(s);
}

void transform3::to_prs(vec3& p, vec4& r, vec3& s) const
{
    Eigen::Matrix3d rm, sm;
    
    p = trans.translation();
    trans.computeRotationScaling(&rm, &sm);
    
    Eigen::Quaterniond q(rm);
    r(0) = q.x();
    r(1) = q.y();
    r(2) = q.z();
    r(3) = q.w();
    
    s(0) = sm(0, 0);
    s(1) = sm(1, 1);
    s(2) = sm(2, 2);
}
