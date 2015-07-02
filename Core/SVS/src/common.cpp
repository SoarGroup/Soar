#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <limits>
#include <cstdlib>
#include <cassert>
#include "common.h"

using namespace std;

const double NAN = std::numeric_limits<double>::quiet_NaN();
const double INF = std::numeric_limits<double>::infinity();

void split(const string& s, const string& delim, vector<string>& fields)
{
    size_t start = 0, end = 0, sz = s.size();
    while (end < sz)
    {
        if (delim.empty())
        {
            for (start = end; start < sz && isspace(s[start]); ++start)
                ;
        }
        else
        {
            start = s.find_first_not_of(delim, end);
        }
        if (start == string::npos || start == sz)
        {
            return;
        }
        if (delim.empty())
        {
            for (end = start + 1; end < sz && !isspace(s[end]); ++end)
                ;
        }
        else
        {
            end = s.find_first_of(delim, start);
            if (end == string::npos)
            {
                end = sz;
            }
        }
        fields.push_back(s.substr(start, end - start));
    }
}

void strip(string& s, const string& whitespace)
{
    size_t begin = s.find_first_not_of(whitespace);
    if (begin == string::npos)
    {
        s.clear();
        return;
    }
    size_t end = s.find_last_not_of(whitespace) + 1;
    s = s.substr(begin, end - begin);
}

istream& get_nonblank_line(istream& is, string& line)
{
    while (getline(is, line))
    {
        for (size_t i = 0, iend = line.size(); i < iend; ++i)
        {
            if (!isspace(line[i]))
            {
                return is;
            }
        }
    }
    return is;
}

/*
 Upper bound on range is non-inclusive.
*/
void sample(int k, int low, int high, std::vector<int>& output)
{
    int range = high - low;
    size_t start = output.size();
    
    assert(k <= range);
    output.resize(start + k);
    for (int i = 0; i < range; ++i)
    {
        if (i < k)
        {
            output[start + i] = low + i;
        }
        else
        {
            int r = rand() % (i + 1);
            if (r < k)
            {
                output[start + r] = low + i;
            }
        }
    }
}

ostream& histogram(const vector<double>& vals, size_t nbins, ostream& os)
{
    assert(nbins > 0);
    double min, max, binsize, hashes_per;
    size_t i;
    int b, maxcount = 0;
    vector<int> counts(nbins, 0);
    min = *min_element(vals.begin(), vals.end());
    max = *max_element(vals.begin(), vals.end());
    
    binsize = (max - min) / (nbins - 1);
    if (binsize == 0)
    {
        return os;
    }
    for (i = 0; i < vals.size(); ++i)
    {
        b = static_cast<int>((vals[i] - min) / binsize);
        assert(b < static_cast<int>(counts.size()));
        counts[b]++;
    }
    maxcount = *max_element(counts.begin(), counts.end());
    hashes_per = 60.0 / maxcount;
    streamsize p = os.precision();
    os.precision(3);
    ios::fmtflags f = os.flags();
    os << scientific;
    for (i = 0; i < nbins; ++i)
    {
        os << min + binsize* i << "|";
        os << setfill('#') << setw((int)(hashes_per * counts[i])) << '/' << counts[i] << endl;
    }
    os.precision(p);
    os.flags(f);
    return os;
}

bool parse_double(const string& s, double& v)
{
    if (s.empty())
    {
        return false;
    }
    
    char* end;
    v = strtod(s.c_str(), &end);
    if (*end != '\0')
    {
        return false;
    }
    return true;
}

bool parse_int(const string& s, int& v)
{
    if (s.empty())
    {
        return false;
    }
    
    char* end;
    v = static_cast<int>(strtol(s.c_str(), &end, 10));
    if (*end != '\0')
    {
        return false;
    }
    return true;
}

bool is_nan(double x)
{
    return (x == numeric_limits<double>::quiet_NaN() || x == numeric_limits<double>::signaling_NaN());
}

bool is_inf(double x)
{
    return x == numeric_limits<double>::infinity();
}

bool approx_equal(double a, double b, double thresh)
{
    return (fabs(a - b) <= thresh);
}

table_printer::table_printer()
    : spacer_width(1)
{}

table_printer& table_printer::skip(int n)
{
    rows.back().resize(rows.back().size() + n);
    return *this;
}

table_printer& table_printer::add_row()
{
    rows.resize(rows.size() + 1);
    return *this;
}

void table_printer::set_precision(int p)
{
    ss.precision(p);
}

void table_printer::set_scientific(bool s)
{
    if (s)
    {
        ss.setf(ios_base::scientific, ios_base::floatfield);
    }
    else
    {
        ss.setf(ios_base::fixed, ios_base::floatfield);
    }
}

void table_printer::set_column_alignment(int col, int align)
{
    alignments[col] = align;
}

void table_printer::set_spacer_width(int w)
{
    spacer_width = w;
}

void table_printer::print(ostream& os) const
{
    std::vector<int> widths;
    for (size_t i = 0; i < rows.size(); ++i)
    {
        const vector<string>& row = rows[i];
        if (row.size() > widths.size())
        {
            widths.resize(row.size());
        }
        for (size_t j = 0; j < row.size(); ++j)
        {
            if (static_cast<int>(row[j].size()) > widths[j])
            {
                widths[j] = static_cast<int>(row[j].size());
            }
        }
    }
    
    for (size_t i = 0; i < rows.size(); ++i)
    {
        const vector<string>& row = rows[i];
        for (size_t j = 0; j < row.size(); ++j)
        {
            int a = -1, pad;
            
            map_get(alignments, static_cast<int>(j), a);
            switch (a)
            {
                case -1:
                    os << left << setw(widths[j]) << row[j];
                    break;
                case 0:
                    pad = (widths[j] - static_cast<int>(row[j].size())) / 2;
                    os << setw(pad) << " ";
                    os << left << setw(widths[j] - pad) << row[j];
                    break;
                case 1:
                    os << right << setw(widths[j]) << row[j];
                    break;
            }
            os << setw(spacer_width) << " ";
        }
        os << endl;
    }
}
