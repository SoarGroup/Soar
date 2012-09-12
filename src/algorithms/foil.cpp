#include <cmath>
#include "foil.h"

using namespace std;

class obj_assign_csp {
public:
	bool test_clause(const clause &c, const relation_table &rels, const set<int> &objs, vector<int> &out) {
		search_state init;
		
		for (int i = 0; i < c.size(); ++i) {
			const tuple &largs = c[i].get_args();
			for (int j = 0; j < largs.size(); ++j) {
				if (largs[j] != 0) {
					init.unassigned.insert(largs[j] - 1);
				}
			}
		}
		init.domains.resize(init.unassigned.size());
		var_pos.resize(init.unassigned.size());
		init.constraints.resize(c.size());
		
		vector<bool> initted(init.unassigned.size(), false);
		for (int i = 0; i < c.size(); ++i) {
			const relation *r = map_get(rels, c[i].get_name());
			if (!r) {
				return false;
			}
			r->drop_first(init.constraints[i]);
	
			const tuple &vars = c[i].get_args();
			for (int j = 1; j < vars.size(); ++j) {
				int v = vars[j] - 1;
				/*
				 Initial domain of each variable is the
				 intersection of all values at the
				 corresponding positions of each predicate it
				 appears in.
				*/
				set<int> &domain = init.domains[v], &cdomain = init.cdoms[make_pair(i, j - 1)];
				r->at_pos(j, cdomain);
				if (!initted[v]) {
					domain = objs;
					initted[v] = true;
				}
				intersect_sets_inplace(domain, cdomain);
				var_pos[v].insert(make_pair(i, j - 1));
			}
		}
		if (!search(init)) {
			return false;
		}
		out.clear();
		for (int i = 0; i < solution.size(); ++i) {
			assert(solution.find(i) != solution.end());
			out.push_back(solution[i]);
		}
		return true;
	}
	
private:
	struct search_state {
		vector<set<int>   >            domains;
		vector<set<tuple> >            constraints;
		map<pair<int, int>, set<int> > cdoms;
		map<int, int>                  assignments;
		set<int>                       unassigned;
	};
	
	/*
	 For each constraint c and position i that var is in, remove
	 all tuples in c whose ith argument is not val.
	*/
	bool assign(int var, int val, search_state &s) {
		s.assignments[var] = val;
		s.unassigned.erase(var);
		if (s.unassigned.empty()) {
			return true;
		}
		
		set<pair<int, int> >::const_iterator i;
		for (i = var_pos[var].begin(); i != var_pos[var].end(); ++i) {
			set<int> &cdom = s.cdoms[*i];
			cdom.clear();
			
			set<tuple>::iterator j = s.constraints[i->first].begin();
			while (j != s.constraints[i->first].end()) {
				if ((*j)[i->second] != val) {
					s.constraints[i->first].erase(j++);
				} else {
					cdom.insert((*j)[i->second]);
					++j;
				}
			}
		}
		
		/*
		 Update each variable's domain to be consistent with
		 all constraints.
		*/
		for (int i = 0; i < var_pos.size(); ++i) {
			set<pair<int, int> >::const_iterator j;
			for (j = var_pos[i].begin(); j != var_pos[i].end(); ++j) {
				intersect_sets_inplace(s.domains[i], s.cdoms[*j]);
				if (s.domains[i].empty()) {
					return false;
				}
			}
		}
		return true;
	}
	
	bool search(search_state &s) {
		if (s.unassigned.empty()) {
			solution = s.assignments;
			return true;
		}
		
		// find MRV
		int mrv = -1;
		set<int>::const_iterator i;
		for (i = s.unassigned.begin(); i != s.unassigned.end(); ++i) {
			if (mrv < 0 || s.domains[*i].size() < s.domains[mrv].size()) {
				mrv = *i;
			}
		}
		
		for (i = s.domains[mrv].begin(); i != s.domains[mrv].end(); ++i) {
			search_state child = s;
			if (assign(mrv, *i, child) && search(child)) {
				return true;
			}
		}
		return false;
	}
	
	vector<set<pair<int, int> > > var_pos;
	map<int, int> solution;
};

/*
 A search tree structure used in FOIL::choose_literal
*/
class literal_tree {
public:
	literal_tree(const FOIL &foil, int nvars, literal_tree **best);
	~literal_tree();
	void expand_df();
	const literal &get_literal() const { return lit; }
	double get_gain() const { return gain; }
	
private:
	literal_tree(literal_tree &parent, const string &r, bool negate);
	literal_tree(literal_tree &parent, int pos, int var);
	void expand();

private:
	int position, nbound;
	literal lit;
	std::vector<literal_tree*> children;
	tuple vars_left;
	double gain, max_gain;
	bool expanded;
	literal_tree **best;
	const FOIL &foil;
};

