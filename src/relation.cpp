#include <limits>
#include "relation.h"
#include "serialize.h"

using namespace std;

struct sliced_relation_tuple {
	tuple match;
	tuple extend;
	const interval_set *lead;
}; 

void slice_tuple(const tuple &t1, const tuple &inds, tuple &t2) {
	int n = inds.size();
	t2.clear();
	t2.resize(inds.size());
	for (int i = 0; i < n; ++i) {
		t2[i] = t1[inds[i]];
	}
}

tuple concat_tuples(const tuple &t1, const tuple &t2) {
	tuple t3(t1.begin(), t1.end());
	t3.insert(t3.end(), t2.begin(), t2.end());
	return t3;
}

typedef vector<interval_set::interval> interval_vec;

void unify_interval_vecs(const interval_vec &v1, const interval_vec &v2, interval_vec &r) {
	interval_vec::const_iterator p1, p2, e1, e2;
	interval_set::interval i, j;
	
	r.reserve(v1.size() + v2.size());
	p1 = v1.begin();
	e1 = v1.end();
	p2 = v2.begin();
	e2 = v2.end();
	
	while (p1 != e1 || p2 != e2) {
		if (p1 != e1 && (p2 == e2 || p1->first < p2->first)) {
			j = *p1++;
		} else {
			j = *p2++;
		}
		
		if (i.first > i.last) {
			// first time
			i = j;
		} else if (i.last >= j.first - 1) {
			i.last = max(i.last, j.last);
		} else {
			r.push_back(i);
			i = j;
		}
	}
	r.push_back(i);
}

void intersect_interval_vecs(const interval_vec &v1, const interval_vec &v2, interval_vec &r) {
	interval_vec::const_iterator p1, p2, e1, e2;
	interval_set::interval i, j;
	
	r.reserve(max(v1.size(), v2.size()));
	p1 = v1.begin();
	e1 = v1.end();
	p2 = v2.begin();
	e2 = v2.end();
	
	while (p1 != e1 || p2 != e2) {
		if (p1 != e1 && (p2 == e2 || p1->first < p2->first)) {
			j = *p1++;
		} else {
			j = *p2++;
		}
		
		if (i.first > i.last) {
			// first time
			i = j;
		} else if (i.last < j.first) {
			// drop i
			i = j;
		} else if (i.last >= j.first && i.last <= j.last) {
			i.first = j.first;
			r.push_back(i);
			i = j;
		} else {
			r.push_back(j);
		}
	}
}

void complement_interval_vec(const interval_vec &v, interval_vec &c) {
	interval_vec::const_iterator p, end;
	interval_set::interval i;
	
	c.clear();
	c.reserve(v.size());
	i.first = numeric_limits<int>::min();
	
	for (p = v.begin(), end = v.end(); p != end; ++p) {
		i.last = p->first - 1;
		c.push_back(i);
		i.first = p->last + 1;
	}
	i.last = numeric_limits<int>::max();
	c.push_back(i);
}

void subtract_interval_vecs(const interval_vec &v1, const interval_vec &v2, interval_vec &r) {
	interval_vec c;
	complement_interval_vec(v2, c);
	intersect_interval_vecs(v1, c, r);
}

void interval_set::interval::serialize(ostream &os) const {
	serializer(os) << first << last;
}

void interval_set::interval::unserialize(std::istream &is) {
	unserializer(is) >> first >> last;
}

interval_set::interval_set() :sz(0) {
	curr = new vector<interval>;
	work = new vector<interval>;
}

interval_set::interval_set(const interval_set &s) {
	curr = new vector<interval>(*s.curr);
	sz = s.sz;
	work = new vector<interval>;
}

interval_set::interval_set(const set<int> &s) {
	curr = new vector<interval>;
	work = new vector<interval>;
	*this = s;
}

interval_set::~interval_set() {
	delete curr;
	delete work;
}

bool interval_first_compare(const interval_set::interval &i1, const interval_set::interval &i2) {
	return i1.first < i2.first;
}

inline vector<interval_set::interval>::iterator upper_bound(vector<interval_set::interval> &v, int x) {
	interval_set::interval i;
	i.first = x;
	return upper_bound(v.begin(), v.end(), i, interval_first_compare);
}

