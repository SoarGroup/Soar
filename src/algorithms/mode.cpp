#include "mode.h"
#include "em.h"
#include "serialize.h"
#include "params.h"

using namespace std;

const regression_type REGRESSION_ALG = FORWARD;

/*
 Generate all possible combinations of sets of items
*/
template <typename T>
class multi_combination_generator {
public:
	multi_combination_generator(const std::vector<std::vector<T> > &elems, bool allow_repeat)
	: elems(elems), indices(elems.size(), 0), allow_repeat(allow_repeat), finished(false)
	{
		empty = false;
		if (elems.empty()) {
			empty = true;
		} else {
			for (int i = 0; i < elems.size(); ++i) {
				if (elems[i].empty()) {
					empty = true;
					break;
				}
			}
		}
	}

	void reset() {
		finished = false;
		fill(indices.begin(), indices.end(), 0);
	}

	bool next(std::vector<T> &comb) {
		if (empty) {
			return false;
		}
		
		comb.resize(elems.size());
		std::set<int> s;
		while (!finished) {
			bool has_repeat = false;
			s.clear();
			for (int i = 0; i < elems.size(); ++i) {
				comb[i] = elems[i][indices[i]];
				if (!allow_repeat) {
					std::pair<std::set<int>::iterator, bool> p = s.insert(comb[i]);
					if (!p.second) {
						has_repeat = true;
						break;
					}
				}
			}
			increment(0);
			if (allow_repeat || !has_repeat) {
				return true;
			}
		}
		return false;
	}

private:
	void increment(int i) {
		if (i >= elems.size()) {
			finished = true;
		} else if (++indices[i] >= elems[i].size()) {
			indices[i] = 0;
			increment(i + 1);
		}
	}

	const std::vector<std::vector<T> > &elems;
	std::vector<int> indices;
	bool allow_repeat, finished, empty;
};

em_mode::em_mode(bool noise, bool manual, const model_train_data &data) 
: noise(noise), manual(manual), data(data), new_fit(true), n_nonzero(-1)
{
	if (noise) {
		stale = false;
	} else {
		stale = true;
	}
	
	proxy_add("model",   new memfunc_proxy<em_mode>(this, &em_mode::cli_model));
	proxy_add("clauses", new memfunc_proxy<em_mode>(this, &em_mode::cli_clauses));
	proxy_add("members", new memfunc_proxy<em_mode>(this, &em_mode::cli_members));
}

/*
 Fit lin_coefs, lin_inter, and sig to the data in data_inds.
*/
void em_mode::set_linear_params(const scene_sig &dsig, int target, const mat &coefs, const rvec &inter) {
	n_nonzero = 0;
	lin_inter = inter;
	if (coefs.size() == 0) {
		lin_coefs.resize(0, 0);
	} else {
		// find relevant objects (with nonzero coefficients)
		vector<int> relevant_objs;
		relevant_objs.push_back(target);
		for (int i = 0; i < dsig.size(); ++i) {
			if (i == target) {
				continue;
			}
			int start = dsig[i].start;
			int end = start + dsig[i].props.size();
			bool relevant = false;
			for (int j = start; j < end; ++j) {
				if (!coefs.row(j).isConstant(0.0)) {
					++n_nonzero;
					relevant = true;
				}
			}
			if (relevant) {
				relevant_objs.push_back(i);
			}
		}
		
		int end = 0;
		lin_coefs.resize(coefs.rows(), 1);
		sig.clear();
		for (int i = 0; i < relevant_objs.size(); ++i) {
			const scene_sig::entry &e = dsig[relevant_objs[i]];
			sig.add(e);
			int start = e.start, n = e.props.size();
			lin_coefs.block(end, 0, n, 1) = coefs.block(start, 0, n, 1);
			end += n;
		}
		lin_coefs.conservativeResize(end, 1);
	}
	new_fit = true;
}

/*
 Upon return, mapping[i] will contain the position in dsig that holds the
 object to be mapped to the i'th variable in the model signature. Again, the
 mapping vector will hold indexes, not ids.
*/
bool em_mode::map_objs(int target, const scene_sig &dsig, const relation_table &rels, vector<int> &mapping) const {
	vector<bool> used(dsig.size(), false);
	used[target] = true;
	mapping.resize(sig.empty() ? 1 : sig.size(), -1);
	
	// target always maps to target
	mapping[0] = target;
	
	var_domains domains;
	
	// 0 = time, 1 = target, 2 = object we're searching for
	domains[0].insert(0);
	domains[1].insert(dsig[target].id);
	
	for (int i = 1; i < sig.size(); ++i) {
		set<int> &d = domains[2];
		d.clear();
		for (int j = 0; j < dsig.size(); ++j) {
			if (!used[j] && dsig[j].type == sig[i].type) {
				d.insert(dsig[j].id);
			}
		}
		if (d.empty()) {
			return false;
		} else if (d.size() == 1 || obj_clauses[i].empty()) {
			mapping[i] = dsig.find_id(*d.begin());
		} else {
			bool found = false;
			for (int j = 0, jend = obj_clauses[i].size(); j < jend; ++j) {
				if (test_clause(obj_clauses[i][j], rels, domains)) {
					assert(d.size() == 1);
					mapping[i] = dsig.find_id(*d.begin());
					found = true;
					break;
				}
			}
			if (!found) {
				return false;
			}
		}
		used[mapping[i]] = true;
	}
	return true;
}

