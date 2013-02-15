#ifndef COMMON_H
#define COMMON_H

#include <iostream>
#include <cstdlib>
#include <cmath>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <ostream>
#include <fstream>
#include <sstream>
#include <cassert>

typedef std::vector<int> tuple;

std::string get_option(const std::string &key);

void split(const std::string &s, const std::string &delim, std::vector<std::string> &fields);
void strip(std::string &s, const std::string &whitespace);

bool parse_double(const std::string &s, double &v);
bool parse_int   (const std::string &s, int &v);

bool read_on_off(const std::vector<std::string> &args, int first, std::ostream &os, bool &var);

void sample(int k, int low, int high, std::vector<int> &output);

template<typename C>
void sample(int k, const C &input, C &output) {
	std::vector<int> inds;
	sample(k, 0, input.size() - 1, inds);
	std::sort(inds.begin(), inds.end());
	
	typename C::const_iterator i = input.begin();
	std::insert_iterator<C> j(output, output.end());
	for (int n = 0; n < inds.size(); ++n) {
		if (n == 0) {
			std::advance(i, inds[0]);
		} else {
			std::advance(i, inds[n] - inds[n-1]);
		}
		j = *i;
	}
}

template <typename T>
T &grow_vec(std::vector<T> &v) {
	v.resize(v.size() + 1);
	return v.back();
}

template <typename A, typename B>
inline B &map_get(std::map<A, B> &m, const A &key) {
	typename std::map<A, B>::iterator i = m.find(key);
	if (i == m.end()) {
		assert(false);
	}
	return i->second;
}

template <typename A, typename B>
inline const B &map_get(const std::map<A, B> &m, const A &key) {
	typename std::map<A, B>::const_iterator i = m.find(key);
	if (i == m.end()) {
		assert(false);
	}
	return i->second;
}

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
inline const B *map_getp(const std::map<A, B> &m, const A &key) {
	typename std::map<A, B>::const_iterator i = m.find(key);
	if (i == m.end()) {
		return NULL;
	}
	return &i->second;
}

template <typename A, typename B>
inline B *map_getp(std::map<A, B> &m, const A &key) {
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
inline bool has(const std::map<A, B> &m, const A &key) {
	return m.find(key) != m.end();
}

template<typename T>
inline bool has(const std::vector<T> &v, const T &e) {
	return std::find(v.begin(), v.end(), e) != v.end();
}

template<typename T>
bool has(const std::set<T> &s, const T &x) {
	return s.find(x) != s.end();
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

template <typename C>
void clear_and_dealloc(C &container) {
	typename C::iterator i, end = container.end();
	for (i = container.begin(); i != end; ++i) {
		delete *i;
	}
	container.clear();
}

std::ostream &histogram(const std::vector<double> &vals, int nbins, std::ostream &os);

inline double gausspdf(double x, double mean, double var) {
	const double SQRT2PI = 2.5066282746310002;
	return (1. / (sqrt(var) * SQRT2PI)) * exp(-((x - mean) * (x - mean) / (2 * var)));
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

class table_printer {
public:
	table_printer() {}
	table_printer &add_row();
	table_printer &skip(int n);
	void set_precision(int p);
	void set_scientific(bool s);
	void print(std::ostream &os) const;
	
	template<typename T>
	table_printer &operator<<(const T &x) {
		ss.str("");
		ss << x;
		rows.back().push_back(ss.str());
		return *this;
	}
	
	/*
	 To save space, print integers when possible
	*/
	table_printer &operator<<(double x) {
		ss.str("");
		long long integral = x;
		if (integral == x) {
			ss << integral;
		} else {
			ss << x;
		}
		rows.back().push_back(ss.str());
		return *this;
	}
	
	template<typename T>
	table_printer &set(int r, int c, const T &x) {
		if (r >= rows.size()) {
			rows.resize(r + 1);
		}
		std::vector<std::string> &row = rows[r];
		if (c >= row.size()) {
			row.resize(c + 1);
		}
		ss.str("");
		ss << x;
		row[c] = ss.str();
		return *this;
	}
	
	template<typename C>
	table_printer &add(const C &container) {
		std::vector<std::string> &r = rows.back();
		r.reserve(container.size());
		typename C::const_iterator i;
		for (i = container.begin(); i != container.end(); ++i) {
			ss.str("");
			ss << *i;
			r.push_back(ss.str());
		}
		return *this;
	}

private:
	std::stringstream ss;
	std::vector<std::vector<std::string> > rows;
};

#endif
