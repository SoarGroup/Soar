/*
 A relation is essentially a list of all tuples of arguments for which
 the associated predicate evaluates to true.
 
 Every relation has a time value for the first argument, with the
 actual objects following. So intersect(5, 1, 2) means that at time 5,
 object 1 intersects object 2. Therefore, a single relation spans the
 entire duration of an experiment. This convention makes the FOIL
 algorithm more straightforward to implement.
 
 In most cases, a predicate will evaluate to true for some set of
 objects for long stretches of time. For example, two objects may
 intersect for the entire duration of an experiment. A data structure
 optimization was made to exploit this fact: Instead of storing a
 relation as a set of tuples, it stored as a map whose keys are the
 tails of the tuples starting from the 2nd argument, and whose values
 are the sets of times for which the predicate was true for the keys.
 For example, the set of tuples:
 
 (0, A, B)
 (1, A, B)
 (2, A, B)
 (2, C, D)
 (3, C, D)
 
 would be stored as
 
 {
   (A, B) -> [ 0, 1, 2 ],
   (C, D) -> [ 2, 3 ]
 }
 
 Note that I'm using A, B, C, D for clarity. The actual tuples in the
 relation would all be integers. This optimization cuts down on memory
 as well as the cost of operations like intersection.
*/

#ifndef RELATION_H
#define RELATION_H

#include <iostream>
#include <vector>
#include <map>
#include <iterator>
#include "common.h"
#include "serializable.h"

/*
 Set implementation with a sorted vector of intervals
*/
class interval_set : public serializable {
public:
	class interval : public serializable {
	public:
		int first;
		int last;
		
		interval() : first(0), last(-1) {}
		bool operator==(const interval &i) const { return first == i.first && last == i.last; }
		void serialize(std::ostream &os) const;
		void unserialize(std::istream &is);
	};
	
	class const_iterator : public std::iterator<std::forward_iterator_tag, int> {
	public:
		const_iterator() : i(-1), j(-1), s(NULL) {}
		const_iterator(const const_iterator &rhs) : i(rhs.i), j(rhs.j), s(rhs.s) {}

		const_iterator &operator++();
		const_iterator operator++(int);
		const_iterator &operator=(const const_iterator &rhs);
		
		int operator*() const  { return j; }
		bool operator==(const const_iterator &rhs) const { return i == rhs.i && j == rhs.j; }
		bool operator!=(const const_iterator &rhs) const { return i != rhs.i || j != rhs.j; }

	private:
		const_iterator(const interval_set &s, bool begin);
		
		const interval_set *s;
		int i, j;
		
		friend class interval_set;
	};
	
	interval_set();
	interval_set(const interval_set &v);
	
	template<typename C>
	interval_set(const C &container);
	
	~interval_set();
	
	bool insert(int x);
	bool erase(int x);
	void unify(const interval_set &v);
	void intersect(const interval_set &v);
	void subtract(const interval_set &v);
	
	void unify(const interval_set &v, interval_set &result) const;
	void intersect(const interval_set &v, interval_set &result) const;
	void subtract(const interval_set &v, interval_set &result) const;
	
	bool contains(int x) const;
	int  ith(int i) const;
	
	int  size() const                        { return sz; }
	bool empty() const                       { return curr->empty(); }
	bool operator==(const interval_set &v) const  { return *curr == *v.curr; }
	void clear()                             { curr->clear(); sz = 0; }

	interval_set &operator=(const interval_set &v);

	template<typename C>
	interval_set &operator=(const C &container);

	void serialize(std::ostream &os) const;
	void unserialize(std::istream &is);

	const_iterator begin() const        { return const_iterator(*this, true); }
	const_iterator end() const   { return const_iterator(*this, false); }
	
	int num_intervals() const { return curr->size(); }
	bool check_size() const;
	
	friend std::ostream &operator<<(std::ostream &os, const interval_set &s);
	
private:
	void update_size();
	
	int sz;
	std::vector<interval> *curr, *work;
};

