#include <iostream>
#include <assert.h>
#include <string>
#include <map>
#include "filter.h"
#include "sgnode.h"
#include "scene.h"
#include "filter_table.h"

using namespace std;

typedef map<sgnode*, const filter_params*> node_param_map;

/*
 This filter takes a "name" parameter and outputs a pointer to the node
 with that name in the scene graph.
*/
class node_filter : public typed_select_filter<const sgnode*>, public sgnode_listener
{
    public:
        node_filter(Symbol* root, soar_interface* si, scene* scn, filter_input* input)
            : typed_select_filter<const sgnode*>(root, si, input), scn(scn)
        {
            enterf("node_filter::node_filter");
            exitf("node_filter::node_filter");
        }
        
        ~node_filter()
        {
            enterf("node_filter::~node_filter");
            exitf("node_filter::~node_filter");
            node_param_map::iterator i;
            for (i = nodes.begin(); i != nodes.end(); i++)
            {
                //cout << padd() << "Unlisten " << i->first->get_name() << endl;
                i->first->unlisten(this);
            }
        }
        
        bool compute(const filter_params* params, bool null_out, const sgnode*& out, bool& select, bool& changed)
        {
            enterf("node_filter::compute");
            //out = NULL;
            //changed = false;
            //select = false;
            //return true;
            string id;
            if (!get_filter_param(this, params, "id", id))
            {
                set_status("expecting parameter id");
                return false;
            }
            
            
            sgnode* n = scn->get_node(id);
            
            if (n == NULL)
            {
                select = false;
                changed = (out != NULL);
                out = NULL;
            }
            else
            {
                changed = true; // Always pass on node changes
                out = n;
                node_param_map::iterator i = nodes.find(n);
                if (i == nodes.end())
                {
                    // First time seeing this node, add it
                    nodes[n] = params;
                    //cout << padd() << "Listen " << n->get_name() << endl;
                    n->listen(this);
                    select = true;
                }
                else if (i->second != params)
                {
                    // Duplicate, don't select
                    select = false;
                }
                else
                {
                    // Seeing a node already in the list
                    select = true;
                }
            }
            
            //cout << padd() << "Change " << (changed ? "T" : "F") << endl;
            //cout << padd() << "Select " << (select ? "T" : "F") << endl;
            //cout << padd() << "Node " << (out == NULL ? "NULL" : out->get_name()) << endl;
            exitf("node_filter::compute");
            
            return true;
        }
        node_param_map nodes;
        
        void node_update(sgnode* n, sgnode::change_type t, const std::string& update_info)
        {
            enterf("node_filter::node_update");
            //cout << padd() << "Change " << t << " on " << n->get_name() << endl;
            switch (t)
            {
                case sgnode::SHAPE_CHANGED:
                case sgnode::TRANSFORM_CHANGED:
                case sgnode::DELETED:
                    node_param_map::iterator i = nodes.find(n);
                    assert(i != nodes.end());
                    mark_stale(i->second);  // Update the filter params
                    if (t == sgnode::DELETED)
                    {
                        //cout << padd() << "Unlisten " << i->first->get_name() << endl;
                        i->first->unlisten(this);
                        nodes.erase(i);
                    }
                    break;
            }
            exitf("node_filter::node_update");
        }
        
        
        scene* scn;
        
//  void node_update(sgnode *n, sgnode::change_type t, const std::string& update_info) {
//      cout << "node_filter::node_update" << this << endl;
//      cout << "  " << t << " on " << n->get_name() << endl;
//      if (t == sgnode::DELETED || t == sgnode::TRANSFORM_CHANGED || t == sgnode::SHAPE_CHANGED) {
//          node_info &info = map_get(nodes, n);
//          std::list<const filter_params*>::const_iterator i;
//          for (i = info.params.begin(); i != info.params.end(); ++i) {
//              mark_stale(*i);
//          }
//          info.changed = true;
//          if (t == sgnode::DELETED) {
//              nodes.erase(n);
//          }
//      }
//  }
//
//private:
//  void add_entry(sgnode *n, const filter_params *params) {
//      cout << "node_filter::add_entry" << this << endl;
//      map<sgnode*, node_info>::iterator i = nodes.find(n);
//      if (i == nodes.end()) {
//          n->listen(this);
//      }
//      cout << "Nodes size: " << nodes.size() << endl;
//      nodes[n].params.push_back(params);
//  }
//
//  void del_entry(sgnode *n, const filter_params *params) {
//      cout << "node_filter::del_entry" << this << endl;
//      map<sgnode*, node_info>::iterator i = nodes.find(n);
//      //JK assert crashes in real world TODO debug
//      //assert(i != nodes.end());
//      if (i== nodes.end())
//        return;
//      std::list<const filter_params*> &p = i->second.params;
//      std::list<const filter_params*>::iterator j = find(p.begin(), p.end(), params);
//      assert(j != p.end());
//      p.erase(j);
//      if (p.empty()) {
//          i->first->unlisten(this);
//          nodes.erase(i);
//      }
//  }
//
//  struct node_info {
//      std::list<const filter_params*> params;
//      bool changed;
//  };
//
//  scene *scn;
//  map<sgnode*, node_info> nodes;
};

