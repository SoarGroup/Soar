#include <iostream>
#include "model.h"

using namespace std;

/* Doesn't do anything */
class null_model : public model {
public:
	null_model() { }
	bool predict(const floatvec &x, floatvec &y) { return true; }
	string get_type() const {
		return "null";
	}
	int get_input_size() const { return 0; }
	int get_output_size() const { return 0; }
};

model *_make_null_model_(soar_interface *si, Symbol* root, scene *scn, const string &name) {
	return new null_model();
}
