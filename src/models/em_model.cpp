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
		
		const filter_table &t = get_filter_table();
		t.get_all_atoms(scn, atoms);
		
		vector<string> preds;
		t.get_predicates(preds);
		vector<string>::const_iterator j;
		for (j = preds.begin(); j != preds.end(); ++j) {
			vector<string> params;
			t.get_params(*j, params);
			pred_params[*j] = params;
		}
		clsfr = new classifier(xdata, ydata, scn);
		init();
	}

	~EM_model() {
		delete em;
		delete clsfr;
		finish();
	}
	
	bool predict(const rvec &x, rvec &y) {
		int mode = clsfr->classify(x);
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

	void learn(const rvec &x, const rvec &y) {
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
				update_tested_atoms();
			}
		}
		
		clsfr->new_data();
		clsfr->update(em->get_map_modes());
	}
	
	void update_tested_atoms() {
		vector<int> a;
		vector<int>::const_iterator i;
		clsfr->get_tested_atoms(a);
		for(i = a.begin(); i != a.end(); ++i) {
			if (atom_wmes.find(*i) == atom_wmes.end()) {
				make_atom_wme(*i);
			}
		}
		
		map<int, wme*>::iterator j = atom_wmes.begin();
		while (j != atom_wmes.end()) {
			if (find(a.begin(), a.end(), j->first) == a.end()) {
				si->remove_wme(j->second);
				atom_wmes.erase(j++);
			} else {
				++j;
			}
		}
	}

	void make_atom_wme(int i) {
		vector<string> parts;
		split(atoms[i], "(),", parts);
		string pred = parts[0];
		vector<string> &params = pred_params[pred];
		assert(params.size() == parts.size() - 1);
		
		wme *w = si->make_id_wme(tests_id, pred);
		Symbol *id = si->get_wme_val(w);
		for (int j = 0; j < params.size(); ++j) {
			si->make_wme(id, params[j], parts[j + 1]);
		}
		atom_wmes[i] = w;
	}
	
	void save(ostream &os) const {
		xdata.save(os);
		ydata.save(os);
		em->save(os);
	}
	
	void load(istream &is) {
		xdata.load(is);
		ydata.load(is);
		em->load(is);
		//update_tested_atoms();
		clsfr->batch_update(em->get_map_modes());
	}
	
	/*
	 In addition to just calculating prediction error, I also want
	 to check whether the decision tree classification was correct.
	*/
	float test(const rvec &x, const rvec &y) {
		if (!get_option("log_predictions").empty()) {
			int best, predicted;
			double besterror;
			best = em->best_mode(x, y(0), besterror);
			predicted = clsfr->classify(x);
			
			stringstream ss;
			ss << "predictions/" << get_name() << ".classify";
			string path(ss.str());
			ofstream out(path.c_str(), ios_base::app);
			out << em->get_nmodels() << " " << (best == predicted) << " " << best << " " << predicted << " " << besterror << endl;
		}
		
		return model::test(x, y);
	}
	
	bool cli_inspect_sub(int first_arg, const vector<string> &args, ostream &os) {
		if (first_arg >= args.size()) {
			os << "EM model learner" << endl;
			os << endl << "subqueries: train classifier em" << endl;
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
		}
		return false;
	}
	
	void set_wm_root(Symbol *r) {
		wm_root = r;
		if (wm_root) {
			revisions_wme = si->make_wme(wm_root, "revisions", revisions);
			tests_id = si->get_wme_val(si->make_id_wme(wm_root, "tests"));
			atom_wmes.clear();
			update_tested_atoms();
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
	vector<string> atoms;
	map<string, vector<string> > pred_params;
	map<int, wme*> atom_wmes;
};

model *_make_em_model_(soar_interface *si, Symbol *root, scene *scn, const string &name) {
	return new EM_model(si, root, scn, name);
}
