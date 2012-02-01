#ifndef SCENE_H
#define SCENE_H

#include <string>
#include <map>
#include "sgnode.h"
#include "linalg.h"
#include "common.h"
#include "drawer.h"
#include "filter_table.h"

class filter;
class filter_input;

class scene : public sgnode_listener {
public:
	scene(const std::string &name, const std::string &rootname, bool display);
	~scene();
	
	scene *copy() const;
	
	sgnode *get_root() { return root; }
	sgnode *get_node(const std::string &name);
	sgnode const* get_node(const std::string &name) const;
	
	// nodes will be in alphabetical name order
	void get_all_nodes(std::vector<sgnode*> &nodes);
	int num_nodes() const;
	int get_dof() const;
	
	bool add_node(const std::string &name, sgnode *n);
	bool del_node(const std::string &name);
	void clear();

	void get_property_names(std::vector<std::string> &names) const;
	void get_properties(evec &vals) const;
	bool get_property(const std::string &obj, const std::string &prop, float &val) const;
	bool add_property(const std::string &obj, const std::string &prop, float val);
	bool set_property(const std::string &obj, const std::string &prop, float val);
	bool set_properties(const evec &vals);
	bool remove_property(const std::string &obj, const std::string &prop);
	
	float get_dt() const;
	
	void parse_sgel(const std::string &s);
	void dump_sgel(std::ostream &os);
	
	void node_update(sgnode *n, sgnode::change_type t, int added_child);
	
	void draw_all(const std::string &prefix, float r, float g, float b);
	void undraw_all(const std::string &prefix);
	
	const std::vector<bool>& get_atom_vals();
	
private:
	int  parse_add(std::vector<std::string> &f);
	int  parse_del(std::vector<std::string> &f);
	int  parse_change(std::vector<std::string> &f);
    int  parse_property(std::vector<std::string> &f);
    int  parse_dt(std::vector<std::string> &f);
    
	void dump_sgel_rec(std::ostream &os, const std::string &name, const std::string &parent);
	
	typedef std::map<std::string, float> property_map;
	
	struct node_info {
		sgnode *node;
		property_map props;
	};
	
	typedef std::map<std::string, node_info> node_map;

	std::string  name;
	std::string  rootname;
	sgnode      *root;
	node_map     nodes;
	bool         display;
	drawer       draw;
	float        dt;          // time passed since last update (as reported by environment)
	
	std::vector<bool> atomvals;
	bool dirty;
};

/* Functions to generate common argument sets */
void all_nodes(scene *scn, std::vector<std::vector<std::string> > &argset);
void all_node_pairs_unordered_no_repeat(scene *scn, std::vector<std::vector<std::string> > &argset);
void all_node_pairs_ordered_no_repeat(scene *scn, std::vector<std::vector<std::string> > &argset);
void all_node_triples_unordered_no_repeat(scene *scn, std::vector<std::vector<std::string> > &argset);
void all_node_triples_ordered_no_repeat(scene *scn, std::vector<std::vector<std::string> > &argset);

#endif
