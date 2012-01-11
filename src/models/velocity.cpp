#include <string>
#include <vector>
#include <algorithm>
#include <iterator>
#include "model.h"
#include "soar_interface.h"
#include "scene.h"

using namespace std;

class velocity_model : public model {
public:
	velocity_model(const string &name, int dims)
	: model(name, "vel"), dims(dims) 
	{}

	~velocity_model() {
		finish();
	}
	
	bool predict(const floatvec &x, floatvec &y) {
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

model *_make_velocity_model_(soar_interface *si, Symbol *root, scene *scn, const string &name) {
	long dims = 1;
	wme_list children;
	wme_list::iterator i;
	si->get_child_wmes(root, children);
	for (i = children.begin(); i != children.end(); ++i) {
		string attr;
		if (si->get_val(si->get_wme_attr(*i), attr) && attr == "dims") {
			if (si->get_val(si->get_wme_val(*i), dims)) {
				break;
			}
		}
	}
	return new velocity_model(name, dims);
}