bool test_clause(const clause &c, const relation_table &rels, const set<int> &objs, vector<int> &assignments) {
	obj_assign_csp csp;
	return csp.test_clause(c, rels, objs, assignments);
}

bool test_clause_vec(const clause_vec &c, const relation_table &rels, const set<int> &objs, vector<int> &assignments) {
	for (int i = 0; i < c.size(); ++i) {
		if (test_clause(c[i], rels, objs, assignments)) {
			cout << "found assignment" << endl;
			map<int, int>::const_iterator j;
			for (int j = 0; j < assignments.size(); ++j) {
				cout << j << " -> " << assignments[j] << endl;
			}
			return true;
		}
	}
	cout << "failed finding assignment" << endl;
	return false;
}

ostream &operator<<(ostream &os, const literal &l) {
	if (l.negate) {
		os << "~";
	}
	os << l.name << "(";
	join(os, l.args, ",") << ")";
	return os;
}

ostream &operator<<(ostream &os, const clause &c) {
	join(os, c, " & ");
	return os;
}

bool sequential(const vector<int> &v) {
	for (int i = 1; i < v.size(); ++i) {
		if (v[i - 1] != v[i] - 1) {
			return false;
		}
	}
	return true;
}

FOIL::FOIL(const relation &p, const relation &n, const map<string, relation> &rels) 
: pos(p), neg(n), rels(rels), nvars(p.arity())
{}

FOIL::FOIL(const vector<int> &p, const vector<int> &n, const map<string, relation> &rels) 
: rels(rels), nvars(1)
{
	pos.init_single(p);
	neg.init_single(n);
}

bool FOIL::learn(clause_vec &clauses) {
	if (neg.empty()) {
		return true;
	}
	clauses.clear();
	tuple t;
	t.push_back(0);
	while (!pos.empty()) {
		pos_curr = pos;
		neg_curr = neg;
		clause c;
		bool dead = false;
		if (!add_clause(c)) {
			dead = true;
		} else {
			clauses.push_back(c);
			dead = !filter_pos_by_clause(c); // can't cover any more positive cases
		}
		if (dead) {
			return false;
		}
	}
	return true;
}

void FOIL::gain(const literal &l, double &g, double &maxg) const {
	double I1, I2;
	int new_pos_size, new_neg_size, pos_match, neg_match;
	map<string, relation>::const_iterator ri = rels.find(l.get_name());
	assert(ri != rels.end());
	const relation &r = ri->second;
	
	const tuple &vars = l.get_args();
	tuple bound_vars, bound_inds, new_inds;
	for (int i = 0; i < vars.size(); ++i) {
		if (vars[i] >= 0) {
			bound_vars.push_back(vars[i]);
			bound_inds.push_back(i);
		} else {
			new_inds.push_back(i);
		}
	}
	if (!l.negated()) {
		pos_curr.count_expansion(r, bound_vars, bound_inds, pos_match, new_pos_size);
		neg_curr.count_expansion(r, bound_vars, bound_inds, neg_match, new_neg_size);
	} else {
		// pretty inefficent
		relation pos_copy(pos_curr), neg_copy(neg_curr);
		
		relation sliced;
		r.slice(bound_inds, sliced);
		pos_copy.subtract(bound_vars, sliced);
		neg_copy.subtract(bound_vars, sliced);
		
		new_pos_size = pos_copy.size();
		pos_match = new_pos_size;
		new_neg_size = neg_copy.size();
		neg_match = new_neg_size;
	}
	
	if (pos_match == 0) {
		g = 0;
		maxg = 0;
	} else {
		I1 = -log2(pos_curr.size() / static_cast<double>(pos_curr.size() + neg_curr.size()));
		I2 = -log2(new_pos_size / static_cast<double>(new_pos_size + new_neg_size));
		g = pos_match * (I1 - I2);
		maxg = pos_match * I1;
	}
	LOG(FOILDBG) << l << " gain " << g << " max " << maxg << endl;
}

double FOIL::choose_literal(literal &l) {
	literal_tree *best_node;
	literal_tree root(*this, nvars, &best_node);
	root.expand_df();
	assert(best_node);
	l = best_node->get_literal();
	return best_node->get_gain();
}

