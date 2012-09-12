#ifndef COMMON_H
#define COMMON_H

#include <iostream>
#include <cstdlib>
#include <cmath>
#include <cassert>
#include <cstring>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <ostream>
#include <fstream>
#include "linalg.h"

typedef std::vector<int> tuple;

void split(const std::string &s, const std::string &delim, std::vector<std::string> &fields);
void strip(std::string &s, const std::string &whitespace);

bool parse_double(const std::string &s, double &v);
bool parse_int   (const std::string &s, int &v);

void sample(int n, int low, int high, bool replace, std::vector<int> &s);

std::string get_option(const std::string &key);

/* I need all my files to have access to a single ofstream */
std::ofstream& get_datavis();

#if 0
#define DATAVIS(x) get_datavis() << x;
#else
#define DATAVIS(x)
#endif

template <typename A, typename B>
inline bool map_get(const std::map<A, B> &m, const A &key, B &val) {
	typename std::map<A, B>::const_iterator i = m.find(key);
	if (i == m.end()) {
		return false;
	}
	val = i->second;
	return true;
}

template <typename A, typename B>
inline const B *map_get(const std::map<A, B> &m, const A &key) {
	typename std::map<A, B>::const_iterator i = m.find(key);
	if (i == m.end()) {
		return NULL;
	}
	return &i->second;
}

template <typename A, typename B>
inline B *map_get(std::map<A, B> &m, const A &key) {
	typename std::map<A, B>::iterator i = m.find(key);
	if (i == m.end()) {
		return NULL;
	}
	return &i->second;
}

template <typename A, typename B>
inline bool map_pop(std::map<A, B> &m, const A &key, B &val) {
	typename std::map<A, B>::iterator i = m.find(key);
	if (i == m.end()) {
		return false;
	}
	val = i->second;
	m.erase(i);
	return true;
}

template<typename A, typename B>
inline bool map_has(const std::map<A, B> &m, const A &key) {
	return m.find(key) != m.end();
}

template<typename T>
bool in_set(const T& x, const std::set<T> &s) {
	return s.find(x) != s.end();
}

template<typename T>
int intersect_sets(const std::set<T> &s1, const std::set<T> &s2, std::set<T> &out) {
	int s = 0;
	typename std::set<T>::const_iterator i = s1.begin(), j = s2.begin();
	while (i != s1.end() && j != s2.end()) {
		if (*i == *j) {
			s++;
			out.insert(*i);
			++i;
			++j;
		} else if (*i < *j) {
			++i;
		} else {
			++j;
		}
	}
	return s;
}

template<typename T>
void intersect_sets_inplace(std::set<T> &s1, const std::set<T> &s2) {
	typename std::set<T>::const_iterator i = s1.begin(), j = s2.begin();
	while (i != s1.end() && j != s2.end()) {
		if (*i < *j) {
			s1.erase(i++);
		} else if (*i > *j) {
			++j;
		} else {
			++i;
			++j;
		}
	}
	s1.erase(i, s1.end());
}

template<typename T>
void subtract_set_inplace(std::set<T> &s1, const std::set<T> &s2) {
	typename std::set<T>::const_iterator i = s1.begin(), j = s2.begin();
	while (i != s1.end() && j != s2.end()) {
		if (*i == *j) {
			s1.erase(i++);
			++j;
		} else if (*i < *j) {
			++i;
		} else {
			++j;
		}
	}
}

template<typename T>
void union_sets_inplace(std::set<T> &s1, const std::set<T> &s2) {
	typename std::set<T>::const_iterator i;
	for (i = s2.begin(); i != s2.end(); ++i) {
		s1.insert(*i);
	}
}

template<typename C, typename D>
std::ostream &join(std::ostream &os, const C& container, const D &delim) {
	if (container.empty()) {
		return os;
	}
	
	typename C::const_iterator i = container.begin();
	os << *i++;
	while (i != container.end()) {
		os << delim << *i++;
	}
	return os;
}

template<typename T>
int argmax(const std::vector<T> &v) {
	if (v.empty()) {
		return -1;
	}
	int m = 0;
	for (int i = 1; i < v.size(); ++i) {
		if (v[i] > v[m]) {
			m = i;
		}
	}
	return m;
}

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
float dir_separation(const ptlist &a, const ptlist &b, const vec3 &u);

