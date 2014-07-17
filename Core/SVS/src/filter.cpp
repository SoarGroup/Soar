#include "filter.h"

#include <sstream>
#include <iterator>
#include <utility>
#include <iostream>

#include "scene.h"
#include "sgnode.h"

using namespace std;

/**********************************************
 * member functions for filter_val_c<sgnode*>
 ***********************************************/
void filter_val_c<const sgnode*>::get_rep(map<string, string>& rep) const
{
    rep.clear();
    rep[""] = v->get_name();
}

string filter_val_c<const sgnode*>::toString() const
{
    stringstream ss;
    ss << v;
    return ss.str();
}

void filter_val_c<sgnode*>::get_rep(map<string, string>& rep) const
{
    rep.clear();
    rep[""] = v->get_name();
}

string filter_val_c<sgnode*>::toString() const
{
    stringstream ss;
    ss << v;
    return ss.str();
}

void filter_output::add(filter_val* fv)
{
    change_tracking_list<filter_val>::add(fv);
    sgnode_filter_val* nodefv = dynamic_cast<sgnode_filter_val*>(fv);
    if (nodefv != 0)
    {
        sgnode* node = nodefv->get_value();
        node_map[node] = fv;
        node->listen(this);
    }
}

void filter_output::remove(filter_val* fv)
{
    sgnode_filter_val* nodefv = dynamic_cast<sgnode_filter_val*>(fv);
    if (nodefv != 0)
    {
        sgnode* node = nodefv->get_value();
        node->unlisten(this);
        node_filter_val_map::iterator i = node_map.find(node);
        if (i != node_map.end())
        {
            node_map.erase(i);
        }
    }
    change_tracking_list<filter_val>::remove(fv);
}

void filter_output::clear()
{
    for (node_filter_val_map::iterator i = node_map.begin(); i != node_map.end(); i++)
    {
        i->first->unlisten(this);
    }
    node_map.clear();
    change_tracking_list<filter_val>::clear();
}

void filter_output::node_update(sgnode* n, sgnode::change_type t, const std::string& update_info)
{
    node_filter_val_map::const_iterator i = node_map.find(n);
    if (i == node_map.end())
    {
        return;
    }
    const filter_val* fv = i->second;

    switch (t)
    {
        case sgnode::TRANSFORM_CHANGED:
        case sgnode::SHAPE_CHANGED:
        case sgnode::PROPERTY_CHANGED:
        case sgnode::PROPERTY_DELETED:
            change(fv);
    }
}

/*********
 * filter
 ********/

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

void filter::set_status(const string& msg)
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
    output.add(v);
    output2params[v] = p;
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
    output.remove(v);
    output2params.erase(v);
}

void filter::change_output(filter_val* v)
{
    output.change(v);
}

bool filter::update()
{
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
    return true;
}

map_filter::map_filter(Symbol* root, soar_interface* si, filter_input* input)
    : filter(root, si, input)
{}

bool map_filter::update_outputs()
{
    const filter_input* input = get_input();
    vector<const filter_params*>::iterator j;

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
    const filter_input* input = get_input();
    vector<const filter_params*>::iterator j;
    bool error = false;

    // Check all added
    for (int i = input->first_added(); i < input->num_current(); ++i)
    {
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
            add_output(out, params);
            io_map[params] = out;
        }
    }
    // Check all removed
    for (int i = 0; i < input->num_removed(); ++i)
    {
        const filter_params* params = input->get_removed(i);
        io_map_t::iterator out_it = io_map.find(params);
        if (out_it != io_map.end())
        {
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
    return true;
}

bool select_filter::update_one(const filter_params* params)
{
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
        remove_output(out_it->second);
        output_removed(out_it->second);
        io_map.erase(out_it);
    }
    else if (out != NULL && is_present && changed)
    {
        // Update the output
        change_output(out);
    }
    else if (out != NULL && !is_present)
    {
        // Need to add
        add_output(out, params);
        io_map[params] = out;
    }
    return true;
}

bool rank_filter::update_outputs()
{
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
        pair<double, const filter_params*> m = *max_element(elems.begin(), elems.end());
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
