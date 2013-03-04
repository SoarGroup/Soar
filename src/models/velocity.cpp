#include <string>
#include <vector>
#include <algorithm>
#include <iterator>
#include "model.h"
#include "soar_interface.h"
#include "svs.h"

using namespace std;

class velocity_model : public model {
public:
	velocity_model(const string &name, int dims)
	: model(name, "vel", false), dims(dims) 
	{}
	
	bool predict(int target, const scene_sig &sig, const relation_table &rels, const rvec &x, rvec &y) {
		if (x.size() != dims * 2 || y.size() != dims) {
			return false;
		}

		for (int i = 0; i < dims; ++i) {
			y[i] = x[i] + x[i + dims];
		}
		return true;
	}
	
	int get_input_size() const {
		return dims * 2;  // current + velocity in each dimension
	}
	
	int get_output_size() const {
		return dims;
	}
	
private:
	int dims;
};

model *_make_velocity_model_(soar_interface *si, Symbol *root, svs_state *state, const string &name) {
	long dims = 1;
	si->get_const_attr(root, "dims", dims);
	return new velocity_model(name, dims);
}