/* Return all nodes from the scene */
class all_nodes_filter : public filter, public sgnode_listener
{
    public:
        all_nodes_filter(Symbol* root, soar_interface* si, scene* scn)
            : filter(root, si, NULL), scn(scn), first(true) {}
            
        ~all_nodes_filter()
        {
            map<sgnode*, filter_val*>::iterator i;
            for (i = outputs.begin(); i != outputs.end(); ++i)
            {
                i->first->unlisten(this);
            }
            
            vector<sgnode*> nodes;
            scn->get_all_nodes(nodes);
            nodes[0]->unlisten(this);
        }
        
        bool update_outputs()
        {
            vector<sgnode*> nodes;
            
            if (!first)
            {
                return true;
            }
            
            scn->get_all_nodes(nodes);
            nodes[0]->listen(this);
            
            for (int i = 1, iend = nodes.size(); i < iend; ++i) // don't add world node
            {
                add_node(nodes[i]);
            }
            first = false;
            return true;
        }
        
        void node_update(sgnode* n, sgnode::change_type t, const std::string& update_info)
        {
            filter_val* r;
            group_node* g;
            int added_child = 0;
            switch (t)
            {
                case sgnode::CHILD_ADDED:
                    if (parse_int(update_info, added_child))
                    {
                        g = n->as_group();
                        add_node(g->get_child(added_child));
                    }
                    break;
                case sgnode::DELETED:
                    if (map_get(outputs, n, r))
                    {
                        remove_output(r);
                        outputs.erase(n);
                    }
                    else
                    {
                        assert(n->get_name() == "world");
                    }
                    break;
                case sgnode::TRANSFORM_CHANGED:
                case sgnode::SHAPE_CHANGED:
                case sgnode::PROPERTY_CHANGED:
                case sgnode::PROPERTY_DELETED:
                    if (map_get(outputs, n, r))
                    {
                        change_output(r);
                    }
                    else
                    {
                        assert(n->get_name() == "world");
                    }
                    break;
            }
        }
        
    private:
        filter_val* add_node(sgnode* n)
        {
            n->listen(this);
            filter_val* r = new filter_val_c<const sgnode*>(n);
            outputs[n] = r;
            add_output(r, NULL);
            return r;
        }
        
        scene* scn;
        bool first;
        
        map<sgnode*, filter_val*> outputs;
};

class remove_node_filter : public typed_select_filter<const sgnode*>
{
    public:
        remove_node_filter(Symbol* root, soar_interface* si, filter_input* input)
            : typed_select_filter<const sgnode*>(root, si, input), scn(scn)
        {}
        
        bool compute(const filter_params* p, bool null_out, const sgnode*& out, bool& select, bool& changed)
        {
            const sgnode* a;
            const sgnode* b;
            
            if (!get_filter_param(this, p, "a", a))
            {
                set_status("expecting parameter a");
                return false;
            }
            if (!get_filter_param(this, p, "b", b))
            {
                set_status("expecting parameter b");
                return false;
            }
            
            out = a;
            select = (a != b);
            if (select)
            {
                // We always say a selected node has changed, so that changes from below get propagated
                changed = true;
            }
            
            return true;
        }
        
    private:
    
        scene* scn;
};

class node_centroid_filter : public typed_map_filter<vec3>
{
    public:
        node_centroid_filter(Symbol* root, soar_interface* si, filter_input* input)
            : typed_map_filter<vec3>(root, si, input)
        {}
        
        bool compute(const filter_params* params, bool adding, vec3& res, bool& changed)
        {
            const sgnode* n;
            
            if (!get_filter_param(this, params, "node", n))
            {
                return false;
            }
            
            vec3 newres = n->get_centroid();
            changed = (newres != res);
            res = newres;
            return true;
        }
};

filter* make_node_filter(Symbol* root, soar_interface* si, scene* scn, filter_input* input)
{
    return new node_filter(root, si, scn, input);
}

filter* make_all_nodes_filter(Symbol* root, soar_interface* si, scene* scn, filter_input* input)
{
    return new all_nodes_filter(root, si, scn);
}

filter* make_node_centroid_filter(Symbol* root, soar_interface* si, scene* scn, filter_input* input)
{
    return new node_centroid_filter(root, si, input);
}

filter* make_remove_node_filter(Symbol* root, soar_interface* si, scene* scn, filter_input* input)
{
    return new remove_node_filter(root, si, input);
}

filter_table_entry* node_fill_entry()
{
    filter_table_entry* e = new filter_table_entry;
    e->name = "node";
    e->parameters.push_back("id");
    e->create = &make_node_filter;
    return e;
}

filter_table_entry* all_nodes_fill_entry()
{
    filter_table_entry* e = new filter_table_entry;
    e->name = "all_nodes";
    e->create = &make_all_nodes_filter;
    return e;
}

filter_table_entry* node_centroid_fill_entry()
{
    filter_table_entry* e = new filter_table_entry;
    e->name = "node_centroid";
    e->parameters.push_back("node");
    e->create = &make_node_centroid_filter;
    return e;
}

filter_table_entry* remove_node_fill_entry()
{
    filter_table_entry* e = new filter_table_entry;
    e->name = "remove_node";
    e->parameters.push_back("a");
    e->parameters.push_back("b");
    e->create = &make_remove_node_filter;
    return e;
}
