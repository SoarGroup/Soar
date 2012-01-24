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
		result_id = si->make_id_wme(root, "result").first;
		tests_id = si->make_id_wme(result_id, "tests").first;
		revisions_wme = si->make_wme(result_id, "revisions", revisions);
		em = new EM(scn);
		
		const filter_table &t = get_filter_table();
		t.get_all_atoms(scn, all_atoms);
		
		// HACKS BEGIN
		vector<string> east, west, north, south;
		east.push_back("east");
		west.push_back("west");
		north.push_back("north");
		south.push_back("south");
		all_atoms.push_back(east);
		all_atoms.push_back(west);
		all_atoms.push_back(north);
		all_atoms.push_back(south);
		// HACKS END
		
		vector<vector<string> >::const_iterator i;
		for (i = all_atoms.begin(); i != all_atoms.end(); ++i) {
			stringstream ss;
			copy(i->begin(), i->end(), ostream_iterator<string>(ss, "_"));
			atom_names.push_back(ss.str());
		}
		
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
		
		if (em) {
			stringstream treepathss;
			treepathss << "trees/" << get_name() << ".dot";
			string treepath = treepathss.str();
			ofstream f(treepath.c_str());
			em->print_tree(f);
			delete em;
		}
	}
	
	bool predict(const floatvec &x, floatvec &y) {
		return em->predict(x, y[0]);
	}
	
	int get_input_size() const {
		return -1;
	}
	
	int get_output_size() const {
		return 1;
	}

	void learn(const floatvec &x, const floatvec &y, float dt) {
		em->add_data(x, y[0]);
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
		em->get_tested_atoms(a);
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
		string pred = all_atoms[i][0];
		vector<string> &params = pred_params[pred];
		assert(params.size() == all_atoms[i].size() - 1);
		
		sym_wme_pair p = si->make_id_wme(tests_id, pred);
		for (int j = 0; j < params.size(); ++j) {
			si->make_wme(p.first, params[j], all_atoms[i][j + 1]);
		}
		atom_wmes[i] = p.second;
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
	float test(const floatvec &x, const floatvec &y) {
		char *v = getenv("SVS_LOG_PREDICTION_ERRORS");
		if (v != NULL && string(v) == "1") {
			int best, predicted;
			double besterror;
			em->test_classify(x, y[0], best, predicted, besterror);
			
			stringstream ss;
			ss << "predictions/" << get_name() << ".classify";
			string path(ss.str());
			ofstream out(path.c_str(), ios_base::app);
			out << em->get_nmodels() << " " << (best == predicted) << " " << best << " " << predicted << " " << besterror << endl;
		}
		
		return model::test(x, y);
	}
	
private:
	soar_interface *si;
	Symbol *root, *result_id, *tests_id;
	wme *revisions_wme;
	int revisions;
	EM *em;
	vector<vector<string> > all_atoms;
	vector<string> atom_names;
	map<string, vector<string> > pred_params;
	map<int, wme*> atom_wmes;
	
	string savepath;
};

model *_make_em_model_(soar_interface *si, Symbol *root, scene *scn, const string &name) {
	return new EM_model(si, root, scn, name);
}