std::ostream &histogram(const rvec &vals, int nbins, std::ostream &os);
std::ostream &histogram(const std::vector<double> &vals, int nbins, std::ostream &os);

class bbox {
public:
	bbox() {
		min.setZero();
		max.setZero();
	}
	
	/* bounding box around single point */
	bbox(const vec3 &v) {
		min = v;
		max = v;
	}
	
	bbox(const ptlist &pts) {
		if (pts.size() == 0) {
			min.setZero();
			max.setZero();
		} else {
			min = pts[0];
			max = pts[0];
		
			for(int i = 1; i < pts.size(); ++i) {
				include(pts[i]);
			}
		}
	}
	
	bbox(const vec3 &min, const vec3 &max) : min(min), max(max) {}
	
	void include(const vec3 &v) {
		for(int d = 0; d < 3; ++d) {
			if (v[d] < min[d]) { min[d] = v[d]; }
			if (v[d] > max[d]) { max[d] = v[d]; }
		}
	}
	
	void include(const ptlist &pts) {
		ptlist::const_iterator i;
		for(i = pts.begin(); i != pts.end(); ++i) {
			include(*i);
		}
	}
	
	void include(const bbox &b) {
		include(b.min);
		include(b.max);
	}
	
	bool intersects(const bbox &b) const {
		int d;
		for (d = 0; d < 3; ++d) {
			if (max[d] < b.min[d] || min[d] > b.max[d]) {
				return false;
			}
		}
		return true;
	}
	
	bool contains(const bbox &b) const {
		int d;
		for (d = 0; d < 3; ++d) {
			if (max[d] < b.max[d] || min[d] > b.min[d]) {
				return false;
			}
		}
		return true;
	}
	
	void get_vals(vec3 &minv, vec3 &maxv) const
	{
		minv = min; maxv = max;
	}
	
	bool operator==(const bbox &b) const {
		return min == b.min && max == b.max;
	}
	
	bool operator!=(const bbox &b) const {
		return min != b.min || max != b.max;
	}
	
	bbox &operator=(const bbox &b) {
		min = b.min;
		max = b.max;
		return *this;
	}
	
	void reset() {
		min.setZero();
		max.setZero();
	}
	
	vec3 get_centroid() const {
		return (max + min) / 2.0;
	}
	
	void get_points(ptlist &p) const {
		p.push_back(vec3(min[0], min[1], min[2]));
		p.push_back(vec3(min[0], min[1], max[2]));
		p.push_back(vec3(min[0], max[1], min[2]));
		p.push_back(vec3(min[0], max[1], max[2]));
		p.push_back(vec3(max[0], min[1], min[2]));
		p.push_back(vec3(max[0], min[1], max[2]));
		p.push_back(vec3(max[0], max[1], min[2]));
		p.push_back(vec3(max[0], max[1], max[2]));
	}
	
	friend std::ostream& operator<<(std::ostream &os, const bbox &b);
	
private:
	vec3 min;
	vec3 max;
};

std::ostream& operator<<(std::ostream &os, const bbox &b);

/*
 A relation is essentially a list of all argument values for which some
 predicate evaluates to true.
*/
class relation {
public:
	relation();
	relation(int n);
	relation(const relation &r);
	relation(int n, const std::vector<tuple> &t);
	
	void init_single(const std::vector<int> &s);
	void add(int i);
	void add(int i, const tuple &t);
	void del(int i);
	void del(int i, const tuple &t);
	bool test(const tuple &t) const;
	void slice(const tuple &inds, relation &out) const;
	bool operator==(const relation &r);
	relation &operator=(const relation &r);
	void expand(const relation &r, const tuple &match1, const tuple &match2, const tuple &extend);
	void count_expansion(const relation  &r, const tuple &match1, const tuple &match2, int &matched, int &new_size) const;
	void intersect(const tuple &inds, const relation &r);
	void subtract(const tuple &inds, const relation &r);
	void at_pos(int n, std::set<int> &elems) const;
	void drop_first(std::set<tuple> &out) const;
	
	int size() const { return sz; }
	int arity() const { return arty; }
	bool empty() const { return sz == 0; }
	
private:
	typedef std::map<tuple, std::set<int> > tuple_map;

	struct sliced_relation_tuple {
		tuple match;
		tuple extend;
		const std::set<int> *lead;
	}; 

