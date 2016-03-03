/***************************************************
 *
 * File: filters/monitor_object.cpp
 *
 * Monitoring Filters
 * 
 * Filter monitor_volume : map_filter<double>
 * 	 Parameters:
 * 	 	sgnode a
 * 	 Returns:
 * 	 	Stores the volume of node a when the filter is first created
 * 	 	Then returns the ratio of the current volume to the initial volume
 * 	 	This can be used to track how the volume is changing over time
 *
 * Filter monitor_position : map_filter<double>
 * 	Parameters: 
 * 		sgnode a
 * 	Returns:
 * 		Stores the position of node a when the filter is first created
 * 		Then returns the euclidean distance between the current 
 * 			position of the node, and its initial position
 * 		This can be used to track how far an object has moved over time
 *
 *********************************************************/
#include <iostream>
#include <assert.h>
#include <string>
#include <map>
#include "filter.h"
#include "sgnode.h"
#include "scene.h"
#include "filter_table.h"

using namespace std;

typedef std::map<sgnode*, double> vol_map;
typedef std::map<sgnode*, vec3> pos_map;

class monitor_volume_filter : public map_filter<double> {
public:
	monitor_volume_filter(Symbol *root, soar_interface *si, scene *scn, filter_input *input)
	: map_filter<double>(root, si, input), scn(scn)
	{
	}

	bool compute(const filter_params *p, double& out) {
		sgnode *a;

		if (!get_filter_param(this, p, "a", a)) {
			set_status("expecting parameter a");
			return false;
		}

		vec3 scale = a->get_trans('s');
		double curVol = scale[0] * scale[1] * scale[2];

		vol_map::const_iterator it = savedVolumes.find(a);
		if(it == savedVolumes.end()){
			savedVolumes[a] = curVol;
			out = 1.0;
		} else {
			out = (curVol + .000000001) / (it->second + .000000001);
		}

		return true;
	}

private:
	vol_map savedVolumes;

	scene *scn;
};

class monitor_position_filter : public map_filter<double> {
public:
	monitor_position_filter(Symbol *root, soar_interface *si, scene *scn, filter_input *input)
	: map_filter<double>(root, si, input), scn(scn)
	{
	}

	bool compute(const filter_params *p, double& out) {
		sgnode *a;

		if (!get_filter_param(this, p, "a", a)) {
			set_status("expecting parameter a");
			return false;
		}

		vec3 pos = a->get_trans('p');

		pos_map::const_iterator it = savedPositions.find(a);
		if(it == savedPositions.end()){
			savedPositions[a] = pos;
			out = 0;
		} else {
			vec3 savedPos = it->second;
			vec3 diff = pos - savedPos;
			out = sqrt(diff[0]*diff[0] + diff[1]*diff[1] + diff[2]*diff[2]);
		}

		return true;
	}

private:
	pos_map savedPositions;

	scene *scn;
};

filter *make_monitor_position_filter(Symbol *root, soar_interface *si, scene *scn, filter_input *input) {
	return new monitor_position_filter(root, si, scn, input);
}

filter *make_monitor_volume_filter(Symbol *root, soar_interface *si, scene *scn, filter_input *input) {
	return new monitor_volume_filter(root, si, scn, input);
}

filter_table_entry *monitor_position_filter_entry() {
	filter_table_entry *e = new filter_table_entry;
	e->name = "monitor_position";
	e->description = "monitor the position of a node over time";
	e->parameters["a"] = "The node whose position you want to monitor";
	e->create = &make_monitor_position_filter;
	return e;
}

filter_table_entry *monitor_volume_filter_entry() {
	filter_table_entry *e = new filter_table_entry;
	e->name = "monitor_volume";
	e->description = "monitor the volume of a node over time";
	e->parameters["a"] = "The node whose volume you want to monitor";
	e->create = &make_monitor_volume_filter;
	return e;
}

