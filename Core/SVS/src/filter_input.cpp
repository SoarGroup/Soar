#include "filter.h"
#include "filter_input.h"

#include <sstream>
#include <iterator>
#include <utility>
#include "scene.h"
#include <iostream>

using namespace std;

/*********************************************************
 * class filter_input
 ********************************************************/


filter_input::~filter_input()
{
    for (size_t i = 0, iend = input_info.size(); i < iend; ++i)
    {
        delete input_info[i].in_fltr;
    }
}

bool filter_input::update()
{
    for (size_t i = 0, iend = input_info.size(); i < iend; ++i)
    {
        if (!input_info[i].in_fltr->update())
        {
            clear();
            return false;
        }
    }
    
    combine(input_info);
    
    for (size_t i = 0, iend = input_info.size(); i < iend; ++i)
    {
        input_info[i].in_fltr->get_output()->clear_changes();
    }
    
    return true;
}

void filter_input::add_param(string name, filter* in_fltr)
{
    param_info i;
    i.name = name;
    i.in_fltr = in_fltr;
    input_info.push_back(i);
}

void filter_input::clear()
{
    change_tracking_list<filter_params>::clear();
    for (size_t i = 0, iend = input_info.size(); i < iend; ++i)
    {
        input_info[i].in_fltr->get_output()->reset();
    }
}

/*********************************************************
 * class concat_filter_input
 ********************************************************/

void concat_filter_input::combine(const input_table& inputs)
{
    for (size_t i = 0, iend = inputs.size(); i < iend; ++i)
    {
        filter_params* p;
        filter_output* o = inputs[i].in_fltr->get_output();
        
        for (size_t j = o->first_added(), jend = o->num_current(); j < jend; ++j)
        {
            p = new filter_params();
            p->push_back(make_pair(inputs[i].name, o->get_current(j)));
            val2params[o->get_current(j)] = p;
            add(p);
        }
        for (size_t j = 0, jend = o->num_removed(); j < jend; ++j)
        {
            if (!map_pop(val2params, o->get_removed(j), p))
            {
                assert(false);
            }
            remove(p);
        }
        for (size_t j = 0, jend = o->num_changed(); j < jend; ++j)
        {
            p = val2params[o->get_changed(j)];
            change(p);
        }
    }
}

void concat_filter_input::reset_sub()
{
    val2params.clear();
}

void concat_filter_input::clear_sub()
{
    val2params.clear();
}

/*********************************************************
 * class product_filter_input
 ********************************************************/
void product_filter_input::combine(const input_table& inputs)
{
    val2param_map::iterator k;
    param_set_list::iterator l;
    for (size_t i = 0, iend = inputs.size(); i < iend; ++i)
    {
        filter_output* o = inputs[i].in_fltr->get_output();
        
        for (size_t j = 0, jend = o->num_removed(); j < jend; ++j)
        {
            filter_val* r = o->get_removed(j);
            k = val2params.find(r);
            
            if (k == val2params.end() || val2params.empty())
            {
                continue;
            }
            //assert(k != val2params.end());
            
            param_set_list temp = k->second;
            for (l = temp.begin(); l != temp.end(); ++l)
            {
                remove(*l);
                erase_param_set(*l);
            }
            val2params.erase(k);
        }
    }
    
    for (size_t i = 0, iend = inputs.size(); i < iend; ++i)
    {
        filter_output* o = inputs[i].in_fltr->get_output();
        for (size_t j = 0, jend = o->num_changed(); j < jend; ++j)
        {
            k = val2params.find(o->get_changed(j));
            if (k == val2params.end() || val2params.empty())
            {
                continue;
            }
            
            //assert(k != val2params.end());
            for (l = k->second.begin(); l != k->second.end(); ++l)
            {
                change(*l);
            }
        }
    }
    gen_new_combinations(inputs);
}

void product_filter_input::reset_sub()
{
    val2params.clear();
}

void product_filter_input::clear_sub()
{
    val2params.clear();
}

/*
 Generate all combinations of inputs that involve at least one new
 input.  Do this by iterating over the input lists.  For the i^th
 input list, take the cartesian product of the old inputs from lists
 0..(i-1), the new inputs of list i, and both old and new inputs from
 lists (i+1)..n.  This will avoid generating duplicates.  I'm assuming
 that new inputs are at the end of each list.
*/
void product_filter_input::gen_new_combinations(const input_table& inputs)
{
    for (size_t i = 0, iend = inputs.size(); i < iend; ++i)
    {
        vector<size_t> begin, end;
        bool empty = false;
        for (size_t j = 0, jend = inputs.size(); j < jend; ++j)
        {
            filter_output* o = inputs[j].in_fltr->get_output();
            if (j < i)
            {
                begin.push_back(0);
                end.push_back(o->first_added()); // same as end of old inputs
            }
            else if (j == i)
            {
                begin.push_back(o->first_added());
                end.push_back(o->num_current());
            }
            else
            {
                begin.push_back(0);
                end.push_back(o->num_current());
            }
            if (begin.back() == end.back())
            {
                empty = true;
                break;
            }
        }
        if (empty)
        {
            continue;
        }
        vector<size_t> curr = begin;
        while (true)
        {
            filter_params* p = new filter_params();
            p->reserve(inputs.size());
            for (size_t j = 0, jend = inputs.size(); j < jend; ++j)
            {
                filter_val* v = inputs[j].in_fltr->get_output()->get_current(curr[j]);
                p->push_back(make_pair(inputs[j].name, v));
                val2params[v].push_back(p);
            }
            add(p);
            
            size_t j, jend;
            for (j = 0, jend = curr.size(); j < jend && ++curr[j] == end[j]; ++j)
            {
                curr[j] = begin[j];
            }
            if (j == curr.size())
            {
                return;
            }
        }
    }
}

void product_filter_input::erase_param_set(filter_params* s)
{
    filter_params::const_iterator i;
    for (i = s->begin(); i != s->end(); ++i)
    {
        param_set_list& l = val2params[i->second];
        l.erase(find(l.begin(), l.end(), s));
    }
}
