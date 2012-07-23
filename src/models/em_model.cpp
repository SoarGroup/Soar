#include <cstdlib>
#include <vector>
#include <sstream>
#include <fstream>
#include "model.h"
#include "em.h"
#include "classify.h"
#include "soar_interface.h"
#include "filter_table.h"
#include "params.h"
#include "scene.h"

using namespace std;

const int MAXITERS = 50;

class EM_model : public model {
public:
	EM_model(soar_interface *si, Symbol *root, scene *scn, const string &name)
	: model(name, "em"), si(si), revisions(0), ydata(0, 1, INIT_NDATA, 1), wm_root(NULL)
	{
		em = new EM(xdata, ydata);
		
		
		clsfr = new classifier(xdata, ydata);
		init();
	}

	~EM_model() {
		delete em;
		delete clsfr;
	}
	
	bool predict(const rvec &x, rvec &y, const boolvec &atoms) {
		int mode = clsfr->classify(x, atoms);
		if (mode < 0) {
			return false;
		}
		return em->predict(mode, x, y(0));
	}
	
	int get_input_size() const {
		return -1;
	}
	
	int get_output_size() const {
		return 1;
	}

	void learn(const rvec &x, const rvec &y, const boolvec &atoms) {
		if (xdata.rows() == 0) {
			xdata.resize(0, x.size());
		}
		assert(xdata.cols() == x.size() && y.size() == 1);
		xdata.append_row(x);
		ydata.append_row();
		ydata(ydata.rows() - 1, 0) = y(0);
		
		em->new_data();
		if (em->run(MAXITERS)) {
			if (wm_root) {
				si->remove_wme(revisions_wme);
				revisions_wme = si->make_wme(wm_root, "revisions", ++revisions);
			}
		}
		
		clsfr->new_data(atoms);
		clsfr->update(em->get_map_modes());
	}
	
	void save(ostream &os) const {
		xdata.save(os);
		ydata.save(os);
		em->save(os);
		clsfr->save(os);
	}
	
	void load(istream &is) {
		xdata.load(is);
		ydata.load(is);
		em->load(is);
		clsfr->load(is);
	}
	
	/*
	 In addition to just calculating prediction error, I also want
	 to check whether the decision tree classification was correct.
	*/
	bool test(const rvec &x, const rvec &y, const boolvec &atoms, rvec &prediction) {
		int best, mode;
		double best_error;
		best = em->best_mode(x, y(0), best_error);
		mode = clsfr->classify(x, atoms);
		test_modes.push_back(mode);
		test_best_modes.push_back(best);
		test_best_errors.push_back(best_error);
		
		return model::test(x, y, atoms, prediction);
	}
	
	bool cli_inspect_sub(int first_arg, const vector<string> &args, ostream &os) {
		if (first_arg >= args.size()) {
			os << "EM model learner" << endl;
			os << endl << "subqueries: train classifier em error" << endl;
			return true;
		} else if (args[first_arg] == "train") {
			const vector<category> &modes = em->get_map_modes();
			int ndata = xdata.rows();
			int start = 0, end = ndata - 1;
			if (first_arg + 1 < args.size()) {
				if (!parse_int(args[first_arg + 1], start) || start < 0 || start >= ndata - 1) {
					os << "invalid data range" << endl;
					return false;
				}
			}
			if (first_arg + 2 < args.size()) {
				if (!parse_int(args[first_arg + 2], end) || end < start || end >= ndata - 1) {
					os << "invalid data range" << endl;
					return false;
				}
			}
			
			os << "   N  CLS | DATA" << endl;  // header
			for (int i = start; i <= end; ++i) {
				os << setw(4) << i << "  " << setw(3) << modes[i] << " | ";
				output_rvec(os, xdata.row(i)) << " ";
				output_rvec(os, ydata.row(i)) << endl;
			}
			return true;
		} else if (args[first_arg] == "classifier") {
			return clsfr->cli_inspect(first_arg + 1, args, os);
		} else if (args[first_arg] == "em") {
			return em->cli_inspect(first_arg + 1, args, os);
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
	dyn_mat xdata;
	dyn_mat ydata;
	
	EM *em;
	classifier *clsfr;
	
	soar_interface *si;
	Symbol *wm_root, *tests_id;
	wme *revisions_wme;
	int revisions;
	map<string, vector<string> > pred_params;
	
	vector<int> test_modes, test_best_modes;
	vector<double> test_best_errors;
};

model *_make_em_model_(soar_interface *si, Symbol *root, scene *scn, const string &name) {
	return new EM_model(si, root, scn, name);
}
