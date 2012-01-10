#include <vector>
#include <sstream>
#include <fstream>
#include "model.h"
#include "em.h"
#include "filter_table.h"

using namespace std;

const int MAXITERS = 50;

class EM_model : public model {
public:
	EM_model(soar_interface *si, Symbol *root, scene *scn, const string &name)
	: si(si), root(root), revisions(0)
	{
		result_id = si->make_id_wme(root, "result").first;
		tests_id = si->make_id_wme(result_id, "tests").first;
		revisions_wme = si->make_wme(result_id, "revisions", revisions);
		em = new EM(scn);
		
		const filter_table &t = get_filter_table();
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
		
		stringstream ss;
		ss << "models/" << name << ".em";
		savepath = ss.str();
		
		ifstream is(savepath.c_str());
		if (is.is_open()) {
			DATAVIS("BEGIN " << name << endl)
			em->load(is);
			update_tested_atoms();
			cout << "LOADED MODEL" << endl;
			DATAVIS("END" << endl)
		}
	}

	~EM_model() {
		ofstream os(savepath.c_str());
		em->save(os);
		os.close();
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
		cout << "Making atom wme ";
		copy(all_atoms[i].begin(), all_atoms[i].end(), ostream_iterator<string>(cout, " "));
		cout << endl;
		
		string pred = all_atoms[i][0];
		vector<string> &params = pred_params[pred];
		assert(params.size() == all_atoms[i].size() - 1);
		
		sym_wme_pair p = si->make_id_wme(tests_id, pred);
		for (int j = 0; j < params.size(); ++j) {
			si->make_wme(p.first, params[j], all_atoms[i][j + 1]);
		}
		atom_wmes[i] = p.second;
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
