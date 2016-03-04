#include <stdlib.h>
#include <ctype.h>
#include <sstream>
#include <limits>
#include <stack>

#include "command.h"
#include "filter.h"
#include "filter_table.h"
#include "svs.h"
#include "scene.h"
#include "soar_interface.h"

#include "symbol.h"

using namespace std;

bool is_reserved_param(const string& name)
{
    return name == "result" || name == "parent";
}

/* Remove weird characters from string */
void cleanstring(string& s)
{
    string::iterator i;
    for (i = s.begin(); i != s.end();)
    {
        if (!isalnum(*i) && *i != '.' && *i != '-' && *i != '_')
        {
            i = s.erase(i);
        }
        else
        {
            ++i;
        }
    }
}

command::command(svs_state* state, Symbol* cmd_root)
    : state(state), si(state->get_svs()->get_soar_interface()), root(cmd_root),
      subtree_size(0), prev_max_time(-1), status_wme(NULL), first(true)
{}

command::~command() {}

bool command::changed()
{
    size_t size;
    uint64_t max_time;
    parse_substructure(size, max_time);
    if (first || size != subtree_size || max_time > prev_max_time)
    {
        first = false;
        subtree_size = size;
        prev_max_time = max_time;
        return true;
    }
    return false;
}

void command::parse_substructure(size_t& size, uint64_t& max_time)
{
    tc_number tc;
    stack< Symbol*> to_process;
    wme_vector childs;
    wme_vector::iterator i;
    Symbol* parent, *v;
    uint64_t tt;
    string attr;
    
    tc = si->new_tc_num();
    size = 0;
    max_time = -1;
    to_process.push(root);
    while (!to_process.empty())
    {
        parent = to_process.top();
        to_process.pop();
        
        si->get_child_wmes(parent, childs);
        for (i = childs.begin(); i != childs.end(); ++i)
        {
            if (get_symbol_value(si->get_wme_attr(*i), attr) &&
                    (attr == "result" || attr == "status"))
            {
                /* result and status wmes are added by svs */
                continue;
            }
            v = si->get_wme_val(*i);
            tt = si->get_timetag(*i);
            size++;
            
            if (tt > max_time)
            {
                max_time = tt;
            }
            
            if (v->is_identifier() && (v->get_tc_num() != tc))
            {
                v->set_tc_num(tc);
                to_process.push(v);
            }
        }
    }
}

bool command::get_str_param(const string& name, string& val)
{
    wme_vector children;
    wme_vector::iterator i;
    string attr, v;
    
    si->get_child_wmes(root, children);
    for (i = children.begin(); i != children.end(); ++i)
    {
        if (get_symbol_value(si->get_wme_attr(*i), attr))
        {
            if (name != attr)
            {
                continue;
            }
            if (get_symbol_value(si->get_wme_val(*i), v))
            {
                val = v;
                return true;
            }
        }
    }
    return false;
}

void command::set_status(const string& s)
{
    if (curr_status == s)
    {
        return;
    }
    if (status_wme)
    {
        si->remove_wme(status_wme);
    }
    status_wme = si->make_wme(root, "status", s);
    curr_status = s;
}

