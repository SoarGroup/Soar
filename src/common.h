#ifndef COMMON_H
#define COMMON_H

#include <iostream>
#include <cstdlib>
#include <cmath>
#include <cassert>
#include <cstring>
#include <sys/time.h>
#include <string>
#include <vector>
#include <algorithm>
#include <ostream>
#include <fstream>
#include <map>
#include <Eigen>

#include "linalg.h"

typedef Eigen::RowVectorXd rvec;
typedef Eigen::VectorXd cvec;
typedef Eigen::MatrixXd mat;
typedef Eigen::MatrixXi imat;

void split(const std::string &s, const std::string &delim, std::vector<std::string> &fields);

/* I need all my files to have access to a single ofstream */
std::ofstream& get_datavis();

#if 1
#define DATAVIS(x) get_datavis() << x;
#else
#define DATAVIS(x)
#endif

class timer {
public:
	timer() : name("") {}

	timer(const std::string &name) : name(name) {
		gettimeofday(&t1, NULL);
	}
	
	~timer() {
		if (!name.empty()) {
			std::cerr << "TIMER " << name << ": " << stop() << std::endl;
		}
	}

	void start() {
		gettimeofday(&t1, NULL);
	}
	
	double stop() {
		timeval t2, t3;
		gettimeofday(&t2, NULL);
		timersub(&t2, &t1, &t3);
		return t3.tv_sec + t3.tv_usec / 1000000.0;
	}
	
private:
	std::string name;
	timeval t1;
};

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
inline bool map_pop(std::map<A, B> &m, const A &key, B &val) {
	typename std::map<A, B>::iterator i = m.find(key);
	if (i == m.end()) {
		return false;
	}
	val = i->second;
	m.erase(i);
	return true;
}


class namedvec {
public:
	namedvec() {}
	
	namedvec(const namedvec &v) 
	: vals(v.vals), name2ind(v.name2ind), ind2name(v.ind2name)
	{}
	
	namedvec(std::vector<std::string> &names) 
	: vals(names.size())
	{
		for (int i = 0; i < names.size(); ++i) {
			name2ind[names[i]] = i;
			ind2name[i] = names[i];
		}
	}
	
	void set_name(int index, const std::string &name) {
		name2ind[name] = index;
		ind2name[index] = name;
	}
	
	void set_names(const std::vector<std::string> &names) {
		for (int i = 0; i < names.size(); ++i) {
			name2ind[names[i]] = i;
			ind2name[i] = names[i];
		}
	}
	
	void add_name(const std::string &name, float v) {
		name2ind[name] = vals.size();
		ind2name[vals.size()] = name;
		vals.resize(vals.size() + 1);
		vals[vals.size() - 1] = v;
	}
	
	bool get_name(int index, std::string &name) const {
		return map_get(ind2name, index, name);
	}
	
	bool get_by_name(const std::string &name, float &val) const {
		int index;
		if (!map_get(name2ind, name, index)) {
			return false;
		}
		val = vals[index];
		return true;
	}
	
	bool set_by_name(const std::string &name, float val) {
		int index;
		if (!map_get(name2ind, name, index)) {
			return false;
		}
		vals[index] = val;
		return true;
	}
	
	bool congruent(const namedvec &v) {
		return name2ind == v.name2ind;
	}
	
	void operator=(const namedvec &v) {
		vals = v.vals;
		name2ind = v.name2ind;
		ind2name = v.ind2name;
	}
	
	int size() const {
		return vals.size();
	}
	
	void get_names(std::vector<std::string> &names) const {
		std::map<int, std::string>::const_iterator i;
		
		names.clear();
		names.reserve(ind2name.size());
		for (i = ind2name.begin(); i != ind2name.end(); ++i) {
			names.push_back(i->second);
		}
	}
	
	void clear() {
		vals.resize(0);
		name2ind.clear();
		ind2name.clear();
	}
	
	rvec vals;
	
private:
	std::map<std::string, int> name2ind;
	std::map<int, std::string> ind2name;
};

std::ostream &operator<<(std::ostream &os, const namedvec &v);

vec3 calc_centroid(const ptlist &pts);

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

void histogram(const rvec &vals, int nbins);

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
	
	bbox(ptlist &pts) {
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
	
	void include(vec3 &v) {
		for(int d = 0; d < 3; ++d) {
			if (v[d] < min[d]) { min[d] = v[d]; }
			if (v[d] > max[d]) { max[d] = v[d]; }
		}
	}
	
	void include(ptlist &pts) {
		ptlist::iterator i;
		for(i = pts.begin(); i != pts.end(); ++i) {
			include(*i);
		}
	}
	
	bool intersects(bbox &b) const {
		int d;
		for (d = 0; d < 3; ++d) {
			if (max[d] < b.min[d] || min[d] > b.max[d]) {
				return false;
			}
		}
		return true;
	}
	
	bool contains(bbox &b) const {
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
	
	friend std::ostream& operator<<(std::ostream &os, const bbox &b);
	
private:
	vec3 min;
	vec3 max;
};

std::ostream& operator<<(std::ostream &os, const bbox &b);

template <class T>
void save_vector(const std::vector<T> &v, std::ostream &os) {
	os << v.size() << std::endl;
	std::copy(v.begin(), v.end(), std::ostream_iterator<T>(os, " "));
	os << std::endl;
}

template <class T>
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

void save_mat(std::ostream &os, const mat &m);
void load_mat(std::istream &is, mat &m);
void save_imat(std::ostream &os, const imat &m);
void load_imat(std::istream &is, imat &m);
void save_rvec(std::ostream &os, const rvec &v);
void load_rvec(std::istream &is, rvec &v);
void save_cvec(std::ostream &os, const cvec &v);
void load_cvec(std::istream &is, cvec &v);

inline double gausspdf(double x, double mean, double std) {
	const double SQRT2PI = 2.5066282746310002;
	return (1. / std * SQRT2PI) * exp(-((x - mean) * (x - mean) / (2 * std * std)));
}

inline void randomize_vec(rvec &v, const rvec &min, const rvec &max) {
	v = min.array() + (rvec::Random(v.size()).array() * (max - min).array());
}

std::string get_option(const std::string &key);

#endif