/*
 pos_obj and neg_obj can probably be cached and updated as data points
 are assigned to modes.
*/
void em_mode::learn_obj_clauses(const relation_table &rels) {
	obj_clauses.resize(sig.size());
	for (int i = 1; i < sig.size(); ++i) {   // 0 is always target, no need to map
		string type = sig[i].type;
		relation pos_obj(3), neg_obj(3);
		tuple objs(2);
		map<int,mem_info>::const_iterator j;
		for (j = members.begin(); j != members.end(); ++j) {
			const model_train_inst &d = data.get_inst(j->first);
			int o = (*d.sig)[j->second.obj_map[i]].id;
			
			objs[0] = d.target;
			objs[1] = o;
			pos_obj.add(j->first, objs);
			for (int k = 0, kend = d.sig->size(); k < kend; ++k) {
				if ((*d.sig)[k].type == type && k != objs[0] && k != o) {
					objs[1] = k;
					neg_obj.add(j->first, objs);
				}
			}
		}
		
		FOIL foil;
		foil.set_problem(pos_obj, neg_obj, rels);
		if (!foil.learn(true, false)) {
			// respond to this situation appropriately
			assert(false);
		}
		obj_clauses[i].resize(foil.num_clauses());
		for (int j = 0, jend = foil.num_clauses(); j < jend; ++j) {
			obj_clauses[i][j] = foil.get_clause(j);
		}
	}
}

void em_mode::cli_model(ostream &os) const {
	if (noise) {
		os << "noise" << endl;
	} else {
		os << "coefficients" << endl;
		table_printer t;
		int ci = 0;
		for (int i = 0; i < sig.size(); ++i) {
			for (int j = 0; j < sig[i].props.size(); ++j) {
				t.add_row() << ci << sig[i].type << sig[i].props[j] << lin_coefs(ci++, 0);
			}
		}
		t.print(os);
		os << endl << "intercept " << lin_inter << endl;
	}
}

void em_mode::cli_clauses(ostream &os) const {
	table_printer t;
	for (int j = 0; j < obj_clauses.size(); ++j) {
		t.add_row() << j;
		if (obj_clauses[j].empty()) {
			t << "empty";
		} else {
			for (int k = 0; k < obj_clauses[j].size(); ++k) {
				if (k > 0) {
					t.add_row().skip(1);
				}
				t << obj_clauses[j][k];
			}
		}
	}
	t.print(os);
}

void em_mode::cli_members(ostream &os) const {
	vector<int> mems;
	get_members(mems);
	interval_set s(mems);
	os << s << endl;
}


/*
 The fields noise, data, and sigs are initialized in the constructor, and
 therefore not (un)serialized.
*/
void em_mode::serialize(ostream &os) const {
	serializer(os) << stale << new_fit << members << sig << obj_clauses << sorted_ys
	               << lin_coefs << lin_inter << n_nonzero << manual;
}

void em_mode::unserialize(istream &is) {
	unserializer(is) >> stale >> new_fit >> members >> sig >> obj_clauses >> sorted_ys
	                 >> lin_coefs >> lin_inter >> n_nonzero >> manual;
}

double em_mode::calc_prob(int target, const scene_sig &dsig, const rvec &x, double y, vector<int> &best_assign, double &best_error) const {
	if (noise) {
		return PNOISE;
		best_error = INFINITY;
	}
	
	rvec py;
	double w = 1.0;
	
	/*
	 Each mode has a signature that specifies the types and orders of
	 objects it expects for inputs. This is recorded in this->sig.
	 Call this the mode signature.
	 
	 Each data point has a signature that specifies which types and
	 orders of object properties encoded by the property vector. This
	 is recorded in dsig. Call this the data signature.
	 
	 P(d, m) = MAX[assignment][P(d, m, assignment)] where 'assignment'
	 is a mapping of objects in the data signature to the objects in
	 the mode signature.
	*/
	
	if (sig.empty()) {
		// should be constant prediction
		assert(lin_coefs.size() == 0);
		py = lin_inter;
		best_error = (y - py(0));
		best_assign.clear();
		double d = gausspdf(y, py(0), MEASURE_VAR);
		double p = (1.0 - EPSILON) * w * d;
		return p;
	}
	
	/*
	 Create the input table for the combination generator to generate
	 all possible assignments. possibles[i] should be a list of all
	 object indices that can be assigned to position i in the model
	 signature.
	*/
	vector<vector<int> > possibles(sig.size());
	possibles[0].push_back(target);
	for (int i = 1; i < sig.size(); ++i) {
		for (int j = 0; j < dsig.size(); ++j) {
			if (dsig[j].type == sig[i].type && j != target) {
				possibles[i].push_back(j);
			}
		}
	}
	multi_combination_generator<int> gen(possibles, false);
	
	/*
	 Iterate through all assignments and find the one that gives
	 highest probability.
	*/
	vector<int> assign;
	int xlen = sig.dim();
	rvec xc(xlen);
	double best_prob = -1.0;
	while (gen.next(assign)) {
		int s = 0;
		for (int i = 0; i < assign.size(); ++i) {
			const scene_sig::entry &e = dsig[assign[i]];
			int l = e.props.size();
			assert(sig[i].props.size() == l);
			xc.segment(s, l) = x.segment(e.start, l);
			s += l;
		}
		assert(s == xlen);
		
		py = (xc * lin_coefs) + lin_inter;
		double d = gausspdf(y, py(0), MEASURE_VAR);
		double p = (1.0 - EPSILON) * w * d;
		if (p > best_prob) {
			best_prob = p;
			best_assign = assign;
			best_error = y - py(0);
		}
	}
	assert(best_prob >= 0.0);
	return best_prob;
}

