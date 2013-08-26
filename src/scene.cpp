#include <cstdlib>
#include <map>
#include <iterator>
#include <iostream>
#include <sstream>
#include <limits>
#include <utility>
#include "scene.h"
#include "sgnode.h"
#include "common.h"
#include "drawer.h"
#include "filter.h"
#include "filter_table.h"
#include "params.h"
#include "svs.h"
#include "logger.h"

using namespace std;

const string root_name = "world";

/*
 Native properties are currently the position, rotation, and scaling
 transforms of a node, named px, py, pz, rx, ry, rz, sx, sy, sz.
*/
const char *NATIVE_PROPS[] = { "px", "py", "pz", "rx", "ry", "rz", "sx", "sy", "sz" };

bool is_native_prop(const std::string &name, char &type, int &dim) {
	int d;
	if (name.size() != 2) {
		return false;
	}
	if (name[0] != 'p' && name[0] != 'r' && name[0] != 's') {
		return false;
	}
	d = name[1] - 'x';
	if (d < 0 || d > 2) {
		return false;
	}
	type = name[0];
	dim = d;
	return true;
}

bool parse_vec3(vector<string> &f, int &start, vec3 &v, string &error) {
	for (int i = 0; i < 3; ++start, ++i) {
		if (start >= f.size() || !parse_double(f[start], v[i])) {
			error = "expecting a number";
			return false;
		}
	}
	return true;
}

bool parse_verts(vector<string> &f, int &start, ptlist &verts, string &error) {
	verts.clear();
	while (start < f.size()) {
		vec3 v;
		int i = start;
		if (!parse_vec3(f, start, v, error)) {
			return (i == start);  // end of list
		}
		verts.push_back(v);
	}
	return true;
}

bool parse_transforms(vector<string> &f, int &start, vec3 &pos, vec3 &rot, vec3 &scale, string &error) {	
	vec3 t;
	char type;
	
	if (f[start] != "p" && f[start] != "r" && f[start] != "s") {
		error = "expecting p, r, or s";
		return false;
	}
	type = f[start++][0];
	if (!parse_vec3(f, start, t, error)) {
		return false;
	}
	switch (type) {
		case 'p':
			pos = t;
			break;
		case 'r':
			rot = t;
			break;
		case 's':
			scale = t;
			break;
		default:
			assert(false);
	}
	return true;
}

scene::scene(const string &name, svs *owner) 
: name(name), owner(owner), draw(false), nodes(1), track_dists(false)
{
	root = new group_node(root_name, "world");
	nodes[0].node = root;
	root->listen(this);
	sig_dirty = true;
	closest_dirty = false;
	loggers = owner->get_loggers();
}

scene::~scene() {
	root->unlisten(this);
	delete root;
}

scene *scene::clone(const string &cname) const {
	scene *c;
	string name;
	std::vector<sgnode*> node_clones;

	update_closest();
	c = new scene(cname, owner);
	delete c->root;
	c->nodes = nodes;
	c->root = root->clone()->as_group(); // root->clone copies entire scene graph
	c->root->walk(node_clones);
	for(int i = 0, iend = node_clones.size(); i < iend; ++i) {
		sgnode *n = node_clones[i];
		c->find_name(n->get_name())->node = n;
		n->listen(c);
	}
	return c;
}

scene::node_info *scene::find_name(const string &name) {
	node_table::iterator i, iend;
	for (i = nodes.begin(), iend = nodes.end(); i != iend; ++i) {
		if (i->node->get_name() == name) {
			return &(*i);
		}
	}
	return NULL;
}

const scene::node_info *scene::find_name(const string &name) const {
	node_table::const_iterator i, iend;
	for (i = nodes.begin(), iend = nodes.end(); i != iend; ++i) {
		if (i->node->get_name() == name) {
			return &(*i);
		}
	}
	return NULL;
}

sgnode *scene::get_node(const string &name) {
	node_info *info = find_name(name);
	if (!info) {
		return NULL;
	}
	return info->node;
}

