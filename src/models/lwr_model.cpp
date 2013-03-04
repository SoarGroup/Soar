#include <iostream>
#include "lwr.h"
#include "soar_interface.h"
#include "svs.h"
#include "model.h"

using namespace std;
using namespace Eigen;

class lwr_model : public model {
public:
	lwr_model(int nnbrs, const string &name)
	: model(name, "lwr", true), lwr(nnbrs, true)
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
	wme *nnbrs_wme = NULL;
	long nnbrs = 50;
	string attrstr;
	
	si->find_child_wme(root, "num-neighbors", nnbrs_wme);
	if (nnbrs_wme && !si->get_val(si->get_wme_val(nnbrs_wme), nnbrs)) {
		LOG(WARN) << "WARNING: attribute num-neighbors does not have integer value, using default 50 neighbors" << endl;
	}
	return new lwr_model(nnbrs, name);
}
