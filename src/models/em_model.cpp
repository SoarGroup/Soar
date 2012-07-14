#include <cstdlib>
#include <vector>
#include <sstream>
#include <fstream>
#include "model.h"
#include "em.h"
#include "soar_interface.h"
#include "filter_table.h"

using namespace std;

const int MAXITERS = 50;

class EM_model : public model {
public:
	EM_model(soar_interface *si, Symbol *root, scene *scn, const string &name)
	: model(name, "em"), si(si), root(root), revisions(0)
	{
		result_id = si->get_wme_val(si->make_id_wme(root, "result"));
		tests_id = si->get_wme_val(si->make_id_wme(result_id, "tests"));
		revisions_wme = si->make_wme(result_id, "revisions", revisions);
		em = new EM(scn);
		
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
		
		init();
	}

	~EM_model() {
		finish();
	}
	
	bool predict(const rvec &x, rvec &y) {
		return em->predict(x, y(0));
	}
	
	int get_input_size() const {
		return -1;
	}
	
	int get_output_size() const {
		return 1;
	}

	void learn(const rvec &x, const rvec &y) {
		em->add_data(x, y(0));
		if (em->run(MAXITERS)) {
			si->remove_wme(revisions_wme);
			revisions_wme = si->make_wme(result_id, "revisions", ++revisions);
			DATAVIS("revisions " << revisions << endl)
			update_tested_atoms();
		}
	}
	
	void update_tested_atoms() {
		vector<int> a;
		vector<int>::const_iterator i;
		em->get_classifier().get_tested_atoms(a);
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
		em->save(os);
	}
	
	void load(istream &is) {
		DATAVIS("BEGIN " << get_name() << endl)
		em->load(is);
		update_tested_atoms();
		DATAVIS("END" << endl)
	}
	
	/*
	 In addition to just calculating prediction error, I also want
	 to check whether the decision tree classification was correct.
	*/
	float test(const rvec &x, const rvec &y) {
		if (!get_option("log_predictions").empty()) {
			int best, predicted;
			double besterror;
			em->test_classify(x, y(0), best, predicted, besterror);
			
			stringstream ss;
			ss << "predictions/" << get_name() << ".classify";
			string path(ss.str());
			ofstream out(path.c_str(), ios_base::app);
			out << em->get_nmodels() << " " << (best == predicted) << " " << best << " " << predicted << " " << besterror << endl;
		}
		
		return model::test(x, y);
	}
	
	bool cli_inspect_sub(int first_arg, const vector<string> &args, ostream &os) {
		return em->cli_inspect(first_arg, args, os);
	}
	
private:
	soar_interface *si;
	Symbol *root, *result_id, *tests_id;
	wme *revisions_wme;
	int revisions;
	EM *em;
	vector<string> atoms;
	map<string, vector<string> > pred_params;
	map<int, wme*> atom_wmes;
	
	string savepath;
};

model *_make_em_model_(soar_interface *si, Symbol *root, scene *scn, const string &name) {
	return new EM_model(si, root, scn, name);
}
