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
#include "relation.h"
#include "cliproxy.h"

class filter;
class filter_input;
class scene;
class sgnode;

class filter_table_entry : public cliproxy
{
    public:
        std::string name;
        std::vector<std::string> parameters;
        bool ordered, allow_repeat;
        
        filter* (*create)(Symbol*, soar_interface*, scene*, filter_input*);
        bool (*calc)(const scene*, const std::vector<const sgnode*>&);
        
        filter_table_entry();
        void proxy_use_sub(const std::vector<std::string>& args, std::ostream& os);
};

class filter_table : public cliproxy
{
    public:
        friend filter_table& get_filter_table();
        
        void get_predicates(std::vector<std::string>& preds) const;
        bool get_params(const std::string& pred, std::vector<std::string>& p) const;
        filter* make_filter(const std::string& pred, Symbol* root, soar_interface* si, scene* scn, filter_input* input) const;
        
        /*
         Returns a list of lists of atoms. Each atom is described by a
         list of strings. The first string is the name of the predicate,
         and the rest are the node names used as arguments.
        */
        void get_all_atoms(scene* scn, std::vector<std::string>& atoms) const;
        
        void update_relations(const scene* scn, const std::vector<int>& dirty, int time, relation_table& rt) const;
        
    private:
        filter_table();
        void add(filter_table_entry* e);
        void proxy_get_children(std::map<std::string, cliproxy*>& c);
        
        std::map<std::string, filter_table_entry*> t;
        
};

/* Get the singleton instance */
filter_table& get_filter_table();

filter* parse_filter_spec(soar_interface* si, Symbol* root, scene* scn);
#endif
