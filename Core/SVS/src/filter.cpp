#include <sstream>
#include <iterator>
#include <utility>
#include "scene.h"
#include "filter.h"
#include <iostream>

using namespace std;

int DEBUG_DEPTH = 0;

filter_input::~filter_input()
{
    for (int i = 0, iend = input_info.size(); i < iend; ++i)
    {
        delete input_info[i].in_fltr;
    }
}

bool filter_input::update()
{
    enterf("filter_input::update");
    for (int i = 0, iend = input_info.size(); i < iend; ++i)
    {
        if (!input_info[i].in_fltr->update())
        {
            clear();
            return false;
        }
    }
    
    combine(input_info);
    
    for (int i = 0, iend = input_info.size(); i < iend; ++i)
    {
        input_info[i].in_fltr->get_output()->clear_changes();
    }
    
    exitf("filter_input::update");
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
    result.clear();
    clear_sub();
    for (int i = 0, iend = input_info.size(); i < iend; ++i)
    {
        input_info[i].in_fltr->get_output()->reset();
    }
}

void filter_input::reset()
{
    result.reset();
    reset_sub();
}

void filter_input::clear_changes()
{
    result.clear_changes();
}

void concat_filter_input::combine(const input_table& inputs)
{
    enterf("concat_filter_input::combine");
    for (int i = 0, iend = inputs.size(); i < iend; ++i)
    {
        filter_params* p;
        filter_output* o = inputs[i].in_fltr->get_output();
        
        for (int j = o->first_added(), jend = o->num_current(); j < jend; ++j)
        {
            p = new filter_params();
            p->push_back(make_pair(inputs[i].name, o->get_current(j)));
            val2params[o->get_current(j)] = p;
            add(p);
        }
        for (int j = 0, jend = o->num_removed(); j < jend; ++j)
        {
            if (!map_pop(val2params, o->get_removed(j), p))
            {
                assert(false);
            }
            remove(p);
        }
        for (int j = 0, jend = o->num_changed(); j < jend; ++j)
        {
            p = val2params[o->get_changed(j)];
            change(p);
        }
    }
    exitf("concat_filter_input::combine");
}

void concat_filter_input::reset_sub()
{
    val2params.clear();
}

void concat_filter_input::clear_sub()
{
    val2params.clear();
}

