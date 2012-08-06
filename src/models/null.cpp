#include <iostream>
#include "model.h"
#include "soar_interface.h"
#include "scene.h"

using namespace std;

/* Doesn't do anything */
class null_model : public model {
public:
	null_model(const string &name) : model(name, "null") { init(); }
	
	bool predict(const rvec &x, rvec &y, const boolvec &atoms) { return true; }
	int get_input_size() const { return 0; }
	int get_output_size() const { return 0; }
};

model *_make_null_model_(soar_interface *si, Symbol* root, scene *scn, const string &name) {
	return new null_model(name);
}
