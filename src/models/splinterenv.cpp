/*
 Interface to use the splinterenv class as a model
*/

#include <iostream>
#include "splinterenv.h"
#include "model.h"
#include "soar_interface.h"
#include "scene.h"

using namespace std;

class splinterenv_model : public model {
public:
	splinterenv_model(const string &name) : model(name, "splinterenv") {
		init();
	}
	
	~splinterenv_model() { finish(); }
	bool predict(const floatvec &x, floatvec &y) {
		return env.predict(x, y);
	}
	
	int get_input_size() const {
		return env.get_input_size();
	}
	
	int get_output_size() const {
		return env.get_output_size();
	}
	
private:
	splinterenv env;
};

model *_make_splinterenv_model_(soar_interface *si, Symbol *root, scene *scn, const string &name) {
	return new splinterenv_model(name);
}
