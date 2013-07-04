#ifndef SCENE_H
#define SCENE_H

#include <string>
#include <map>
#include <cassert>
#include "sgnode.h"
#include "common.h"
#include "scene_sig.h"
#include "relation.h"
#include "cliproxy.h"

class svs;
class logger_set;

class scene : public sgnode_listener, public cliproxy {
public:
	scene(const std::string &name, svs *owner);
	~scene();
	
	scene *clone(const std::string &name) const;
	
	group_node   *get_root() { return root; }
	sgnode       *get_node(const std::string &name);
	sgnode const *get_node(const std::string &name) const;
	sgnode       *get_node(int id);
	sgnode const *get_node(int id) const;
	
	void get_all_nodes(std::vector<sgnode*> &nodes);
	void get_all_nodes(std::vector<const sgnode*> &nodes) const;

	int get_dof() const;
	
	bool add_node(const std::string &name, sgnode *n);
	bool del_node(const std::string &name);
	void clear();

	void get_properties(rvec &vals) const;
	bool set_property(const std::string &obj, const std::string &prop, double val);
	bool set_properties(const rvec &vals);
	void remove_property(const std::string &obj, const std::string &prop);
	bool parse_sgel(const std::string &s);
	void node_update(sgnode *n, sgnode::change_type t, int added_child);
	double convex_distance(const sgnode *a, const sgnode *b) const;
	bool intersects(const sgnode *a, const sgnode *b) const;
	const scene_sig &get_signature() const;
	
	std::string get_name() const { return name; }
	void get_relations(relation_table &rt) const;
	bool tracking_distances() const { return track_dists; }
	void set_track_distances(bool v);
	
	void proxy_get_children(std::map<std::string, cliproxy*> &c);
	
	void refresh_draw();
	void set_draw(bool d) { draw = d; }

private:
	typedef std::map<std::string, double> property_map;
	
	struct node_info {
		node_info() : node(NULL), rels_dirty(true), closest(-1) {}
		
		sgnode *node;
		property_map props;
		
		// these fields are used by the model learning system
		std::vector<double> dists;
		mutable int closest;
		mutable bool rels_dirty;
	};
	
	typedef std::vector<node_info> node_table;
	
	std::string  name;
	group_node  *root;
	svs         *owner;
	logger_set  *loggers;
	node_table   nodes;
	bool         draw;
	bool         sig_dirty;
	bool         track_dists;
	mutable bool closest_dirty;
	
	mutable scene_sig sig;
	
	mutable relation_table cached_rels;
	relation_table type_rels;

	node_info       *find_name(const std::string &name);
	const node_info *find_name(const std::string &name) const;
	
	group_node *get_group(const std::string &name);
	void update_sig() const;
	void get_property_names(int i, std::vector<std::string> &names) const;

	int parse_add(std::vector<std::string> &f, std::string &error);
	int parse_del(std::vector<std::string> &f, std::string &error);
	int parse_change(std::vector<std::string> &f, std::string &error);
	int parse_property(std::vector<std::string> &f, std::string &error);

	void update_closest() const;
	void update_dists(int i);
	void update_all_dists();
	
	void cli_props(const std::vector<std::string> &args, std::ostream &os) const;
	void cli_dist(const std::vector<std::string> &args, std::ostream &os) const;
	void cli_sgel(const std::vector<std::string> &args, std::ostream &os);
	void cli_relations(const std::vector<std::string> &args, std::ostream &os) const;
	void cli_draw(const std::vector<std::string> &args, std::ostream &os);
};

#endif
