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

#include <iostream>
#include <vector>
#include <list>
#include <set>
#include <map>
#include <cassert>
#include "serializable.h"

void serialize(const serializable& v, std::ostream& os);
void unserialize(serializable& v, std::istream& is);

void serialize(char c, std::ostream& os);
void unserialize(char& c, std::istream& is);

void serialize(int v, std::ostream& os);
void serialize(long v, std::ostream& os);
void serialize(std::size_t v, std::ostream& os);
void unserialize(int& v, std::istream& is);

void serialize(bool b, std::ostream& os);
void unserialize(bool& b, std::istream& is);

void serialize(double v, std::ostream& os);
void unserialize(double& v, std::istream& is);

void serialize(const std::string& s, std::ostream& os);
void serialize(const char* s, std::ostream& os);
void unserialize(std::string& s, std::istream& is);

template <typename T>
void unserialize(std::vector<T>& v, std::istream& is);

template <typename T>
void serialize(const std::vector<T>& v, std::ostream& os);

template <typename U, typename V>
void serialize(const std::pair<U, V>& p, std::ostream& os)
{
    os << "( ";
    serialize(p.first, os);
    os << " , ";
    serialize(p.second, os);
    os << " )";
}

template <typename U, typename V>
void unserialize(std::pair<U, V>& p, std::istream& is)
{
    char delim;
    is >> delim;
    assert(delim == '(');
    unserialize(p.first, is);
    is >> delim;
    assert(delim == ',');
    unserialize(p.second, is);
    is >> delim;
    assert(delim == ')');
}

template <typename T>
void serialize(T const* v, std::ostream& os)
{
    if (!v)
    {
        os << "0";
    }
    else
    {
        os << "1 ";
        serialize(*v, os);
    }
}

template <typename T>
void unserialize(T*& v, std::istream& is)
{
    char c;
    is >> c;
    if (c == '0')
    {
        v = NULL;
    }
    else
    {
        assert(c == '1');
        T* t = new T;
        unserialize(*t, is);
        v = t;
    }
}

template <typename T>
void unserialize(T const*& v, std::istream& is)
{
    T*& u = const_cast<T*&>(v);
    unserialize(u, is);
}

template <typename C>
void serialize_container(const C& container, std::ostream& os)
{
    os << container.size() << " [ ";
    typename C::const_iterator i;
    for (i = container.begin(); i != container.end(); ++i)
    {
        serialize(*i, os);
        os << " , ";
    }
    os << ']';
}

template <typename T>
void serialize(const std::vector<T>& v, std::ostream& os)
{
    serialize_container<std::vector<T> >(v, os);
}

template <typename T>
void unserialize(std::vector<T>& v, std::istream& is)
{
    char delim;
    int n = 0;
    if (!(is >> n >> delim) || delim != '[')
    {
        assert(false);
    }
    v.resize(n);
    for (int i = 0; i < n; ++i)
    {
        unserialize(v[i], is);
        if (!(is >> delim) || delim != ',')
        {
            assert(false);
        }
    }
    if (!(is >> delim) || delim != ']')
    {
        assert(false);
    }
}

template <typename T>
void serialize(const std::set<T>& s, std::ostream& os)
{
    serialize_container<std::set<T> >(s, os);
}

template <typename T>
void unserialize(std::set<T>& s, std::istream& is)
{
    char delim;
    int n = 0;
    if (!(is >> n >> delim) || delim != '[')
    {
        assert(false);
    }
    s.clear();
    T elem;
    for (int i = 0; i < n; ++i)
    {
        unserialize(elem, is);
        s.insert(elem);
        if (!(is >> delim) || delim != ',')
        {
            assert(false);
        }
    }
    if (!(is >> delim) || delim != ']')
    {
        assert(false);
    }
    /*
     If elem contains a pointer and doesn't have a custom assignment operator,
     then at this point elem may be holding the same pointer as the last element
     inserted into the set. That pointer may be deallocated by the destructor when
     elem goes out of scope, which would result in an invalid pointer in the set.
     This next line fixes the problem by setting the pointer to something else,
     hopefully NULL.
    */
    elem = T();
}

template <typename T>
void serialize(const std::list<T>& l, std::ostream& os)
{
    serialize_container<std::list<T> >(l, os);
}

template <typename T>
void unserialize(std::list<T>& l, std::istream& is)
{
    char delim;
    int n = 0;
    if (!(is >> n >> delim) || delim != '[')
    {
        assert(false);
    }
    l.clear();
    T elem;
    for (int i = 0; i < n; ++i)
    {
        unserialize(elem, is);
        l.push_back(elem);
        if (!(is >> delim) || delim != ',')
        {
            assert(false);
        }
    }
    if (!(is >> delim) || delim != ']')
    {
        assert(false);
    }
    /*
     Reset key for the same reason as in the set unserializer.
    */
    elem = T();
}

template <typename K, typename V>
void serialize(const std::map<K, V>& m, std::ostream& os)
{
    serialize_container<std::map<K, V> >(m, os);
}

template <typename K, typename V>
void unserialize(std::map<K, V>& m, std::istream& is)
{
    char delim;
    int n = 0;
    if (!(is >> n >> delim) || delim != '[')
    {
        assert(false);
    }
    m.clear();
    typename std::pair<K, V> entry;
    for (int j = 0; j < n; ++j)
    {
        unserialize(entry, is);
        m.insert(entry);
        if (!(is >> delim) || delim != ',')
        {
            assert(false);
        }
    }
    if (!(is >> delim) || delim != ']')
    {
        assert(false);
    }
    /*
     Reset key for the same reason as in the set unserializer.
    */
    entry = std::pair<K, V>();
}

class serializer
{
    public:
        serializer(std::ostream& os) : delim(true), os(os) {}
        
        ~serializer()
        {
            os.put('\n');
        }
        
        template <typename T>
        serializer& operator<<(const T& obj)
        {
            if (!delim)
            {
                os.put(' ');
            }
            ::serialize(obj, os);
            delim = false;
            return *this;
        }
        
        serializer& operator<<(char c);
        
    private:
        std::ostream& os;
        bool delim;
};

class unserializer
{
    public:
        unserializer(std::istream& is) : is(is) {}
        
        template <typename T>
        unserializer& operator>>(T& obj)
        {
            ::unserialize(obj, is);
            return *this;
        }
        
    private:
        std::istream& is;
};


#endif

