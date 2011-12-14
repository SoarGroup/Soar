#include <vector>
#include "model.h"
#include "em.h"
#include "filter_table.h"

using namespace std;

class EM_model : public model {
public:
	EM_model(soar_interface *si, Symbol *root)
	: si(si), root(root), result(NULL), resultwme(NULL), em(NULL)
	{ }

	~EM_model() {
		delete em;
		
	}
	
	bool predict(const floatvec &x, floatvec &y) {
		if (!em) {
			return false;
		}
		return em->predict(x, y[0]);
	}
	
	string get_type() const {
		return string("EM");
	}
	
	int get_input_size() const {
		return -1;
	}
	
	int get_output_size() const {
		return 1;
	}

	void learn(scene *scn, const floatvec &x, const floatvec &y, float dt) {
		if (!em) {
			const filter_table &t = get_filter_table();
			em = new EM(0.001, scn);
			t.get_all_atoms(scn, all_atoms);
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
		}
		em->add_data(x, y[0]);
		em->run(50);
		update_tested_atoms();
	}
	
	void update_tested_atoms() {
		vector<int> all_splits;
		vector<int>::const_iterator i;
		em->dtree->get_all_splits(all_splits);
		DATAVIS() << "'tree size' " << em->dtree->size() << endl;
		for(i = all_splits.begin(); i != all_splits.end(); ++i) {
			if (atom_wmes.find(*i) == atom_wmes.end()) {
				make_atom_wme(*i);
			}
		}
		
		map<int, wme*>::iterator j = atom_wmes.begin();
		while (j != atom_wmes.end()) {
			if (find(all_splits.begin(), all_splits.end(), j->first) == all_splits.end()) {
				si->remove_wme(j->second);
				atom_wmes.erase(j++);
			} else {
				++j;
			}
		}
	}

	void make_atom_wme(int i) {
		cout << "Making atom wme ";
		copy(all_atoms[i].begin(), all_atoms[i].end(), ostream_iterator<string>(cout, " "));
		cout << endl;
		
		string pred = all_atoms[i][0];
		vector<string> &params = pred_params[pred];
		assert(params.size() == all_atoms[i].size() - 1);
		
		if (result == NULL) {
			sym_wme_pair p = si->make_id_wme(root, "result");
			result = p.first;
			resultwme = p.second;
		}
		sym_wme_pair p = si->make_id_wme(result, pred);
		for (int j = 0; j < params.size(); ++j) {
			si->make_wme(p.first, params[j], all_atoms[i][j + 1]);
		}
		atom_wmes[i] = p.second;
	}
	
private:
	soar_interface *si;
	Symbol *root, *result;
	wme *resultwme;
	EM *em;
	vector<vector<string> > all_atoms;
	vector<string> atom_names;
	map<string, vector<string> > pred_params;
	
	map<int, wme*> atom_wmes;
};

model *_make_em_model_(soar_interface *si, Symbol *root) {
	return new EM_model(si, root);
}
