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

using namespace std;

const int MAXITERS = 50;

class EM_model : public model {
public:
	EM_model(soar_interface *si, Symbol *root, svs_state *state, const string &name)
	: model(name, "em"), si(si), revisions(0), wm_root(NULL)
	{}

	bool predict(const state_sig &sig, const relation_table &rels, const rvec &x, rvec &y)  {
		int mode;
		return em.predict(sig, rels, x, mode, y);
	}
	
	int get_input_size() const {
		return -1;
	}
	
	int get_output_size() const {
		return 1;
	}

	void learn(const state_sig &sig, const relation_table &rels, const rvec &x, const rvec &y) {
		assert(y.size() == 1);
		em.learn(sig, rels, x, y);
		if (em.run(MAXITERS)) {
			if (wm_root) {
				si->remove_wme(revisions_wme);
				revisions_wme = si->make_wme(wm_root, "revisions", ++revisions);
			}
		}
	}
	
	void serialize(ostream &os) const {
		em.serialize(os);
	}
	
	void unserialize(istream &is) {
		em.unserialize(is);
	}
	
	/*
	 In addition to just calculating prediction error, I also want
	 to check whether classification was correct.
	*/
	bool test(state_sig &sig, const relation_table &rels, const rvec &x, const rvec &y, rvec &prediction) {
		int best, mode;
		double best_error;
		vector<int> assign;
		best = em.best_mode(sig, x, y(0), best_error);
		test_best_modes.push_back(best);
		test_best_errors.push_back(best_error);
		bool success = em.predict(sig, rels, x, mode, prediction);
		test_modes.push_back(mode);
		return success;
	}
	
	bool cli_inspect_sub(int first_arg, const vector<string> &args, ostream &os) {
		if (first_arg >= args.size()) {
			os << "EM model learner" << endl;
			os << endl << "subqueries: em error" << endl;
			return true;
		} else if (args[first_arg] == "em") {
			return em.cli_inspect(first_arg + 1, args, os);
		} else if (args[first_arg] == "error") {
			assert(test_modes.size() == test_best_modes.size() && test_modes.size() == test_best_errors.size());
			for (int i = 0; i < test_modes.size(); ++i) {
				os << setw(4) << i << " " << test_modes[i] << " " << test_best_modes[i] << " "
				   << test_best_errors[i] << endl;
			}
		}
		return false;
	}
	
	void set_wm_root(Symbol *r) {
		wm_root = r;
		if (wm_root) {
			revisions_wme = si->make_wme(wm_root, "revisions", revisions);
			tests_id = si->get_wme_val(si->make_id_wme(wm_root, "tests"));
		}
	}

private:
	EM em;
	
	soar_interface *si;
	Symbol *wm_root, *tests_id;
	wme *revisions_wme;
	int revisions;
	
	vector<int> test_modes, test_best_modes;
	vector<double> test_best_errors;
};

model *_make_em_model_(soar_interface *si, Symbol *root, svs_state *state, const string &name) {
	return new EM_model(si, root, state, name);
}
