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
#include "serializable.h"

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

inline double gausspdf(double x, double mean, double std) {
	const double SQRT2PI = 2.5066282746310002;
	return (1. / std * SQRT2PI) * exp(-((x - mean) * (x - mean) / (2 * std * std)));
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

class sig_entry : public serializable {
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
	
	void serialize(std::ostream &os) const;
	void unserialize(std::istream &is);
};

typedef std::vector<sig_entry> state_sig;

#endif