sgnode const *scene::get_node(const string &name) const {
	const node_info *info = find_name(name);
	if (!info) {
		return NULL;
	}
	return info->node;
}

sgnode *scene::get_node(int id) {
	node_table::iterator i, iend;
	for (i = nodes.begin(), iend = nodes.end(); i != iend; ++i) {
		if (i->node->get_id() == id) {
			return i->node;
		}
	}
	return NULL;
}

const sgnode *scene::get_node(int id) const {
	node_table::const_iterator i, iend;
	for (i = nodes.begin(), iend = nodes.end(); i != iend; ++i) {
		if (i->node->get_id() == id) {
			return i->node;
		}
	}
	return NULL;
}

group_node *scene::get_group(const string &name) {
	sgnode *n = get_node(name);
	if (n) {
		return n->as_group();
	}
	return NULL;
}

void scene::get_all_nodes(vector<sgnode*> &n) {
	n.resize(nodes.size());
	for (int i = 0, iend = nodes.size(); i < iend; ++i) {
		n[i] = nodes[i].node;
	}
}

void scene::get_all_nodes(vector<const sgnode*> &n) const {
	n.resize(nodes.size());
	for (int i = 0, iend = nodes.size(); i < iend; ++i) {
		n[i] = nodes[i].node;
	}
}

bool scene::add_node(const string &name, sgnode *n) {
	group_node *par = get_group(name);
	if (!par) {
		return false;
	}
	par->attach_child(n);
	/* rest is handled in node_update */
	return true;
}

bool scene::del_node(const string &name) {
	node_info *ni = find_name(name);
	if (ni) {
		delete ni->node;
		/* rest is handled in node_update */
		return true;
	}
	return false;
}

void scene::clear() {
	for (int i = root->num_children() - 1; i >= 0; --i) {
		delete root->get_child(i);
	}
}

enum node_class { CONVEX_NODE, BALL_NODE, GROUP_NODE };