	int sz, arty;
	tuple_map tuples;
	
	friend std::ostream &operator<<(std::ostream &os, const relation &r);
};

typedef std::map<std::string, relation> relation_table;
std::ostream &operator<<(std::ostream &os, const relation &r);
std::ostream &operator<<(std::ostream &os, const relation_table &t);

template <typename T>
void save_vector(const std::vector<T> &v, std::ostream &os) {
	os << v.size() << std::endl;
	std::copy(v.begin(), v.end(), std::ostream_iterator<T>(os, " "));
	os << std::endl;
}

template <typename T>
void save_vector_rec(const std::vector<T> &v, std::ostream &os) {
	os << v.size() << std::endl;
	for (int i = 0; i < v.size(); ++i) {
		v[i].save(os);
	}
	os << std::endl;
}

template <typename T>
void save_vector_recp(const std::vector<T*> &v, std::ostream &os) {
	os << v.size() << std::endl;
	for (int i = 0; i < v.size(); ++i) {
		v[i]->save(os);
	}
	os << std::endl;
}

template <typename T>
void load_vector(std::vector<T> &v, std::istream &is) {
	int n = 0;
	T x;
	if (!(is >> n)) {
		assert(false);
	}
	v.clear();
	v.reserve(n);
	for (int i = 0; i < n; ++i) {
		if (!(is >> x)) {
			assert(false);
		}
		v.push_back(x);
	}
}

template <typename T>
void load_vector_rec(std::vector<T> &v, std::istream &is) {
	int n = 0;
	if (!(is >> n)) {
		assert(false);
	}
	v.clear();
	v.resize(n);
	for (int i = 0; i < n; ++i) {
		v[n].load(is);
	}
}

template <typename T>
void load_vector_recp(std::vector<T*> &v, std::istream &is) {
	int n = 0;
	if (!(is >> n)) {
		assert(false);
	}
	v.clear();
	v.resize(n);
	for (int i = 0; i < n; ++i) {
		v[n] = new T();
		v[n]->load(is);
	}
}

inline double gausspdf(double x, double mean, double std) {
	const double SQRT2PI = 2.5066282746310002;
	return (1. / std * SQRT2PI) * exp(-((x - mean) * (x - mean) / (2 * std * std)));
}

inline void randomize_vec(rvec &v, const rvec &min, const rvec &max) {
	//v = min.array() + (rvec::Random(v.size()).array() * (max - min).array());
	// apparently rvec::Random will generate numbers outside of [0, 1]
	for (int i = 0; i < v.size(); ++i) {
		v(i) = min(i) + (rand() / (double) RAND_MAX) * (max(i) - min(i));
	}
}

enum log_type {
	WARN,
	ERROR,
	CTRLDBG,
	EMDBG,
	SGEL,
	FOILDBG,
	NUM_LOG_TYPES,
};

// Don't forget to update this in common.cpp when you add new log types
extern const char* log_type_names[NUM_LOG_TYPES];

class logger {
public:
	logger() : is_null(false) {
		on.resize(NUM_LOG_TYPES, false);
	}
	
	void turn_on(log_type t) {
		on[t] = true;
	}
	
	void turn_off(log_type t) {
		on[t] = false;
	}
	
	bool is_on(log_type t) {
		return on[t];
	}

	logger &operator()(log_type t) {
		static logger null_logger(true);
		
		if (is_null) {
			return *this;
		}
		if (on[t]) {
			return *this;
		}
		return null_logger;
	}
	
	template<class T>
	logger &operator<<(const T& v) {
		if (!is_null) {
			std::cout << v;
		}
		return *this;
	}
	
	logger& operator<<(std::ostream& (*pf)(std::ostream&)) {
		if (!is_null) {
			pf(std::cout);
		}
		return *this;
	}
	
private:
	logger(bool is_null) : is_null(is_null) {}

	bool is_null;
	std::vector<bool> on;
};

extern logger LOG;

class sig_entry {
public:
	std::string name;
	int type;
	int length;
	int start;
	int target;

	sig_entry() : type(-1), length(-1), start(-1), target(-1) {}
	
	bool operator==(const sig_entry &e) const {
		return name == e.name && type == e.type && length == e.length && start == e.start && target == e.target;
	}
};

typedef std::vector<sig_entry> state_sig;

#endif
