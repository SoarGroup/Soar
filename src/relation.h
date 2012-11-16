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
#include "common.h"
#include "serializable.h"

/*
 Set implementation with a sorted vector
*/
class vec_set : public serializable {
public:
	vec_set();
	vec_set(const vec_set &v);
	~vec_set();
	
	bool insert(int x);
	bool erase(int x);
	void unify(const vec_set &v);
	void intersect(const vec_set &v);
	void subtract(const vec_set &v);
	
	void unify(const vec_set &v, vec_set &result) const;
	void intersect(const vec_set &v, vec_set &result) const;
	void difference(const vec_set &v, vec_set &result) const;
	
	bool contains(int x) const               { return binary_search(curr->begin(), curr->end(), x); }
	int  size() const                        { return curr->size(); }
	bool empty() const                       { return curr->empty(); }
	bool operator==(const vec_set &v) const  { return *curr == *v.curr; }
	const std::vector<int> &vec() const      { return *curr; }
	void clear()                             { curr->clear(); }

	vec_set &operator=(const vec_set &v);
	vec_set &operator=(const std::set<int> &s);

	void serialize(std::ostream &os) const;
	void unserialize(std::istream &is);
	
private:
	void fix(int old_size);
	
	std::vector<int> *curr, *work;
};

class relation : public serializable {
public:
	typedef std::map<tuple, vec_set > tuple_map;

	class iter {
	public:
		iter() : j(0) {}
		iter(const iter &rhs) : i(rhs.i), end(rhs.end), j(rhs.j), t(rhs.t) {}

		iter &operator++() {
			if (i != end) {
				if (++j >= i->second.size()) {
					j = 0;
					if (++i != end) {
						copy(i->first.begin(), i->first.end(), t.begin() + 1);
						t[0] = i->second.vec()[j];
					}
				} else {
					t[0] = i->second.vec()[j];
				}
			}
			return *this;
		}
		
		iter operator++(int) {
			iter c(*this);
			++(*this);
			return c;
		}
		
		iter &operator=(const iter &rhs)	{
			i = rhs.i;
			end = rhs.end;
			j = rhs.j;
			t = rhs.t;
			return *this;
		}
		
		const tuple &operator*() const  { return t; }
		const tuple *operator->() const { return &t; }
		bool operator==(const iter &rhs) const { return i == rhs.i && j == rhs.j; }
		bool operator!=(const iter &rhs) const { return i != rhs.i || j != rhs.j; }

	private:
		iter(const relation &r, bool begin)
		: end(r.tuples.end()), j(0), t(r.arity())
		{
			if (begin) {
				i = r.tuples.begin();
				if (i != end) {
					copy(i->first.begin(), i->first.end(), t.begin() + 1);
					t[0] = i->second.vec()[j];
				}
			} else {
				i = end;
			}
		}
		
		tuple_map::const_iterator i;
		tuple_map::const_iterator end;
		int j;
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
	void intersect(const tuple &inds, const relation &r);
	void subtract(const relation &r);
	void subtract(const tuple &inds, const relation &r);
	void expand(const relation &r, const tuple &match1, const tuple &match2, const tuple &extend);
	void filter(const tuple &pattern);
	void clear();
	void reset(int new_arity);
	relation &operator=(const relation &r);
	
	bool has(const tuple &t) const;
	void slice(const tuple &inds, relation &out) const;
	void slice(int n, relation &out) const;
	bool operator==(const relation &r) const;
	void count_expansion(const relation  &r, const tuple &match1, const tuple &match2, int &matched, int &new_size) const;
	void at_pos(int n, vec_set &elems) const;
	void drop_first(std::set<tuple> &out) const;
	void match(const tuple &pattern, relation &r) const;
	void random_split(int k, relation *r1, relation *r2) const;
	void subtract(const relation &r, relation &out) const;
	
	void serialize(std::ostream &os) const;
	void unserialize(std::istream &is);
	void foil6_rep(std::ostream &os) const;
		
	int size() const { return sz; }
	int arity() const { return arty; }
	bool empty() const { return sz == 0; }
	
	const_iterator begin() const { return iter(*this, true); }
	const const_iterator &end() const   { return end_iter; }
	
	template<typename C>
	void dump(C &out) const {
		std::insert_iterator<C> ins(out, out.end());
		tuple_map::const_iterator i;
		tuple t(arty);
		for (i = tuples.begin(); i != tuples.end(); ++i) {
			copy(i->first.begin(), i->first.end(), t.begin() + 1);
			const std::vector<int> &s = i->second.vec();
			for (int j = 0; j < s.size(); ++j) {
				t[0] = s[j];
				ins = t;
			}
		}
	}

private:
	void update_size();
	
	int sz, arty;
	tuple_map tuples;
	
	iter end_iter;
	
	friend std::ostream &operator<<(std::ostream &os, const relation &r);
};

typedef std::map<std::string, relation> relation_table;

std::ostream &operator<<(std::ostream &os, const relation &r);
std::ostream &operator<<(std::ostream &os, const relation_table &t);

#endif
