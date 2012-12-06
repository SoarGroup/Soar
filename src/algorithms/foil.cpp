#include <cmath>
#include "foil.h"
#include "serialize.h"
#include "params.h"

using namespace std;

void clause_vars(const clause &c, vector<int> &vars) {
	for (int i = 0; i < c.size(); ++i) {
		const tuple &args = c[i].get_args();
		for (int j = 0; j < args.size(); ++j) {
			if (args[j] >= 0) {
				vars.push_back(args[j]);
			}
		}
	}
	sort(vars.begin(), vars.end());
	vector<int>::iterator end = unique(vars.begin(), vars.end());
	vars.resize(end - vars.begin());
}

class csp_node {
public:
	csp_node() {}

	csp_node(const csp_node &par)
	: vars(par.vars), constraints(par.constraints), unassigned(par.unassigned)
	{}

	bool init(const clause &c, const relation_table &rels, const var_domains &domains) {
		vector<int> all_vars;
		map<int, int> var_map;
		
		clause_vars(c, all_vars);
		unassigned = all_vars.size();
		vars.resize(unassigned);
		for (int i = 0; i < vars.size(); ++i) {
			int v = all_vars[i];
			var_map[v] = i;
			vars[i].label = v;
			if (has(domains, v) && !map_get(domains, v).empty()) {
				vars[i].infinite_domain = false;
				vars[i].domain = map_get(domains, v);
			}
		}

		constraints.resize(c.size());
		for (int i = 0; i < c.size(); ++i) {
			const tuple &args = c[i].get_args();
			const relation &r = map_get(rels, c[i].get_name());
			constraint_info &cons = constraints[i];
			cons.negated = c[i].negated();
			cons.tuples = r;
			cons.doms.resize(args.size());
			cons.vars.resize(args.size());
			cons.unbound = 0;
			for (int j = 0; j < args.size(); ++j) {
				if (args[j] >= 0) {
					cons.vars[j] = map_get(var_map, args[j]);
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
			for (int i = 0; i < vars.size(); ++i) {
				out[vars[i].label] = vars[i].value;
			}
			return true;
		}
		
		// find MRV
		int mrv = -1;
		for (int i = 0; i < vars.size(); ++i) {
			if (vars[i].value >= 0) {
				continue;
			}
			if (mrv < 0 || 
			    (!vars[i].infinite_domain && vars[mrv].infinite_domain) ||
			    (!vars[i].infinite_domain && vars[i].domain.size() < vars[mrv].domain.size()))
			{
				mrv = i;
			}
		}
		assert(!vars[mrv].infinite_domain);
		interval_set::const_iterator i, end;
		for (i = vars[mrv].domain.begin(), end = vars[mrv].domain.end(); i != end; ++i) {
			csp_node child(*this);
			if (child.assign(mrv, *i) && child.search(out)) {
				return true;
			}
		}
		return false;
	}

private:
	struct constraint_info {
		bool negated;
		int unbound;
		relation tuples;
		vector<interval_set> doms;
		vector<int> vars;
	};

	struct var_info {
		interval_set domain;
		bool infinite_domain;  // variable can be any value
		int label;
		int value;

		var_info() : label(-1), value(-1), infinite_domain(true) {}
	};

	vector<var_info>        vars;
	vector<constraint_info> constraints;
	int unassigned;
	
	/*
	 For each constraint c and position i that var is in, remove
	 all tuples in c whose ith argument is not val.
	*/
	bool assign(int v, int value) {
		assert(0 <= v && v < vars.size());
		var_info &var = vars[v];
		if (!var.infinite_domain && !var.domain.contains(value)) {
			return false;
		}
		var.value = value;
		var.domain.clear();
		var.domain.insert(value);
		var.infinite_domain = false;
		if (--unassigned == 0) {
			return true;
		}
		
		vector<bool> need_update(constraints.size(), false);
		for (int i = 0; i < constraints.size(); ++i) {
			constraint_info &cons = constraints[i];
			vector<tuple> pat(cons.tuples.arity());
			for (int j = 0; j < cons.vars.size(); ++j) {
				if (cons.vars[j] == v) {
					pat[j].push_back(value);
					--cons.unbound;
					need_update[i] = true;
				}
			}
			cons.tuples.filter(pat, false);
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
			return true;
		}
		if (!cons.negated) {
			if (var.infinite_domain) {
				var.domain = cons.doms[j];
				var.infinite_domain = false;
			} else {
				var.domain.intersect(cons.doms[j]);
			}
		} else if (cons.unbound == 1) {
			assert(!var.infinite_domain);
			var.domain.subtract(cons.doms[j]);
		}
		return var.infinite_domain || !var.domain.empty();
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

bool test_clause(const clause &c, const relation_table &rels, var_domains &domains) {
	csp_node csp;
	if (!csp.init(c, rels, domains)) {
		return false;
	}
	map<int, int> assign;
	if (!csp.search(assign)) {
		return false;
	}
	map<int, int>::const_iterator i;
	for (i = assign.begin(); i != assign.end(); ++i) {
		domains[i->first].clear();
		domains[i->first].insert(i->second);
	}
	return true;
}

int test_clause_n(const clause &c, bool pos, const relation &tests, const relation_table &rels, relation *correct) {
	int ncorrect = 0;
	var_domains doms;
	relation::const_iterator i;
	for (i = tests.begin(); i != tests.end(); ++i) {
		doms.clear();
		const tuple &t = *i;
		for (int j = 0; j < t.size(); ++j) {
			doms[j].insert(t[j]);
		}
		if (test_clause(c, rels, doms) == pos) {
			++ncorrect;
			if (correct) {
				correct->add(t);
			}
		}
	}
	return ncorrect;
}

int test_clause_vec(const clause_vec &c, const relation_table &rels, var_domains &domains) {
	for (int i = 0; i < c.size(); ++i) {
		var_domains d = domains;
		if (test_clause(c[i], rels, d)) {
			LOG(FOILDBG) << "found assignment" << endl;
			for (int j = 0; j < d.size(); ++j) {
				LOG(FOILDBG) << j << " -> " << *d[j].begin() << endl;
			}
			domains = d;
			return i;
		}
	}
	LOG(FOILDBG) << "failed finding assignment" << endl;
	return -1;
}

double clause_success_rate(const clause &c, const relation &pos, const relation &neg, const relation_table &rels) {
	int correct = test_clause_n(c, true, pos, rels, NULL);
	correct += test_clause_n(c, false, neg, rels, NULL);
	return correct / static_cast<double>(pos.size() + neg.size());
}

/*
 A variable is unbound if it only appears in negated literals. These should be
 set to -1 to indicate they don't need to be bound when testing for
 satisfaction.
*/
void fix_unbound_variables(clause &c) {
	vector<int> status;
	for (int i = 0; i < c.size(); ++i) {
		const literal &l = c[i];
		const tuple &args = l.get_args();
		for (int j = 0; j < args.size(); ++j) {
			int v = args[j];
			if (v < 0) {
				continue;
			}
			if (v >= status.size()) {
				status.resize(v + 1, 0);
			}
			if (!l.negated()) {
				status[v] = 1;
			} else if (status[v] == 0) {
				status[v] = -1;
			}
		}
	}
	for (int i = 0; i < status.size(); ++i) {
		if (status[i] >= 0) {
			continue;
		}
		for (int j = 0; j < c.size(); ++j) {
			const tuple &args = c[j].get_args();
			for (int k = 0; k < args.size(); ++k) {
				if (args[k] == i) {
					c[j].set_arg(k, -1);
				}
			}
		}
	}
}

double prune_clause(clause &c, const relation &pos, const relation &neg, const relation_table &rels) {
	while (true) {
		int best_lit = -1;
		double best_rate = clause_success_rate(c, pos, neg, rels);
		cout << "original: " << c << " " << best_rate << endl;
		for (int i = 0; i < c.size(); ++i) {
			clause pruned = c;
			pruned.erase(pruned.begin() + i);
			fix_unbound_variables(pruned);
			double r = clause_success_rate(pruned, pos, neg, rels);
			if (r > best_rate) {
				best_lit = i;
				best_rate = r;
			}
		}
		if (best_lit < 0) {
			return best_rate;
		}
		c.erase(c.begin() + best_lit);
		fix_unbound_variables(c);
		cout << "pruned:   " << c << " " << best_rate << endl;
	}
}

void split_training(double ratio, const relation &all, relation &grow, relation &test) {
	assert(0 <= ratio && ratio < 1);
	grow.reset(all.arity());
	test.reset(all.arity());
	int ngrow = max(all.size() * ratio, 1.0);
	all.random_split(ngrow, &grow, &test);
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
: pos(p), neg(n), rels(rels), init_vars(p.arity())
{
	assert(p.arity() == n.arity());
}

/*
 Learns a list of clauses to classify the positive and negative
 examples. The residuals vector will be filled as follows:
 
 for i < clauses.size(), residuals[i] contains negative examples
                         incorrectly covered by out by clauses[i]
 
 for i == clauses.size(), residuals[i] contains positive examples not
                          covered by any of the clauses

 If all positive examples are covered by the clauses, the residuals
 vector will be the same length as the clauses vector, and the second
 case will not exist.
*/
bool FOIL::learn(clause_vec &clauses, vector<relation*> *residuals) {
	relation pos_test, neg_test;
	if (residuals) {
		clear_and_dealloc(*residuals);
	}
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
		relation *res = NULL;
		if (residuals) {
			res = new relation(init_vars);
			choose_clause(c, res);
		} else {
			choose_clause(c, NULL);
		}
		
		double success_rate = prune_clause(c, pos_test, neg_test, rels);
		// should test false positive rate here, rather than success rate
		if (!c.empty() && success_rate > FOIL_MIN_SUCCESS_RATE) {
			clauses.push_back(c);
			if (residuals) {
				residuals->push_back(res);
			}
			
			// this repeats computation, make more efficient
			relation covered_pos(init_vars);
			relation::const_iterator i;
			for (i = pos.begin(); i != pos.end(); ++i) {
				var_domains d;
				for (int j = 0; j < i->size(); ++j) {
					d[j].insert(i->at(j));
				}
				if (test_clause(c, rels, d)) {
					covered_pos.add(*i);
				}
			}
			int old_size = pos.size();
			pos.subtract(covered_pos);
			assert(pos.size() + covered_pos.size() == old_size);
			dead = (pos.size() == old_size);
		}
		
		if (dead) {
			if (residuals) {
				residuals->push_back(new relation(pos));
			}
			assert(!residuals || clauses.size() + 1 == residuals->size());
			return false;
		}
	}
	assert(!residuals || clauses.size() == residuals->size());
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
		
		relation sliced(bound_inds.size());
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

bool FOIL::choose_clause(clause &c, relation *neg_left) {
	int n = init_vars;
	while (!neg_grow.empty() && c.size() < FOIL_MAX_CLAUSE_LEN) {
		literal l;
		double gain = choose_literal(l, n);
		if (gain <= 0) {
			if (neg_left) {
				neg_grow.slice(init_vars, *neg_left);
				LOG(FOILDBG) << "No more suitable literals." << endl;
				LOG(FOILDBG) << "unfiltered negatives: "; 
				LOG(FOILDBG) << *neg_left << endl;
			}
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
			relation *rn = new relation(bound_inds.size());
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

bool FOIL::tuple_satisfies_literal(const tuple &t, const literal &l) {
	tuple ground_lit;
	tuple::const_iterator i;
	for (i = l.get_args().begin(); i != l.get_args().end(); ++i) {
		ground_lit.push_back(t[*i]);
	}
	bool inrel = get_rel(l.get_name()).contains(ground_lit);
	if (l.negated()) {
		return !inrel;
	}
	return inrel;
}

const relation &FOIL::get_rel(const string &name) const {
	map<string, relation>::const_iterator i = rels.find(name);
	assert(i != rels.end());
	return i->second;
}

void FOIL::dump_foil6(ostream &os) const {
	tuple zero(1, 0);
	relation all_times_rel(1);
	interval_set all_times, all_objs;
	
	pos.slice(zero, all_times_rel);
	neg.slice(zero, all_times_rel);
	all_times_rel.at_pos(0, all_times);
	
	relation_table::const_iterator i;
	for (i = rels.begin(); i != rels.end(); ++i) {
		for (int j = 1; j < i->second.arity(); ++j) {
			i->second.at_pos(j, all_objs);
		}
	}
	
	os << "O: ";
	join(os, all_objs, ",") << "." << endl;
	os << "T: ";
	join(os, all_times, ",") << "." << endl << endl;
	
	for (i = rels.begin(); i != rels.end(); ++i) {
		os << "*" << i->first << "(T";
		for (int j = 1; j < i->second.arity(); ++j) {
			os << ",O";
		}
		os << ") #";
		for (int j = 1; j < i->second.arity(); ++j) {
			os << "-";
		}
		os << endl;
		relation r(i->second);
		r.intersect(zero, all_times_rel);
		r.dump_foil6(os);
	}
	
	os << "positive(T";
	for (int j = 1; j < pos.arity(); ++j) {
		os << ",O";
	}
	os << ") ";
	for (int j = 0; j < pos.arity(); ++j) {
		os << "#";
	}
	os << endl;
	pos.dump_foil6(os, false);
	os << ";" << endl;
	neg.dump_foil6(os);
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
			if (true && //(!c->lit.negated() || c->vars_left.empty()) &&
				(*best == NULL || c->gain > (**best).gain || (c->gain == (**best).gain && c->nbound > (**best).nbound)))
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
