#include <iostream>
#include <string>
#include "model.h"
#include "splinter.h"
#include "soar_interface.h"
#include "svs.h"

using namespace std;

class splinter_model : public model {
public:
	splinter_model(const string &name) : model(name, "splinter", false) {}
	
	bool predict(int target, const scene_sig &sig, const relation_table &rels, const rvec &x, rvec &y) {
		if (x.size() != 10 || y.size() != 8) {
			return false;
		}
		double px = x[0], py = x[1], vx = x[2], vy = x[3], rz = x[4], rtz = x[5],
		       lrps = x[6], rrps = x[7], lvolt = x[8], rvolt = x[9];
		splinter_update(px, py, vx, vy, rz, rtz, lrps, rrps, lvolt, rvolt);

		y[0] = px; y[1] = py; y[2] = vx; y[3] = vy; y[4] = rz; y[5] = rtz; y[6] = lrps; y[7] = rrps;
		return true;
	}
	
	int get_input_size() const {
		return 10; // px py vx vy rz rtz lrps rrps lvolts rvolts 
	}
	
	int get_output_size() const {
		return 8;  // px py vx vy rz rtz lrps rrps
	}
};

model *_make_splinter_model_(soar_interface *si, Symbol *root, svs_state *state, const string &name) {
	return new splinter_model(name);
}
