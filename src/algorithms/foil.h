#ifndef FOIL_H
#define FOIL_H

#include <vector>
#include "common.h"
#include "serializable.h"
#include "relation.h"

class literal : public serializable {
public:
	literal() : negate(false) {}
	
	literal(const std::string &name, const tuple &args, bool negate)
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
	
	const std::string &get_name() const { return name; }
	const tuple &get_args() const { return args; }
	bool negated() const { return negate; }
	
	void set_arg(int i, int v) { args[i] = v; }

	int operator<<(const std::string &s);
	void serialize(std::ostream &os) const;
	void unserialize(std::istream &is);
	
private:
	std::string name;
	tuple args;
	bool negate;

	friend std::ostream &operator<<(std::ostream &os, const literal &l);
};

std::ostream &operator<<(std::ostream &os, const literal &l);

typedef std::vector<literal> clause;
typedef std::vector<clause> clause_vec;

std::ostream &operator<<(std::ostream &os, const clause &c);

class FOIL {
public:
	FOIL(const relation &pos, const relation &neg, const relation_table &rels) ;
	bool learn(clause_vec &clauses);
	void gain(const literal &l, double &g, double &maxg) const;

	const relation_table &get_relations() const { return rels; }
	const relation &get_rel(const std::string &name) const;
	
private:
	double choose_literal(literal &l);
	bool add_clause(clause &c);
	bool tuple_satisfies_literal(const tuple &t, const literal &l);
	bool filter_pos_by_clause(const clause &c);

private:
	int nvars;
	relation pos, neg, pos_curr, neg_curr;
	const relation_table &rels;
};

bool test_clause(const clause &c,
                 const relation_table &rels,
                 const std::set<int> &objs,
                 std::map<int,int> &assignments);

int test_clause_vec(const clause_vec &c,
                    const relation_table &rels,
                    const std::set<int> &objs,
                    std::map<int,int> &assignments);

#endif
