#include "filter_table.h"
#include "scene.h"

using namespace std;

const filter_table& get_filter_table() {
	static filter_table inst;
	return inst;
}

filter_table_entry intersect_fill_entry();
filter_table_entry distance_fill_entry();
filter_table_entry bbox_fill_entry();
filter_table_entry bbox_int_fill_entry();
filter_table_entry bbox_contains_fill_entry();
filter_table_entry ontop_fill_entry();
filter_table_entry north_of_fill_entry();
filter_table_entry south_of_fill_entry();
filter_table_entry east_of_fill_entry();
filter_table_entry west_of_fill_entry();
filter_table_entry x_overlap_fill_entry();
filter_table_entry y_overlap_fill_entry();
filter_table_entry z_overlap_fill_entry();
filter_table_entry above_fill_entry();
filter_table_entry below_fill_entry();
filter_table_entry node_fill_entry();
filter_table_entry all_nodes_fill_entry();
filter_table_entry node_centroid_fill_entry();
filter_table_entry compare_fill_entry();
filter_table_entry absval_fill_entry();
filter_table_entry vec3_fill_entry();
filter_table_entry max_fill_entry();
filter_table_entry closest_fill_entry();

filter_table::filter_table() {
	add(intersect_fill_entry());
	add(distance_fill_entry());
	add(bbox_fill_entry());
	add(bbox_int_fill_entry());
	add(bbox_contains_fill_entry());
	add(ontop_fill_entry());
	add(north_of_fill_entry());
	add(south_of_fill_entry());
	add(east_of_fill_entry());
	add(west_of_fill_entry());
	add(x_overlap_fill_entry());
	add(y_overlap_fill_entry());
	add(z_overlap_fill_entry());
	add(above_fill_entry());
	add(below_fill_entry());
	add(node_fill_entry());
	add(all_nodes_fill_entry());
	add(node_centroid_fill_entry());
	add(compare_fill_entry());
	add(absval_fill_entry());
	add(vec3_fill_entry());
	add(max_fill_entry());
	add(closest_fill_entry());
}

template <typename T>
class single_combination_generator {
public:
	single_combination_generator(const std::vector<T> &elems, int n, bool ordered, bool allow_repeat)
	: elems(elems), indices(n), nelems(elems.size()), n(n), 
	  ordered(ordered), allow_repeat(allow_repeat), finished(false)
	{
		assert(n <= nelems);
		reset();
	}

	void reset() {
		if (!ordered && !allow_repeat) {
			for (int i = 0; i < n; ++i) {
				indices[i] = n - i - 1;
			}
		} else {
			fill(indices.begin(), indices.end(), 0);
		}
	}

	bool next(std::vector<T> &comb) {
		if (nelems == 0 || n == 0) {
			return false;
		}
		
		comb.resize(n);
		std::set<int> s;
		while (!finished) {
			bool has_repeat = false;
			s.clear();
			for (int i = 0; i < n; ++i) {
				comb[i] = elems[indices[i]];
				if (!has_repeat && !allow_repeat && ordered) {
					/*
					 incrementing technique guarantees no
					 repeats in the case ordered = false
					 and allow_repeats = false
					*/
					std::pair<std::set<int>::iterator, bool> p = s.insert(indices[i]);
					if (!p.second) {
						has_repeat = true;
					}
				}
			}
			increment(nelems - 1, 0);
			if (allow_repeat || !has_repeat) {
				return true;
			}
		}
		return false;
	}

private:
	int increment(int max, int i) {
		if (i == n - 1) {
			if (++indices[i] > max) {
				finished = true;
			}
			return indices[i];
		}
		if (++indices[i] > max) {
			if (ordered) {
				increment(max, i + 1);
				indices[i] = 0;
			} else {
				if (allow_repeat) {
					// maintain indices[i] >= indices[i+1]
					indices[i] = increment(max, i + 1);
				} else {
					// maintain indices[i] > indices[i+1]
					indices[i] = increment(max - 1, i + 1) + 1;
				}
			}
		}
		return indices[i];
	}

	const std::vector<T> &elems;
	std::vector<int> indices;
	int n, nelems;
	bool ordered, allow_repeat, finished;
};

filter* filter_table::make_filter(const std::string &pred, Symbol *root, soar_interface *si, scene *scn, filter_input *input) const
{
	std::map<std::string, filter_table_entry>::const_iterator i = t.find(pred);
	if (i == t.end() || i->second.create == NULL) {
		return NULL;
	}
	return (*(i->second.create))(root, si, scn, input);
}

void filter_table::get_all_atoms(scene *scn, vector<string> &atoms) const {
	vector<const sgnode*> all_nodes;
	vector<string> all_node_names;
	scn->get_all_nodes(all_nodes);
	all_nodes.erase(all_nodes.begin()); // don't use world node
	
	all_node_names.reserve(all_nodes.size());
	for (int i = 0; i < all_nodes.size(); ++i) {
		all_node_names.push_back(all_nodes[i]->get_name());
	}
	
	map<string, filter_table_entry>::const_iterator i;
	for(i = t.begin(); i != t.end(); ++i) {
		const filter_table_entry &e = i->second;
		if (e.calc != NULL) {
			vector<string> args;
			single_combination_generator<string> gen(all_node_names, e.parameters.size(), e.ordered, e.allow_repeat);
			while (gen.next(args)) {
				stringstream ss;
				ss << e.name << "(";
				for (int j = 0; j < args.size() - 1; ++j) {
					ss << args[j] << ",";
				}
				ss << args.back() << ")";
				atoms.push_back(ss.str());
				args.clear();
			}
		}
	}
}

void filter_table::update_relations(const scene *scn, int time, relation_table &rt) const {
	vector<int> node_inds;
	scn->get_all_node_indices(node_inds);
	node_inds.erase(node_inds.begin());
	
	map<string, filter_table_entry>::const_iterator i;
	for(i = t.begin(); i != t.end(); ++i) {
		const filter_table_entry &e = i->second;
		if (e.calc != NULL && node_inds.size() >= e.parameters.size()) {
			relation &r = rt[e.name];
			if (r.arity() == 0) {
				// +1 for the time argument
				r.reset(e.parameters.size() + 1);
			}
			vector<const sgnode*> args;
			tuple inds;
			single_combination_generator<int> gen(node_inds, e.parameters.size(), e.ordered, e.allow_repeat);
			while (gen.next(inds)) {
				scn->get_nodes(inds, args);
				timer &t = timers.get_or_add(i->first.c_str());
				t.start();
				bool pos = (*e.calc)(scn, args);
				t.stop();
				if (pos) {
					if (e.ordered) {
						r.add(time, inds);
					} else {
						// true for all permutations
						single_combination_generator<int> gen2(inds, inds.size(), true, e.allow_repeat);
						tuple perm;
						while (gen2.next(perm)) {
							r.add(time, perm);
						}
					}
				}
				inds.clear();
			}
		}
	}
	
	// add type relations
	for (int j = 0; j < node_inds.size(); ++j) {
		string type = scn->get_node(node_inds[j])->get_type();
		if (!has(rt, type)) {
			rt[type] = relation(2);
		}
		relation &type_rel = rt[type];
		type_rel.add(0, node_inds[j]);
	}
}
