#include <cmath>
#include "foil.h"
#include "serialize.h"
#include "params.h"

using namespace std;

class csp_node {
public:
	csp_node() {}

	csp_node(const csp_node &par)
	: vars(par.vars), constraints(par.constraints), unassigned(par.unassigned)
	{}

	bool init(const clause &c, const relation_table &rels, const set<int> &objs) {
		set<int> all_vars;
		for (int i = 0; i < c.size(); ++i) {
			const tuple &largs = c[i].get_args();
			for (int j = 0; j < largs.size(); ++j) {
				if (largs[j] >= 0) {
					all_vars.insert(largs[j]);
				}
			}
		}
		// make sure variables are sequential integers starting from 0
		assert(*all_vars.begin() == 0 && *max_element(all_vars.begin(), all_vars.end()) == all_vars.size() - 1);
		unassigned = all_vars.size();
		vars.resize(unassigned);
		for (int i = 0; i < vars.size(); ++i) {
			vars[i].domain = objs;
		}

		constraints.resize(c.size());
		for (int i = 0; i < c.size(); ++i) {
			constraint_info &cons = constraints[i];
			const relation &r = map_get(rels, c[i].get_name());
			cons.tuples = r;
			const tuple &args = c[i].get_args();
	
			cons.negated = c[i].negated();
			cons.doms.resize(args.size());
			cons.vars.resize(args.size());
			cons.unbound = 0;
			for (int j = 0; j < args.size(); ++j) {
				if (args[j] >= 0) {
					cons.vars[j] = args[j];
					r.at_pos(j, cons.doms[j]);
					++cons.unbound;
				} else {
					cons.vars[j] = -1;
				}
			}
		}
		for (int i = 0; i < constraints.size(); ++i) {
			constraint_info &cons = constraints[i];
			for (int j = 0; j < cons.vars.size(); ++j) {
				if (cons.vars[j] >= 0 && !update_vardom(i, j)) {
					return false;
				}
			}
		}
		return true;
	}

	bool search(map<int, int> &out) {
		if (unassigned == 0) {
			out.clear();
			for (int i = 0; i < vars.size(); ++i) {
				out[i] = vars[i].value;
			}
			return true;
		}
		
		// find MRV
		int mrv = -1;
		for (int i = 0; i < vars.size(); ++i) {
			if (vars[i].value >= 0) {
				continue;
			}
			if (mrv < 0 || vars[i].domain.size() < vars[mrv].domain.size()) {
				mrv = i;
			}
		}
		
		set<int>::iterator i;
		for (i = vars[mrv].domain.begin(); i != vars[mrv].domain.end(); ++i) {
			csp_node child(*this);
			if (child.assign(mrv, *i) && child.search(out)) {
				return true;
			}
		}
		return false;
	}

	/*
	 For each constraint c and position i that var is in, remove
	 all tuples in c whose ith argument is not val.
	*/
	bool assign(int v, int value) {
		assert(0 <= v && v < vars.size());
		if (!in_set(value, vars[v].domain)) {
			return false;
		}
		vars[v].value = value;
		if (--unassigned == 0) {
			return true;
		}
		
		vector<bool> need_update(constraints.size(), false);
		for (int i = 0; i < constraints.size(); ++i) {
			constraint_info &cons = constraints[i];
			for (int j = 0; j < cons.vars.size(); ++j) {
				if (cons.vars[j] == v) {
					vector<int> pat(cons.tuples.arity(), -1);
					pat[j] = value;
					cons.tuples.filter(pat);
					--cons.unbound;
					need_update[i] = true;
				}
			}
		}

		/*
		 Update the domains of each constraint position. Then update the domain
		 of the variable at that position.
		*/
		for (int i = 0; i < constraints.size(); ++i) {
			if (!need_update[i]) {
				continue;
			}
			update_cdoms(i);
			constraint_info &cons = constraints[i];
			for (int j = 0; j < cons.vars.size(); ++j) {
				if (!update_vardom(i, j)) {
					return false;
				}
			}
		}
		return true;
	}
private:
	struct constraint_info {
		bool negated;
		int unbound;
		relation tuples;
		vector<set<int> > doms;
		vector<int> vars;
	};

