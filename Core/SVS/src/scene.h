#ifndef SCENE_H
#define SCENE_H

#include <string>
#include <map>
#include <cassert>
#include "sgnode.h"
#include "common.h"
#include "relation.h"
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
        sgnode*       get_node(const std::string& name);
        sgnode const* get_node(const std::string& name) const;
        sgnode*       get_node(int id);
        sgnode const* get_node(int id) const;
        
        void get_all_nodes(std::vector<sgnode*>& nodes);
        void get_all_nodes(std::vector<const sgnode*>& nodes) const;
        
        int get_dof() const;
        
        bool add_node(const std::string& name, sgnode* n);
        bool del_node(const std::string& name);
        void clear();
        
        void get_properties(rvec& vals) const;
        bool set_properties(const rvec& vals);
        bool parse_sgel(const std::string& s);
        
        std::string parse_query(const std::string& query) const;
        
        void node_update(sgnode* n, sgnode::change_type t, const std::string& update_info);
        double get_convex_distance(const sgnode* a, const sgnode* b) const;
        
        bool intersects(const sgnode* a, const sgnode* b) const;
        
        std::string get_name() const
        {
            return name;
        }
        void get_relations(relation_table& rt) const;
        
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
        
        
        mutable relation_table cached_rels;
        relation_table type_rels;
        
        group_node* get_group(const std::string& name);
        
        int parse_add(std::vector<std::string>& f, std::string& error);
        int parse_del(std::vector<std::string>& f, std::string& error);
        int parse_change(std::vector<std::string>& f, std::string& error);
        int parse_tag(std::vector<std::string>& f, std::string& error);
        
        
        void cli_props(const std::vector<std::string>& args, std::ostream& os) const;
        void cli_sgel(const std::vector<std::string>& args, std::ostream& os);
        void cli_relations(const std::vector<std::string>& args, std::ostream& os) const;
        void cli_draw(const std::vector<std::string>& args, std::ostream& os);
        void cli_clear(const std::vector<std::string>& args, std::ostream& os);
        
        int parse_object_query(std::vector<std::string>& f, std::string& result, std::string& error) const;
        int parse_objects_with_flag_query(std::vector<std::string>& f, std::string& result, std::string& error) const;
};

#endif
