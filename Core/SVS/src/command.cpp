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

#include "symtab.h"

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
    int size, max_time;
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

void command::parse_substructure(int& size, int& max_time)
{
    tc_number tc;
    stack< Symbol*> to_process;
    wme_list childs;
    wme_list::iterator i;
    Symbol* parent, *v;
    int tt;
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
    wme_list children;
    wme_list::iterator i;
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

void command::proxy_get_children(map<string, cliproxy*>& c)
{
    c["timers"] = &timers;
}

command* _make_extract_command_(svs_state* state, Symbol* root);
command* _make_project_command_(svs_state* state, Symbol* root);
command* _make_extract_once_command_(svs_state* state, Symbol* root);
command* _make_add_node_command_(svs_state* state, Symbol* root);
command* _make_create_model_command_(svs_state* state, Symbol* root);
command* _make_assign_model_command_(svs_state* state, Symbol* root);
command* _make_property_command_(svs_state* state, Symbol* root);
command* _make_seek_command_(svs_state* state, Symbol* root);
command* _make_random_control_command_(svs_state* state, Symbol* root);
command* _make_copy_node_command_(svs_state* state, Symbol* root);
command* _make_del_node_command_(svs_state* state, Symbol* root);

command* make_command(svs_state* state, wme* w)
{
    string name;
    Symbol* id;
    soar_interface* si;

    si = state->get_svs()->get_soar_interface();
    if (!get_symbol_value(si->get_wme_attr(w), name))
    {
        return NULL;
    }
    if (!si->get_wme_val(w)->is_identifier())
    {
        return NULL;
    }
    id = si->get_wme_val(w);
    if (name == "extract")
    {
        return _make_extract_command_(state, id);
    }
    else if (name == "project")
    {
        return _make_project_command_(state, id);
    }
    else if (name == "extract_once")
    {
        return _make_extract_once_command_(state, id);
    }
    else if (name == "add_node")
    {
        return _make_add_node_command_(state, id);
    }
    else if (name == "seek")
    {
        return _make_seek_command_(state, id);
    }
    else if (name == "random_control")
    {
        return _make_random_control_command_(state, id);
    }
    else if (name == "create-model")
    {
        return _make_create_model_command_(state, id);
    }
    else if (name == "assign-model")
    {
        return _make_assign_model_command_(state, id);
    }
    else if (name == "property")
    {
        return _make_property_command_(state, id);
    }
    else if (name == "copy_node")
    {
        return _make_copy_node_command_(state, id);
    }
    else if (name == "del_node")
    {
        return _make_del_node_command_(state, id);
    }
    return NULL;
}

/*
 Example input:

 (<ot> ^type on-top ^a <ota> ^b <otb>)
 (<ota> ^type node ^name box1)
 (<otb> ^type node ^name box2)
*/
filter* parse_filter_spec(soar_interface* si, Symbol* root, scene* scn)
{
    wme_list children, params;
    wme_list::iterator i;
    string pname, ftype, itype;
    filter_input* input;
    bool fail;
    filter* f;

    if (!root->is_identifier())
    {
        string strval;
        long intval;
        double floatval;

        if (get_symbol_value(root, strval))
        {
            return new const_filter<string>(strval);
        }
        else if (get_symbol_value(root, intval))
        {
            return new const_filter<int>(intval);
        }
        else if (get_symbol_value(root, floatval))
        {
            return new const_filter<double>(floatval);
        }
        return NULL;
    }

    fail = false;
    si->get_child_wmes(root, children);
    for (i = children.begin(); i != children.end(); ++i)
    {
        if (!get_symbol_value(si->get_wme_attr(*i), pname))
        {
            continue;
        }
        Symbol* cval = si->get_wme_val(*i);
        if (pname == "type")
        {
            if (!get_symbol_value(cval, ftype))
            {
                return NULL;
            }
        }
        else if (pname == "input-type")
        {
            if (!get_symbol_value(cval, itype))
            {
                return NULL;
            }
        }
        else if (pname != "status" && pname != "result")
        {
            params.push_back(*i);
        }
    }

    // The combine type check is a bit of a hack
    if (itype == "concat" || ftype == "combine")
    {
        input = new concat_filter_input();
    }
    else if (params.size() == 0)
    {
        input = new null_filter_input();
    }
    else
    {
        input = new product_filter_input();
    }

    for (i = params.begin(); i != params.end(); ++i)
    {
        if (!get_symbol_value(si->get_wme_attr(*i), pname))
        {
            continue;
        }
        Symbol* cval = si->get_wme_val(*i);
        filter* cf = parse_filter_spec(si, cval, scn);
        if (!cf)
        {
            fail = true;
            break;
        }
        input->add_param(pname, cf);
    }

    if (!fail)
    {
        if (ftype == "combine")
        {
            f = new passthru_filter(root, si, input);
        }
        else
        {
            f = get_filter_table().make_filter(ftype, root, si, scn, input);
        }
    }

    if (fail || ftype == "" || f == NULL)
    {
        delete input;
        return NULL;
    }
    return f;
}