bool em_mode::update_fits() {
	if (!stale || manual) {
		return false;
	}
	if (members.empty()) {
		lin_coefs.setConstant(0);
		lin_inter.setConstant(0);
		return false;
	}
	int xcols = 0;
	for (int i = 0; i < sig.size(); ++i) {
		xcols += sig[i].props.size();
	}
	
	mat X(members.size(), xcols), Y(members.size(), 1);
	map<int,mem_info>::const_iterator i;
	int j = 0;
	for (i = members.begin(); i != members.end(); ++i) {
		const model_train_inst &d = data.get_inst(i->first);
		const vector<int> &obj_map = i->second.obj_map;
		assert(obj_map.size() == sig.size());
		rvec x(xcols);
		int s = 0;
		for (int k = 0, kend = obj_map.size(); k < kend; ++k) {
			const scene_sig::entry &e = (*d.sig)[obj_map[k]];
			int n = e.props.size();
			x.segment(s, n) = d.x.segment(e.start, n);
			s += n;
		}
		assert(s == xcols);
		X.row(j) = x;
		Y.row(j++) = d.y;
	}
	linreg_d(REGRESSION_ALG, X, Y, cvec(), lin_coefs, lin_inter);
	stale = false;
	new_fit = true;
	return true;
}

void em_mode::predict(const scene_sig &dsig, const rvec &x, const vector<int> &obj_map, rvec &y) const {
	if (lin_coefs.size() == 0) {
		y = lin_inter;
		return;
	}
	
	assert(obj_map.size() == sig.size());
	rvec xc(x.size());
	int xsize = 0;
	for (int j = 0, jend = obj_map.size(); j < jend; ++j) {
		const scene_sig::entry &e = dsig[obj_map[j]];
		int n = e.props.size();
		xc.segment(xsize, n) = x.segment(e.start, n);
		xsize += n;
	}
	xc.conservativeResize(xsize);
	y = (xc * lin_coefs) + lin_inter;
}

void em_mode::add_example(int t, const vector<int> &obj_map) {
	assert(!has(members, t));
	
	const model_train_inst &d = data.get_inst(t);
	members[t].obj_map = obj_map;
	if (noise) {
		sorted_ys.insert(make_pair(d.y(0), t));
	} else {
		rvec y;
		predict(*d.sig, d.x, obj_map, y);
		if ((y - d.y).norm() > MODEL_ERROR_THRESH) {
			stale = true;
		}
	}
}

void em_mode::del_example(int t) {
	const model_train_inst &d = data.get_inst(t);

	members.erase(t);
	if (noise) {
		sorted_ys.erase(make_pair(d.y(0), t));
	}
}

void em_mode::largest_const_subset(vector<int> &subset) {
	vector<int> s;
	set<pair<double, int> >::const_iterator i;
	double last = NAN;
	subset.clear();
	for (i = sorted_ys.begin(); i != sorted_ys.end(); ++i) {
		if (i->first == last) {
			s.push_back(i->second);
		} else {
			if (s.size() > subset.size()) {
				subset = s;
			}
			last = i->first;
			s.clear();
			s.push_back(i->second);
		}
	}
	if (s.size() > subset.size()) {
		subset = s;
	}
}

bool em_mode::uniform_sig(int sig, int target) const {
	map<int,mem_info>::const_iterator i;
	for (i = members.begin(); i != members.end(); ++i) {
		const model_train_inst &d = data.get_inst(i->first);
		if (d.sig_index != sig || d.target != target) {
			return false;
		}
	}
	return true;
}

int em_mode::get_num_nonzero_coefs() const {
	if (noise) {
		return numeric_limits<int>::max();
	}
	assert(n_nonzero >= 0);
	return n_nonzero;
}

void em_mode::get_members(vector<int> &mem) const {
	map<int,mem_info>::const_iterator i;
	for (i = members.begin(); i != members.end(); ++i) {
		mem.push_back(i->first);
	}
}

void em_mode::mem_info::serialize(std::ostream &os) const {
	serializer(os) << obj_map;
}

void em_mode::mem_info::unserialize(std::istream &is) {
	unserializer(is) >> obj_map;
}

