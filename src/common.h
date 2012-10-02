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
void subtract_sets_inplace(std::set<T> &s1, const std::set<T> &s2) {
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

template<typename A, typename B>
void extend(A &c1, const B &c2) {
	std::insert_iterator<A> i(c1, c1.end());
	typename B::const_iterator j;
	for (j = c2.begin(); j != c2.end(); ++j) {
		i = *j;
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

std::ostream &histogram(const std::vector<double> &vals, int nbins, std::ostream &os);

inline double gausspdf(double x, double mean, double var) {
	const double SQRT2PI = 2.5066282746310002;
	return (1. / sqrt(var) * SQRT2PI) * exp(-((x - mean) * (x - mean) / (2 * var)));
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
	logger(std::ostream &os) : os(os), is_null(false) {
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
		static logger null_logger(os, true);
		
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
			os << v;
		}
		return *this;
	}
	
	logger& operator<<(std::ostream& (*pf)(std::ostream&)) {
		if (!is_null) {
			pf(os);
		}
		return *this;
	}

	std::ostream &get_ostream() { return os; }
	
private:
	logger(std::ostream &os, bool is_null) : os(os), is_null(is_null) {}

	bool is_null;
	std::vector<bool> on;
	std::ostream &os;
};

extern logger LOG;

#endif
