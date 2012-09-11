#include <iostream>
#include <iomanip>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <limits>
#include "common.h"

using namespace std;

logger LOG;

const char *log_type_names[NUM_LOG_TYPES] = {
	"WARN",
	"ERROR",
	"CTRLDBG",
	"EMDBG",
	"SGEL"
};

void split(const string &s, const string &delim, vector<string> &fields) {
	int start, end = 0;
	while (end < s.size()) {
		start = s.find_first_not_of(delim, end);
		if (start == string::npos) {
			return;
		}
		end = s.find_first_of(delim, start);
		if (end == string::npos) {
			end = s.size();
		}
		fields.push_back(s.substr(start, end - start));
	}
}

void strip(string &s, const string &whitespace) {
	size_t begin = s.find_first_not_of(whitespace);
	if (begin == string::npos) {
		s.clear();
		return;
	}
	size_t end = s.find_last_not_of(whitespace) + 1;
	s = s.substr(begin, end - begin);
}

void sample(int n, int low, int high, bool replace, vector<int> &s) {
	int range = high - low;
	while (s.size() < n) {
		int r = low + (rand() % range);
		if (!replace && find(s.begin(), s.end(), r) != s.end()) {
			continue;
		}
		s.push_back(r);
	}
}

ofstream& get_datavis() {
	static bool first = true;
	static ofstream f;
	if (first) {
		string path = get_option("datavis");
		if (path.empty() || access(path.c_str(), W_OK) < 0) {
			cout << "Datavis output to /dev/null" << endl;
			f.open("/dev/null");
		} else {
			cout << "Datavis output to " << path << endl;
			f.open(path.c_str());
			f << "CLEAR" << endl;
		}
		first = false;
	}
	return f;
}

vec3 project(const vec3 &v, const vec3 &u) {
	float m = u.squaredNorm();
	if (m == 0.) {
		return vec3::Zero();
	}
	return u * (v.dot(u) / m);
}

