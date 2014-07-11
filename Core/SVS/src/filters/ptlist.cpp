#include <list>
#include "filter.h"
#include "sgnode.h"
#include "mat.h"
#include "scene.h"

using namespace std;

class node_ptlist_filter : public typed_map_filter<ptlist*>
{
    public:
        node_ptlist_filter(Symbol* root, soar_interface* si, filter_input* input, bool local)
            : typed_map_filter<ptlist*>(root, si, input), local(local)
        {}
        
        ~node_ptlist_filter()
        {
            std::list<ptlist*>::iterator i;
            for (i = lists.begin(); i != lists.end(); ++i)
            {
                delete *i;
            }
        }
        
        bool compute(const filter_params* params, bool adding, ptlist*& res, bool& changed)
        {
            const sgnode* n;
            if (!get_filter_param(this, params, "node", n))
            {
                return false;
            }
            const convex_node* cn = dynamic_cast<const convex_node*>(n);
            if (!cn)
            {
                return false;
            }
            if (adding)
            {
                res = new ptlist();
                lists.push_back(res);
            }
            
            if (local)
            {
                changed = (*res != cn->get_verts());
                *res = cn->get_verts();
            }
            else
            {
                changed = (*res != cn->get_world_verts());
                *res = cn->get_world_verts();
            }
            return true;
        }
        
        void output_removed(ptlist*& o)
        {
            lists.remove(o);
            delete o;
        }
        
    private:
        bool local;
        std::list<ptlist*> lists;
};

filter* _make_local_filter_(Symbol* root, soar_interface* si, scene* scn, filter_input* input)
{
    return new node_ptlist_filter(root, si, input, true);
}

filter* _make_world_filter_(Symbol* root, soar_interface* si, scene* scn, filter_input* input)
{
    return new node_ptlist_filter(root, si, input, false);
}

class ptlist_filter : public typed_map_filter<ptlist*>
{
    public:
        ptlist_filter(Symbol* root, soar_interface* si, filter_input* input)
            : typed_map_filter<ptlist*>(root, si, input)
        {}
        
        ~ptlist_filter()
        {
            std::list<ptlist*>::iterator i;
            for (i = lists.begin(); i != lists.end(); ++i)
            {
                delete *i;
            }
        }
        
        bool compute(const filter_params* params, bool adding, ptlist*& res, bool& changed)
        {
            filter_params::const_iterator i;
            ptlist newres;
            
            for (i = params->begin(); i != params->end(); ++i)
            {
                vec3 v;
                if (!get_filter_val(i->second, v))
                {
                    set_status("all parameters must be vec3");
                    return false;
                }
                newres.push_back(v);
            }
            
            if (adding)
            {
                res = new ptlist();
                lists.push_back(res);
            }
            changed = (newres != *res);
            *res = newres;
            return true;
        }
        
        void output_removed(ptlist*& o)
        {
            lists.remove(o);
            delete o;
        }
        
    private:
        std::list<ptlist*> lists;
};

filter* _make_ptlist_filter_(Symbol* root, soar_interface* si, scene* scn, filter_input* input)
{
    return new ptlist_filter(root, si, input);
}
