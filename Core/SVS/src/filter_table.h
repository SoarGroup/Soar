#ifndef FILTER_TABLE_H
#define FILTER_TABLE_H

#include <iostream>
#include <iterator>
#include <cassert>
#include <vector>
#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include "common.h"
#include "soar_interface.h"
#include "cliproxy.h"

class filter;
class filter_input;
class scene;
class sgnode;

class filter_table_entry : public cliproxy
{
    public:
        filter_table_entry();
        
        filter* (*create)(Symbol, soar_interface*, scene*, filter_input*);
        
        std::string name;
        std::string description;
        std::map<std::string, std::string> parameters;
        
        void proxy_use_sub(const std::vector<std::string>& args, std::ostream& os);
};

class filter_table : public cliproxy
{
    public:
        friend filter_table& get_filter_table();
        
        filter* make_filter(const std::string& pred, Symbol root, soar_interface* si, scene* scn, filter_input* input) const;
        
    private:
        filter_table();
        void add(filter_table_entry* e);
        void proxy_get_children(std::map<std::string, cliproxy*>& c);
        void proxy_use_sub(const std::vector<std::string>& args, std::ostream& os);
        
        std::map<std::string, filter_table_entry*> t;
};

/* Get the singleton instance */
filter_table& get_filter_table();

filter* parse_filter_spec(soar_interface* si, Symbol root, scene* scn);
#endif