bool FOIL::add_clause(clause &c) {
	nvars = pos_curr.arity();
	while (!neg_curr.empty()) {
		literal l;
		double gain = choose_literal(l);
		if (gain <= 0) {
			return false;
		}
		
		LOG(FOILDBG) << endl << "CHOSE " << l << endl << endl;
		const relation &r = get_rel(l.get_name());
		const tuple &vars = l.get_args();
		tuple bound_vars, bound_inds, new_inds;
		for (int i = 0; i < vars.size(); ++i) {
			if (vars[i] >= 0) {
				bound_vars.push_back(vars[i]);
				bound_inds.push_back(i);
			} else {
				new_inds.push_back(i);
			}
		}
		
		bool needs_slice;
		const relation *rp;
		
		if (bound_inds.size() < r.arity() || !sequential(bound_inds)) {
			relation *rn = new relation;
			r.slice(bound_inds, *rn);
			needs_slice = true;
			rp = const_cast<relation*>(rn);
		} else {
			rp = &r;
			needs_slice = false;
		}
		
		if (!l.negated()) {
			pos_curr.expand(r, bound_vars, bound_inds, new_inds);
			neg_curr.expand(r, bound_vars, bound_inds, new_inds);
			for (int i = 0; i < new_inds.size(); ++i) {
				l.set_arg(new_inds[i], nvars++);
			}
		} else {
			pos_curr.subtract(bound_vars, *rp);
			neg_curr.subtract(bound_vars, *rp);
		}
		if (needs_slice) {
			delete rp;
		}
		c.push_back(l);
	}
	return true;
}

bool FOIL::tuple_satisfies_literal(const tuple &t, const literal &l) {
	tuple ground_lit;
	tuple::const_iterator i;
	for (i = l.get_args().begin(); i != l.get_args().end(); ++i) {
		ground_lit.push_back(t[*i]);
	}
	bool inrel = get_rel(l.get_name()).test(ground_lit);
	if (l.negated()) {
		return !inrel;
	}
	return inrel;
}

bool FOIL::filter_pos_by_clause(const clause &c) {
	int old_size = pos.size();
	for (int i = 0; i < c.size(); ++i) {
		const tuple &args = c[i].get_args();
		const relation &r = get_rel(c[i].get_name());
		relation sliced;
		tuple bound_vars, bound_inds;
		for (int j = 0; j < args.size(); ++j) {
			if (args[j] >= 0 && args[j] < pos.arity()) {
				bound_vars.push_back(args[j]);
				bound_inds.push_back(j);
			}
		}
		r.slice(bound_inds, sliced);
		if (!c[i].negated()) {
			pos.subtract(bound_vars, sliced);
		} else {
			pos.intersect(bound_vars, sliced);
		}
	}
	return pos.size() < old_size;
}

const relation &FOIL::get_rel(const string &name) const {
	map<string, relation>::const_iterator i = rels.find(name);
	assert(i != rels.end());
	return i->second;
}

literal_tree::literal_tree(const FOIL &foil, int nvars, literal_tree **best) 
: foil(foil), best(best), expanded(false), position(-1), nbound(-1)
{
	for (int i = 0; i < nvars; ++i) {
		vars_left.push_back(i);
	}
	*best = NULL;
	map<string, relation>::const_iterator i;
	const map<string, relation> &rels = foil.get_relations();
	for (i = rels.begin(); i != rels.end(); ++i) {
		children.push_back(new literal_tree(*this, i->first, false));
		children.push_back(new literal_tree(*this, i->first, true));
	}
	expanded = true;
}

literal_tree::literal_tree(literal_tree &par, const string &r, bool negate)
: foil(par.foil), best(par.best), lit(r, tuple(par.foil.get_rel(r).arity(), -1), negate),
  expanded(false), position(0), vars_left(par.vars_left.begin() + 1, par.vars_left.end()),
  nbound(1)
{
	lit.set_arg(0, 0);
	foil.gain(lit, gain, max_gain);
	if (*best == NULL || gain > (**best).gain) {
		*best = this;
	}
}

literal_tree::literal_tree(literal_tree &par, int pos, int var)
: foil(par.foil), position(pos), lit(par.lit), best(par.best), expanded(false)
{ 
	lit.set_arg(position, var);
	nbound = par.nbound + 1;
	tuple::const_iterator i;
	for (i = par.vars_left.begin(); i != par.vars_left.end(); ++i) {
		if (*i != var) {
			vars_left.push_back(*i);
		}
	}
}

literal_tree::~literal_tree() {
	vector<literal_tree*>::iterator i;
	for (i = children.begin(); i != children.end(); ++i) {
		delete *i;
	}
}
	
void literal_tree::expand() {
	const tuple &vars = lit.get_args();
	for (int i = position + 1; i < vars.size(); ++i) {
		if (vars[i] >= 0) {
			continue;
		}
		for (int j = 0; j < vars_left.size(); ++j) {
			literal_tree *c = new literal_tree(*this, i, vars_left[j]);
			foil.gain(c->lit, c->gain, c->max_gain);
			if (*best != NULL && c->max_gain < (**best).gain) {
				delete c;
				continue;
			}
			if (*best == NULL || c->gain > (**best).gain ||
			    (c->gain == (**best).gain && c->nbound > (**best).nbound))
			{
				*best = c;
			}

			children.push_back(c);
		}
	}
	expanded = true;
}

void literal_tree::expand_df() {
	if (!expanded) {
		expand();
	}
	for (int i = 0; i < children.size(); ++i) {
		children[i]->expand_df();
	}
}
