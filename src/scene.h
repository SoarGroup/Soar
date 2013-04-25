#ifndef SCENE_H
#define SCENE_H

#include <string>
#include <map>
#include "sgnode.h"
#include "linalg.h"
#include "common.h"
#include "drawer.h"
#include "filter_table.h"
#include "collision.h"

class filter;
class filter_input;

class scene : public sgnode_listener {
public:
	scene(const std::string &name, drawer *d);
	~scene();
	
	scene *clone(const std::string &name = "", drawer *d = NULL) const;
	
	group_node *get_root() { return root; }
	sgnode *get_node(const std::string &name);
	sgnode const* get_node(const std::string &name) const;
	group_node *get_group(const std::string &name);
	
	// nodes will be in alphabetical name order
	void get_all_nodes(std::vector<sgnode*> &nodes);
	void get_all_nodes(std::vector<const sgnode*> &nodes);
	int num_nodes() const;
	int get_dof() const;
	
	bool add_node(const std::string &name, sgnode *n);
	bool del_node(const std::string &name);
	void clear();

	void get_property_names(std::vector<std::string> &names) const;
	void get_properties(rvec &vals) const;
	bool get_property(const std::string &obj, const std::string &prop, float &val) const;
	bool set_property(const std::string &obj, const std::string &prop, float val);
	bool set_properties(const rvec &vals);
	bool remove_property(const std::string &obj, const std::string &prop);
	
	void parse_sgel(const std::string &s);
	void dump_sgel(std::ostream &os);
	
	void node_update(sgnode *n, sgnode::change_type t, int added_child);
	
	const std::vector<bool>& get_atom_vals();
	
	bool intersects(const std::string &a, const std::string &b);
	bool intersects(const sgnode *a, const sgnode *b);
	
	void print_object_verts(std::ostream &os) const;
	
private:
	int  parse_add(std::vector<std::string> &f);
	int  parse_del(std::vector<std::string> &f);
	int  parse_change(std::vector<std::string> &f);
	int  parse_property(std::vector<std::string> &f);

	typedef std::map<std::string, float> property_map;
	
	struct node_info {
		sgnode *node;
		property_map props;
	};
	
	typedef std::map<std::string, node_info> node_map;

	std::string  name;
	std::string  rootname;
	group_node  *root;
	node_map     nodes;
	drawer      *draw;
	
	std::vector<bool> atomvals;
	bool dirty;
	
	collision_detector cdetect;
};

#endif
