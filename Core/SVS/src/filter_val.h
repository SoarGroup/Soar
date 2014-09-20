/*****************************************************************************
 *
 * File: SVS/src/filter_val.h
 *
 * Contains:
 *  class filter_val
 *  class filter_val_c<T>
 *  function  bool get_filter_val (const filter_val *fv, T &v)
 *  function  bool set_filter_val (filter_val *fv, const T &v)
 *
 * filter values uniformly wrap types used in filters with a uniform interface
 * The get and set helper functions are used to wrap and unwrap the values
 *
 * The filter_val_c is a concrete class which templates over the type
 ****************************************************************************/
#ifndef FILTER_VAL_H
#define FILTER_VAL_H

#include <iostream>
#include <string>
#include <sstream>
#include <iterator>

#include "common.h"

class sgnode;

/*
 Wrapper for all filter value types so we can cache them uniformly.
*/
class filter_val
{
    public:
        virtual ~filter_val() {}
        virtual void get_rep(std::map<std::string, std::string>& rep) const = 0;
        virtual filter_val* clone() const = 0;
        virtual filter_val& operator=(const filter_val& rhs) = 0;
        virtual bool operator==(const filter_val& rhs) const = 0;
        virtual std::string toString() const = 0;
        virtual bool is_dirty() const = 0;
        virtual void reset_dirty() = 0;
};


////////////////////////////////////////
// class filter_val_c<T>
//   implements the filter_val interface
//   for most types
/////////////////////////////////////////
template <typename T>
class filter_val_c : public filter_val    // c for concrete
{
    public:
        filter_val_c(const T& v) : v(v), dirty(true) {}
        virtual ~filter_val_c() {}
        
        void get_rep(std::map<std::string, std::string>& rep) const
        {
            rep.clear();
            std::stringstream ss;
            ss << v;
            rep[""] = ss.str();
        }
        
        filter_val* clone() const
        {
            return new filter_val_c<T>(v);
        }
        
        filter_val& operator=(const filter_val& rhs)
        {
            const filter_val_c<T>* c = dynamic_cast<const filter_val_c<T>*>(&rhs);
            assert(c);
            if (v != c->v)
            {
                dirty = true;
            }
            v = c->v;
            return *this;
        }
        
        bool operator==(const filter_val& rhs) const
        {
            const filter_val_c<T>* c = dynamic_cast<const filter_val_c<T>*>(&rhs);
            if (!c)
            {
                return false;
            }
            return v == c->v;
        }
        
        T get_value() const
        {
            return v;
        }
        
        void set_value(const T& n)
        {
            if (v != n)
            {
                dirty = true;
            }
            v = n;
        }
        
        std::string toString() const
        {
            std::stringstream ss;
            ss << v;
            return ss.str();
        }
        
        bool is_dirty() const
        {
            return dirty;
        }
        
        void reset_dirty()
        {
            dirty = false;
        }
        
    private:
        T v;
        bool dirty;
};

template<>
void inline filter_val_c<vec3>::get_rep(std::map<std::string, std::string>& rep) const
{
    rep.clear();
    rep["x"] = tostring(v[0]);
    rep["y"] = tostring(v[1]);
    rep["z"] = tostring(v[2]);
}

template<>
void inline filter_val_c<bbox>::get_rep(std::map<std::string, std::string>& rep) const
{
    rep.clear();
    vec3 min = v.get_min();
    vec3 max = v.get_max();
    rep["min-x"] = tostring(min[0]);
    rep["min-y"] = tostring(min[1]);
    rep["min-z"] = tostring(min[2]);
    rep["max-x"] = tostring(max[0]);
    rep["max-y"] = tostring(max[1]);
    rep["max-z"] = tostring(max[2]);
}



//////////////////////////////////////
// template specialization for sgnode
//////////////////////////////////////

typedef filter_val_c<sgnode*> sgnode_filter_val;

template <>
class filter_val_c<sgnode*> : public filter_val
{
    public:
        filter_val_c(sgnode* v) : v(v), dirty(true) {}
        virtual ~filter_val_c() {}
        
        filter_val* clone() const
        {
            return new sgnode_filter_val(v);
        }
        
        filter_val& operator=(const filter_val& rhs)
        {
            const sgnode_filter_val* c = dynamic_cast<const sgnode_filter_val*>(&rhs);
            assert(c);
            if (v != c->v)
            {
                dirty = true;
            }
            v = c->v;
            return *this;
        }
        
        bool operator==(const filter_val& rhs) const
        {
            const sgnode_filter_val* c = dynamic_cast<const sgnode_filter_val*>(&rhs);
            if (!c)
            {
                return false;
            }
            return v == c->v;
        }
        
        sgnode* get_value() const
        {
            return v;
        }
        
        void set_value(sgnode* n)
        {
            if (v != n)
            {
                dirty = true;
            }
            v = n;
        }
        
        bool is_dirty() const
        {
            return dirty;
        }
        
        void reset_dirty()
        {
            dirty = false;
        }
        
        // Implementation is at top of file filter.cpp
        void get_rep(std::map<std::string, std::string>& rep) const;
        
        // Implementation is at top of file filter.cpp
        std::string toString() const;
        
    private:
        sgnode* v;
        bool dirty;
};

/*
 Convenience functions for getting filter outputs as specific values
 with error checking
 */
template <class T>
inline bool get_filter_val(const filter_val* fv, T& v)
{
    const filter_val_c<T>* cast;
    
    if (!(cast = dynamic_cast<const filter_val_c<T>*>(fv)))
    {
        return false;
    }
    v = cast->get_value();
    return true;
}

/*
 Specialization for floats to allow getting floats, doubles, and ints
*/
template <>
inline bool get_filter_val<double>(const filter_val* fv, double& v)
{
    const filter_val_c<double>* dfv;
    const filter_val_c<float>* ffv;
    const filter_val_c<int>* ifv;
    
    if ((dfv = dynamic_cast<const filter_val_c<double>*>(fv)))
    {
        v = dfv->get_value();
        return true;
    }
    if ((ffv = dynamic_cast<const filter_val_c<float>*>(fv)))
    {
        v = ffv->get_value();
        return true;
    }
    
    if ((ifv = dynamic_cast<const filter_val_c<int>*>(fv)))
    {
        v = ifv->get_value();
        return true;
    }
    
    return false;
}

template <class T>
inline bool set_filter_val(filter_val* fv, const T& v)
{
    filter_val_c<T>* cast;
    
    if (!(cast = dynamic_cast<filter_val_c<T>*>(fv)))
    {
        return false;
    }
    cast->set_value(v);
    return true;
}

#endif
