#include <stdlib.h>
#include <ctype.h>
#include <sstream>
#include <limits>
#include <iomanip>

#include "command.h"
#include "command_table.h"
#include "svs.h"
#include "scene.h"
#include "soar_interface.h"
#include "symbol.h"


command_table& get_command_table()
{
    static command_table inst;
    return inst;
}

command_table_entry* extract_command_entry();
command_table_entry* extract_once_command_entry();

command_table_entry* add_node_command_entry();
command_table_entry* copy_node_command_entry();
command_table_entry* set_transform_command_entry();
command_table_entry* copy_transform_command_entry();
command_table_entry* delete_node_command_entry();

command_table_entry* set_tag_command_entry();
command_table_entry* delete_tag_command_entry();


command_table::command_table()
{
    set_help("Prints out a list of all soar commands");

    add(extract_command_entry());
    add(extract_once_command_entry());

    add(add_node_command_entry());
    add(copy_node_command_entry());
    add(set_transform_command_entry());
		add(copy_transform_command_entry());
    add(delete_node_command_entry());

    add(set_tag_command_entry());
    add(delete_tag_command_entry());

}

command* command_table::make_command(svs_state* state, wme* w)
{
    std::string name;
    Symbol* id;
    soar_interface* si;

    si = state->get_svs()->get_soar_interface();
    if (!get_symbol_value(si->get_wme_attr(w), name))
    {
        return NULL;
    }
    if (!si->get_wme_val(w)->is_sti())
    {
        return NULL;
    }
    id = si->get_wme_val(w);

    std::map<std::string, command_table_entry*>::iterator i = table.find(name);
    if (i != table.end())
    {
        return i->second->create(state, id);
    }
    else
    {
        return NULL;
    }
}

void command_table::add(command_table_entry* e)
{
    table[e->name] = e;
}

void command_table::proxy_get_children(std::map<std::string, cliproxy*>& c)
{
    std::map<std::string, command_table_entry*>::iterator i, iend;
    for (i = table.begin(), iend = table.end(); i != iend; ++i)
    {
        c[i->first] = i->second;
    }
}

void command_table::proxy_use_sub(const std::vector<std::string>& args, std::ostream& os)
{
    os << "====================== COMMAND TABLE =======================" << std::endl;
    std::map<std::string, command_table_entry*>::iterator i;
    for (i = table.begin(); i != table.end(); i++)
    {
        os << "  " << std::setw(22) << std::left << i->first << " | " << i->second->description << std::endl;
    }
    os << "===========================================================" << std::endl;
    os << "For specific command info, use the command 'svs commands.command_name'" << std::endl;
}

command_table_entry::command_table_entry()
    : create(NULL), description("")
{
    set_help("Reports information about this command");
}

void command_table_entry::proxy_use_sub(const std::vector<std::string>& args, std::ostream& os)
{
    os << "Command: " << name << std::endl;
    os << "  " << description << std::endl;
    os << "  Parameters:" << std::endl;
    std::map<std::string, std::string>::iterator i;
    for (i = parameters.begin(); i != parameters.end(); i++)
    {
        os << "    " << std::setw(15) << std::left << i->first << " | " << i->second << std::endl;
    }
}