	struct var_info {
		set<int> domain;
		int value;

		var_info() : value(-1) {}
	};

	vector<var_info>        vars;
	vector<constraint_info> constraints;
	int unassigned;
	

	void update_cdoms(int c) {
		constraint_info &cons = constraints[c];
		
		for (int i = 0; i < cons.doms.size(); ++i) {
			cons.doms[i].clear();
			cons.tuples.at_pos(i, cons.doms[i]);
		}
	}

	bool update_vardom(int i, int j) {
		constraint_info &cons = constraints[i];
		if(cons.vars[j] < 0) {
			return true;
		}
		var_info &var = vars[cons.vars[j]];
		if (var.value >= 0) {
			assert(in_set(var.value, var.domain));
			var.domain.clear();
			var.domain.insert(var.value);
			return true;
		}
		if (!cons.negated) {
			intersect_sets_inplace(var.domain, cons.doms[j]);
		} else if (cons.unbound == 1) {
			subtract_sets_inplace(var.domain, cons.doms[j]);
		}
		return !var.domain.empty();
	}
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

bool test_clause(const clause &c, const relation_table &rels, const set<int> &objs, map<int, int> &assignments) {
	csp_node csp;
	if (!csp.init(c, rels, objs)) {
		return false;
	}
	map<int, int>::iterator i;
	for (i = assignments.begin(); i != assignments.end(); ++i) {
		if (!csp.assign(i->first, i->second)) {
			return false;
		}
	}
	return csp.search(assignments);
}

int test_clause_vec(const clause_vec &c, const relation_table &rels, const set<int> &objs, map<int,int> &assignments) {
	for (int i = 0; i < c.size(); ++i) {
		if (test_clause(c[i], rels, objs, assignments)) {
			LOG(FOILDBG) << "found assignment" << endl;
			map<int, int>::const_iterator j;
			for (int j = 0; j < assignments.size(); ++j) {
				LOG(FOILDBG) << j << " -> " << assignments[j] << endl;
			}
			return i;
		}
	}
	LOG(FOILDBG) << "failed finding assignment" << endl;
	return -1;
}

void split_training(double ratio, const relation &all, relation &grow, vector<tuple> &test) {
	assert(0 <= ratio && ratio < 1);
	grow.reset(all.arity());
	test.clear();
	int ngrow = max(all.size() * ratio, 1.0);
	all.sample(ngrow, grow);
	
	relation test_rel(all.arity());
	all.difference(grow, test_rel);
	test_rel.dump(test);
}

void literal::serialize(std::ostream &os) const {
	serializer(os) << name << negate << args;
}

void literal::unserialize(std::istream &is) {
	unserializer(is) >> name >> negate >> args;
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

int literal::operator<<(const std::string &s) {
	int a, b, c;
	if (s[0] == '~') {
		negate = true;
		a = 1;
	} else {
		negate = false;
		a = 0;
	}
	b = s.find('(', a);
	assert(b != string::npos);
	c = s.find(')', b);
	assert(c != string::npos);
	name = s.substr(a, b - a);
	vector<string> num_strs;
	split(s.substr(b + 1, c - b - 1), ",", num_strs);
	args.resize(num_strs.size());
	for (int i = 0; i < num_strs.size(); ++i) {
		if (!parse_int(num_strs[i], args[i])) {
			assert(false);
		}
	}
	return c + 1;
}

FOIL::FOIL(const relation &p, const relation &n, const relation_table &rels) 
: pos(p), neg(n), rels(rels), nvars(p.arity())
{
	assert(p.arity() == n.arity());
}

bool FOIL::learn(clause_vec &clauses, relation &uncovered) {
	if (neg.empty()) {
		return true;
	}
	clauses.clear();
	tuple t;
	t.push_back(0);
	while (!pos.empty()) {
		split_training(FOIL_GROW_RATIO, pos, pos_grow, pos_test);
		split_training(FOIL_GROW_RATIO, neg, neg_grow, neg_test);
		
		clause c;
		bool dead = true;
		add_clause(c);
		if (!c.empty() && clause_success_rate(c) > FOIL_MIN_SUCCESS_RATE) {
			clauses.push_back(c);
			dead = !filter_pos_by_clause(c); // can't cover any more positive cases
		}
		
		if (dead) {
			uncovered = pos;
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
		pos_grow.count_expansion(r, bound_vars, bound_inds, pos_match, new_pos_size);
		neg_grow.count_expansion(r, bound_vars, bound_inds, neg_match, new_neg_size);
	} else {
		// pretty inefficent
		relation pos_copy(pos_grow), neg_copy(neg_grow);
		
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
		I1 = -log2(pos_grow.size() / static_cast<double>(pos_grow.size() + neg_grow.size()));
		I2 = -log2(new_pos_size / static_cast<double>(new_pos_size + new_neg_size));
		g = pos_match * (I1 - I2);
		maxg = pos_match * I1;
	}
	LOG(FOILDBG) << l << " gain " << g << " max " << maxg << endl;
}

double FOIL::choose_literal(literal &l, int n) {
	literal_tree *best_node = NULL;
	literal_tree root(*this, n, &best_node);
	root.expand_df();
	assert(best_node);
	l = best_node->get_literal();
	return best_node->get_gain();
}

bool FOIL::add_clause(clause &c) {
	int n = nvars;
	while (!neg_grow.empty()) {
		literal l;
		double gain = choose_literal(l, n);
		if (gain <= 0) {
			set<int> neg_left;
			set<int>::const_iterator i;
			neg_grow.at_pos(0, neg_left);
			LOG(FOILDBG) << "No more suitable literals. " << endl << "unfiltered negatives: "; 
			for (i = neg_left.begin(); i != neg_left.end(); ++i) {
				LOG(FOILDBG) << *i << " ";
			}
			LOG(FOILDBG) << endl;
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
			pos_grow.expand(r, bound_vars, bound_inds, new_inds);
			neg_grow.expand(r, bound_vars, bound_inds, new_inds);
			for (int i = 0; i < new_inds.size(); ++i) {
				l.set_arg(new_inds[i], n++);
			}
		} else {
			pos_grow.subtract(bound_vars, *rp);
			neg_grow.subtract(bound_vars, *rp);
		}
		if (needs_slice) {
			delete rp;
		}
		c.push_back(l);
	}
	return true;
}

double FOIL::clause_success_rate(const clause &c) const {
	double correct = 0;
	set<int> objs;
	relation_table::const_iterator ri;
	for (ri = rels.begin(); ri != rels.end(); ++ri) {
		const relation &r = ri->second;
		for (int i = 0; i < r.arity(); ++i) {
			r.at_pos(i, objs);
		}
	}
	
	map<int, int> assign;
	set<int>::const_iterator i;
	for (int i = 0; i < pos_test.size(); ++i) {
		assign.clear();
		for(int j = 0; j < nvars; ++j) {
			assign[j] = pos_test[i][j];
		}
		if (test_clause(c, rels, objs, assign)) {
			++correct;
		}
	}
	for (int i = 0; i < neg_test.size(); ++i) {
		assign.clear();
		for(int j = 0; j < nvars; ++j) {
			assign[j] = neg_test[i][j];
		}
		if (!test_clause(c, rels, objs, assign)) {
			++correct;
		}
	}
	
	return correct / (pos_test.size() + neg_test.size());
}

bool FOIL::tuple_satisfies_literal(const tuple &t, const literal &l) {
	tuple ground_lit;
	tuple::const_iterator i;
	for (i = l.get_args().begin(); i != l.get_args().end(); ++i) {
		ground_lit.push_back(t[*i]);
	}
	bool inrel = get_rel(l.get_name()).has(ground_lit);
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
			if (args[j] >= 0 && args[j] < nvars) {
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
