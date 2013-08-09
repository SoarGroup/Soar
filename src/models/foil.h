#ifndef FOIL_H
#define FOIL_H

#include <vector>
#include "common.h"
#include "serializable.h"
#include "relation.h"

class logger_set;

class literal : public serializable {
public:
	literal() : negate(false) {}
	
	literal(const std::string &name, const int_tuple &args, bool negate)
	: name(name), args(args), negate(negate)
	{}
	
	literal(const literal &l)
	: name(l.name), args(l.args), negate(l.negate)
	{}
	
	literal &operator=(const literal &l) {
		name = l.name;
		args = l.args;
		negate = l.negate;
		return *this;
	}
	
	bool operator==(const literal &l) const { return name == l.name && args == l.args && negate == l.negate; }
	
	int new_vars() const;
	const std::string &get_name() const { return name; }
	const int_tuple &get_args() const { return args; }
	bool negated() const { return negate; }
	
	void set_arg(int i, int v) { args[i] = v; }

	int operator<<(const std::string &s);
	void serialize(std::ostream &os) const;
	void unserialize(std::istream &is);
	
private:
	std::string name;
	int_tuple args;
	bool negate;

	friend std::ostream &operator<<(std::ostream &os, const literal &l);
};

std::ostream &operator<<(std::ostream &os, const literal &l);

typedef std::vector<literal> clause;

std::ostream &operator<<(std::ostream &os, const clause &c);

class FOIL {
public:
	FOIL(logger_set *loggers);
	~FOIL();
	void set_problem(const relation &pos, const relation &neg, const relation_table &rels);
	bool learn(bool prune, bool track_training);
	void gain(const literal &l, double &g, double &maxg) const;
	void dump_foil6(std::ostream &os) const;
	bool load_foil6(std::istream &is);

	const relation_table &get_relations() const { return *rels; }
	const relation &get_pos() const { return pos; }
	const relation &get_neg() const { return neg; }
	const relation &get_rel(const std::string &name) const;
	
	const int num_clauses() const { return clauses.size(); }
	const clause &get_clause(int i) const { return clauses[i].cl; }
	const relation &get_false_positives(int i) const { return clauses[i].false_positives; }
	const relation &get_true_positives(int i) const { return clauses[i].true_positives; }
	const relation &get_false_negatives() const { return false_negatives; }
	const relation &get_true_negatives() const { return true_negatives; }
	
private:
	double choose_literal(literal &l, int nvars);
	bool choose_clause(clause &c, relation *neg_left);

private:
	struct clause_info {
		clause cl;
		relation true_positives;
		relation false_positives;
	};
	
	relation pos, neg, pos_grow, neg_grow;
	relation_table const *rels;
	bool own_rels;
	int train_dim;
	
	std::vector<clause_info> clauses;
	
	relation false_negatives;
	relation true_negatives;
	
	logger_set *loggers;
};

typedef std::map<int, std::set<int> > var_domains;
bool test_clause(const clause &c, const relation_table &rels, var_domains &domains);
void clause_success_rate(const clause &c, const relation &pos, const relation &neg, const relation_table &rels, double &success_rate, double &fp_rate, double &fn_rate);

#endif
