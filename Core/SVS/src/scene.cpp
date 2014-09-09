#include <cstdlib>
#include <map>
#include <iterator>
#include <iostream>
#include <sstream>
#include <limits>
#include <utility>
#include "scene.h"
#include "sgnode.h"
#include "sgnode_algs.h"
#include "common.h"
#include "drawer.h"
#include "filter.h"
#include "filter_table.h"
#include "params.h"
#include "svs.h"

using namespace std;

const string root_id = "world";

/*
 Native properties are currently the position, rotation, and scaling
 transforms of a node, named px, py, pz, rx, ry, rz, sx, sy, sz.
*/
const char* NATIVE_PROPS[] = { "px", "py", "pz", "rx", "ry", "rz", "sx", "sy", "sz" };

bool is_native_prop(const std::string& name, char& type, int& dim)
{
    int d;
    if (name.size() != 2)
    {
        return false;
    }
    if (name[0] != 'p' && name[0] != 'r' && name[0] != 's')
    {
        return false;
    }
    d = name[1] - 'x';
    if (d < 0 || d > 2)
    {
        return false;
    }
    type = name[0];
    dim = d;
    return true;
}

bool parse_vec3(vector<string>& f, int& start, vec3& v, string& error)
{
    for (int i = 0; i < 3; ++start, ++i)
    {
        if (start >= f.size() || !parse_double(f[start], v[i]))
        {
            error = "expecting a number";
            return false;
        }
    }
    return true;
}

bool parse_verts(vector<string>& f, int& start, ptlist& verts, string& error)
{
    verts.clear();
    while (start < f.size())
    {
        vec3 v;
        int i = start;
        if (!parse_vec3(f, start, v, error))
        {
            return (i == start);  // end of list
        }
        verts.push_back(v);
    }
    return true;
}


scene::scene(const string& name, svs* owner)
    : name(name), owner(owner), draw(false)
{
    root = new group_node(root_id);
    nodes.push_back(root);
    root->listen(this);
}

scene::~scene()
{
    root->unlisten(this);
    delete root;
}

scene* scene::clone(const string& cname) const
{
    scene* c;
    string name;
    std::vector<sgnode*> node_clones;
    
    c = new scene(cname, owner);
    // Remove empty root
    c->root->unlisten(c);
    c->nodes.clear();
    delete c->root;

    // Replace with copy of root
    c->root = root->clone()->as_group(); // root->clone copies entire scene graph
    c->root->walk(c->nodes);
    for (int i = 0, iend = c->nodes.size(); i < iend; ++i)
    {
      c->nodes[i]->listen(c);
    }
    return c;
}

sgnode* scene::get_node(const string& id)
{
    node_table::iterator i, iend;
    for (i = nodes.begin(), iend = nodes.end(); i != iend; ++i)
    {
        if ((*i)->get_id() == id)
        {
            return *i;
        }
    }
    return NULL;
}

const sgnode* scene::get_node(const string& id) const
{
    node_table::const_iterator i, iend;
    for (i = nodes.begin(), iend = nodes.end(); i != iend; ++i)
    {
        if ((*i)->get_id() == id)
        {
            return *i;
        }
    }
    return NULL;
}

group_node* scene::get_group(const string& id)
{
    sgnode* n = get_node(id);
    if (n)
    {
        return n->as_group();
    }
    return NULL;
}

void scene::get_all_nodes(vector<sgnode*>& n)
{
    n.resize(nodes.size());
    for (int i = 0, iend = nodes.size(); i < iend; ++i)
    {
        n[i] = nodes[i];
    }
}

void scene::get_all_nodes(vector<const sgnode*>& n) const
{
    n.resize(nodes.size());
    for (int i = 0, iend = nodes.size(); i < iend; ++i)
    {
        n[i] = nodes[i];
    }
}

bool scene::add_node(const string& parent_id, sgnode* n)
{
    group_node* par = get_group(parent_id);
    if (!par)
    {
        return false;
    }
    par->attach_child(n);
    /* rest is handled in node_update */
    return true;
}

bool scene::del_node(const string& id)
{
  sgnode* node = get_node(id);
  if(node)
  {
    delete node;
    /* rest is handled in node_update */
    return true;
  }
  return false;
}

void scene::clear()
{
    for (int i = root->num_children() - 1; i >= 0; --i)
    {
        delete root->get_child(i);
    }
}

enum node_class { CONVEX_NODE, BALL_NODE, GROUP_NODE };

