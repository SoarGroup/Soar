#include <iostream>
#include "lwr.h"
#include "svs.h"
#include "model.h"

using namespace std;
using namespace Eigen;

class lwr_model : public model {
public:
	lwr_model(int nnbrs, double noise_var, const string &name)
	: model(name, "lwr", true), lwr(nnbrs, noise_var, true)
	{}
	
	void update() {
		const model_train_inst &i = get_data().get_last_inst();
		lwr.learn(i.x, i.y);
	}
	
	bool predict(int target, const scene_sig &sig, const relation_table &rels, const rvec &x, rvec &y) {
		return lwr.predict(x, y);
	}
	
	int size() const {
		return lwr.size();
	}
	
	void unserialize(istream &is) {
		lwr.unserialize(is);
	}
	
	void serialize(ostream &os) const {
		lwr.serialize(os);
	}
	
	int get_input_size() const {
		return lwr.xsize();
	}
	
	int get_output_size() const {
		return lwr.ysize();
	}
	
private:
	LWR lwr;
};

model *_make_lwr_model_(svs *owner, const string &name) {
	Symbol *attr;
	wme *nnbrs_wme = NULL, *var_wme = NULL;
	long nnbrs = 50;
	double noise_var = 1e-8;
	string attrstr;
	
	return new lwr_model(nnbrs, noise_var, name);
}
