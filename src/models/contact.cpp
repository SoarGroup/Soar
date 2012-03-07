#include <iostream>
#include <map>
#include <vector>
#include <sstream>
#include "soar_interface.h"
#include "scene.h"
#include "model.h"
#include "filter_table.h"
#include "lwr.h"

using namespace std;

class contact_model : public model {
public:
	contact_model(const string &name, scene *scn, const string &subject, int nnbrs)
	: model(name, "contact"), subject(subject), xsz(-1), ysz(-1), sz(0), nnbrs(nnbrs),
	  ft(get_filter_table()), scn_copy(scn->copy())
	{
		vector<sgnode*> ns;
		scn_copy->get_all_nodes(ns);
		for (int i = 0; i < ns.size(); ++i) {
			if (ns[i]->get_name() != subject) {
				others.push_back(ns[i]->get_name());
			}
		}
		init();
	}
	
	~contact_model() {
		finish();
		model_tbl::iterator i;
		for (i = models.begin(); i != models.end(); ++i) {
			delete i->second;
		}
	}
	
	void learn(const rvec &x, const rvec &y, float dt) {
		if (xsz == -1 && ysz == -1) {
			xsz = x.size();
			ysz = y.size();
		}
		assert(xsz == x.size() && ysz == y.size());
		
		vector<bool> intersects;
		calc_intersections(x, intersects);
		model_tbl::iterator i = models.find(intersects);
		LWR* m;
		if (i == models.end()) {
			m = new LWR(nnbrs);
			models[intersects] = m;
		} else {
			m = i->second;
		}
		m->learn(x, y);
		++sz;
	}
	
	bool predict(const rvec &x, rvec &y) {
		vector<bool> intersects;
		calc_intersections(x, intersects);
		model_tbl::iterator i = models.find(intersects);
		if (i == models.end()) {
			return false;
		}
		return i->second->predict(x, y);
	}
	
	int size() const {
		return sz;
	}
	
	void load(istream &is) {
		int n;
		is >> n;
		for (int i = 0; i < n; ++i) {
			string s;
			vector<bool> b;
			is >> s;
			b.reserve(s.size());
			for (int j = 0; j < s.size(); ++j) {
				b.push_back(s[j] == '1');
			}
			LWR *m = new LWR(nnbrs);
			m->load(is);
			models[b] = m;
		}
	}
	
	void save(ostream &os) const {
		model_tbl::const_iterator i;
		os << models.size() << endl;
		for (i = models.begin(); i != models.end(); ++i) {
			vector<bool>::const_iterator j;
			for (j = i->first.begin(); j != i->first.end(); ++j) {
				os << *j;
			}
			os << endl;
			i->second->save(os);
		}
	}
	
	int get_input_size() const {
		return xsz;
	}
	
	int get_output_size() const {
		return ysz;
	}
	
	bool cli_inspect_drv(int first_arg, const vector<string> &args, ostream &os) const {
		model_tbl::const_iterator i;
		for (i = models.begin(); i != models.end(); ++i) {
			vector<bool>::const_iterator j;
			for (j = i->first.begin(); j != i->first.end(); ++j) {
				os << *j;
			}
			os << " " << i->second->size() << endl;
		}
		return true;
	}

private:
	typedef map<vector<bool>, LWR*> model_tbl;
	
	void calc_intersections(const rvec &x, vector<bool> &out) {
		vector<string> p(2);
		bool result;
		
		p[0] = subject;
		scn_copy->set_properties(x);
		out.reserve(others.size());
		for (int i = 0; i < others.size(); ++i) {
			p[1] = others[i];
			if (!ft.calc("intersect", scn_copy, p, result)) {
				assert(false);
			}
			out.push_back(result);
		}
	}
	
	string subject;
	vector<string> others;
	model_tbl models;
	scene *scn_copy;
	int xsz, ysz, sz, nnbrs;
	const filter_table &ft;
};

model *_make_contact_model_(soar_interface *si, Symbol *root, scene *scn, const string &name) {
	Symbol *attr;
	wme_list children;
	wme_list::iterator i;
	long nnbrs = 50;
	string attrstr, subject;
	
	si->get_child_wmes(root, children);
	for (i = children.begin(); i != children.end(); ++i) {
		attr = si->get_wme_attr(*i);
		if (!si->get_val(attr, attrstr)) {
			continue;
		}
		if (attrstr == "subject") {
			if (!si->get_val(si->get_wme_val(*i), subject)) {
				cerr << "INVALID SUBJECT NAME" << endl;
				return NULL;
			}
		}
		if (attrstr == "num-neighbors") {
			if (!si->get_val(si->get_wme_val(*i), nnbrs)) {
				cerr << "WARNING: attribute num-neighbors does not have integer value, using default 50 neighbors" << endl;
			}
		}
	}
	
	assert(scn->get_node(subject) != NULL);
	return new contact_model(name, scn, subject, nnbrs);
}
