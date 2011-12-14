#include <string>
#include <vector>
#include <algorithm>
#include <iterator>
#include "model.h"
#include "soar_interface.h"

using namespace std;

class velocity_model : public model {
public:
	velocity_model() { }

	bool predict(const floatvec &x, floatvec &y) {
		if (x.size() != 6 || y.size() != 3) {
			return false;
		}

		for (int i = 0; i < 3; ++i) {
			y[i] = x[i] + x[i + 3];
		}
		return true;
	}
	
	std::string get_type() const {
		return "velocity";
	}
	
	int get_input_size() const {
		return 6; // px py pz vx vy vz 
	}
	
	int get_output_size() const {
		return 3; // px py pz
	}
};

model *_make_velocity_model_(soar_interface *si, Symbol *root) {
	return new velocity_model();
}