bool parse_mods(vector<string> &f, int &start, string &mods, vector<ptlist> &vals, string &error) {
	ptlist v;
	char m;
	while (start < f.size()) {
		if (f[start].size() != 1) {
			return true;
		}
		m = f[start][0];
		v.clear();
		switch (m) {
			case 'p':
			case 'r':
			case 's':
				v.push_back(vec3());
				if (!parse_vec3(f, ++start, v[0], error)) {
					return false;
				}
				break;
			case 'v':
				if (!parse_verts(f, ++start, v, error)) {
					return false;
				}
				break;
			case 'b':
				++start;
				v.push_back(vec3());
				if (start >= f.size() || !parse_double(f[start], v[0](0))) {
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

int scene::parse_add(vector<string> &f, string &error) {
	int p;
	sgnode *n = NULL;
	group_node *par = NULL;
	string name, type, mods;
	vector<ptlist> vals;
	ptlist vertices;
	double radius;
	bool is_convex, is_ball;

	if (f.size() < 2) {
		return f.size();
	}
	name = f[0], type = f[1];
	if (get_node(name)) {
		error = "node already exists";
		return 0;
	}
	par = get_group(f[2]);
	if (!par) {
		error = "parent node does not exist, or is not group node";
		return 1;
	}
	
	p = 3;
	if (!parse_mods(f, p, mods, vals, error)) {
		return p;
	}
	assert(mods.size() == vals.size());
	
	/*
	 Go through once to figure out what type of node this should be
	*/
	is_convex = false;
	is_ball = false;
	for (int i = 0, iend = mods.size(); i < iend; ++i) {
		switch (mods[i]) {
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
	if (is_convex && is_ball) {
		error = "conflicting node type";
		return 0; // don't know how to find a more accurate position
	} else if (is_convex) {
		n = new convex_node(name, type, vertices);
	} else if (is_ball) {
		n = new ball_node(name, type, radius);
	} else {
		n = new group_node(name, type);
	}
	
	/*
	 Go through again to apply transforms
	*/
	for (int i = 0, iend = mods.size(); i < iend; ++i) {
		switch (mods[i]) {
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

int scene::parse_del(vector<string> &f, string &error) {
	if (f.size() < 1) {
		error = "expecting node name";
		return f.size();
	}
	if (!del_node(f[0])) {
		error = "node does not exist";
		return 0;
	}
	return -1;
}

int scene::parse_change(vector<string> &f, string &error) {
	int p;
	sgnode *n;
	convex_node *cn;
	ball_node *bn;
	string mods;
	vector<ptlist> vals;

	if (f.size() < 1) {
		error = "expecting node name";
		return f.size();
	}
	if (!(n = get_node(f[0]))) {
		error = "node does not exist";
		return 0;
	}
	
	p = 1;
	if (!parse_mods(f, p, mods, vals, error)) {
		return p;
	}
	
	for (int i = 0, iend = mods.size(); i < iend; ++i) {
		switch (mods[i]) {
			case 'p':
			case 'r':
			case 's':
				n->set_trans(mods[i], vals[i][0]);
				break;
			case 'v':
				cn = dynamic_cast<convex_node*>(n);
				if (!cn) {
					return 0; // maybe not as informative as it could be
				}
				cn->set_verts(vals[i]);
				break;
			case 'b':
				bn = dynamic_cast<ball_node*>(n);
				if (!bn) {
					return 0;
				}
				bn->set_radius(vals[i][0](0));
				break;
		}
	}
	return -1;
}

// parse_property(vector<string> &f, string &error)
//   parses a property command (command 'p')
//	 f is a list of the parameters given
//   Changed the format of the command to be
//     p <name> <subcommand> <property> <value?>
//		<name> - name of the node
//      <subcommand> - either 'a' for add, 'c' for change, or 'd' for delete
//		<property> - the name of the property
// 		<value?> - the value of the property, needed for the add or change subcommands only
int scene::parse_property(vector<string> &f, string &error) {
	int p = 0;
	
	// Parameter 1: Node Name
	if (p >= f.size()) {
		error = "Property Command P1: Expecting node name";
		return p;
	}
	string name = f[p++];
	
	node_info *ninfo = find_name(name);
	if (!ninfo) {
		error = "Property Command P1: Node " + name + " does not exist";
		return (p-1);
	}

	// Parameter 2: Subcommand (a = add, d = delete, c = change)
	if (p >= f.size() || f[p].length() == 0){
		error = "Property Command P2: Expecting subcommand";
		return p;
	}
	char subcommand = tolower(f[p++][0]);

	// Parameter 3: Property Name
	if (p >= f.size()){
		error = "Property Command P3: Expecting property name";
		return p;
	}
	string propName = f[p++];

	// Parameter 4 (For add/change): property value
	string value;
	if(subcommand == 'a' || subcommand == 'c'){
		if(p >= f.size()){
			error = "Property Command P4: Expecting value for " + propName;
			return p;
		}
		value = f[p++];
	}

	switch(subcommand){
	case 'a':
		// Add property
	case 'c':
		// Change property
		ninfo->node->set_property(propName, value);
		break;
	case 'd':
		// Delete property
		ninfo->node->delete_property(propName);
		break;
	default:
		error = "Property Command P3: Unrecognized subcommand (Expecting a, c, d)";
		return 2;
	}

	return -1;
}

bool scene::parse_sgel(const string &s) {
	vector<string> lines, fields;
	vector<string>::iterator i;
	char cmd;
	int errfield;
	string error;
	
	loggers->get(LOG_SGEL) << s << endl;
	split(s, "\n", lines);
	for (i = lines.begin(); i != lines.end(); ++i) {
		split(*i, "", fields);
		
		if (fields.size() == 0)
			continue;
		if (fields[0].size() != 1) {
			loggers->get(LOG_ERR) << "expecting a|d|c|p at start of line '" << *i << "'" << endl;
			return false;
		}
		
		cmd = fields[0][0];
		fields.erase(fields.begin());
		error = "unknown error";
		
		switch(cmd) {
			case 'a':
				errfield = parse_add(fields, error);
				break;
			case 'd':
				errfield = parse_del(fields, error);
				break;
			case 'c':
				errfield = parse_change(fields, error);
				break;
			case 'p':
				errfield = parse_property(fields, error);
				break;
			default:
				loggers->get(LOG_ERR) << "expecting a|d|c|p at start of line '"
				                       << *i << "'" << endl;
				return false;
		}
		
		if (errfield >= 0) {
			loggers->get(LOG_ERR) << "error in field " << errfield + 1 << " of line '"
			                       << *i << "': " << error << endl;
			return false;
		}
	}
	return true;
}

void scene::get_properties(rvec &vals) const {
	node_table::const_iterator i, iend;
	property_map::const_iterator j, jend;
	int k = 0;
	
	vals.resize(get_dof());
	for (int i = 0, iend = nodes.size(); i < iend; ++i) {
		const node_info &info = nodes[i];
		for (const char *t = "prs"; *t != '\0'; ++t) {
			vec3 trans = info.node->get_trans(*t);
			vals.segment(k, 3) = trans;
			k += 3;
		}
		
		const numeric_properties_map &pm = info.node->get_numeric_properties();
		numeric_properties_map::const_iterator j, jend;
		for (j = pm.begin(), jend = pm.end(); j != jend; ++j) {
			vals(k++) = j->second;
		}
	}
}

bool scene::set_properties(const rvec &vals) {
	const char *types = "prs";
	vec3 trans;
	int l = 0;
	
	for (int i = 0, iend = nodes.size(); i < iend; ++i) {
		sgnode *n = nodes[i].node;
		int k1, k2;
		for (k1 = 0; k1 < 3; ++k1) {
			for (k2 = 0; k2 < 3; ++k2) {
				if (l >= vals.size()) {
					return false;
				}
				trans(k2) = vals(l++);
			}
			n->set_trans(types[k1], trans);
		}
		
		const numeric_properties_map &pm = n->get_numeric_properties();
		numeric_properties_map::const_iterator j, jend;
		for (j = pm.begin(), jend = pm.end(); j != jend; ++j) {
			if (l >= vals.size()) {
				return false;
			}
			n->set_property(j->first, vals(l++));
		}
	}
	return true;
}

int scene::get_dof() const {
	int dof = 0;
	for (int i = 0, iend = nodes.size(); i < iend; ++i) {
		dof += NUM_NATIVE_PROPS + nodes[i].node->get_numeric_properties().size();
	}
	return dof;
}

void velocity_hack(const sgnode *n, drawer *d) {
	if (n->get_name() != "b1") {
		return;
	}
	vec3 pos = n->get_trans('p');
	stringstream ss;
	ss << "* vx_pred_line p " << pos(0) << " " << pos(1) << " " << pos(2) << endl;
	ss << "* vz_pred_line p " << pos(0) << " " << pos(1) << " " << pos(2) << endl;
	ss << "* pred_line    p " << pos(0) << " " << pos(1) << " " << pos(2) << endl;
	d->send(ss.str());
}

void scene::node_update(sgnode *n, sgnode::change_type t, const std::string& update_info) {
	sgnode *child;
	group_node *g;
	relation *tr;
	drawer *d = owner->get_drawer();
	
	if (t == sgnode::CHILD_ADDED) {
		int added_child = 0;
		if(!parse_int(update_info, added_child)){
			return;
		}
		g = n->as_group();
		child = g->get_child(added_child);
		child->listen(this);
		node_info &ninfo = grow_vec(nodes);
		ninfo.node = child;
		sig_dirty = true;
		update_dists(nodes.size() - 1);
		
		tr = map_getp(type_rels, child->get_type());
		if (!tr) {
			tr = &type_rels[child->get_type()];
			tr->reset(2);
		}
		tr->add(0, child->get_id());
		
		if (draw) {
			d->add(name, child);
		}
		return;
	}
	
	int i, iend;
	for (i = 0, iend = nodes.size(); i < iend && nodes[i].node != n; ++i)
		;
	assert(i != nodes.size());
	if (i == 0) {
		return;
	}
	
	switch (t) {
		case sgnode::DELETED:
			nodes.erase(nodes.begin() + i);
			
			if (track_dists) {
				// update distance vectors for other nodes
				for (int j = 1, jend = nodes.size(); j < jend; ++j) {
					node_info &info = nodes[j];
					assert(info.dists.size() == nodes.size() + 1);
					info.dists.erase(info.dists.begin() + i);
				}
				closest_dirty = true;
			}
			tr = map_getp(type_rels, n->get_type());
			if (tr) {
				tr->del(0, i);
			}
			sig_dirty = true;
			if (draw && i != 0) {
				d->del(name, n);
			}
			break;
		case sgnode::SHAPE_CHANGED:
			update_dists(i);
			if (!n->is_group() && draw) {
				d->change(name, n, drawer::SHAPE);
			}
			nodes[i].rels_dirty = true;
			break;
		case sgnode::TRANSFORM_CHANGED:
			update_dists(i);
			if (draw) {
				d->change(name, n, drawer::POS | drawer::ROT | drawer::SCALE);
			}
			nodes[i].rels_dirty = true;
			velocity_hack(n, owner->get_drawer());
			break;
		case sgnode::PROPERTY_CHANGED:
			sig_dirty = true;
			nodes[i].rels_dirty = true;
			break;
	}
}

double scene::get_convex_distance(const sgnode *a, const sgnode *b) const {
	if (!track_dists) {
		return convex_distance(a, b);
	}
	
	int i, j;
	for (i = 0; i < nodes.size() && nodes[i].node != a; ++i)
		;
	for (j = 0; j < nodes.size() && nodes[j].node != b; ++j)
		;
	assert(i != nodes.size() && j != nodes.size());
	return nodes[i].dists[j];
}

bool scene::intersects(const sgnode *a, const sgnode *b) const {
	return this->get_convex_distance(a, b) < INTERSECT_THRESH;
}

void scene::update_dists(int i) {
	if (i == 0 || !track_dists)
		return;
	
	node_info &n1 = nodes[i];
	n1.dists.resize(nodes.size(), -1);
	n1.dists[0] = 0.0;
	n1.dists[i] = 0.0;
	for (int j = 1, jend = nodes.size(); j < jend; ++j) {
		if (i == j)
			continue;
		
		node_info &n2 = nodes[j];
		n2.dists.resize(nodes.size(), -1);
		double d = convex_distance(n1.node, n2.node);
		n1.dists[j] = d;
		n2.dists[i] = d;
	}
	closest_dirty = true;
}

void scene::update_closest() const {
	if (!closest_dirty || !track_dists)
		return;
	
	for (int i = 1, iend = nodes.size(); i < iend; ++i) {
		int c = -1;
		const vector<double> &d = nodes[i].dists;
		for (int j = 1, jend = d.size(); j < jend; ++j) {
			if (i != j && d[j] >= 0 && (c < 0 || d[j] < d[c])) {
				c = j;
			}
		}
		nodes[i].closest = c;
	}
	closest_dirty = false;
}

void scene::update_all_dists() {
	for (int i = 1, iend = nodes.size(); i < iend; ++i) {
		nodes[i].dists.resize(nodes.size(), -1);
	}
	for (int i = 1, iend = nodes.size(); i < iend; ++i) {
		for (int j = i + 1, jend = nodes.size(); j < jend; ++j) {
			double d = convex_distance(nodes[i].node, nodes[j].node);
			nodes[i].dists[j] = nodes[j].dists[i] = d;
		}
	}
	closest_dirty = true;
	update_closest();
}

void scene::set_track_distances(bool v) {
	if (!track_dists && v) {
		update_all_dists();
	}
	track_dists = v;
}

void scene::update_sig() const {
	vector<string> common_props;
	for (int j = 0; j < NUM_NATIVE_PROPS; ++j) {
		common_props.push_back(NATIVE_PROPS[j]);
	}
	sig.clear();
	for (int i = 0, iend = nodes.size(); i < iend; ++i) {
		scene_sig::entry e;
		numeric_properties_map::const_iterator j, jend;
		const numeric_properties_map &pm = nodes[i].node->get_numeric_properties();
		
		e.id = nodes[i].node->get_id();
		e.name = nodes[i].node->get_name();
		e.type = nodes[i].node->get_type();
		
		e.props = common_props;
		for (j = pm.begin(), jend = pm.end(); j != jend; ++j) {
			e.props.push_back(j->first);
		}
		sig.add(e);
	}
}

const scene_sig &scene::get_signature() const {
	if (sig_dirty) {
		update_sig();
	}
	return sig;
}

void scene::get_relations(relation_table &rt) const {
	int_tuple closest(2);
	vector<int> dirty_nodes;
	
	rt = type_rels;
	relation &closest_rel = rt["closest"];
	closest_rel.reset(3);
	update_closest();
	for (int i = 1, iend = nodes.size(); i < iend; ++i) {
		const node_info &ni = nodes[i];
		if (ni.rels_dirty) {
			dirty_nodes.push_back(ni.node->get_id());
			ni.rels_dirty = false;
		}
		if (ni.closest != -1) {
			closest[0] = ni.node->get_id();
			closest[1] = nodes[ni.closest].node->get_id();
			closest_rel.add(0, closest);
		} else {
			// if there's only one real object, then it won't be closest to anything
			assert(nodes.size() == 2);
		}
	}
	
	relation_table::iterator j, jend;
	for (j = cached_rels.begin(), jend = cached_rels.end(); j != jend; ++j) {
		relation &r = j->second;
		for (int k = 1, kend = r.arity(); k < kend; ++k) {
			r.filter(k, dirty_nodes, true);
		}
	}
	get_filter_table().update_relations(this, dirty_nodes, 0, cached_rels);
	
	for (j = cached_rels.begin(), jend = cached_rels.end(); j != jend; ++j) {
		rt[j->first] = j->second;
	}
}

void scene::proxy_get_children(map<string, cliproxy*> &c) {
	c["world"] = root;
	
	c["properties"] = new memfunc_proxy<scene>(this, &scene::cli_props);
	c["properties"]->set_help("Get scene properties.");
	
	c["distance"] = new memfunc_proxy<scene>(this, &scene::cli_dist);
	c["distance"]->set_help("Compute distance between nodes.")
	               .add_arg("N1", "First node name.")
	               .add_arg("N2", "Second node name.")
	               ;
	
	c["sgel"] = new memfunc_proxy<scene>(this, &scene::cli_sgel);
	c["sgel"]->set_help("Modify scene graph with SGEL.")
	           .add_arg("SGEL", "SGEL string (spaces are okay).")
	           ;

	c["relations"] = new memfunc_proxy<scene>(this, &scene::cli_relations);
	c["relations"]->set_help("Prints all true relations if called without arguments, "
	                         "otherwise print only matching relations.")
	                .add_arg("[RELATION]", "Relation name pattern. Can be * for any.")
	                .add_arg("[PARAMS]",   "Argument patterns. Can be * for any.")
	                ;

	c["draw"] = new memfunc_proxy<scene>(this, &scene::cli_draw);
	c["draw"]->set_help("Draw this scene in the viewer.")
	           .add_arg("[VALUE]", "New value. Must be (0|1|on|off|true|false).");

	c["clear"] = new memfunc_proxy<scene>(this, &scene::cli_clear);
	c["clear"]->set_help("Delete all objects in scene except world");
}

void scene::cli_props(const vector<string> &args, ostream &os) const {
	rvec vals;
	table_printer t;
	
	get_properties(vals);
	int i = 0;
	for (int j = 0, jend = sig.size(); j < jend; ++j) {
		for (int k = 0, kend = sig[j].props.size(); k < kend; ++k) {
			t.add_row() << sig[j].name + ':' + sig[j].props[k] << vals(i++);
		}
	}
	t.print(os);
}

void scene::cli_dist(const vector<string> &args, ostream &os) const {
	if (args.size() != 2) {
		os << "specify two nodes" << endl;
		return;
	}
	
	int i0 = -1, i1 = -1;
	for (int i = 0, iend = nodes.size(); i < iend; ++i) {
		if (nodes[i].node->get_name() == args[0]) {
			i0 = i;
		} else if (nodes[i].node->get_name() == args[1]) {
			i1 = i;
		}
	}
	if (i0 < 0) {
		os << "node " << args[0] << " does not exist" << endl;
		return;
	}
	if (i1 < 0) {
		os << "node " << args[1] << " does not exist" << endl;
		return;
	}
	os << get_convex_distance(nodes[i0].node, nodes[i1].node) << endl;
}

void scene::cli_sgel(const vector<string> &args, ostream &os) {
	stringstream ss;
	for (int i = 0, iend = args.size(); i < iend; ++i) {
		ss << args[i] << " ";
	}
	parse_sgel(ss.str());
}

void scene::cli_relations(const vector<string> &args, ostream &os) const {
	relation_table rels;
	relation_table::const_iterator i, begin, end;
	get_relations(rels);
	bool print_names;
	
	if (!args.empty() && args[0] != "*") {
		begin = end = rels.find(args[0]);
		if (end == rels.end()) {
			os << "relation not found" << endl;
			return;
		} else {
			++end;
		}
		print_names = false;
	} else {
		begin = rels.begin();
		end = rels.end();
		print_names = true;
	}
	
	vector<int> ids;
	for (int j = 1; j < args.size(); ++j) {
		if (args[j] != "*") {
			const sgnode *n = get_node(args[j]);
			if (!n) {
				os << "object " << args[j] << " not found" << endl;
				return;
			}
			ids.push_back(n->get_id());
		} else {
			ids.push_back(-1);
		}
	}
	
	for (i = begin; i != end; ++i) {
		relation r = i->second;
		int_tuple t(1);
		
		for (int j = 0, jend = min(int(ids.size()), r.arity() - 1); j < jend; ++j) {
			if (ids[j] != -1) {
				t[0] = ids[j];
				r.filter(j + 1, t, false);
			}
		}
		
		if (r.empty()) {
			continue;
		}
		
		if (print_names)
			os << i->first << endl;
		
		table_printer p;
		relation::const_iterator j;
		for (j = r.begin(); j != r.end(); ++j) {
			p.add_row();
			for (int k = 1; k < j->size(); ++k) {
				const sgnode *n = get_node((*j)[k]);
				assert(n != NULL);
				p << n->get_name();
			}
		}
		p.print(os);
	}
}

void scene::cli_draw(const vector<string> &args, ostream &os) {
	bool_proxy p(&draw, "");
	bool old_draw = draw;
	
	p.proxy_use("", args, os);
	if (!old_draw && draw) {
		refresh_draw();
	} else if (old_draw && !draw) {
		owner->get_drawer()->delete_scene(name);
	}
}

void scene::cli_clear(const vector<string> &args, ostream &os) {
	clear();
}

void scene::refresh_draw() {
	if (!draw) return;
	
	drawer *d = owner->get_drawer();
	d->delete_scene(name);
	for (int i = 1, iend = nodes.size(); i < iend; ++i) {
		d->add(name, nodes[i].node);
	}
}

void scene::verify_listeners() const {
	for (int i = 0, iend = nodes.size(); i < iend; ++i) {
		std::list<sgnode_listener*> l;
		nodes[i].node->get_listeners(l);
		assert(l.size() == 1 && l.front() == this);
	}
}
