#ifndef COMMAND_H
#define COMMAND_H

#include <string>
#include "soar_interface.h"

class svs_state;
class scene;
class filter;

class command
{
    public:
        virtual std::string description() = 0;
        virtual bool early() = 0;
        
        bool update()
        {
            return update_sub();
        }
        
        command(svs_state* state, Symbol* root);
        virtual ~command();
        
        /* check if any substructure in the command changed */
        bool changed();
        
        /* get the value of a string wme */
        bool get_str_param(const std::string& name, std::string& val);
        
        void set_status(const std::string& s);
        
        Symbol* get_root()
        {
            return root;
        }
        
        svs_state* get_state()
        {
            return state;
        }
        
    protected:
        virtual bool update_sub() = 0;
        
    private:
        void parse_substructure(size_t& size, uint64_t& max_time);
        
        svs_state*      state;
        soar_interface* si;
        Symbol*         root;
        wme*            status_wme;
        std::string     curr_status;
        size_t             subtree_size;
        size_t             prev_max_time;
        bool            first;
        
};

#endif