#include <iostream>
using namespace std;
void product_filter_input::combine(const input_table& inputs)
{
    val2param_map::iterator k;
    param_set_list::iterator l;
    enterf("product_filter_input::combine");
    for (int i = 0, iend = inputs.size(); i < iend; ++i)
    {
        filter_output* o = inputs[i].in_fltr->get_output();
        
//      cout << padd() << "  " << inputs[i].name << " = " << o << endl;
//      cout << padd() << "  - Current: " << o->num_current() << endl;
//      for(int j = 0; j < o->num_current(); j++){
//          cout << padd() << "    * " << o->get_current(j)->toString() << endl;
//      }
//      cout << padd() << "  - Changed: " << o->num_changed() << endl;
//      for(int j = 0; j < o->num_changed(); j++){
//          cout << padd() << "    * " << o->get_changed(j)->toString() << endl;
//      }
//      cout << padd() << "  - Removed: " << o->num_removed() << endl;
//      for(int j = 0; j < o->num_removed(); j++){
//          cout << padd() << "    * " << o->get_removed(j)->toString() << endl;
//      }

        for (int j = 0, jend = o->num_removed(); j < jend; ++j)
        {
            filter_val* r = o->get_removed(j);
            k = val2params.find(r);
            
            if (k == val2params.end() || val2params.empty())
            {
                cout << padd() << "  COULDNT FIND 1 " << endl;
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
    //cout << padd() << "  finished deleting" << endl;
    
    for (int i = 0, iend = inputs.size(); i < iend; ++i)
    {
        filter_output* o = inputs[i].in_fltr->get_output();
        for (int j = 0, jend = o->num_changed(); j < jend; ++j)
        {
            k = val2params.find(o->get_changed(j));
            if (k == val2params.end() || val2params.empty())
            {
                //  cout << padd() << "  COULDNT FIND 2 " <<  endl;
                continue;
            }
            
            //assert(k != val2params.end());
            for (l = k->second.begin(); l != k->second.end(); ++l)
            {
                change(*l);
            }
        }
    }
    //cout << padd() << "  finished changing" << endl;
    gen_new_combinations(inputs);
    exitf("product_filter_input::combine");
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
    for (int i = 0, iend = inputs.size(); i < iend; ++i)
    {
        vector<int> begin, end;
        bool empty = false;
        for (int j = 0, jend = inputs.size(); j < jend; ++j)
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
        vector<int> curr = begin;
        while (true)
        {
            filter_params* p = new filter_params();
            p->reserve(inputs.size());
            for (int j = 0, jend = inputs.size(); j < jend; ++j)
            {
                filter_val* v = inputs[j].in_fltr->get_output()->get_current(curr[j]);
                p->push_back(make_pair(inputs[j].name, v));
                val2params[v].push_back(p);
            }
            add(p);
            
            int j, jend;
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

filter::filter(Symbol* root, soar_interface* si, filter_input* in)
    : root(root), si(si), status_wme(NULL), input(in)
{
    if (input == NULL)
    {
        input = new null_filter_input();
    }
    if (root && si)
    {
        si->find_child_wme(root, "status", status_wme);
    }
}

filter::~filter()
{
    delete input;
}

void filter::set_status(const std::string& msg)
{
    if (status == msg)
    {
        return;
    }
    status = msg;
    if (status_wme)
    {
        si->remove_wme(status_wme);
    }
    if (root && si)
    {
        status_wme = si->make_wme(root, si->get_common_syms().status, status);
    }
}

void filter::add_output(filter_val* v, const filter_params* p)
{
    enterf("filter::add_output");
    output.add(v);
    output2params[v] = p;
    exitf("filter::add_output");
}

void filter::get_output_params(filter_val* v, const filter_params*& p)
{
    if (!map_get(output2params, v, p))
    {
        p = NULL;
    }
}

void filter::remove_output(filter_val* v)
{
    enterf("filter::remove_output");
    output.remove(v);
    output2params.erase(v);
    exitf("filter::remove_output");
}

void filter::change_output(filter_val* v)
{
    output.change(v);
}

bool filter::update()
{
    enterf("filter::update");
    if (!input->update())
    {
        set_status("Errors in input");
        output.clear();
        output2params.clear();
        return false;
    }
    
    if (!update_outputs())
    {
        output.clear();
        input->reset();
        output2params.clear();
        return false;
    }
    set_status("success");
    input->clear_changes();
    exitf("filter::update");
    return true;
}

map_filter::map_filter(Symbol* root, soar_interface* si, filter_input* input)
    : filter(root, si, input)
{}

bool map_filter::update_outputs()
{
    enterf("map_filter::update_outputs");
    const filter_input* input = get_input();
    std::vector<const filter_params*>::iterator j;
    
    for (int i = input->first_added(); i < input->num_current(); ++i)
    {
        filter_val* v = NULL;
        bool changed = false;
        if (!compute(input->get_current(i), v, changed))
        {
            return false;
        }
        add_output(v, input->get_current(i));
        io_map[input->get_current(i)] = v;
    }
    for (int i = 0; i < input->num_removed(); ++i)
    {
        io_map_t::iterator r = io_map.find(input->get_removed(i));
        assert(r != io_map.end());
        remove_output(r->second);
        output_removed(r->second);
        io_map.erase(r);
    }
    for (int i = 0; i < input->num_changed(); ++i)
    {
        if (!update_one(input->get_changed(i)))
        {
            return false;
        }
    }
    for (j = stale.begin(); j != stale.end(); ++j)
    {
        if (!update_one(*j))
        {
            return false;
        }
    }
    stale.clear();
    exitf("map_filter::update_outputs");
    return true;
}

void map_filter::reset()
{
    io_map.clear();
}

bool map_filter::update_one(const filter_params* params)
{
    filter_val* v = io_map[params];
    bool changed = false;
    if (!compute(params, v, changed))
    {
        return false;
    }
    if (changed)
    {
        change_output(v);
    }
    return true;
}



bool select_filter::update_outputs()
{
    enterf("select_filter::update_outputs");
    const filter_input* input = get_input();
    std::vector<const filter_params*>::iterator j;
    bool error = false;
    
    // Check all added
    for (int i = input->first_added(); i < input->num_current(); ++i)
    {
        //cout << padd() << "New output" << endl;
        const filter_params* params = input->get_current(i);
        bool changed = false;
        filter_val* out = NULL;
        if (!compute(params, out, changed))
        {
            return false;
        }
        if (out != NULL)
        {
            // An output was provided, add it
            //cout << padd() << "Adding output " << endl;
            add_output(out, params);
            io_map[params] = out;
        }
    }
    // Check all removed
    for (int i = 0; i < input->num_removed(); ++i)
    {
        //cout << padd() << "Removed input" << endl;
        const filter_params* params = input->get_removed(i);
        io_map_t::iterator out_it = io_map.find(params);
        if (out_it != io_map.end())
        {
            //cout << padd() << "Delete output" << endl;
            // Only delete if an output value was actually created
            filter_val* out = out_it->second;
            remove_output(out);
            output_removed(out);
            io_map.erase(out_it);
        }
    }
    // Check all changed
    for (int i = 0; i < input->num_changed(); ++i)
    {
        if (!update_one(input->get_changed(i)))
        {
            return false;
        }
    }
    exitf("select_filter::update_outputs");
    return true;
}

bool select_filter::update_one(const filter_params* params)
{
    enterf("select_filter::update_one");
    io_map_t::iterator out_it = io_map.find(params);
    bool is_present = (out_it != io_map.end());
    bool changed = false;
    filter_val* out = NULL;
    if (is_present)
    {
        out = out_it->second;
    }
    if (!compute(params, out, changed))
    {
        return false;
    }
    if (out == NULL && is_present)
    {
        // Have to remove the output
        //cout << padd() << "update_one - remove output" << endl;
        remove_output(out_it->second);
        output_removed(out_it->second);
        io_map.erase(out_it);
    }
    else if (out != NULL && is_present && changed)
    {
        // Update the output
        //cout << padd() << "update_one - change output" << endl;
        change_output(out);
    }
    else if (out != NULL && !is_present)
    {
        // Need to add
        //cout << padd() << "update_one - add output" << endl;
        add_output(out, params);
        io_map[params] = out;
    }
    //cout << padd() << "is_present = " << (is_present ? "T" : "F") << endl;
    //cout << padd() << "changed = " << (changed ? "T" : "F") << endl;
    //cout << padd() << "null out = " << (out == NULL ? "T" : "F") << endl;
    exitf("select_filter::update_one");
    return true;
}

bool rank_filter::update_outputs()
{
    enterf("rank_filter::update_outputs");
    const filter_input* input = get_input();
    double r;
    const filter_params* p;
    for (int i = input->first_added(); i < input->num_current(); ++i)
    {
        p = input->get_current(i);
        if (!rank(p, r))
        {
            return false;
        }
        elems.push_back(make_pair(r, p));
    }
    for (int i = 0; i < input->num_changed(); ++i)
    {
        p = input->get_changed(i);
        if (!rank(p, r))
        {
            return false;
        }
        bool found = false;
        for (int j = 0; j < elems.size(); ++j)
        {
            if (elems[j].second == p)
            {
                elems[j].first = r;
                found = true;
                break;
            }
        }
        assert(found);
    }
    for (int i = 0; i < input->num_removed(); ++i)
    {
        p = input->get_removed(i);
        bool found = false;
        for (int j = 0; j < elems.size(); ++j)
        {
            if (elems[j].second == p)
            {
                elems.erase(elems.begin() + j);
                found = true;
                break;
            }
        }
        assert(found);
    }
    
    if (!elems.empty())
    {
        std::pair<double, const filter_params*> m = *std::max_element(elems.begin(), elems.end());
        if (m.second != old)
        {
            if (output)
            {
                remove_output(output);
            }
            output = new filter_val_c<double>(m.first);
            add_output(output, m.second);
            old = m.second;
        }
        else
        {
            assert(output);
            set_filter_val(output, m.first);
            change_output(output);
        }
    }
    else if (output)
    {
        remove_output(output);
        output = NULL;
    }
    exitf("rank_filter::update_ouputs");
    return true;
}

bool passthru_filter::compute(const filter_params* params, filter_val*& out, bool& changed)
{
    if (params->empty())
    {
        return false;
    }
    if (out == NULL)
    {
        out = params->begin()->second->clone();
        changed = true;
    }
    else
    {
        changed = (*out == *params->begin()->second);
        if (changed)
        {
            *out = *params->begin()->second;
        }
    }
    return true;
}
