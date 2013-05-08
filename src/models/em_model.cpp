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

void error_color(double error, double color[]) {
	double maxerror = 1e-3;
	color[0] = color[1] = color[2] = 0.0;
	if (is_nan(error)) {
		return;
	} else if (error > maxerror) {
		color[0] = 1.0;
	} else {
		color[0] = error / maxerror;
		color[1] = 1 - (error / maxerror);
	}
}

void draw_predictions(int mode, int nmodes, double pred, double real, const string &name, EM *em) {
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
	const int mode_text_x = 50;
	const int xmode_text_y = 460;
	const int zmode_text_y = 410;
	const int font_size = 12;
	
	static double vx = NAN, vz = NAN, vxerror, vzerror;
	static int xmode = 0, zmode = 0, xnmodes = 0, znmodes = 0;
	static bool init = true;
	static drawer *d = get_drawer();
	
	stringstream ss;
	
	if (init) {
		ss << "layer 1 l 0 n 0\n"
		   << "layer 2 l 0 n 0 f 1\n"
		   << "* +b1:vx_mode_header t b1:vx l 2 c 1 1 1 p " << mode_text_x << " " << xmode_text_y << " 0\n"
		   << "* +b1:vz_mode_header t b1:vz l 2 c 1 1 1 p " << mode_text_x << " " << zmode_text_y << " 0\n"
		   << "* +vx_pred_line l 1 w 2\n"
		   << "* +vz_pred_line l 1 w 2\n"
		   << "* +pred_line    l 1 w 2\n";
		
		init = false;
	}

	int old_nmodes, old_mode, y;
	if (name == "b1:vx") {
		old_nmodes = xnmodes;
		old_mode = xmode;
		xnmodes = nmodes;
		xmode = mode;
		y = xmode_text_y;
		vx = pred * stretch;
		vxerror = abs(real - pred);
	} else if (name == "b1:vz") {
		old_nmodes = znmodes;
		old_mode = zmode;
		znmodes = nmodes;
		zmode = mode;
		y = zmode_text_y;
		vz = pred * stretch;
		vzerror = abs(real - pred);
	}
	
	/* draw mode text */
	for (int i = 0; i < nmodes; ++i) {
		string t;
		em->get_mode_function_string(i, t);
		y -= font_size;
		ss << "* +" << name << "_mode_" << i
		   << " t \"" << t << "\""
		   << " c 1 1 1 p " << mode_text_x + font_size * 4 << " " << y << " 0 l 2\n";
	}
	
	for (int i = nmodes; i < old_nmodes; ++i) {
		ss << "* -" << name << "_mode_" << i << "\n";
	}
	
	/* set color for selected mode */
	ss << "* " << name << "_mode_" << old_mode << " c 1 1 1\n";
	ss << "* " << name << "_mode_" << mode << " c 1 1 0\n";
	
	double cx[3], cz[3], cp[3];
	
	error_color(vxerror, cx);
	error_color(vzerror, cz);
	error_color(vxerror + vzerror / 2, cp);
	
	bool vx_valid = !is_nan(vx);
	bool vz_valid = !is_nan(vz);
	
	ss << "* vx_pred_line v 0 0 0 " << (vx_valid ? vx : 1000.0) << " 0 0"
	   << " i 0 1 c " << cx[0] << " " << cx[1] << " " << cx[2] << "\n";
	ss << "* vz_pred_line v 0 0 0 0 0 " << (vz_valid ? vz : 1000.0)
	   << " i 0 1 c " << cz[0] << " " << cz[1] << " " << cz[2] << "\n";
	
	if (vx_valid && vz_valid) {
		ss << "* pred_line v 0 0 0 " << vx << " 0 " << vz << " i 0 1";
		ss << " c " << cp[0] << " " << cp[1] << " " << cp[2] << "\n";
	} else {
		ss << "* pred_line v 0 0 0 0 0 0 i 0 1\n";
	}
	d->send(ss.str());
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
		draw_predictions(ti.mode, em.num_modes(), ti.pred, y(0), get_name(), &em);
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
