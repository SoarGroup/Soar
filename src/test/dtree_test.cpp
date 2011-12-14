#include <iostream>
#include <fstream>
#include <vector>
#include "dtree.h"

using namespace std;

const bool change_cats = false;

void read_data(const char *path, vector<instance> &data, vector<category> &true_cats) {
	ifstream f(path);
	string line;
	while (getline(f, line)) {
		vector<int> v;
		stringstream ss(line);
		int x;
		while (ss >> x) {
			v.push_back(x);
		}
		instance inst;
		for (int i = 0; i < v.size() - 1; ++i) {
			inst.attrs.push_back(v[i] != 0);
		}
		true_cats.push_back(v.back());
		if (change_cats) {
			inst.cat = -1;
		} else {
			inst.cat = v.back();
		}
		data.push_back(inst);
	}
}

int main() {
	vector<string> attr_names {
		"east_of_box1_splinter",
		"east_of_box1_world",
		"east_of_splinter_box1",
		"east_of_splinter_world",
		"east_of_world_box1",
		"east_of_world_splinter",
		"horizontally_aligned_box1_splinter",
		"horizontally_aligned_box1_world",
		"horizontally_aligned_splinter_box1",
		"horizontally_aligned_splinter_world",
		"horizontally_aligned_world_box1",
		"horizontally_aligned_world_splinter",
		"intersect_box1_splinter",
		"intersect_box1_world",
		"intersect_splinter_world",
		"north_of_box1_splinter",
		"north_of_box1_world",
		"north_of_splinter_box1",
		"north_of_splinter_world",
		"north_of_world_box1",
		"north_of_world_splinter",
		"south_of_box1_splinter",
		"south_of_box1_world",
		"south_of_splinter_box1",
		"south_of_splinter_world",
		"south_of_world_box1",
		"south_of_world_splinter",
		"vertically_aligned_box1_splinter",
		"vertically_aligned_box1_world",
		"vertically_aligned_splinter_box1",
		"vertically_aligned_splinter_world",
		"vertically_aligned_world_box1",
		"vertically_aligned_world_splinter",
		"west_of_box1_splinter",
		"west_of_box1_world",
		"west_of_splinter_box1",
		"west_of_splinter_world",
		"west_of_world_box1",
		"west_of_world_splinter" };

	vector<instance> data;
	vector<category> true_cats;
	
	read_data("tree.data", data, true_cats);
	ID5Tree tree(data);
	
	if (change_cats) {
		for (int i = 0; i < data.size(); ++i) {
			cout << "adding data " << i << endl;
			tree.update_tree(i);
		}
		for (int i = 0; i < data.size(); ++i) {
			cout << "changing category " << i << endl;
			data[i].cat = true_cats[i];
			tree.update_category(i, -1);
			tree.update_tree(-1);
			tree.output(attr_names);
		}
	} else {
		for (int i = 0; i < data.size(); ++i) {
			cout << "adding data " << i << endl;
			tree.update_tree(i);
			tree.output(attr_names);
		}
	}
	
	cout << "Final:" << endl;
	tree.output(attr_names);
}