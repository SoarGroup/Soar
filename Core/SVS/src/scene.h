#ifndef SCENE_H
#define SCENE_H

#include <string>
#include <map>
#include <cassert>
#include "sgnode.h"
#include "common.h"
#include "cliproxy.h"

class svs;


// Returns true if the given property name is a native type
// If true, type is given either 'p' (pos), 'r' (rot), or 's' (scale)
//      and dim is given 0=x, 1=y, 2=z
bool is_native_prop(const std::string& name, char& type, int& dim);

class scene : public sgnode_listener, public cliproxy
{
    public:
        scene(const std::string& name, svs* owner);
        ~scene();
        
        scene* clone(const std::string& name) const;
        
        group_node*   get_root()
        {
            return root;
        }
        const group_node* get_root() const
        {
            return root;
        }
        sgnode*       get_node(const std::string& id);
        sgnode const* get_node(const std::string& id) const;
        group_node* get_group(const std::string& id);
        
        void get_all_nodes(std::vector<sgnode*>& nodes);
        void get_all_nodes(std::vector<const sgnode*>& nodes) const;
        
        bool add_node(const std::string& parent_id, sgnode* n);
        bool del_node(const std::string& id);
        void clear();
        
        bool parse_sgel(const std::string& s);
        
        std::string parse_query(const std::string& query) const;
        
        void node_update(sgnode* n, sgnode::change_type t, const std::string& update_info);
        
        std::string get_name() const
        {
            return name;
        }
        
        void proxy_get_children(std::map<std::string, cliproxy*>& c);
        
        void refresh_draw();
        void set_draw(bool d)
        {
            draw = d;
        }
        
        void verify_listeners() const;
    private:
        typedef std::vector<sgnode*> node_table;
        
        std::string  name;
        group_node*  root;
        svs*         owner;
        node_table   nodes;
        bool         draw;
        
        
        int parse_add(std::vector<std::string>& f, std::string& error);
        int parse_del(std::vector<std::string>& f, std::string& error);
        int parse_change(std::vector<std::string>& f, std::string& error);
        int parse_tag(std::vector<std::string>& f, std::string& error);
        
        void cli_props(const std::vector<std::string>& args, std::ostream& os) const;
        void cli_sgel(const std::vector<std::string>& args, std::ostream& os);
        void cli_draw(const std::vector<std::string>& args, std::ostream& os);
        void cli_clear(const std::vector<std::string>& args, std::ostream& os);
        
        int parse_object_query(std::vector<std::string>& f, std::string& result, std::string& error) const;
        int parse_objects_with_flag_query(std::vector<std::string>& f, std::string& result, std::string& error) const;
};

#endif