bool parse_mods(vector<string>& f, int& start, string& mods, vector<ptlist>& vals, string& error)
{
    ptlist v;
    char m;
    while (start < f.size())
    {
        if (f[start].size() != 1)
        {
            return true;
        }
        m = f[start][0];
        v.clear();
        switch (m)
        {
            case 'p':
            case 'r':
            case 's':
                v.push_back(vec3());
                if (!parse_vec3(f, ++start, v[0], error))
                {
                    return false;
                }
                break;
            case 'v':
                if (!parse_verts(f, ++start, v, error))
                {
                    return false;
                }
                break;
            case 'b':
                ++start;
                v.push_back(vec3());
                if (start >= f.size() || !parse_double(f[start], v[0](0)))
                {
                    error = "expecting radius";
                    return false;
                }
                ++start;
                break;
            default:
                // end of modifiers
                return true;
        }
        mods += m;
        vals.push_back(v);
    }
    return true;
}

int scene::parse_add(vector<string>& f, string& error)
{
    int p;
    sgnode* n = NULL;
    group_node* par = NULL;
    string id, mods;
    vector<ptlist> vals;
    ptlist vertices;
    double radius;
    bool is_convex, is_ball;
    
    if (f.size() < 1)
    {
        return f.size();
    }
    id = f[0];
    if (get_node(id))
    {
        error = "node already exists";
        return 0;
    }
    par = get_group(f[1]);
    if (!par)
    {
        error = "parent node does not exist, or is not group node";
        return 1;
    }
    
    p = 2;
    if (!parse_mods(f, p, mods, vals, error))
    {
        return p;
    }
    assert(mods.size() == vals.size());
    
    /*
     Go through once to figure out what type of node this should be
    */
    is_convex = false;
    is_ball = false;
    for (int i = 0, iend = mods.size(); i < iend; ++i)
    {
        switch (mods[i])
        {
            case 'v':
                vertices = vals[i];
                is_convex = true;
                break;
            case 'b':
                radius = vals[i][0](0);
                is_ball = true;
                break;
        }
    }
    if (is_convex && is_ball)
    {
        error = "conflicting node type";
        return 0; // don't know how to find a more accurate position
    }
    else if (is_convex)
    {
        n = new convex_node(id, vertices);
    }
    else if (is_ball)
    {
        n = new ball_node(id, radius);
    }
    else
    {
        n = new group_node(id);
    }
    
    /*
     Go through again to apply transforms
    */
    for (int i = 0, iend = mods.size(); i < iend; ++i)
    {
        switch (mods[i])
        {
            case 'p':
            case 'r':
            case 's':
                n->set_trans(mods[i], vals[i][0]);
                break;
        }
    }
    
    par->attach_child(n);
    return -1;
}

int scene::parse_del(vector<string>& f, string& error)
{
    if (f.size() < 1)
    {
        error = "expecting node id";
        return f.size();
    }
    if (!del_node(f[0]))
    {
        error = "node does not exist";
        return 0;
    }
    return -1;
}

int scene::parse_change(vector<string>& f, string& error)
{
    int p;
    sgnode* n;
    convex_node* cn;
    ball_node* bn;
    string mods;
    vector<ptlist> vals;
    
    if (f.size() < 1)
    {
        error = "expecting node id";
        return f.size();
    }
    if (!(n = get_node(f[0])))
    {
        error = "node does not exist";
        return 0;
    }
    
    p = 1;
    if (!parse_mods(f, p, mods, vals, error))
    {
        return p;
    }
    
    for (int i = 0, iend = mods.size(); i < iend; ++i)
    {
        switch (mods[i])
        {
            case 'p':
            case 'r':
            case 's':
                n->set_trans(mods[i], vals[i][0]);
                break;
            case 'v':
                cn = dynamic_cast<convex_node*>(n);
                if (!cn)
                {
                    return 0; // maybe not as informative as it could be
                }
                cn->set_verts(vals[i]);
                break;
            case 'b':
                bn = dynamic_cast<ball_node*>(n);
                if (!bn)
                {
                    return 0;
                }
                bn->set_radius(vals[i][0](0));
                break;
        }
    }
    return -1;
}

