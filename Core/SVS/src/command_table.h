#ifndef COMMAND_TABLE_H
#define COMMAND_TABLE_H

#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <map>
#include "soar_interface.h"
#include "cliproxy.h"
#include "common.h"

class svs_state;
class scene;

class command_table_entry : public cliproxy
{
  public:
    command_table_entry();
    command* (*create)(svs_state*, Symbol*);

    std::string name;
    std::string description;
    std::map<std::string, std::string> parameters;

    void proxy_use_sub(const std::vector<std::string>& args, std::ostream& os);
};


// class command_table
//   A wrapper for the table of make_command function pointers
//     used to construct a new command from its name
class command_table : public cliproxy
{
    public:
        command_table();
        
        command* make_command(svs_state* state, wme* w);
        
    private:
        void add(command_table_entry* e);
        void proxy_get_children(std::map<std::string, cliproxy*>& c);
        void proxy_use_sub(const std::vector<std::string>& args, std::ostream& os);
        std::map<std::string, command_table_entry*> table;
};

command_table& get_command_table();

#endif
