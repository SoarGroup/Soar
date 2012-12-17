#ifndef SCENE_H
#define SCENE_H

#include <string>
#include <map>
#include <cassert>
#include "sgnode.h"
#include "common.h"
#include "collision.h"
#include "scene_sig.h"

class filter;
class filter_input;
class drawer;

class scene : public sgnode_listener {
public:
	scene(const std::string &name, drawer *d);
	~scene();
	
	scene *clone(const std::string &name = "", drawer *d = NULL) const;
	
	group_node *get_root() { return root; }
	sgnode *get_node(const std::string &name);
	sgnode const* get_node(const std::string &name) const;
	sgnode *get_node(int i);
	sgnode const* get_node(int i) const;
	
	void get_all_nodes(std::vector<sgnode*> &nodes);
	void get_all_nodes(std::vector<const sgnode*> &nodes) const;
	void get_all_node_indices(std::vector<int> &inds) const;
	void get_nodes(const std::vector<int> &inds, std::vector<const sgnode*> &n) const;

	int num_nodes() const;
	int get_dof() const;
	
	bool add_node(const std::string &name, sgnode *n);
	bool del_node(const std::string &name);
	void clear();

	void get_properties(rvec &vals) const;
	bool get_property(const std::string &obj, const std::string &prop, double &val) const;
	bool set_property(const std::string &obj, const std::string &prop, double val);
	bool set_properties(const rvec &vals);
	void remove_property(const std::string &obj, const std::string &prop);
	void parse_sgel(const std::string &s);
	void node_update(sgnode *n, sgnode::change_type t, int added_child);
	bool intersects(const sgnode *a, const sgnode *b) const;
	int get_closest(int i) const;
	const scene_sig &get_signature() const;
	
	std::string get_name() const { return name; }
	
private:
	typedef std::map<std::string, double> property_map;
	
	struct node_info {
		sgnode *node;
		property_map props;
	};
	
	typedef std::map<int, node_info> node_table;
	
	group_node *get_group(const std::string &name);
	node_info *get_node_info(int i);
	const node_info *get_node_info(int i) const;
	node_info *get_node_info(const std::string &name);
	const node_info *get_node_info(const std::string &name) const;
	void update_sig() const;
	void get_property_names(int i, std::vector<std::string> &names) const;
	void update_closest(const sgnode *n);

	int parse_add(std::vector<std::string> &f, std::string &error);
	int parse_del(std::vector<std::string> &f, std::string &error);
	int parse_change(std::vector<std::string> &f, std::string &error);
	int parse_property(std::vector<std::string> &f, std::string &error);

	std::string  name;
	group_node  *root;
	int          root_id;
	node_table   nodes;
	drawer      *draw;
	bool         dirty;
	bool         sig_dirty;
	
	collision_detector cdetect;

	std::map<std::string, int> node_ids;
	mutable scene_sig sig;
	
	struct close_info {
		int id;
		double dist;
	};
	
	std::map<std::pair<const sgnode *, const sgnode *>, double> distances;
	std::map<const sgnode *, close_info> closest;
};

#endif
