#include <cmath>
#include "foil.h"
#include "serialize.h"
#include "params.h"
#include "platform_specific.h" // for log2

using namespace std;

void clause_vars(const clause &c, vector<int> &vars) {
	vars.clear();
	vars.push_back(0);
	vars.push_back(1);  // these should be used for every clause
	for (int i = 0; i < c.size(); ++i) {
		const tuple &args = c[i].get_args();
		for (int j = 0; j < args.size(); ++j) {
			if (args[j] >= 0 && !has(vars, args[j])) {
				vars.push_back(args[j]);
			}
		}
	}
	sort(vars.begin(), vars.end());
}

class csp_node {
public:
	csp_node() {}

	csp_node(const csp_node &par)
	: vars(par.vars), constraints(par.constraints), num_unassigned(par.num_unassigned)
	{}

	bool init(const clause &c, const relation_table &rels, const var_domains &domains) {
		vector<int> all_vars;
		
		/*
		 Fill out variable information, and remap variables to a consecutive list 0 .. nvars-1
		*/
		clause_vars(c, all_vars);
		num_unassigned = all_vars.size();
		vars.resize(num_unassigned);
		for (int i = 0, iend = vars.size(); i < iend; ++i) {
			int v = all_vars[i];
			vars[i].label = v;
			if (has(domains, v) && !map_get(domains, v).empty()) {
				vars[i].infinite_domain = false;
				vars[i].domain = map_get(domains, v);
			}
		}

		/*
		 Each literal in the clause becomes a constraint.
		*/
		constraints.resize(c.size());
		for (int i = 0, iend = c.size(); i < iend; ++i) {
			literal_to_constraint(c[i], rels, constraints[i]);
		}
		for (int i = 0, iend = constraints.size(); i < iend; ++i) {
			constraint_info &cons = constraints[i];
			for (int j = 0, jend = cons.vars.size(); j < jend; ++j) {
				if (!update_vardom(i, j))
					return false;
			}
		}
		return true;
	}

