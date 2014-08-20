/**********************************************************
 *
 * File: commands/add_node.cpp
 * Contains:
 *  class add_node_command
 *  
 *  Soar Command to add a node to the world
 *  Parameters:
 *     ^id <string> - name to give the node, must not already exist
 *     ^parent <string> [Optional] - parent to add the node to
 *     ^geometry << box point sphere none >> - geometry of new node
 *     ^position <vec3> [Optional] - position of the new node
 *     ^rotation <vec3> [Optional] - rotation of the new node
 *     ^scale <vec3> [Optional] - scale of the new node
 **********************************************************/
#include <string>
#include "command.h"
#include "scene.h"
#include "filter.h"
#include "filter_table.h"
#include "svs.h"
#include "symtab.h"

using namespace std;

enum GeometryType{
  BOX, SPHERE, NONE, POINT
};

class add_node_command : public command
{
    public:
        add_node_command(svs_state* state, Symbol* root)
            : command(state, root), root(root), scn(state->get_scene()), parent(NULL), first(true)
        {
            si = state->get_svs()->get_soar_interface();
        }
        
        ~add_node_command()
        {
        }
        
        string description()
        {
            return string("add_node");
        }
        
        bool update_sub(){
            if (first){
                first = false;
                if (!parse()){
                    return false;
                }
                return add_node();
            }
            return true;
        }

        
        bool early()
        {
            return false;
        }
        
        
    private:
        bool parse(){
          // ^parent <id>
          // The name of the parent to attach the node to
          // Default is the root
          string parent_name;
          if(!si->get_const_attr(root, "parent", parent_name)){
            parent = scn->get_root();
          } else {
            parent = scn->get_node(parent_name);
            if(parent == NULL){
              set_status("no parent node found");
              return false;
            }
          }

          // ^id <id>
          // The id to give the node
          if(!si->get_const_attr(root, "id", node_name)){
            set_status("no id specified");
            return false;
          }
          if(scn->get_node(node_name)){
            set_status("id already exists");
            return false;
          }

          // ^position <vec3>
          // ^rotation <vec3>
          // ^scale <vec3>
          // All optional - specify transforms on the node
          vec3 trans;
          if(si->get_vec3(root, "position", trans)){
            transforms['p'] = trans;
          }
          if(si->get_vec3(root, "rotation", trans)){
            transforms['r'] = trans;
          }
          if(si->get_vec3(root, "scale", trans)){
            transforms['s'] = trans;
          }

          // ^geometry << box point sphere none >>
          // Optional - default is none
          // The geometry of the new node
          string geom;
          if(!si->get_const_attr(root, "geometry", geom)){
            geom = "none";
          }
          if(geom == "box"){
            geom_type = BOX;
          } else if(geom == "point"){
            geom_type = POINT;
          } else if(geom == "sphere"){
            geom_type = SPHERE;
          } else {
            geom_type = NONE;
          }
          return true;
        }

        bool add_node(){
            sgnode* n;
            ptlist verts;

            switch(geom_type){
              case NONE:
                n = new group_node(node_name, "object");
                break;
              case SPHERE:
                n = new ball_node(node_name, "object", 1.0);
                break;
              case POINT:
                verts.push_back(vec3(0, 0, 0));
                n = new convex_node(node_name, "object", verts);
                break;
              case BOX:
                verts = bbox_vertices();
                n = new convex_node(node_name, "object", verts);
                break;
            }

            for(std::map<char, vec3>::iterator i = transforms.begin(); i != transforms.end(); i++){
              n->set_trans(i->first, i->second);
            }

            if(!scn->add_node(parent->get_name(), n)){
              set_status("error adding node to scene");
              return false;
            }

            set_status("success");
            return true;
        }

        ptlist bbox_vertices(){
          ptlist pts;
          for(double x = -.5; x <= .5; x += 1.0){
            for(double y = -.5; y <= .5; y += 1.0){
              for(double z = -.5; z <= .5; z += 1.0){
                pts.push_back(vec3(x, y, z));
              }
            }
          }
          return pts;
        }

        scene*             scn;
        Symbol*            root;
        soar_interface*    si;

        bool first;

        GeometryType geom_type;
        map<char, vec3> transforms;
        sgnode* parent;
        string node_name;
      
};

command* _make_add_node_command_(svs_state* state, Symbol* root)
{
    return new add_node_command(state, root);
}
