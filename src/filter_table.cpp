#include "filter_table.h"

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
