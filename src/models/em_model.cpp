#include <cstdlib>
#include <vector>
#include <sstream>
#include <fstream>
#include "model.h"
#include "em.h"
#include "soar_interface.h"
#include "filter_table.h"
#include "params.h"
#include "svs.h"
#include "scene.h"

using namespace std;

const int MAXITERS = 50;

class EM_model : public model {
public:
	EM_model(soar_interface *si, Symbol *root, svs_state *state, const string &name)
	: model(name, "em", true), em(get_data()), si(si), revisions(0), wm_root(NULL)
	{
	}

	bool predict(int target, const scene_sig &sig, const relation_table &rels, const rvec &x, rvec &y)  {
		int mode;
		return em.predict(target, sig, rels, x, mode, y);
	}
	
	int get_input_size() const {
		return -1;
	}
	
	int get_output_size() const {
		return 1;
	}

	void update() {
		assert(get_data().get_last_inst().y.size() == 1);
		em.update();
		if (em.run(MAXITERS)) {
			if (wm_root) {
				si->remove_wme(revisions_wme);
				revisions_wme = si->make_wme(wm_root, "revisions", ++revisions);
			}
		}
	}
	
	/*
	 In addition to just calculating prediction error, I also want
	 to check whether classification was correct.
	*/
	void test(int target, const scene_sig &sig, const relation_table &rels, const rvec &x, rvec &y) {
		int best, mode;
		double best_error;
		vector<int> assign;
		best = em.best_mode(target, sig, x, y(0), best_error);
		test_best_modes.push_back(best);
		test_best_errors.push_back(best_error);
		em.predict(target, sig, rels, x, mode, y);
		test_modes.push_back(mode);
	}
	
	void proxy_get_children(map<string, cliproxy*> &c) {
		model::proxy_get_children(c);
		c["em"] = &em;
		c["mode_error"] = new memfunc_proxy<EM_model>(this, &EM_model::cli_mode_error);
	}
	
	void cli_mode_error(ostream &os) const {
		assert(test_modes.size() == test_best_modes.size() && test_modes.size() == test_best_errors.size());
		int correct = 0, incorrect = 0;
		table_printer t;
		t.add_row() << "N" << "PRED. MODE" << "BEST MODE" << "BEST ERROR";
		for (int i = 0; i < test_modes.size(); ++i) {
			if (test_modes[i] == test_best_modes[i] && test_best_modes[i] != 0) {
				++correct;
			} else {
				++incorrect;
			}
			t.add_row() << i << test_modes[i] << test_best_modes[i] << test_best_errors[i];
		}
		t.print(os);
		os << correct << " correct, " << incorrect << " incorrect" << endl;
	}
	
private:
	EM em;
	
	soar_interface *si;
	Symbol *wm_root, *tests_id;
	wme *revisions_wme;
	int revisions;
	
	vector<int> test_modes, test_best_modes;
	vector<double> test_best_errors;

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