// parse_tag(vector<string> &f, string &error)
//   parses a tag command (command 't')
//   f is a list of the parameters given
//   Changed the format of the command to be
//     tag <subcommand> <id> <tag_name> <value?>
//      <subcommand> - either add, change, or delete (we just test for the first character)
//      <id> - id of the node
//      <tag_name> - the name of the tag
//      <value?> - the value of the tag, needed for the add or change subcommands only
int scene::parse_tag(vector<string>& f, string& error){
    int p = 0;
    
    // Parameter 1: subcommand
    if (p >= f.size()){
        error = "Tag Command P1: Expecting subcommand";
        return p;
    }
    char subcommand = f[p][0];
    p++;

    // Parameters 2: node id
    if (p >= f.size()){
        error = "Tag Command P2: Expecting node id";
        return p;
    }
    string id = f[p];
    
    sgnode* node = get_node(id);
    if (!node){
        error = "Tag Command P2: Node " + id + " does not exist";
        return p;
    }
    p++;

    // Parameter 3: tag name
    if (p >= f.size()){
        error = "Tag Command P3: Expecting tag name";
        return p;
    }
    string tag_name = f[p];
    p++;

    // Parameter 4: tag value
    string tag_value;
    if (subcommand == 'a' || subcommand == 'c'){
      if (p >= f.size()){
          error = "Tag Command P4: Expecting tag value";
          return p;
      }
      tag_value = f[p];
    }
    p++;
    
    switch (subcommand){
      case 'a':
      case 'c':
        node->set_tag(tag_name, tag_value);
        break;
      case 'd':
        node->delete_tag(tag_name);
        break;
      default:
        error = "Tag Command P2: Unrecognized subcommand (Expecting add, change, delete)";
        return 1;
    }
    
    return -1;
}

bool scene::parse_sgel(const string& s){
    vector<string> lines;
    split(s, "\n", lines);

    vector<string>::iterator i;
    for (i = lines.begin(); i != lines.end(); ++i){
        vector<string> fields;
        split(*i, "", fields);
        
        if (fields.size() == 0){
            continue;
        }

        char cmd = fields[0][0];
        fields.erase(fields.begin());

        int errfield;
        string error = "unknown error";
        switch(cmd){
          case 'a':
            errfield = parse_add(fields, error);
            break;
          case 'c':
            errfield = parse_change(fields, error);
            break;
          case 'd':
            errfield = parse_del(fields, error);
            break;
          case 't':
            errfield = parse_tag(fields, error);
            break;
          default:
            cerr << "expecting add, delete, change, tag at start of line '" << *i << "'" << endl;
        }
        
        if (errfield >= 0){
            cerr << "error in field " << errfield + 1 << " of line '"
                                  << *i << "': " << error << endl;
            return false;
        }
    }
    return true;
}

void scene::node_update(sgnode* n, sgnode::change_type t, const std::string& update_info)
{
    sgnode* child;
    group_node* g;
    drawer* d = owner->get_drawer();
    
    if (t == sgnode::CHILD_ADDED)
    {
        int added_child = 0;
        if (!parse_int(update_info, added_child))
        {
            return;
        }
        g = n->as_group();
        child = g->get_child(added_child);
        child->listen(this);
        sgnode*& node = grow_vec(nodes);
        node = child;
        
        if (draw)
        {
            d->add(name, child);
        }
        return;
    }
    
    int i, iend;
    for (i = 0, iend = nodes.size(); i < iend && nodes[i] != n; ++i)
        ;
    assert(i != nodes.size());
    if (i == 0)
    {
        return;
    }
    
    switch (t)
    {
        case sgnode::CHILD_ADDED:
        case sgnode::TAG_DELETED:
        case sgnode::TAG_CHANGED:
            break;
        case sgnode::DELETED:
            nodes.erase(nodes.begin() + i);
            
            if (draw && i != 0)
            {
                d->del(name, n);
            }
            break;
        case sgnode::SHAPE_CHANGED:
            if (!n->is_group() && draw)
            {
                d->change(name, n, drawer::SHAPE);
            }
            break;
        case sgnode::TRANSFORM_CHANGED:
            if (draw)
            {
                d->change(name, n, drawer::POS | drawer::ROT | drawer::SCALE);
            }
            break;
    }
}

void scene::proxy_get_children(map<string, cliproxy*>& c)
{
    c["world"] = root;
    
    c["properties"] = new memfunc_proxy<scene>(this, &scene::cli_props);
    c["properties"]->set_help("Get scene properties.");

    c["sgel"] = new memfunc_proxy<scene>(this, &scene::cli_sgel);
    c["sgel"]->set_help("Modify scene graph with SGEL.")
    .add_arg("SGEL", "SGEL string (spaces are okay).")
    ;
    
    c["draw"] = new memfunc_proxy<scene>(this, &scene::cli_draw);
    c["draw"]->set_help("Draw this scene in the viewer.")
    .add_arg("[VALUE]", "New value. Must be (0|1|on|off|true|false).");
    
    c["clear"] = new memfunc_proxy<scene>(this, &scene::cli_clear);
    c["clear"]->set_help("Delete all objects in scene except world");
}