std::ostream &operator<<(std::ostream &os, const interval_set &s);

class relation : public serializable {
public:
	typedef std::map<tuple, interval_set > tuple_map;

	class iter : public std::iterator<std::forward_iterator_tag, int> {
	public:
		iter() {}
		iter(const iter &rhs) : i(rhs.i), end(rhs.end), j(rhs.j), t(rhs.t) {}

		iter &operator++();
		iter operator++(int);
		iter &operator=(const iter &rhs);
		const tuple &operator*() const  { return t; }
		const tuple *operator->() const { return &t; }
		bool operator==(const iter &rhs) const {
			if (i == end && rhs.i == end)
				return true;
			return i == rhs.i && j == rhs.j;
		}
		bool operator!=(const iter &rhs) const { return !((*this) == rhs); }

	private:
		iter(const relation &r, bool begin);
		
		tuple_map::const_iterator i;
		tuple_map::const_iterator end;
		interval_set::const_iterator j, jend;
		tuple t;
		
		friend class relation;
	};
	
	typedef iter const_iterator;
	
	relation();
	relation(int n);
	relation(const relation &r);
	relation(int n, const std::vector<tuple> &t);
	
	void add(const tuple &t);
	void add(int i, int n);  // convenience for arity = 2
	void add(int i, const tuple &t);
	void del(const tuple &t);
	void del(int i, int n);  // convenience for arity = 2
	void del(int i, const tuple &t);

	void clear();
	void reset(int new_arity);
	relation &operator=(const relation &r);
	
	void intersect(const tuple &inds, const relation &r);
	void subtract(const relation &r);
	void subtract(const relation &r, relation &out) const;
	void subtract(const tuple &inds, const relation &r);
	void filter(int i, tuple &vals, bool complement);

	void expand(const relation &r, const tuple &match1, const tuple &match2, const tuple &extend);
	void slice(const tuple &inds, relation &out) const;
	void slice(int n, relation &out) const;

	void count_expansion(const relation  &r, const tuple &match1, const tuple &match2, int &matched, int &new_size) const;
	void at_pos(int n, interval_set &elems) const;
	bool contains(const tuple &t) const;
	bool operator==(const relation &r) const;
	void random_split(int k, relation *r1, relation *r2) const;
	
	void serialize(std::ostream &os) const;
	void unserialize(std::istream &is);
	void dump_foil6(std::ostream &os, bool terminate = true) const;
	bool load_foil6(std::istream &is);
		
	int size() const { return sz; }
	int arity() const { return arty; }
	bool empty() const { return sz == 0; }
	
	const_iterator begin() const { return iter(*this, true); }
	const_iterator end() const { return iter(*this, false); }
	
private:
	bool check_size() const;
	void update_size();
	
	int sz, arty;
	tuple_map tuples;
	
	friend std::ostream &operator<<(std::ostream &os, const relation &r);
};

typedef std::map<std::string, relation> relation_table;

std::ostream &operator<<(std::ostream &os, const relation &r);
std::ostream &operator<<(std::ostream &os, const relation_table &t);

// Add tuples from a single time point into the relation table
void extend_relations(relation_table &rels, const relation_table &add, int time);

// extract only tuples involving objects closest to target
void get_context_rels(int target, const relation_table &rels, relation_table &context_rels);

template<typename C>
interval_set::interval_set(const C &container) {
	curr = new std::vector<interval>;
	work = new std::vector<interval>;
	*this = container;
}

template<typename C>
interval_set &interval_set::operator=(const C &container) {
	interval in;
	typename C::const_iterator i, iend;
	
	curr->clear();
	if (!container.empty()) {
		i = container.begin();
		in.first = *i;
		in.last = *i;
		for (++i, iend = container.end(); i != iend; ++i) {
			if (*i > in.last + 1) {
				curr->push_back(in);
				in.first = *i;
				in.last = *i;
			} else {
				in.last++;
			}
		}
		curr->push_back(in);
	}
	sz = container.size();
	return *this;
}

#endif
