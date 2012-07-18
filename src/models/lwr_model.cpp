#include <iostream>
#include "lwr.h"
#include "soar_interface.h"
#include "scene.h"
#include "model.h"

using namespace std;
using namespace Eigen;

class lwr_model : public model {
public:
	lwr_model(int nnbrs, const string &name)
	: model(name, "lwr"), lwr(nnbrs)
	{
		init();
	}
	
	~lwr_model() {
		finish();
	}
	
	void learn(const rvec &x, const rvec &y) {
		lwr.learn(x, y);
	}
	
	bool predict(const rvec &x, rvec &y) {
		return lwr.predict(x, y);
	}
	
	int size() const {
		return lwr.size();
	}
	
	void load(istream &is) {
		lwr.load(is);
	}
	
	void save(ostream &os) const {
		lwr.save(os);
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

model *_make_lwr_model_(soar_interface *si, Symbol *root, scene *scn, const string &name) {
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
