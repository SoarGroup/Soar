#ifndef SCENE_H
#define SCENE_H

#include <string>
#include <map>
#include <cassert>
#include "sgnode.h"
#include "common.h"
#include "collision.h"
#include "relation.h"
#include "serializable.h"

class filter;
class filter_input;
class drawer;

class scene_sig : public serializable {
public:
	class entry : public serializable {
	public:
		entry() : id(-1), type(-1), start(-1) {}
		
		bool operator==(const entry &e) const {
			/*
			 don't need to compare names or props, the equivalence of those
			 should follow from the equivalence of id and type
			*/
			return id == e.id && type == e.type && start == e.start;
		}
		
		bool similar(const entry &e) const {
			return type == e.type && start == e.start;
		}
		
		void serialize(std::ostream &os) const;
		void unserialize(std::istream &is);

		int id;
		int type;
		int start;
		std::string name;
		std::vector<std::string> props;
		
	};

	scene_sig() {}
	
	entry &operator[](int i)                    { return s.at(i); }
	const entry &operator[](int i) const        { return s.at(i); }
	int size() const                            { return s.size(); } 
	bool empty() const                          { return s.empty(); } 
	void clear()                                { s.clear(); }
	bool operator==(const scene_sig &sig) const { return s == sig.s; }
	bool similar(const scene_sig &sig) const;
	int find_id(int id) const;

	void serialize(std::ostream &os) const;
	void unserialize(std::istream &is);
	int dim() const;
	void add(const entry &e);
	bool get_dim(const std::string &obj, const std::string &prop, int &obj_ind, int &prop_ind) const;
private:
	std::vector<entry> s;
};

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
	bool get_property(const std::string &obj, const std::string &prop, float &val) const;
	bool add_property(const std::string &obj, const std::string &prop, float val);
	bool set_property(const std::string &obj, const std::string &prop, float val);
	bool set_properties(const rvec &vals);
	void remove_property(const std::string &obj, const std::string &prop);
	
	void parse_sgel(const std::string &s);
	
	void node_update(sgnode *n, sgnode::change_type t, int added_child);
	
	bool intersects(const sgnode *a, const sgnode *b) const;
	
	void calc_relations(relation_table &rels) const;
	void print_relations(std::ostream &os) const;
	
	const scene_sig &get_signature() const;
	
private:
	typedef std::map<std::string, float> property_map;
	
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
};

#endif
