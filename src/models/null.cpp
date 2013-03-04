#include <iostream>
#include "model.h"
#include "soar_interface.h"
#include "svs.h"

using namespace std;

/* Doesn't do anything */
class null_model : public model {
public:
	null_model(const string &name) : model(name, "null", false) {}
	
	bool predict(int target, const scene_sig &sig, const relation_table &rels, const rvec &x, rvec &y) { return true; }
	int get_input_size() const { return 0; }
	int get_output_size() const { return 0; }
};

model *_make_null_model_(soar_interface *si, Symbol* root, svs_state *state, const string &name) {
	return new null_model(name);
}
