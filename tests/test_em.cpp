#include <iostream>
#include "em.h"
#include "model.h"
#include "common.h"

using namespace std;

void read_sig(const vector<string> &fields, scene_sig &sig, int &target) {
	int id = 0;
	scene_sig::entry ent;
	if (!parse_int(fields[0], target)) {
		assert(false);
	}
	sig.clear();
	for (int i = 1, iend = fields.size(); i < iend; ++i) {
		vector<string> prop_fields;
		split(fields[i], ":", prop_fields);
		assert(prop_fields.size() == 3);
		if (prop_fields[0] != ent.name) {
			if (!ent.name.empty()) {
				sig.add(ent);
			}
			ent.id = id++;
			ent.start = i - 1;
			ent.name = prop_fields[0];
			ent.type = prop_fields[1];
			ent.props.clear();
		}
		ent.props.push_back(prop_fields[2]);
	}
	sig.add(ent);
}

void read_tabular(ifstream &is, model_train_data &data) {
	string line;
	vector<string> fields;
	scene_sig sig;
	relation_table empty; // not concerned with relations here
	int target;

	while (getline(is, line)) {
		fields.clear();
		split(line, "", fields);
		if (fields[0] == "#") {
			fields.erase(fields.begin());
			read_sig(fields, sig, target);
		} else {
			assert(fields.size() == sig.dim() + 1);
			rvec x(sig.dim());
			rvec y(1);
			for (int i = 0, iend = fields.size() - 1; i < iend; ++i) {
				if (!parse_double(fields[i], x(i))) {
					assert(false);
				}
			}
			if (!parse_double(fields[fields.size()-1], y(0))) {
				assert(false);
			}
			data.add(target, sig, empty, x, y);
		}
	}
}

void error(const string &msg) {
	cerr << msg << endl;
	exit(1);
}

int main(int argc, char *argv[]) {
	int i = 1;
	bool serialized_input = false;
	model_train_data data;
	double noise_var = 1e-8;
	string load_path, save_path;
	
	while (i < argc && argv[i][0] == '-') {
		switch (argv[i][1]) {
		case 'i':
			serialized_input = true;
			break;
		case 'n':
			if (++i >= argc || !parse_double(argv[i], noise_var)) {
				error("invalid noise");
			}
			break;
		case 'l':
			if (++i >= argc) {
				error("specify load path");
			}
			load_path = argv[++i];
			break;
		case 's':
			if (++i >= argc) {
				error("specify save path");
			}
			save_path = argv[++i];
			break;
		default:
			error(string("unknown option ") + argv[i]);
		}
		++i;
	}
	
	if (i == argc) {
		cerr << "specify data file" << endl;
		exit(1);
	}
	
	ifstream input(argv[i]);
	if (!input) {
		error(string("can't open ") + argv[i]);
	}
	
	if (serialized_input) {
		data.unserialize(input);
	} else {
		read_tabular(input, data);
	}
	
	EM em(data);
	if (!load_path.empty()) {
		ifstream load_file(load_path.c_str());
		if (!load_file) {
			error(string("couldn't read ") + load_path);
		}
		em.unserialize(load_file);
	}
	
	em.set_noise_var(noise_var);
	for (int i = 0, iend = data.size(); i < iend; ++i) {
		em.add_data(i);
		if (!em.run(50)) {
			cerr << "max iterations reached" << endl;
		}
	}
	
	em.print_ptable();
	em.print_modes(cout);
	
	if (!save_path.empty()) {
		ofstream save_file(save_path.c_str());
		if (!save_file) {
			error(string("couldn't write ") + save_path);
		}
		em.serialize(save_file);
	}
	
	return 0;
}
