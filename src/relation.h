/*
 A relation is essentially a list of all argument values for which some
 predicate evaluates to true.
*/

#ifndef RELATION_H
#define RELATION_H

#include <iostream>
#include <vector>
#include <map>
#include "common.h"
#include "serializable.h"

class relation : public serializable {
public:
	relation();
	relation(int n);
	relation(const relation &r);
	relation(int n, const std::vector<tuple> &t);
	
	void init_single(const std::vector<int> &s);
	void add(int i);
	void add(int i, const tuple &t);
	void del(int i);
	void del(int i, const tuple &t);
	bool test(const tuple &t) const;
	void slice(const tuple &inds, relation &out) const;
	bool operator==(const relation &r) const;
	relation &operator=(const relation &r);
	void expand(const relation &r, const tuple &match1, const tuple &match2, const tuple &extend);
	void count_expansion(const relation  &r, const tuple &match1, const tuple &match2, int &matched, int &new_size) const;
	void intersect(const tuple &inds, const relation &r);
	void subtract(const tuple &inds, const relation &r);
	void at_pos(int n, std::set<int> &elems) const;
	void drop_first(std::set<tuple> &out) const;
	void dump(std::set<tuple> &out) const;
	
	int size() const { return sz; }
	int arity() const { return arty; }
	bool empty() const { return sz == 0; }
	
	void serialize(std::ostream &os) const;
	void unserialize(std::istream &is);
	
private:
	typedef std::map<tuple, std::set<int> > tuple_map;

	struct sliced_relation_tuple {
		tuple match;
		tuple extend;
		const std::set<int> *lead;
	}; 

	int sz, arty;
	tuple_map tuples;
	
	friend std::ostream &operator<<(std::ostream &os, const relation &r);
};

typedef std::map<std::string, relation> relation_table;

std::ostream &operator<<(std::ostream &os, const relation &r);
std::ostream &operator<<(std::ostream &os, const relation_table &t);

#endif
