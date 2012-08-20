#ifndef FOIL_H
#define FOIL_H

#include <vector>
#include "common.h"

typedef std::pair<std::string, tuple> literal;
typedef std::vector<literal> clause;
typedef std::vector<clause> clause_vec;

class FOIL {
public:
	FOIL(const std::vector<int> &pos_insts, const std::vector<int> &neg_insts, const relation_table &rels) ;
	bool learn(clause_vec &clauses);
	void gain(const literal &l, double &g, double &maxg) const;

	const relation_table &get_relations() const { return rels; }
	const relation &get_rel(const std::string &name) const;
	
private:
	void filter_neg_by_relation(const index_vec &rinds, const index_vec &tinds, const relation &r);
	double choose_literal(literal &l);
	bool add_clause(clause &c);
	bool tuple_satisfies_literal(const tuple &t, const literal &l);
	bool filter_pos_by_clause(const clause &c);

private:
	int nvars;
	relation pos, neg;
	const relation_table &rels;
};

bool test_clause(const clause &c, const relation_table &t);
bool test_clause_vec(const clause_vec &c, const relation_table &t);

#endif