	bool search(map<int, int> &out) {
		if (num_unassigned == 0) {
			for (int i = 0; i < vars.size(); ++i) {
				out[vars[i].label] = vars[i].value;
			}
			return true;
		}
		
		// find variable with minimum remaining values
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
		int num_unassigned;
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
	int num_unassigned;
	
	int get_var_by_label(int label) {
		int i;
		for (i = 0; i < vars.size() && vars[i].label != label; ++i)
			;
		assert(i < vars.size());
		return i;
	}
	
	void literal_to_constraint(const literal &l, const relation_table &rels, constraint_info &cons) {
		const tuple &args = l.get_args();
		const relation &r = map_get(rels, l.get_name());
		cons.negated = l.negated();
		cons.tuples = r;
		cons.doms.resize(args.size());
		cons.vars.resize(args.size());
		cons.num_unassigned = 0;
		for (int i = 0, iend = args.size(); i < iend; ++i) {
			if (args[i] >= 0) {
				cons.vars[i] = get_var_by_label(args[i]);
				r.at_pos(i, cons.doms[i]);
				++cons.num_unassigned;
			} else {
				cons.vars[i] = -1;
			}
		}
	}

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
		if (--num_unassigned == 0) {
			return true;
		}
		
		vector<bool> need_update(constraints.size(), false);
		for (int i = 0; i < constraints.size(); ++i) {
			constraint_info &cons = constraints[i];
			tuple t(1, value);
			for (int j = 0; j < cons.vars.size(); ++j) {
				if (cons.vars[j] == v) {
					cons.tuples.filter(j, t, false);
					--cons.num_unassigned;
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

	void update_cdoms(int c) {
		constraint_info &cons = constraints[c];
		
		for (int i = 0; i < cons.doms.size(); ++i) {
			cons.doms[i].clear();
			cons.tuples.at_pos(i, cons.doms[i]);
		}
	}

	/*
	 Constrain the domain of the j'th variable in constraint i to be consistent
	 with i. Remember: j is an index into constraints[i].vars, not an index into
	 the global vars array.
	*/
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
				// intersect with infinite set
				var.domain = cons.doms[j];
				var.infinite_domain = false;
			} else {
				var.domain.intersect(cons.doms[j]);
			}
		} else if (cons.num_unassigned == 1) {
			/*
			 a negated literal doesn't impose any real constraints on possible variable
			 values until it has only one unassigned variable left. Because all other
			 variables are assigned, we know what the unassigned variable _can't_ be.
			*/
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
	int compare(const literal_tree *t) const;
	int new_vars() const;
	
private:
	literal_tree(literal_tree *parent, const string &name, const relation &r, bool negate);
	literal_tree(literal_tree *parent, int pos, int var);
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

bool test_clause(const clause &c, const relation_table &rels, tuple &domains) {
	var_domains d;
	for (int i = 0, iend = domains.size(); i < iend; ++i) {
		d[i].insert(domains[i]);
	}
	if (test_clause(c, rels, d)) {
		for (int i = domains.size(), iend = d.size(); i < iend; ++i) {
			assert(d[i].size() == 1);
			domains.push_back(*d[i].begin());
		}
		return true;
	}
	return false;
}

int test_clause_n(const clause &c, bool pos, const relation &tests, const relation_table &rels, relation *correct) {
	int ncorrect = 0;
	relation::const_iterator i, iend;
	var_domains d;
	
	for (i = tests.begin(), iend = tests.end(); i != iend; ++i) {
		d.clear();
		for (int j = 0, jend = i->size(); j < jend; ++j) {
			d[j].insert((*i)[j]);
		}
		if (test_clause(c, rels, d) == pos) {
			++ncorrect;
			if (correct) {
				correct->add(*i);
			}
		}
	}
	return ncorrect;
}

void clause_success_rate(const clause &c, const relation &pos, const relation &neg, const relation_table &rels, double &success_rate, double &fp_rate, double &fn_rate) {
	int pos_correct, neg_correct;
	pos_correct = test_clause_n(c, true, pos, rels, NULL);
	fn_rate = (pos.size() - pos_correct) / static_cast<double>(pos.size());
	neg_correct = test_clause_n(c, false, neg, rels, NULL);
	fp_rate = (neg.size() - neg_correct) / static_cast<double>(neg.size());
	success_rate = (pos_correct + neg_correct) / static_cast<double>(pos.size() + neg.size());
}

/*
 A variable is unbound if it only appears in negated literals. These should be
 set to -1 to indicate they don't need to be bound when testing for
 satisfaction.
*/
void fix_variables(int num_auto_bound, clause &c) {
	vector<int> remap(num_auto_bound);
	for (int i = 0; i < num_auto_bound; ++i) {
		remap[i] = i;
	}
	for (int i = 0; i < c.size(); ++i) {
		const literal &l = c[i];
		const tuple &args = l.get_args();
		for (int j = 0; j < args.size(); ++j) {
			int v = args[j];
			if (v < num_auto_bound) {
				continue;
			}
			if (v >= remap.size()) {
				remap.resize(v + 1, 0);
			}
			if (!l.negated()) {
				remap[v] = v;
			} else if (remap[v] == 0) {
				remap[v] = -1;
			}
		}
	}
	for (int i = num_auto_bound, j = num_auto_bound; i < remap.size(); ++i) {
		if (remap[i] > 0) {
			remap[i] = j++;
		}
	}
	for (int i = 0, iend = c.size(); i < iend; ++i) {
		const tuple &args = c[i].get_args();
		for (int j = 0, jend = args.size(); j < jend; ++j) {
			if (args[j] >= 0) {
				assert(args[j] == 0 || remap[args[j]] != 0);
				c[i].set_arg(j, remap[args[j]]);
			}
		}
	}
}

/*
 Remove literals from the clause that result in a better clause. Several metrics can be used:
 
 - Accuracy (percentage of all positive and negative examples correctly classified)
 
   This metric is problematic if there are many positive examples and few
   negative examples. Removing literals lowers the false negative rate at the
   expense of the false positive rate, and in the case of many positive
   examples, accuracy is unfairly biased toward lowering false negative rate.
   The result is that in some cases all literals will be pruned.

 - False positive rate
 
   It's impossible to lower the false positive rate by removing literals, since
   the clause can only become less restrictive. But if a literal can be removed
   without increasing the false positive rate, it's a good sign that the
   literal is not useful.

 - False negative rate
 
   Using this metric would result in pruning all literals every time. Not a good idea.
   
 So currently it looks like the false positive rate is the way to go.
*/
 
void prune_clause_accuracy(clause &c, const relation &pos, const relation &neg, const relation_table &rels, double &success_rate, double &fp_rate, double &fn_rate) {
	double s, fp, fn;
	int best_lit;
	
	if (pos.empty() && neg.empty()) {
		success_rate = 1.0;
		fp_rate = 0.0;
		fn_rate = 0.0;
		return;
	}
	
	clause_success_rate(c, pos, neg, rels, success_rate, fp_rate, fn_rate);
	LOG(FOILDBG) << "original: " << c << " " << success_rate << endl;
	while (true) {
		best_lit = -1;
		for (int i = 0; i < c.size(); ++i) {
			clause pruned = c;
			pruned.erase(pruned.begin() + i);
			fix_variables(pos.arity(), pruned);
			clause_success_rate(pruned, pos, neg, rels, s, fp, fn);
			if (s > success_rate) {
				best_lit = i;
				success_rate = s;
				fp_rate = fp;
				fn_rate = fn;
			}
		}
		if (best_lit < 0) {
			break;
		}
		c.erase(c.begin() + best_lit);
		fix_variables(pos.arity(), c);
	}
	LOG(FOILDBG) << "pruned:   " << c << " " << success_rate << endl;
}

void prune_clause_fp(clause &c, const relation &pos, const relation &neg, const relation_table &rels, double &success_rate, double &fp_rate, double &fn_rate) {
	double s, fp, fn;
	
	if (pos.empty() && neg.empty()) {
		success_rate = 1.0;
		fp_rate = 0.0;
		fn_rate = 0.0;
		return;
	}
	
	clause_success_rate(c, pos, neg, rels, success_rate, fp_rate, fn_rate);
	LOG(FOILDBG) << "original: " << c << " " << success_rate << endl;
	for (int i = c.size() - 1; i >= 0; --i) {
		clause pruned = c;
		pruned.erase(pruned.begin() + i);
		fix_variables(pos.arity(), pruned);
		clause_success_rate(pruned, pos, neg, rels, s, fp, fn);
		LOG(FOILDBG) << "removing " << c[i] << " results in " << fp_rate << " -> " << fp << " " << fn_rate << " -> " << fn << endl;
		if (fp <= fp_rate) {
			c.erase(c.begin() + i);
		}
	}
	fix_variables(pos.arity(), c);
	LOG(FOILDBG) << "pruned:   " << c << " " << success_rate << endl;
}

void split_training(double ratio, const relation &all, relation &grow, relation &test) {
	assert(0 <= ratio && ratio < 1);
	grow.reset(all.arity());
	test.reset(all.arity());
	int ngrow = max(all.size() * ratio, 1.0);
	all.random_split(ngrow, &grow, &test);
}

int literal::new_vars() const {
	int n = 0;
	for (int i = 0, iend = args.size(); i < iend; ++i) {
		if (args[i] < 0)	
			++n;
	}
	return n;
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

FOIL::FOIL() : rels(NULL), own_rels(false) {}

FOIL::~FOIL() {
	if (own_rels)
		delete rels;
}

void FOIL::set_problem(const relation &p, const relation &n, const relation_table &rt) {
	pos = p;
	neg = n;
	train_dim = pos.arity();
	if (own_rels)
		delete rels;
	rels = &rt;
	own_rels = false;
}

/*
 Returns true if all training examples were correctly classified, false if not
*/
bool FOIL::learn(bool prune, bool record_errors) {
	relation pos_test, neg_test, pos_left;
	double success_rate, fp_rate, fn_rate;
	bool dead;
	
	if (neg.empty()) {
		return true;
	}
	clauses.clear();
	tuple t;
	t.push_back(0);
	pos_left = pos;
	dead = false;
	while (!pos_left.empty() && !dead) {
		split_training(FOIL_GROW_RATIO, pos_left, pos_grow, pos_test);
		split_training(FOIL_GROW_RATIO, neg, neg_grow, neg_test);
		
		clause_info &ci = grow_vec(clauses);
		relation *false_pos = NULL;
		dead = true;
		
		if (record_errors) {
			choose_clause(ci.cl, &ci.false_positives); // false_positives only contains examples from neg_grow, add from neg_test later
		} else {
			choose_clause(ci.cl, NULL);
		}
		
		clause old = ci.cl;
		if (prune) {
			prune_clause_fp(ci.cl, pos_test, neg_test, *rels, success_rate, fp_rate, fn_rate);
		} else {
			clause_success_rate(ci.cl, pos_test, neg_test, *rels, success_rate, fp_rate, fn_rate);
		}
		
		if (!ci.cl.empty() && fp_rate < MAX_CLAUSE_FP_RATE) {
			vector<int> vars;
			clause_vars(ci.cl, vars);
			if (ci.true_positives.arity() == 0) {
				ci.true_positives.reset(vars.back() + 1);
			}
			if (ci.false_positives.arity() == 0) {
				ci.false_positives.reset(vars.back() + 1);
			}
			
			// this repeats computation, make more efficient
			relation covered_pos(train_dim);
			relation::const_iterator i, iend;
			for (i = pos_left.begin(), iend = pos_left.end(); i != iend; ++i) {
				tuple t = *i;
				if (test_clause(ci.cl, *rels, t)) {
					covered_pos.add(*i);
					if (record_errors) {
						ci.true_positives.add(t);
					}
				}
			}
			if (record_errors) {
				for (i = neg_test.begin(), iend = neg_test.end(); i != iend; ++i) {
					tuple t = *i;
					if (test_clause(ci.cl, *rels, t)) {
						ci.false_positives.add(t);
					}
				}
			}
			int old_size = pos_left.size();
			pos_left.subtract(covered_pos);
			assert(pos_left.size() + covered_pos.size() == old_size);
			dead = (pos_left.size() == old_size);
		} else {
			clauses.pop_back();
		}
	}
	
	if (record_errors) {
		false_negatives = pos_left;
		true_negatives = neg;
		for (int i = 0, iend = clauses.size(); i < iend; ++i) {
			relation fp(train_dim);
			clauses[i].false_positives.slice(train_dim, fp);
			true_negatives.subtract(fp);
		}
	}
	return !dead;
}

void FOIL::gain(const literal &l, double &g, double &maxg) const {
	double I1, I2;
	int new_pos_size, new_neg_size, pos_match, neg_match;
	map<string, relation>::const_iterator ri = rels->find(l.get_name());
	assert(ri != rels->end());
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
	vector<double> gains;
	int n = train_dim;
	bool quiescence = false;
	
	while (!neg_grow.empty() && c.size() < FOIL_MAX_CLAUSE_LEN) {
		literal l;
		double gain = choose_literal(l, n);
		if (gain < 0 || (!c.empty() && l == c.back())) {
			if (neg_left) {
				neg_grow.slice(train_dim, *neg_left);
				LOG(FOILDBG) << "No more suitable literals." << endl;
				LOG(FOILDBG) << "unfiltered negatives: "; 
				LOG(FOILDBG) << *neg_left << endl;
			}
			quiescence = true;
			break;
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
		gains.push_back(gain);
	}
	
	/*
	 Delete literals at the end of the clause that had 0 gain. These don't
	 contribute to discrimination but were added in case they introduced new
	 variables that potentially would have increased gain later. Remove the ones
	 that didn't pay off.
	*/
	for (int i = gains.size() - 1; i >= 0; --i) {
		if (gains[i] == 0.0) {
			c.pop_back();
		} else {
			break;
		}
	}
	return quiescence;
}

const relation &FOIL::get_rel(const string &name) const {
	map<string, relation>::const_iterator i = rels->find(name);
	assert(i != rels->end());
	return i->second;
}

void FOIL::dump_foil6(ostream &os) const {
	tuple zero(1, 0);
	relation all_times_rel(1);
	interval_set all_times, all_objs;
	
	pos.slice(1, all_times_rel);
	neg.slice(1, all_times_rel);
	all_times_rel.at_pos(0, all_times);
	
	relation_table::const_iterator i, iend;
	for (i = rels->begin(), iend = rels->end(); i != iend; ++i) {
		for (int j = 1, jend = i->second.arity(); j < jend; ++j) {
			i->second.at_pos(j, all_objs);
		}
	}
	
	os << "O: ";
	join(os, all_objs, ",") << "." << endl;
	os << "T: ";
	join(os, all_times, ",") << "." << endl << endl;
	
	for (i = rels->begin(), iend = rels->end(); i != iend; ++i) {
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
	
	os << "target(T";
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

bool FOIL::load_foil6(istream &is) {
	string line;
	vector<string> fields;
	bool error = false;
	relation_table *new_rels = new relation_table;
	
	// skip time and object enumeration
	while (getline(is, line)) {
		if (line.empty())
			break;
	}
	
	while (getline(is, line)) {
		int arity;
		
		fields.clear();
		split(line, "(,) ", fields);
		assert(fields.size() >= 3);      // at least name, one argument, and argument description
		arity = fields.back().size();
		assert(arity > 0);
		
		if (fields[0] == "target") {
			pos.reset(arity);
			neg.reset(arity);
			if (!pos.load_foil6(is)) {
				cerr << "invalid input for positive" << endl;
				error = true;
				break;
			}
			if (!neg.load_foil6(is)) {
				cerr << "invalid input for negative" << endl;
				error = true;
				break;
			}
			assert(pos.arity() == neg.arity());
			train_dim = pos.arity();
			break;
		} else {
			assert(fields[0][0] == '*');
			string name = fields[0].substr(1);
			relation &r = (*new_rels)[name];
			r.reset(arity);
			if (!r.load_foil6(is)) {
				cerr << "invalid input for relation " << fields[0] << endl;
				error = true;
				break;
			}
		}
	}
	
	if (error) {
		delete new_rels;
		return false;
	}
	
	if (own_rels)
		delete rels;
	
	rels = new_rels;
	own_rels = true;
	return true;
}

literal_tree::literal_tree(const FOIL &foil, int nvars, literal_tree **best) 
: foil(foil), best(best), expanded(false), position(-1), nbound(-1)
{
	static int count = 0;
	for (int i = 0; i < nvars; ++i) {
		vars_left.push_back(i);
	}
	*best = NULL;
	map<string, relation>::const_iterator i;
	const map<string, relation> &rels = foil.get_relations();
	for (i = rels.begin(); i != rels.end(); ++i) {
		literal_tree *t1, *t2;
		t1 = new literal_tree(this, i->first, i->second, false);
		t2 = new literal_tree(this, i->first, i->second, true);
		children.push_back(t1);
		children.push_back(t2);
	}
	expanded = true;
}

literal_tree::literal_tree(literal_tree *par, const string &name, const relation &r, bool negate)
: foil(par->foil), best(par->best), lit(name, tuple(r.arity(), -1), negate),
  expanded(false), position(0), vars_left(par->vars_left.begin() + 1, par->vars_left.end()),
  nbound(1)
{
	lit.set_arg(0, 0);
}

literal_tree::literal_tree(literal_tree *par, int pos, int var)
: foil(par->foil), position(pos), lit(par->lit), best(par->best), expanded(false)
{ 
	lit.set_arg(position, var);
	nbound = par->nbound + 1;
	tuple::const_iterator i, iend;
	for (i = par->vars_left.begin(), iend = par->vars_left.end(); i != iend; ++i) {
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
int literal_tree::compare(const literal_tree *t) const {
	if (gain > t->gain) {
		return 1;
	} else if (gain < t->gain) {
		return -1;
	}

	if (!lit.negated() && t->lit.negated()) {
		return 1;
	} else if (lit.negated() && !t->lit.negated()) {
		return -1;
	}
	
	if (!lit.negated() && !t->lit.negated()) {
		if (lit.new_vars() > t->lit.new_vars()) {
			return 1;
		} else if (lit.new_vars() < t->lit.new_vars()) {
			return -1;
		}
	}
	
	if (max_gain > t->max_gain) {
		return 1;
	} else if (max_gain < t->max_gain) {
		return -1;
	}
	
	return 0;
}

void literal_tree::expand() {
	const tuple &vars = lit.get_args();
	for (int i = position + 1; i < vars.size(); ++i) {
		if (vars[i] >= 0) {
			continue;
		}
		for (int j = 0; j < vars_left.size(); ++j) {
			literal_tree *c = new literal_tree(this, i, vars_left[j]);
			foil.gain(c->lit, c->gain, c->max_gain);
			if (*best != NULL && c->max_gain < (**best).gain) {
				delete c;
				continue;
			}
			if (true && //(!c->lit.negated() || c->vars_left.empty()) &&
				(*best == NULL || c->compare(*best) > 0))
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

