/*
 This file declares all non-member ::serialize and ::unserialize
 functions for classes from external libraries, such as STL containers
 and Eigen matrices. The template functions defined here allow the
 serialization of most STL containers used in the rest of the code,
 even ones that are nested to arbitrary depths.
 
 Any class you define that needs to be serialized should inherit from
 the pure abstract class "serializable", defined in serializable.h.
*/

#ifndef SERIALIZE_H
#define SERIALIZE_H

#define PRECISION 12

#include <iostream>
#include <vector>
#include <set>
#include <map>
#include <cassert>
#include "serializable.h"

template <typename U, typename V> void serialize(const std::pair<U, V> &p, std::ostream &os);
template <typename U, typename V> void unserialize(std::pair<U, V> &p, std::istream &is);

template <typename T> void serialize(const T* v, std::ostream &os);
template <typename T> void unserialize(T* &v, std::istream &is);

template <typename C> void serialize_container(const C &container, std::ostream &os);
template <typename C> void unserialize_container(C &container, std::istream &is);

template <typename T> void serialize(const std::vector<T> &v, std::ostream &os);
template <typename T> void unserialize(std::vector<T> &v, std::istream &is);

template <typename T> void serialize(const std::set<T> &s, std::ostream &os);
template <typename T> void unserialize(std::set<T> &s, std::istream &is);

template <typename K, typename V> void serialize(const std::map<K, V> &m, std::ostream &os);
template <typename K, typename V> void unserialize(std::map<K, V> &m, std::istream &is);


inline void serialize(const serializable &p, std::ostream &os) {
	p.serialize(os);
}

inline void unserialize(serializable &p, std::istream &is) {
	p.unserialize(is);
}

inline void serialize(int i, std::ostream &os) {
	os << i << std::endl;
}

inline void unserialize(int &i, std::istream &is) {
	if (!(is >> i)) {
		assert(false);
	}
}

inline void serialize(bool b, std::ostream &os) {
	os << (b ? 't' : 'f') << std::endl;
}

inline void unserialize(bool &b, std::istream &is) {
	char c;
	if (!(is >> c)) {
		assert(false);
	}
	assert(c == 't' || c == 'f');
	b = (c == 't');
}

inline void serialize(double v, std::ostream &os) {
	std::ios_base::fmtflags flags = os.flags();
	os << std::scientific;
	int p = os.precision();
	os.precision(PRECISION);
	os << v << std::endl;
	os.precision(p);
	os.flags(flags);
}

inline void unserialize(double &v, std::istream &is) {
	if (!(is >> v)) {
		assert(false);
	}
}

inline void serialize(const std::string &s, std::ostream &os) {
	os << s.size() << std::endl;
	os << s << std::endl;
}

inline void unserialize(std::string &s, std::istream &is) {
	int n;
	is >> n;
	s.resize(n);
	if (is.get() != '\n') {
		assert(false);
	}
	for (int i = 0; i < n; ++i) {
		if (!is.get(s[i])) {
			assert(false);
		}
	}
}

template <typename U, typename V>
void serialize(const std::pair<U, V> &p, std::ostream &os) {
	serialize(p.first, os);
	serialize(p.second, os);
}

template <typename U, typename V>
void unserialize(std::pair<U, V> &p, std::istream &is) {
	unserialize(p.first, is);
	unserialize(p.second, is);
}

template <typename T>
void serialize(T const *v, std::ostream &os) {
	if (!v) {
		os << "0" << std::endl;
	} else {
		os << "1" << std::endl;
		serialize(*v, os);
	}
}

template <typename T>
void unserialize(T *&v, std::istream &is) {
	char c;
	is >> c;
	if (c == '0') {
		v = NULL;
	} else {
		assert(c == '1');
		T *t = new T;
		unserialize(*t, is);
		v = t;
	}
}

template <typename T>
void unserialize(T const *&v, std::istream &is) {
	T *&u = const_cast<T*&>(v);
	unserialize(u, is);
}

template <typename C>
void serialize_container(const C &container, std::ostream &os) {
	os << container.size() << " [" << std::endl;
	typename C::const_iterator i;
	for (i = container.begin(); i != container.end(); ++i) {
		serialize(*i, os);
	}
	os << ']' << std::endl;
}

template <typename C>
void unserialize_container(C &container, std::istream &is) {
	char bracket;
	int n = 0;
	if (!(is >> n >> bracket) || bracket != '[') {
		assert(false);
	}
	container.clear();
	std::insert_iterator<C> i(container, container.end());
	typename C::value_type elem;
	for (int j = 0; j < n; ++j) {
		unserialize(elem, is);
		i = elem;
	}
	if (!(is >> bracket) || bracket != ']') {
		assert(false);
	}
}

template <typename T>
void serialize(const std::vector<T> &v, std::ostream &os) {
	serialize_container<std::vector<T> >(v, os);
}

template <typename T>
void unserialize(std::vector<T> &v, std::istream &is) {
	unserialize_container<std::vector<T> >(v, is);
}

template <typename T>
void serialize(const std::set<T> &s, std::ostream &os) {
	serialize_container<std::set<T> >(s, os);
}

template <typename T>
void unserialize(std::set<T> &s, std::istream &is) {
	unserialize_container<std::set<T> >(s, is);
}

template <typename K, typename V>
void serialize(const std::map<K, V> &m, std::ostream &os) {
	serialize_container<std::map<K,V> >(m, os);
}

/*
 map<K,V>::value_type = pair<const K, V>, so we can't call unserialize(pair<K, V>,
 ...) on it. That means we have to specialize this one differently from
 unserialize_container.
*/
template <typename K, typename V>
void unserialize(std::map<K, V> &m, std::istream &is) {
	char bracket;
	int n = 0;
	if (!(is >> n >> bracket) || bracket != '[') {
		assert(false);
	}
	m.clear();
	for (int j = 0; j < n; ++j) {
		K key;
		V val;
		unserialize(key, is);
		unserialize(val, is);
		m[key] = val;
	}
	if (!(is >> bracket) || bracket != ']') {
		assert(false);
	}
}

class serializer {
public:
	serializer(std::ostream &os) : os(os) {}

	template <typename T>
	serializer &operator<<(const T &obj) {
		::serialize(obj, os);
		return *this;
	}

private:
	std::ostream &os;
};

class unserializer {
public:
	unserializer(std::istream &is) : is(is) {}

	template <typename T>
	unserializer &operator>>(T &obj) {
		::unserialize(obj, is);
		return *this;
	}

private:
	std::istream &is;
};

#endif

