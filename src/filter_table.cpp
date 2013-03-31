#include "filter_table.h"
#include "scene.h"

using namespace std;

const filter_table& get_filter_table() {
	static filter_table inst;
	return inst;
}

filter_table_entry intersect_fill_entry();
filter_table_entry distance_fill_entry();
filter_table_entry distance_xyz_fill_entry();
filter_table_entry smaller_fill_entry();
filter_table_entry bbox_fill_entry();
filter_table_entry bbox_int_fill_entry();
filter_table_entry bbox_contains_fill_entry();
filter_table_entry ontop_fill_entry();
filter_table_entry between_fill_entry();
filter_table_entry behind_fill_entry();
filter_table_entry north_of_fill_entry();
filter_table_entry south_of_fill_entry();
filter_table_entry east_of_fill_entry();
filter_table_entry west_of_fill_entry();
filter_table_entry horizontally_aligned_fill_entry();
filter_table_entry vertically_aligned_fill_entry();
filter_table_entry planar_aligned_fill_entry();
filter_table_entry above_fill_entry();
filter_table_entry below_fill_entry();
filter_table_entry node_fill_entry();
filter_table_entry all_nodes_fill_entry();
filter_table_entry node_centroid_fill_entry();
filter_table_entry compare_fill_entry();
filter_table_entry absval_fill_entry();
filter_table_entry gen_node_fill_entry();
filter_table_entry vec3_fill_entry();
filter_table_entry max_fill_entry();
filter_table_entry closest_fill_entry();

filter_table::filter_table() {
	add(intersect_fill_entry());
	add(distance_fill_entry());
	add(distance_xyz_fill_entry());
	add(smaller_fill_entry());
	add(bbox_fill_entry());
	add(bbox_int_fill_entry());
	add(bbox_contains_fill_entry());
	add(ontop_fill_entry());
	add(between_fill_entry());
	add(behind_fill_entry());
	add(north_of_fill_entry());
	add(south_of_fill_entry());
	add(east_of_fill_entry());
	add(west_of_fill_entry());
	add(horizontally_aligned_fill_entry());
	add(vertically_aligned_fill_entry());
	add(planar_aligned_fill_entry());
	add(above_fill_entry());
	add(below_fill_entry());
	add(node_fill_entry());
	add(all_nodes_fill_entry());
	add(node_centroid_fill_entry());
	add(compare_fill_entry());
	add(absval_fill_entry());
	add(gen_node_fill_entry());
	add(vec3_fill_entry());
	add(max_fill_entry());
	add(closest_fill_entry());
	
	std::map<std::string, filter_table_entry>::const_iterator i;
	for (i = t.begin(); i != t.end(); ++i) {
		timers.add(i->first);
	}
}

filter* filter_table::make_filter(const std::string &pred, Symbol *root, soar_interface *si, scene *scn, filter_input *input) const
{
	std::map<std::string, filter_table_entry>::const_iterator i = t.find(pred);
	if (i == t.end() || i->second.create == NULL) {
		return NULL;
	}
	return (*(i->second.create))(root, si, scn, input);
}

template <typename T>
class combination_generator {
public:
	combination_generator(const std::vector<T> &elems, int n, bool ordered, bool allow_repeat)
	: elems(elems), indices(n, 0), nelems(elems.size()), n(n), 
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

void filter_table::get_all_atoms(scene *scn, vector<string> &atoms) const {
	vector<const sgnode*> all_nodes;
	vector<string> all_node_names;
	scn->get_all_nodes(all_nodes);
	all_node_names.reserve(all_nodes.size());
	for (int i = 0; i < all_nodes.size(); ++i) {
		all_node_names.push_back(all_nodes[i]->get_name());
	}
	
	map<string, filter_table_entry>::const_iterator i;
	for(i = t.begin(); i != t.end(); ++i) {
		const filter_table_entry &e = i->second;
		if (e.calc != NULL) {
			vector<string> args;
			combination_generator<string> gen(all_node_names, e.parameters.size(), e.ordered, e.allow_repeat);
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

void filter_table::calc_all_atoms(scene *scn, boolvec &results) const {
	vector<const sgnode*> all_nodes;
	scn->get_all_nodes(all_nodes);
	
	map<string, filter_table_entry>::const_iterator i;
	int ii = 0;
	for(i = t.begin(); i != t.end(); ++i, ++ii) {
		const filter_table_entry &e = i->second;
		if (e.calc != NULL) {
			vector<const sgnode*> args;
			combination_generator<const sgnode*> gen(all_nodes, e.parameters.size(), e.ordered, e.allow_repeat);
			while (gen.next(args)) {
				timers.start(ii);
				results.push_back((*e.calc)(scn, args));
				timers.stop(ii);
				args.clear();
			}
		}
	}
}