inline vector<interval_set::interval>::const_iterator upper_bound(const vector<interval_set::interval> &v, int x) {
	interval_set::interval i;
	i.first = x;
	return upper_bound(v.begin(), v.end(), i, interval_first_compare);
}

void interval_set::update_size() {
	vector<interval>::const_iterator i, end;
	sz = 0;
	for (i = curr->begin(), end = curr->end(); i != end; ++i) {
		sz += (i->last - i->first) + 1;
	}
}

bool interval_set::contains(int x) const {
	vector<interval>::const_iterator p = upper_bound(*curr, x);
	
	if (p != curr->begin() && (p - 1)->last >= x)
		return true;
	
	return false;
}

bool interval_set::insert(int x) {
	vector<interval>::iterator p, prev, b, e;
	
	b = curr->begin();
	e = curr->end();
	if (!curr->empty() && curr->back().first <= x) {
		// this is the most common case
		p = e;
	} else {
		p = upper_bound(*curr, x);
	}
	
	if (p != b && (p - 1)->last >= x)
		return false;
	
	if (p != b && (prev = p - 1)->last == x - 1) {
		prev->last++;
		if (p != e && prev->last == p->first - 1) {
			prev->last = p->last;
			curr->erase(p);
		}
	} else if (p != e && p->first == x + 1) {
		p->first--;
		if (p != b && (prev = p - 1)->last == p->first - 1) {
			prev->last = p->last;
			curr->erase(p);
		}
	} else {
		interval i;
		i.first = x;
		i.last = x;
		curr->insert(p, i);
	}
	
	sz++;
	return true;
}

bool interval_set::erase(int x) {
	vector<interval>::iterator p, prev;
	
	p = upper_bound(*curr, x);
	if (p == curr->begin() || (p - 1)->last < x)
		return false;
	
	prev = p - 1;
	if (prev->last == x) {
		prev->last--;
		if (prev->first > prev->last)
			curr->erase(prev);
	} else if (prev->first == x) {
		prev->first++;
		if (prev->first > prev->last)
			curr->erase(prev);
	} else {
		// split prev into 2
		interval i;
		i.first = x + 1;
		i.last = prev->last;
		prev->last = x - 1;
		curr->insert(p, i);
	}
	
	sz--;
	return true;
}

void interval_set::unify(const interval_set &s) {
	if (s.empty())
		return;
	
	if (empty()) {
		*curr = *s.curr;
		sz = s.sz;
		return;
	}
	
	work->clear();
	unify_interval_vecs(*curr, *s.curr, *work);
	swap(curr, work);
	update_size();
}

void interval_set::unify(const interval_set &s, interval_set &r) const {
	if (empty()) {
		r = s;
		return;
	}
	if (s.empty()) {
		r = *this;
		return;
	}
	
	r.curr->clear();
	unify_interval_vecs(*work, *s.work, *r.curr);
	r.update_size();
}

void interval_set::intersect(const interval_set &s) {
	if (empty() || s.empty()) {
		clear();
		return;
	}
	
	work->clear();
	intersect_interval_vecs(*curr, *s.curr, *work);
	swap(curr, work);
	update_size();
}

void interval_set::intersect(const interval_set &s, interval_set &r) const {
	if (empty() || s.empty()) {
		r.clear();
		return;
	}
	
	r.curr->clear();
	intersect_interval_vecs(*curr, *s.curr, *r.curr);
	r.update_size();
}

void interval_set::subtract(const interval_set &s) {
	if (empty())
		return;
	
	work->clear();
	subtract_interval_vecs(*curr, *s.curr, *work);
	swap(curr, work);
	update_size();
}

void interval_set::subtract(const interval_set &s, interval_set &r) const {
	if (empty()) {
		r.clear();
		return;
	}
	if (s.empty()) {
		r = *this;
		return;
	}
	
	r.curr->clear();
	subtract_interval_vecs(*curr, *s.curr, *r.curr);
	r.update_size();
}

void interval_set::serialize(ostream &os) const {
	serializer(os) << *curr << sz;
}

void interval_set::unserialize(istream &is) {
	unserializer(is) >> *curr >> sz;
}

interval_set &interval_set::operator=(const interval_set &v) {
	*curr = *v.curr;
	sz = v.sz;
	return *this;
}

