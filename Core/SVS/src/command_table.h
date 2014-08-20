#ifndef COMMAND_TABLE_H
#define COMMAND_TABLE_H

#include <string>
#include <map>
#include "soar_interface.h"
#include "cliproxy.h"

class svs_state;
class scene;
class filter;

// make_command_fp
//   A function pointer for a function that makes a 
//     new command given the root symbol and the state it was on
typedef command* make_command_fp(svs_state*, Symbol*);

// class command_table
//   A wrapper for the table of make_command function pointers
//     used to construct a new command from its name
class command_table
{
    public:
        command_table();
        
        command* make_command(svs_state* state, wme* w);
        
    private:
        std::map<std::string, make_command_fp*> table;
};

command_table& get_command_table();

#endif
