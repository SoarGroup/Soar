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
	
	scene *clone() const;
	
	group_node *get_root() { return root; }
	sgnode *get_node(const std::string &name);
	sgnode const* get_node(const std::string &name) const;
	sgnode *get_node(int i);
	sgnode const* get_node(int i) const;
	group_node *get_group(const std::string &name);
	
	void get_all_nodes(std::vector<sgnode*> &nodes);
	void get_all_nodes(std::vector<const sgnode*> &nodes) const;
	void get_all_node_indices(std::vector<int> &inds) const;
	void get_nodes(const std::vector<int> &inds, std::vector<const sgnode*> &n) const;

	int num_nodes() const;
	int get_dof() const;
	
	bool add_node(const std::string &name, sgnode *n);
	bool del_node(const std::string &name);
	void clear();

	void get_property_names(std::vector<std::string> &names) const;
	void get_properties(rvec &vals) const;
	bool get_property(const std::string &obj, const std::string &prop, float &val) const;
	bool add_property(const std::string &obj, const std::string &prop, float val);
	bool set_property(const std::string &obj, const std::string &prop, float val);
	bool set_properties(const rvec &vals);
	bool remove_property(const std::string &obj, const std::string &prop);
	
	void parse_sgel(const std::string &s);
	
	void node_update(sgnode *n, sgnode::change_type t, int added_child);
	
	bool intersects(const sgnode *a, const sgnode *b) const;
	
	void calc_relations(relation_table &rels) const;
	void print_relations(std::ostream &os) const;
	
	void get_signature(state_sig &sig) const;
private:
	int parse_add(std::vector<std::string> &f, std::string &error);
	int parse_del(std::vector<std::string> &f, std::string &error);
	int parse_change(std::vector<std::string> &f, std::string &error);
	int parse_property(std::vector<std::string> &f, std::string &error);

	typedef std::map<std::string, float> property_map;
	
	struct node_info {
		sgnode *node;
		property_map props;
	};
	
	typedef std::vector<node_info> node_vec;

	std::string  name;
	std::string  rootname;
	group_node  *root;
	node_vec     nodes;
	drawer      *draw;
	
	std::vector<bool> atomvals;
	bool dirty;
	
	collision_detector cdetect;

	std::map<std::string, int> node_ids;
};

#endif