interval_set &interval_set::operator=(const set<int> &s) {
	interval in;
	set<int>::const_iterator i, iend;
	
	curr->clear();
	if (!s.empty()) {
		i = s.begin();
		in.first = *i;
		in.last = *i;
		for (++i, iend = s.end(); i != iend; ++i) {
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
	sz = s.size();
	return *this;
}

bool interval_set::check_size() const {
	vector<interval>::const_iterator i, iend;
	int s = 0;
	for (i = curr->begin(), iend = curr->end(); i != iend; ++i) {
		s += i->last - i->first + 1;
	}
	return s == sz;
}

ostream &operator<<(ostream &os, const interval_set &s) {
	interval_vec::const_iterator i, end;
	string sep = "";
	for (i = s.curr->begin(), end = s.curr->end(); i != end; ++i) {
		if (i->first == i->last) {
			os << sep << i->first;
		} else {
			os << sep << i->first << "-" << i->last;
		}
		sep = ", ";
	}
	os << " (" << s.size() << ")";
	return os;
}

relation::relation()
: sz(0), arty(0)
{}

relation::relation(int n) 
: sz(0), arty(n)
{}

relation::relation(const relation &r)
: sz(r.sz), arty(r.arty), tuples(r.tuples)
{}
	
relation::relation(int n, const vector<tuple> &ts)
: sz(ts.size()), arty(n)
{
	assert(arty > 0);
	vector<tuple>::const_iterator i;
	for (i = ts.begin(); i != ts.end(); ++i) {
		assert(i->size() == arty);
		tuple tail(i->begin() + 1, i->end());
		tuples[tail].insert(i->front());
	}
}
	
bool relation::contains(const tuple &t) const {
	assert(t.size() == arty);
	tuple tail(t.begin() + 1, t.end());
	tuple_map::const_iterator i = tuples.find(tail);
	if (i == tuples.end()) {
		return false;
	}
	return i->second.contains(t[0]);
}

void relation::slice(const tuple &inds, relation &out) const {
	assert(0 < inds.size() && inds.size() <= arty && inds[0] == 0 && out.arity() == inds.size());
	tuple tinds;
	for (int i = 1; i < inds.size(); ++i) {
		tinds.push_back(inds[i] - 1);
	}
	
	tuple t;
	tuple_map::const_iterator i;
	for (i = tuples.begin(); i != tuples.end(); ++i) {
		slice_tuple(i->first, tinds, t);
		out.tuples[t].unify(i->second);
	}
	out.update_size();
}

void relation::slice(int n, relation &out) const {
	if (n <= 0) {
		return;
	} else if (n > arty) {
		n = arty;
	}
	tuple_map::const_iterator i;
	for (i = tuples.begin(); i != tuples.end(); ++i) {
		tuple t(i->first);
		t.resize(n - 1);
		out.tuples[t].unify(i->second);
	}
	out.update_size();
}

bool relation::operator==(const relation &r) const {
	return arty == r.arty && sz == r.sz && tuples == r.tuples;
}

relation &relation::operator=(const relation &r) {
	sz = r.sz;
	arty = r.arty;
	tuples = r.tuples;
	return *this;
}

/*
 Remove all tuples in this relation that does not match any tuple in r
 along indexes inds.
*/
void relation::intersect(const tuple &inds, const relation &r) {
	assert(!inds.empty() && inds.front() == 0);
	tuple s;
	tuple tinds;
	for (int i = 1; i < inds.size(); ++i) {
		tinds.push_back(inds[i] - 1);
	}

	tuple_map::iterator i = tuples.begin();
	tuple_map::const_iterator j;
	while (i != tuples.end()) {
		slice_tuple(i->first, tinds, s);
		j = r.tuples.find(s);
		if (j == r.tuples.end()) {
			tuples.erase(i++);
		} else {
			i->second.intersect(j->second);
			++i;
		}
	}
	update_size();
}

/*
 In-place subtraction
*/
void relation::subtract(const relation &r) {
	assert(arty == r.arty);
	
	tuple_map::iterator i = tuples.begin();
	tuple_map::const_iterator j = r.tuples.begin();
	while (i != tuples.end() && j != r.tuples.end()) {
		if (i->first < j->first) {
			++i;
		} else if (j->first < i->first) {
			++j;
		} else {
			i->second.subtract(j->second);
			++i;
			++j;
		}
	}
	update_size();
}

void relation::subtract(const relation &r, relation &out) const {
	assert(arty == r.arty && arty == out.arty);
	tuple_map::const_iterator i, j;
	for (i = tuples.begin(); i != tuples.end(); ++i) {
		j = r.tuples.find(i->first);
		if (j != r.tuples.end()) {
			i->second.subtract(j->second, out.tuples[i->first]);
		} else {
			out.tuples[i->first] = i->second;
		}
	}
	out.update_size();
}

/*
 Remove all tuples in this relation that matches some tuple in r along indexes
 inds.
*/
void relation::subtract(const tuple &inds, const relation &r) {
	assert(!inds.empty() && inds.front() == 0);
	tuple s;
	tuple tinds;
	for (int i = 1; i < inds.size(); ++i) {
		tinds.push_back(inds[i] - 1);
	}

	tuple_map::iterator i;
	tuple_map::const_iterator j;
	for (i = tuples.begin(); i != tuples.end(); ++i) {
		slice_tuple(i->first, tinds, s);
		j = r.tuples.find(s);
		if (j != r.tuples.end()) {
			i->second.subtract(j->second);
		}
	}
	update_size();
}

/*
 For each tuple t1 in this relation, find all tuples t2 in r such that
 t1[match1] == t2[match2], and extend t1 with t2[extend]. Upon
 completion, this relation will contain all such t1's.
*/
void relation::expand(const relation  &r,
                      const tuple &match1,
                      const tuple &match2,
                      const tuple &extend)
{
	assert(!match1.empty() && match1.front() == 0 && !match2.empty() && match2.front() == 0);
	tuple m1, m2, ex;

	for (int i = 1; i < match1.size(); ++i) {
		m1.push_back(match1[i] - 1);
	}
	for (int i = 1; i < match2.size(); ++i) {
		m2.push_back(match2[i] - 1);
	}
	for (int i = 0; i < extend.size(); ++i) {
		ex.push_back(extend[i] - 1);
	}

	tuple_map old_tuples = tuples;
	tuples.clear();
	
	// preprocess r to avoid redundant slicing
	vector<sliced_relation_tuple> sliced(r.tuples.size());
	tuple_map::const_iterator i;
	int j = 0;
	for (i = r.tuples.begin(); i != r.tuples.end(); ++i) {
		slice_tuple(i->first, m2, sliced[j].match);
		slice_tuple(i->first, ex, sliced[j].extend);
		sliced[j++].lead = &i->second;
	}
	
	sz = 0;
	tuple t1, t2;
	for (i = old_tuples.begin(); i != old_tuples.end(); ++i) {
		slice_tuple(i->first, m1, t1);
		for (int j = 0; j < sliced.size(); ++j) {
			if (t1 == sliced[j].match) {
				t2 = concat_tuples(i->first, sliced[j].extend);
				i->second.intersect(*sliced[j].lead, tuples[t2]);
			}
		}
	}
	
	arty += extend.size();
	update_size();
}

void relation::count_expansion(const relation  &r,
                               const tuple &match1,
                               const tuple &match2,
							   int &matched,
							   int &new_size) const
{
	assert(!match1.empty() && match1.front() == 0 && !match2.empty() && match2.front() == 0);
	tuple m1, m2;
	tuple_map::const_iterator i;
	tuple t1, t2;

	for (int i = 1; i < match1.size(); ++i) {
		m1.push_back(match1[i] - 1);
	}
	for (int i = 1; i < match2.size(); ++i) {
		m2.push_back(match2[i] - 1);
	}
	// preprocess r to avoid redundant slicing
	vector<sliced_relation_tuple> sliced(r.tuples.size());
	int j = 0;
	for (i = r.tuples.begin(); i != r.tuples.end(); ++i) {
		slice_tuple(i->first, m2, sliced[j].match);
		sliced[j++].lead = &i->second;
	}

	matched = 0;
	new_size = 0;
	
	interval_set matched_insts, inter;
	for (i = tuples.begin(); i != tuples.end(); ++i) {
		matched_insts.clear();
		slice_tuple(i->first, m1, t1);
		for (int j = 0; j < sliced.size(); ++j) {
			if (t1 == sliced[j].match) {
				inter.clear();
				i->second.intersect(*sliced[j].lead, inter);
				matched_insts.unify(inter);
				new_size += inter.size();
			}
		}
		matched += matched_insts.size();
	}
}

void relation::add(const tuple &t) {
	assert(t.size() == arty);
	tuple tail(arty - 1);
	copy(t.begin() + 1, t.end(), tail.begin());
	add(t[0], tail);
}

void relation::add(int i, const tuple &t) {
	assert(t.size() + 1 == arty);
	if (tuples[t].insert(i)) {
		++sz;
	}
}

void relation::add(int i, int n) {
	assert(arty == 2);
	tuple t(1, n);
	add(i, t);
}

void relation::del(const tuple &t) {
	assert(t.size() == arty);
	tuple tail(arty - 1);
	copy(t.begin() + 1, t.end(), tail.begin());
	del(t[0], tail);
}

void relation::del(int i, const tuple &t) {
	assert(t.size() + 1 == arty);
	tuple_map::iterator j = tuples.find(t);
	if (j != tuples.end()) {
		if (j->second.erase(i)) {
			--sz;
		}
	}
}

void relation::del(int i, int n) {
	assert(arty == 2);
	tuple t(1, n);
	del(i, t);
}

void relation::at_pos(int n, interval_set &elems) const {
	assert(0 <= n && n < arty);
	tuple_map::const_iterator i;
	if (n == 0) {
		for (i = tuples.begin(); i != tuples.end(); ++i) {
			elems.unify(i->second);
		}
	} else {
		for (i = tuples.begin(); i != tuples.end(); ++i) {
			elems.insert(i->first[n - 1]);
		}
	}
}

void relation::drop_first(set<tuple> &out) const {
	tuple_map::const_iterator i;
	for (i = tuples.begin(); i != tuples.end(); ++i) {
		out.insert(i->first);
	}
}

void relation::clear() {
	sz = 0;
	tuples.clear();
}

void relation::reset(int new_arity) {
	tuples.clear();
	sz = 0;
	arty = new_arity;
}

/*
 Remove any tuples that don't match pattern. pat is a vector of tuples, with
 tuple i enumerating the possible values for argument i. An empty tuple is
 considered a wildcard. If pat is shorter than the relation's arity, the
 difference are considered wildcards.

*/
void relation::filter(const vector<tuple> &pat, bool negate) {
	assert(pat.size() <= arty);
	if (pat.empty()) {
		return;
	}

	tuple_map::iterator i;
	for (i = tuples.begin(); i != tuples.end(); ++i) {
		bool matched = true;
		for (int j = 1; j < pat.size(); ++j) {
			if (!pat[j].empty() && !has(pat[j], i->first[j - 1])) {
				matched = false;
				break;
			}
		}
		interval_set &s = i->second;
		if (!negate) {
			if (!matched) {
				s.clear();
			} else if (!pat[0].empty()) {
				interval_set p0;
				for (int j = 0; j < pat[0].size(); ++j)
					p0.insert(pat[0][j]);
				s.intersect(p0);
			}
		} else if (matched) {
			if (!pat[0].empty()) {
				interval_set p0;
				for (int j = 0; j < pat[0].size(); ++j)
					p0.insert(pat[0][j]);
				s.subtract(p0);
			} else {
				s.clear();
			}
		}
	}
	update_size();
}

void relation::update_size() {
	sz = 0;
	tuple_map::iterator i = tuples.begin();
	while (i != tuples.end()) {
		if (i->second.empty()) {
			tuples.erase(i++);
		} else {
			sz += i->second.size();
			++i;
		}
	}
}

void relation::random_split(int k, relation *r1, relation *r2) const {
	vector<bool> choose(sz, false);
	for (int i = 0; i < k; ++i) {
		choose[i] = true;
	}
	random_shuffle(choose.begin(), choose.end());
	int i = 0;
	tuple_map::const_iterator j;
	for (j = tuples.begin(); j != tuples.end(); ++j) {
		interval_set::const_iterator j2, j2end;
		for (j2 = j->second.begin(), j2end = j->second.end(); j2 != j2end; ++j2) {
			if (choose[i++]) {
				if (r1) { r1->add(*j2, j->first); }
			} else {
				if (r2) { r2->add(*j2, j->first); }
			}
		}
	}
}

void relation::serialize(std::ostream &os) const {
	serializer(os) << arty << sz << tuples;
}

void relation::unserialize(std::istream &is) {
	unserializer(is) >> arty >> sz >> tuples;
	int s = sz;
	update_size();
	assert (s == sz);
}

void relation::dump_foil6(ostream &os, bool terminate) const {
	const_iterator i, e;
	for (i = begin(), e = end(); i != e; ++i) {
		join(os, *i, ",");
		os << endl;
	}
	if (terminate)
		os << "." << endl;
}

bool relation::load_foil6(istream &is) {
	string line;
	vector<string> fields;
	tuple t(arty);
	
	tuples.clear();
	while (getline(is, line)) {
		if (line == "." || line == ";") {
			update_size();
			return true;
		}
		fields.clear();
		split(line, ",", fields);
		if (fields.size() != arty) {
			return false;
		}
		for (int i = 0; i < arty; ++i) {
			if (!parse_int(fields[i], t[i])) {
				return false;
			}
		}
		add(t);
		assert(check_size());
	}
	return false;
}

bool relation::check_size() const {
	tuple_map::const_iterator i, iend;
	int s = 0;
	for (i = tuples.begin(), iend = tuples.end(); i != iend; ++i) {
		if (!i->second.check_size())
			return false;
		s += i->second.size();
	}
	return sz == s;
}

void relation::gdb_print() const {
	relation::tuple_map::const_iterator i, end;
	for (i = tuples.begin(), end = tuples.end(); i != end; ++i) {
		join(cout, i->first, " ") << " -> " << i->second << endl;
	}
}

ostream &operator<<(ostream &os, const relation &r) {
	relation::const_iterator i, end;
	for (i = r.begin(), end = r.end(); i != end; ++i) {
		join(os, *i, " ");
		os << endl;
	}
	return os;
}

ostream &operator<<(ostream &os, const relation_table &t) {
	relation_table::const_iterator i;
	for (i = t.begin(); i != t.end(); ++i) {
		os << i->first << endl << i->second;
	}
	return os;
}

relation::iter::iter(const relation &r, bool begin)
: end(r.tuples.end()), t(r.arity())
{
	if (begin) {
		i = r.tuples.begin();
		if (i != end) {
			copy(i->first.begin(), i->first.end(), t.begin() + 1);
			j = i->second.begin();
			jend = i->second.end();
			t[0] = *j;
		}
	} else {
		i = end;
	}
}

relation::iter &relation::iter::operator++() {
	if (i != end) {
		if (++j == jend) {
			if (++i != end) {
				j = i->second.begin();
				jend = i->second.end();
				copy(i->first.begin(), i->first.end(), t.begin() + 1);
				t[0] = *j;
			}
		} else {
			t[0] = *j;
		}
	}
	return *this;
}

relation::iter relation::iter::operator++(int) {
	iter c(*this);
	++(*this);
	return c;
}

relation::iter &relation::iter::operator=(const relation::iter &rhs) {
	i = rhs.i;
	end = rhs.end;
	j = rhs.j;
	jend = rhs.jend;
	t = rhs.t;
	return *this;
}

interval_set::const_iterator &interval_set::const_iterator::operator++() {
	int iend = s->curr->size();
	if (i >= 0 && i < iend) {
		if (++j > (*s->curr)[i].last) {
			if (++i < iend) {
				j = (*s->curr)[i].first;
			} else {
				i = -1;
				j = -1;
			}
		}
	} else {
		i = -1;
		j = -1;
	}
	return *this;
}

interval_set::const_iterator interval_set::const_iterator::operator++(int) {
	const_iterator c(*this);
	++(*this);
	return c;
}

interval_set::const_iterator &interval_set::const_iterator::operator=(const interval_set::const_iterator &rhs) {
	i = rhs.i;
	j = rhs.j;
	s = rhs.s;
	return *this;
}

interval_set::const_iterator::const_iterator(const interval_set &is, bool begin)
: i(-1), j(-1), s(&is)
{
	if (begin && !s->curr->empty()) {
		i = 0;
		j = (*s->curr)[0].first;
	}
}
