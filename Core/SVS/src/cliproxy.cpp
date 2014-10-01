#include "cliproxy.h"
#include "common.h"

using namespace std;

bool partition(const string& s, string& first, string& rest)
{
    size_t i = s.find('.');
    if (i == string::npos)
    {
        first = s;
        rest.clear();
        return true;
    }
    first = s.substr(0, i);
    rest = s.substr(i + 1);
    return false;
}

cliproxy::~cliproxy() {}

void cliproxy::proxy_use(const string& path, const vector<std::string>& args, std::ostream& os)
{

    if (path.empty() || path == ".")
    {
        if (args.size() > 0 && args[0] == "help")
        {
            print_help(os);
        }
        else if (args.size() > 0 && args[0] == "dir")
        {
            list_children(0, os);
        }
        else
        {
            proxy_use_sub(args, os);
        }
    }
    else
    {
        string child, rest;
        map<string, cliproxy*> c;
        
        partition(path, child, rest);
        proxy_get_children(c);
        if (has(c, child))
        {
            c[child]->proxy_use(rest, args, os);
        }
        else
        {
            os << "path not found" << endl;
        }
        
        map<string, cliproxy*>::const_iterator i, iend;
        for (i = c.begin(), iend = c.end(); i != iend; ++i)
        {
            if (i->second->temporary())
            {
                delete i->second;
            }
        }
    }
}

void cliproxy::list_children(int level, std::ostream& os)
{
    map<string, cliproxy*> c;
    map<string, cliproxy*>::const_iterator i, iend;
    
    proxy_get_children(c);
    for (i = c.begin(), iend = c.end(); i != iend; ++i)
    {
        for (int j = 0; j < level; ++j)
        {
            os << "  ";
        }
        os << i->first << endl;
        i->second->list_children(level + 1, os);
        if (i->second->temporary())
        {
            delete i->second;
        }
    }
}

cliproxy& cliproxy::set_help(const std::string& t)
{
    help_text = t;
    return *this;
}

cliproxy& cliproxy::add_arg(const string& arg, const string& help)
{
    args_help.push_back(arg);
    args_help.push_back(help);
    return *this;
}

void cliproxy::print_help(ostream& os) const
{
    if (!help_text.empty())
    {
        os << help_text << endl;
    }
    if (!args_help.empty())
    {
        table_printer t;
        
        os << endl << "ARGUMENTS" << endl;
        t.set_column_alignment(0, -1);
        t.set_column_alignment(2, -1);
        for (int i = 0, iend = args_help.size(); i < iend; i += 2)
        {
            t.add_row() << args_help[i] << '-' << args_help[i + 1];
        }
        t.print(os);
    }
}

int_proxy::int_proxy(int* p, const string& description)
    : p(p)
{
    set_help(description);
    add_arg("[VALUE]", "New value. Must be an integer.");
}

void int_proxy::proxy_use_sub(const vector<string>& args, ostream& os)
{
    if (args.empty())
    {
        os << *p << endl;
        return;
    }
    if (!parse_int(args[0], *p))
    {
        os << "invalid integer" << endl;
    }
}

float_proxy::float_proxy(double* p, const string& description)
    : p(p)
{
    set_help(description);
    add_arg("[VALUE]", "New value. Must be a float.");
}

void float_proxy::proxy_use_sub(const vector<string>& args, ostream& os)
{
    if (args.empty())
    {
        os << *p << endl;
        return;
    }
    if (!parse_double(args[0], *p))
    {
        os << "invalid float" << endl;
    }
}

bool_proxy::bool_proxy(bool* p, const string& description)
    : p(p)
{
    set_help(description);
    add_arg("[VALUE]", "New value. Must be (0|1|true|false|on|off)");
}

void bool_proxy::proxy_use_sub(const vector<string>& args, ostream& os)
{
    if (args.empty())
    {
        os << (*p ? "true" : "false") << endl;
        return;
    }
    if (args[0] == "true" || args[0] == "on" || args[0] == "1")
    {
        *p = true;
    }
    else if (args[0] == "false" || args[0] == "off" || args[0] == "0")
    {
        *p = false;
    }
    else
    {
        os << "invalid boolean" << endl;
    }
}

void proxy_group::add(const std::string& name, cliproxy* p)
{
    children[name] = p;
}

void proxy_group::proxy_get_children(std::map<std::string, cliproxy*>& c)
{
    c = children;
}
