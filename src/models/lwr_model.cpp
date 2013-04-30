#include <iostream>
#include "lwr.h"
#include "soar_interface.h"
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

model *_make_lwr_model_(soar_interface *si, Symbol *root, svs_state *state, const string &name) {
	Symbol *attr;
	wme *nnbrs_wme = NULL, *var_wme = NULL;
	long nnbrs = 50;
	double noise_var = 1e-8;
	string attrstr;
	
	si->find_child_wme(root, "num-neighbors", nnbrs_wme);
	if (nnbrs_wme && !si->get_val(si->get_wme_val(nnbrs_wme), nnbrs)) {
		LOG(WARN) << "WARNING: attribute num-neighbors does not have integer value, using default" << endl;
	}
	si->find_child_wme(root, "noise-var", var_wme);
	if (var_wme && !si->get_val(si->get_wme_val(var_wme), noise_var)) {
		LOG(WARN) << "WARNING: attribute noise-var does not have double value, using default" << endl;
	}
	return new lwr_model(nnbrs, noise_var, name);
}