float dir_separation(const ptlist &a, const ptlist &b, const vec3 &u) {
	int counter = 0;
	ptlist::const_iterator i;
	vec3 p;
	float x, min = numeric_limits<float>::max(), max = -numeric_limits<float>::max();
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

ostream &histogram(const rvec &vals, int nbins, ostream &os) {
	assert(nbins > 0);
	float min = vals[0], max = vals[0], binsize, hashes_per;
	int i, b, maxcount = 0;
	vector<int> counts(nbins, 0);
	for (i = 1; i < vals.size(); ++i) {
		if (vals[i] < min) {
			min = vals[i];
		}
		if (vals[i] > max) {
			max = vals[i];
		}
	}
	binsize = (max - min) / (nbins - 1);
	if (binsize == 0) {
		LOG(WARN) << "All values identical (" << min << "), not drawing histogram" << endl;
		return os;
	}
	for (i = 0; i < vals.size(); ++i) {
		b = (int) ((vals[i] - min) / binsize);
		assert(b < counts.size());
		counts[b]++;
		if (counts[b] > maxcount) {
			maxcount = counts[b];
		}
	}
	hashes_per = 60.0 / maxcount;
	streamsize p = os.precision();
	os.precision(3);
	ios::fmtflags f = os.flags();
	os << scientific;
	for (i = 0; i < nbins; ++i) {
		os << min + binsize * i << "|";
		os << setfill('#') << setw((int) (hashes_per * counts[i])) << '/' << counts[i] << endl;
	}
	os.precision(p);
	os.flags(f);
	return os;
}

ostream &histogram(const vector<double> &vals, int nbins, ostream &os) {
	rvec v(vals.size());
	for (int i = 0; i < vals.size(); ++i) {
		v(i) = vals[i];
	}
	return histogram(v, nbins, os);
}

ostream& operator<<(ostream &os, const bbox &b) {
	os << b.min[0] << " " << b.min[1] << " " << b.min[2] << " " << b.max[0] << " " << b.max[1] << " " << b.max[2];
	return os;
}

string get_option(const string &key) {
	static map<string, string> options;
	static bool first = true;
	
	if (first) {
		char *s;
		if ((s = getenv("SVS_OPTS")) != NULL) {
			string optstr(s);
			vector<string> fields;
			split(optstr, ",", fields);
			for (int i = 0; i < fields.size(); ++i) {
				size_t p = fields[i].find_first_of(':');
				if (p == string::npos) {
					options[fields[i]] = "-";
				} else {
					options[fields[i].substr(0, p)] = fields[i].substr(p+1);
				}
			}
		}
		first = false;
	}
	return options[key];
}

bool parse_double(const string &s, double &v) {
	if (s.empty()) {
		return false;
	}
	
	char *end;
	v = strtod(s.c_str(), &end);
	if (*end != '\0') {
		return false;
	}
	return true;
}

bool parse_int(const string &s, int &v) {
	if (s.empty()) {
		return false;
	}
	
	char *end;
	v = strtol(s.c_str(), &end, 10);
	if (*end != '\0') {
		return false;
	}
	return true;
}

void slice_tuple(const tuple &t1, const index_vec &inds, tuple &t2) {
	int n = inds.size();
	t2.clear();
	t2.resize(inds.size());
	for (int i = 0; i < n; ++i) {
		t2[i] = t1[inds[i]];
	}
}

tuple concat_tuples(const tuple &t1, const tuple &t2) {
	tuple t3(t1.begin(), t1.end());
	t3.insert(t3.end(), t2.begin(), t2.end());
	return t3;
}


relation::relation() : sz(0), arty(0) {}
relation::relation(int n) : sz(0), arty(n) {}
relation::relation(const relation &r) : sz(r.sz), arty(r.arty), tuples(r.tuples) {}
	
relation::relation(int n, const vector<tuple> &ts) : sz(ts.size()), arty(n) {
	assert(arty > 0);
	vector<tuple>::const_iterator i;
	for (i = ts.begin(); i != ts.end(); ++i) {
		assert(i->size() == arty);
		tuple tail(i->begin() + 1, i->end());
		tuples[tail].insert(i->front());
	}
}
	
bool relation::test(const tuple &t) const {
	assert(t.size() == arty);
	tuple tail(t.begin() + 1, t.end());
	tuple_map::const_iterator i = tuples.find(tail);
	if (i == tuples.end()) {
		return false;
	}
	return in_set(t[0], i->second);
}
	
void relation::slice(const index_vec &inds, relation &out) const {
	assert(0 < inds.size() && inds.size() <= arty && inds[0] == 0);
	out.arty = inds.size();
	out.tuples.clear();
	index_vec tinds;
	for (int i = 1; i < inds.size(); ++i) {
		tinds.push_back(inds[i] - 1);
	}
	
	out.sz = 0;
	tuple t;
	tuple_map::const_iterator i;
	for (i = tuples.begin(); i != tuples.end(); ++i) {
		slice_tuple(i->first, tinds, t);
		set<int> &s = out.tuples[t];
		out.sz -= s.size();
		s.insert(i->second.begin(), i->second.end());
		out.sz += s.size();
	}
}

relation &relation::operator=(const relation &r) {
	sz = r.sz;
	arty = r.arty;
	tuples = r.tuples;
	return *this;
}

/*
 Remove all tuples in this relation that does not match any tuple in r
 along indexes inds.
*/
void relation::filter(const index_vec &inds, const relation &r) {
	assert(!inds.empty() && inds.front() == 0);
	tuple s;
	index_vec tinds;
	for (int i = 1; i < inds.size(); ++i) {
		tinds.push_back(inds[i] - 1);
	}

	tuple_map::iterator i = tuples.begin();
	tuple_map::const_iterator j;
	while (i != tuples.end()) {
		slice_tuple(i->first, tinds, s);
		sz -= i->second.size();
		j = r.tuples.find(s);
		if (j == r.tuples.end()) {
			tuples.erase(i++);
		} else {
			intersect_sets_inplace(i->second, j->second);
			sz += i->second.size();
			++i;
		}
	}
}

/*
 Remove all tuples in this relation that matches some tuple in r along indexes
 inds.
*/
void relation::subtract(const index_vec &inds, const relation &r) {
	assert(!inds.empty() && inds.front() == 0);
	tuple s;
	index_vec tinds;
	for (int i = 1; i < inds.size(); ++i) {
		tinds.push_back(inds[i] - 1);
	}

	tuple_map::iterator i = tuples.begin();
	tuple_map::const_iterator j;
	while (i != tuples.end()) {
		slice_tuple(i->first, tinds, s);
		j = r.tuples.find(s);
		if (j != r.tuples.end()) {
			sz -= i->second.size();
			subtract_set_inplace(i->second, j->second);
			if (i->second.empty()) {
				tuples.erase(i++);
			} else {
				sz += i->second.size();
				++i;
			}
		}
	}
}

/*
 For each tuple t1 in this relation, find all tuples t2 in r such that
 t1[match1] == t2[match2], and extend t1 with t2[extend]. Upon
 completion, this relation will contain all such t1's.
*/
void relation::expand(const relation  &r,
                      const index_vec &match1,
                      const index_vec &match2,
                      const index_vec &extend)
{
	//static int count = 0;
	//cout << "expand " << count++;

	assert(!match1.empty() && match1.front() == 0 && !match2.empty() && match2.front() == 0);
	index_vec m1, m2, ex;
	tuple_map::const_iterator i;
	tuple t1, t2;

	for (int i = 1; i < match1.size(); ++i) {
		m1.push_back(match1[i] - 1);
	}
	for (int i = 1; i < match2.size(); ++i) {
		m2.push_back(match2[i] - 1);
	}
	for (int i = 0; i < extend.size(); ++i) {
		ex.push_back(extend[i] - 1);
	}

	tuple_map old_tuples = tuples;
	tuples.clear();
	
	// preprocess r to avoid redundant slicing
	vector<sliced_relation_tuple> sliced(r.tuples.size());
	int j = 0;
	for (i = r.tuples.begin(); i != r.tuples.end(); ++i) {
		slice_tuple(i->first, m2, sliced[j].match);
		slice_tuple(i->first, ex, sliced[j].extend);
		sliced[j++].lead = &i->second;
	}
	
	sz = 0;
	for (i = old_tuples.begin(); i != old_tuples.end(); ++i) {
		slice_tuple(i->first, m1, t1);
		for (int j = 0; j < sliced.size(); ++j) {
			if (t1 == sliced[j].match) {
				t2 = concat_tuples(i->first, sliced[j].extend);
				set<int> &s = tuples[t2];
				sz -= s.size();
				intersect_sets(i->second, *sliced[j].lead, s);
				if (s.empty()) {
					tuples.erase(t2);
				}
				sz += s.size();
			}
		}
	}
	
	arty += extend.size();
	//cout << " size = " << sz << endl;
}

void relation::count_expansion(const relation  &r,
                               const index_vec &match1,
                               const index_vec &match2,
							   int &matched,
							   int &new_size) const
{
	assert(!match1.empty() && match1.front() == 0 && !match2.empty() && match2.front() == 0);
	index_vec m1, m2;
	tuple_map::const_iterator i;
	tuple t1, t2;

	for (int i = 1; i < match1.size(); ++i) {
		m1.push_back(match1[i] - 1);
	}
	for (int i = 1; i < match2.size(); ++i) {
		m2.push_back(match2[i] - 1);
	}
	// preprocess r to avoid redundant slicing
	vector<sliced_relation_tuple> sliced(r.tuples.size());
	int j = 0;
	for (i = r.tuples.begin(); i != r.tuples.end(); ++i) {
		slice_tuple(i->first, m2, sliced[j].match);
		sliced[j++].lead = &i->second;
	}

	matched = 0;
	new_size = 0;
	
	set<int> matched_insts;
	for (i = tuples.begin(); i != tuples.end(); ++i) {
		matched_insts.clear();
		slice_tuple(i->first, m1, t1);
		for (int j = 0; j < sliced.size(); ++j) {
			if (t1 == sliced[j].match) {
				new_size += intersect_sets(i->second, *sliced[j].lead, matched_insts);
			}
		}
		matched += matched_insts.size();
	}
}

void relation::init_single(const vector<int> &s) {
	tuples.clear();
	tuples[tuple()].insert(s.begin(), s.end());
	arty = 1;
	sz = s.size();
}

void relation::add(int i) {
	assert(arty == 1);
	if (tuples.empty()) {
		tuples[tuple()];
	}
	set<int> &s = tuples.begin()->second;
	s.insert(i);
	sz = s.size();
}

void relation::add(int i, const tuple &t) {
	assert(t.size() + 1 == arty);
	set<int> &s = tuples[t];
	sz -= s.size();
	s.insert(i);
	sz += s.size();
}

void relation::del(int i, const tuple &t) {
	assert(t.size() + 1 == arty);
	tuple_map::iterator j = tuples.find(t);
	if (j != tuples.end()) {
		j->second.erase(i);
	}
}

void relation::del(int i) {
	assert(arty == 1);
	if (!tuples.empty()) {
		set<int> &s = tuples.begin()->second;
		s.erase(i);
		sz = s.size();
	}
}

void relation::at_pos(int n, set<int> &elems) const {
	assert(0 <= n && n < arty);
	tuple_map::const_iterator i;
	if (n == 0) {
		for (i = tuples.begin(); i != tuples.end(); ++i) {
			union_sets_inplace(elems, i->second);
		}
	} else {
		for (i = tuples.begin(); i != tuples.end(); ++i) {
			elems.insert(i->first[n - 1]);
		}
	}
}

void relation::drop_first(set<tuple> &out) const {
	tuple_map::const_iterator i;
	for (i = tuples.begin(); i != tuples.end(); ++i) {
		out.insert(i->first);
	}
}

ostream &operator<<(ostream &os, const relation &r) {
	tuple t(r.arty);
	set<tuple> sorted;
	relation::tuple_map::const_iterator i;
	for (i = r.tuples.begin(); i != r.tuples.end(); ++i) {
		copy(i->first.begin(), i->first.end(), t.begin() + 1);
		set<int>::const_iterator j;
		for (j = i->second.begin(); j != i->second.end(); ++j) {
			t[0] = *j;
			sorted.insert(t);
		}
	}
	set<tuple>::iterator k;
	for (k = sorted.begin(); k != sorted.end(); ++k) {
		join(os, *k, ",") << endl;
	}
	return os;
}

ostream &operator<<(ostream &os, const relation_table &t) {
	relation_table::const_iterator i;
	for (i = t.begin(); i != t.end(); ++i) {
		os << i->first << endl << i->second;
	}
	return os;
}
