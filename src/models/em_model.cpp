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
#include "svs.h"

using namespace std;

const int MAXITERS = 50;

class EM_model : public model {
public:
	EM_model(soar_interface *si, Symbol *root, svs_state *state, const string &name)
	: model(name, "em"), si(si), revisions(0), wm_root(NULL)
	{
		em = new EM(data, sigs);
		clsfr = new rel_classifier(data, state->get_svs()->get_rels());
		init();
	}

	~EM_model() {
		delete em;
		delete clsfr;
	}
	
	bool predict(const propvec_sig &sig, const rvec &x, rvec &y, const relation_table &rels) {
		vector<int> obj_map;
		int mode = clsfr->classify(x, rels, obj_map);
		if (mode < 0) {
			return false;
		}
		
		rvec xc(x.size());
		int xsize = 0;
		for (int i = 0; i < obj_map.size(); ++i) {
			int n = sig[obj_map[i]].length;
			xc.segment(xsize, n) = x.segment(sig[obj_map[i]].start, n);
			xsize += n;
		}
		xc.conservativeResize(xsize);
		
		return em->predict(mode, xc, y(0));
	}
	
	int get_input_size() const {
		return -1;
	}
	
	int get_output_size() const {
		return 1;
	}

	void learn(const propvec_sig &sig, const rvec &x, const rvec &y, int time) {
		assert(y.size() == 1);
		
		data.push_back(train_inst());
		train_inst &d = data.back();
		d.x = x;
		d.y = y;
		d.time = time;
		d.sig_index = -1;
		
		for (int i = 0; i < sigs.size(); ++i) {
			if (sigs[i] == sig) {
				d.sig_index = i;
				break;
			}
		}
		
		if (d.sig_index < 0) {
			sigs.push_back(sig);
			d.sig_index = sigs.size() - 1;
		}

		em->new_data();
		if (em->run(MAXITERS)) {
			if (wm_root) {
				si->remove_wme(revisions_wme);
				revisions_wme = si->make_wme(wm_root, "revisions", ++revisions);
			}
		}
		
		clsfr->new_data(time);
		clsfr->update(em->get_map_modes());
	}
	
	void save(ostream &os) const {
		save_vector(data, os);
		em->save(os);
		clsfr->save(os);
	}
	
	void load(istream &is) {
		load_vector(data, is);
		em->load(is);
		clsfr->load(is);
	}
	
	/*
	 In addition to just calculating prediction error, I also want
	 to check whether the decision tree classification was correct.
	*/
	bool test(propvec_sig &sig, const rvec &x, const rvec &y, const relation_table &rels, rvec &prediction) {
		int best, mode;
		double best_error;
		vector<int> assign;
		best = em->best_mode(sig, x, y(0), best_error);
		mode = clsfr->classify(x, rels, assign);
		test_modes.push_back(mode);
		test_best_modes.push_back(best);
		test_best_errors.push_back(best_error);
		
		if (mode < 0) {
			return false;
		}
		return em->predict(mode, x, prediction(0));
	}
	
	bool cli_inspect_sub(int first_arg, const vector<string> &args, ostream &os) {
		if (first_arg >= args.size()) {
			os << "EM model learner" << endl;
			os << endl << "subqueries: train classifier em error" << endl;
			return true;
		} else if (args[first_arg] == "train") {
			const vector<category> &modes = em->get_map_modes();
			int ndata = data.size();
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
				output_rvec(os, data[i].x) << " ";
				output_rvec(os, data[i].y) << endl;
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
	vector<train_inst> data;
	vector<propvec_sig> sigs;
	
	EM *em;
	rel_classifier *clsfr;
	
	soar_interface *si;
	Symbol *wm_root, *tests_id;
	wme *revisions_wme;
	int revisions;
	map<string, vector<string> > pred_params;
	
	vector<int> test_modes, test_best_modes;
	vector<double> test_best_errors;
};

model *_make_em_model_(soar_interface *si, Symbol *root, svs_state *state, const string &name) {
	return new EM_model(si, root, state, name);
}