void scene::cli_props(const vector<string>& args, ostream& os) const
{
    table_printer t;
    const char* props = "prs";
    const char* axes = "xyz";

    // For each node, add each property to the output
    for (int i = 0, iend = nodes.size(); i < iend; ++i)
    {
        string id = nodes[i]->get_id();

        for(int p = 0; p < 3; p++){
          vec3 trans = nodes[i]->get_trans(props[p]);
          for(int dim = 0; dim < 3; dim++){
            t.add_row() << id + ':' + props[p] + axes[dim] << trans[dim];
          }
        }


        const tag_map& tags = nodes[i]->get_all_tags();
        tag_map::const_iterator ti;
        for(ti = tags.begin(); ti != tags.end(); ti++){
          t.add_row() << id + ':' + ti->first << ti->second;
        }
    }

    t.print(os);
}

void scene::cli_sgel(const vector<string>& args, ostream& os)
{
    stringstream ss;
    for (int i = 0, iend = args.size(); i < iend; ++i)
    {
        ss << args[i] << " ";
    }
    parse_sgel(ss.str());
}


void scene::cli_draw(const vector<string>& args, ostream& os)
{
    bool_proxy p(&draw, "");
    bool old_draw = draw;
    
    p.proxy_use("", args, os);
    if (!old_draw && draw)
    {
        refresh_draw();
    }
    else if (old_draw && !draw)
    {
        owner->get_drawer()->delete_scene(name);
    }
}

void scene::cli_clear(const vector<string>& args, ostream& os)
{
    clear();
}

void scene::refresh_draw()
{
    if (!draw)
    {
        return;
    }
    
    drawer* d = owner->get_drawer();
    d->delete_scene(name);
    for (int i = 1, iend = nodes.size(); i < iend; ++i)
    {
        d->add(name, nodes[i]);
    }
}

void scene::verify_listeners() const
{
    for (int i = 0, iend = nodes.size(); i < iend; ++i)
    {
        std::list<sgnode_listener*> l;
        nodes[i]->get_listeners(l);
        assert(l.size() == 1 && l.front() == this);
    }
}

std::string scene::parse_query(const std::string& query) const
{
    vector<string> lines;
    vector<string>::iterator i;
    string cmd;
    int errfield;
    string error;
    string result;
    string output = "";
    
    split(query, "\n", lines);
    for (i = lines.begin(); i != lines.end(); ++i)
    {
        vector<string> fields;
        split(*i, "", fields);
        error = "unknown error";
        
        if (fields.size() == 0)
        {
            continue;
        }
        
        cmd = fields[0];
        fields.erase(fields.begin());
        
        if (cmd == "obj-info")
        {
            errfield = parse_object_query(fields, result, error);
        }
        else if (cmd == "objs-with-flag")
        {
            errfield = parse_objects_with_flag_query(fields, result, error);
        }
        else
        {
            errfield = 0;
            error = "Unknown command";
        }
        
        if (errfield >= 0)
        {
            stringstream ss;
            ss << "Error in F[" << (1 + errfield) << "] of line [" << *i << "]: " << error << endl;
            output += ss.str();
        }
        else
        {
            output += result + "\n";
        }
    }
    return output;
}

int scene::parse_object_query(std::vector<std::string>& f, std::string& result, std::string& error) const
{
    if (f.size() == 0)
    {
        error = "Expecting id argument";
        return 1;
    }
    
    string id = f[0];
    const sgnode* node = this->get_node(id);
    if (node == 0)
    {
        error = "Node not found";
        return 1;
    }
    
    vec3 pos, rot, scale;
    node->get_trans(pos, rot, scale);

    const tag_map& tags = node->get_all_tags();
    
    stringstream ss;
    ss << "o " << id;
    ss << " p " << pos[0] << " " << pos[1] << " " << pos[2];
    ss << " r " << rot[0] << " " << rot[1] << " " << rot[2];
    ss << " s " << scale[0] << " " << scale[1] << " " << scale[2];
    ss << " t " << tags.size();
    for(tag_map::const_iterator i = tags.begin(); i != tags.end(); i++){
      ss << "   " << i->first << " = " << i->second;
    }
    
    result = ss.str();
    return -1;
}

int scene::parse_objects_with_flag_query(std::vector<std::string>& f, std::string& result, std::string& error) const
{
    if (f.size() < 2)
    {
        error = "Expecting 2 arguments";
        return 1;
    }
    
    string tag_name = f[0];
    string query_value = f[1];
    
    vector<string> nodeIds;
    
    vector<const sgnode*> nodes;
    this->get_all_nodes(nodes);
    for (vector<const sgnode*>::const_iterator i = nodes.begin(); i != nodes.end(); i++)
    {
        string tag_value;
        if((*i)->get_tag(tag_name, tag_value) && query_value == tag_value){
          nodeIds.push_back((*i)->get_id());
        }
    }
    
    stringstream ss;
    ss << "objs " << nodeIds.size();
    for (vector<string>::iterator i = nodeIds.begin(); i != nodeIds.end(); i++)
    {
        ss << " " << *i;
    }
    
    result = ss.str();
    
    return -1;
}
