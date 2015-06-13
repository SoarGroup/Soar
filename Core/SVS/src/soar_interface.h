#ifndef SOAR_INTERFACE_H
#define SOAR_INTERFACE_H

#include <utility>
#include <list>
#include <sstream>
#include <vector>
#include "portability.h"
#include "wmem.h"
#include "mat.h"

tc_number get_new_tc_number(agent* thisAgent);
extern Symbol* make_str_constant(agent* thisAgent, char const* name);
extern Symbol* make_int_constant(agent* thisAgent, int64_t value);
extern Symbol* make_float_constant(agent* thisAgent, double value);

typedef std::vector<wme*> wme_list;

class soar_interface;

class common_syms
{
    public:
        common_syms(soar_interface* si);
        ~common_syms();
        
        Symbol* svs, *cmd, *scene, *child, *result, *id, *status;
        
    private:
        soar_interface* si;
};

class soar_interface
{
    public:
        soar_interface(agent* a);
        ~soar_interface();
        
        Symbol*      make_sym(const std::string& val);
        Symbol*      make_sym(int val);
        Symbol*      make_sym(double val);
        void         del_sym(Symbol* s);
        
        wme*         make_id_wme(Symbol* id, const std::string& attr);
        wme*         make_id_wme(Symbol* id, Symbol* attr);
        
        wme*         make_wme(Symbol* id, Symbol* attr, Symbol* val);
        wme*         make_wme(Symbol* id, const std::string& attr, Symbol* val);
        
        template<class T>
        wme*         make_wme(Symbol* id, const std::string& attr, const T& val);
        
        template<class T>
        wme*         make_wme(Symbol* id, Symbol* attr, const T& val);
        
        void         remove_wme(wme* w);
        bool         get_child_wmes(Symbol* id, wme_list& childs);
        bool         find_child_wme(Symbol* id, const std::string& attr, wme*& w);
        
        template<class T>
        bool         get_const_attr(Symbol* id, const std::string& attr, T& val);
        
        bool         get_vec3(Symbol* id, const std::string& attr, vec3& val);
        
        Symbol*      get_wme_id(wme* w);
        Symbol*      get_wme_attr(wme* w);
        Symbol*      get_wme_val(wme* w);
        
        tc_number    new_tc_num();
        
        uint64_t          get_timetag(wme* w);
        common_syms& get_common_syms()
        {
            return cs;
        }
        
        void         print(const std::string& msg);
        
    private:
        agent* thisAgent;
        common_syms cs;
};

inline tc_number soar_interface::new_tc_num()
{
    return get_new_tc_number(thisAgent);
}

inline Symbol* soar_interface::make_sym(const std::string& val)
{
    return make_str_constant(thisAgent, val.c_str());
}

inline Symbol* soar_interface::make_sym(int val)
{
    return make_int_constant(thisAgent, val);
}

inline Symbol* soar_interface::make_sym(double val)
{
    return make_float_constant(thisAgent, val);
}

template<class T>
wme* soar_interface::make_wme(Symbol* id, const std::string& attr, const T& val)
{
    Symbol* valsym = make_sym(val);
    return make_wme(id, attr, valsym);
}

template<class T>
wme* soar_interface::make_wme(Symbol* id, Symbol* attr, const T& val)
{
    Symbol* valsym = make_sym(val);
    return make_wme(id, attr, valsym);
}

inline Symbol* soar_interface::get_wme_id(wme* w)
{
    return w->id;
}

inline Symbol* soar_interface::get_wme_attr(wme* w)
{
    return w->attr;
}

inline Symbol* soar_interface::get_wme_val(wme* w)
{
    return w->value;
}

inline uint64_t soar_interface::get_timetag(wme* w)
{
    return w->timetag;
}

template<class T>
bool soar_interface::get_const_attr(Symbol* id, const std::string& attr, T& val)
{
    wme* w;
    if (!find_child_wme(id, attr, w))
    {
        return false;
    }
    return get_symbol_value(get_wme_val(w), val);
}

#endif
