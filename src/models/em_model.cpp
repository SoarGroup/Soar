#include <cstdlib>
#include <vector>
#include <sstream>
#include <fstream>
#include "model.h"
#include "em.h"
#include "filter_table.h"
#include "params.h"
#include "svs.h"
#include "scene.h"
#include "serialize.h"

using namespace std;

const int MAXITERS = 50;

void velocity_hack(int mode, double pred, double real, const string &name) {
	static double mode_colors[][3] = {
		{ 0.0, 0.0, 0.0 },
		{ 1.0, 0.0, 1.0 },
		{ 0.0, 1.0, 1.0 },
		{ 1.0, 1.0, 0.0 },
		{ 1.0, 0.5, 0.0 },
		{ 0.5, 0.0, 0.5 },
	};
	static int ncolors = sizeof(mode_colors) / sizeof(mode_colors[0]);
	const double stretch = 20.0;
	
	static double vx = NAN, vz = NAN, vxerror, vzerror;
	static int xmode = 0, zmode = 0;

	assert(mode < ncolors);
	if (name == "b1:vx") {
		vx = pred * stretch;
		vxerror = abs(real - pred);
		xmode = mode;
	} else if (name == "b1:vz") {
		vz = pred * stretch;
		vzerror = abs(real - pred);
		zmode = mode;
	} else {
		return;
	}
	
	double *cx = isnan(vx) ? mode_colors[0] : mode_colors[xmode];
	double *cz = isnan(vz) ? mode_colors[0] : mode_colors[zmode];
	
	stringstream ss;
	
	ss << "* +vx v 0 0 0 " << (isnan(vx) ? 1000.0 : vx) << " 0 0";
	ss << " i 0 1 c " << cx[0] << " " << cx[1] << " " << cx[2] << " l 1 w 2\n";
	ss << "* +vz v 0 0 0 0 0 " << (isnan(vz) ? 1000.0 : vz);
	ss << " i 0 1 c " << cz[0] << " " << cz[1] << " " << cz[2] << " l 1 w 2\n";
	
	if (!isnan(vx) && !isnan(vz)) {
		double error = vxerror + vzerror;
		double ch[] = { 0.0, 0.0, 0.0 };
		double maxerror = 1e-3;
		if (error > maxerror) {
			ch[0] = 1.0;
		} else {
			ch[0] = error / maxerror;
			ch[1] = 1 - (error / maxerror);
		}
		ss << "* +vh v 0 0 0 " << vx << " 0 " << vz << " i 0 1";
		ss << " c " << ch[0] << " " << ch[1] << " " << ch[2];
		ss << " l 1 w 2\n";
	}
	get_drawer()->send(ss.str());
}

class EM_model : public model {
public:
	EM_model(soar_interface *si, Symbol *root, svs_state *state, const string &name)
	: model(name, "em", true), em(get_data()), si(si)
	{}

	bool predict(int target, const scene_sig &sig, const relation_table &rels, const rvec &x, rvec &y)  {
		int mode;
		bool success = em.predict(target, sig, rels, x, mode, y(0));
		return success;
	}
	
	int get_input_size() const {
		return -1;
	}
	
	int get_output_size() const {
		return 1;
	}

	void update() {
		assert(get_data().get_last_inst().y.size() == 1);
		em.add_data(get_data().size() - 1);
		em.run(MAXITERS);
	}
	
	/*
	 In addition to just calculating prediction error, I also want
	 to check whether classification was correct.
	*/
	void test(int target, const scene_sig &sig, const relation_table &rels, const rvec &x, const rvec &y) {
		rvec yp;
		bool success;
		
		test_info &ti = grow_vec(test_rec);
		ti.x = x;
		ti.y = y(0);
		extend_relations(test_rels, rels, test_rec.size() - 1);
		success = em.predict(target, sig, rels, x, ti.mode, ti.pred);
		ti.best_mode = em.best_mode(target, sig, x, y(0), ti.best_error);
		velocity_hack(ti.mode, ti.pred, y(0), get_name());
	}
	
	void proxy_get_children(map<string, cliproxy*> &c) {
		model::proxy_get_children(c);
		c["em"] = &em;
		c["error"] = new memfunc_proxy<EM_model>(this, &EM_model::cli_error);
		c["tests"] = new memfunc_proxy<EM_model>(this, &EM_model::cli_tests);
		c["test_rels"] = new memfunc_proxy<EM_model>(this, &EM_model::cli_test_rels);
	}
	
	void cli_error(ostream &os) const {
		int correct = 0, incorrect = 0;
		table_printer t;
		t.add_row() << "N" << "REAL" << "PRED" << "ERROR" << "MODE" << "BESTMODE" << "BESTERROR";
		for (int i = 0, iend = test_rec.size(); i < iend; ++i) {
			const test_info &ti = test_rec[i];
			if (ti.mode == ti.best_mode && ti.best_mode != 0) {
				++correct;
			} else {
				++incorrect;
			}
			t.add_row() << i << ti.y << ti.pred << ti.pred - ti.y << ti.mode << ti.best_mode << ti.best_error;
		}
		t.print(os);
		os << correct << " correct, " << incorrect << " incorrect" << endl;
	}
	
	void cli_tests(ostream &os) const {
		for (int i = 0, iend = test_rec.size(); i < iend; ++i) {
			const test_info &ti = test_rec[i];
			for (int j = 0, jend = ti.x.size(); j < jend; ++j) {
				os << ti.x(j) << ' ';
			}
			os << ti.y << endl;
		}
	}
	
	void cli_test_rels(ostream &os) const {
		serializer(os) << test_rels << '\n';
	}
	
	
private:
	struct test_info {
		scene_sig sig;
		rvec x;
		double y;
		double pred;
		double best_error;
		int mode;
		int best_mode;
	};
	
	EM em;
	
	soar_interface *si;
	
	vector<test_info> test_rec;
	relation_table test_rels;

	void serialize_sub(ostream &os) const {
		em.serialize(os);
	}
	
	void unserialize_sub(istream &is) {
		em.unserialize(is);
	}
};

model *_make_em_model_(soar_interface *si, Symbol *root, svs_state *state, const string &name) {
	return new EM_model(si, root, state, name);
}
