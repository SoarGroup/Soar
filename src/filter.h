#ifndef FILTER_H
#define FILTER_H

#include <iostream>
#include <string>
#include <list>
#include <map>
#include <sstream>
#include <iterator>

#include "linalg.h"
#include "sgnode.h"
#include "scene.h"
#include "common.h"

/*
 Wrapper for all filter value types so we can cache them uniformly.
*/
class filter_val {
public:
	virtual std::string get_string() = 0;
};

template <class T>
class filter_val_c : public filter_val {  // c for concrete
public:
	filter_val_c(const T &v) : v(v) {}
	
	std::string get_string() {
		std::stringstream ss;
		ss << v;
		return ss.str();
	}
	
	T get_value() { return v; }
	void set_value(const T &n) { v = n; }
private:
	T v;
};

template <>
class filter_val_c <sgnode*> : public filter_val {
public:
	filter_val_c(sgnode *v) : v(v) {}
	std::string get_string() { return v->get_name(); }
	sgnode *get_value() { return v; }
	void set_value(sgnode *n) { v = n; }
private:
	sgnode *v;
};

/*
 Convenience functions for getting filter results as specific values
 with error checking
 */
template <class T>
inline bool get_filter_val (filter_val *fv, T &v) {
	filter_val_c<T> *cast;
	
	if (!(cast = dynamic_cast<filter_val_c<T>*>(fv))) {
		return false;
	}
	v = cast->get_value();
	return true;
}

/*
 Specialization for floats to allow getting int values as well
*/
template <>
inline bool get_filter_val<float>(filter_val *fv, float &v) {
	filter_val_c<float> *ffv;
	filter_val_c<int> *ifv;
	
	if (!(ffv = dynamic_cast<filter_val_c<float>*>(fv))) {
		if (!(ifv = dynamic_cast<filter_val_c<int>*>(fv))) {
			return false;
		} else {
			v = ifv->get_value();
		}
	} else {
		v = ffv->get_value();
	}
	return true;
}

template <class T>
inline bool set_filter_val (filter_val *fv, const T &v) {
	filter_val_c<T> *cast;
	
	if (!(cast = dynamic_cast<filter_val_c<T>*>(fv))) {
		return false;
	}
	cast->set_value(v);
	return true;
}

/*
 A list that keeps track of changes made to it, so that users can respond
 to only the things that changed. Both the filter result list and filter
 input lists derive from this class. This class also assumes that it owns
 the memory of any element added to it, so it will free that memory when
 items are removed from the list.
*/
template<class T>
class change_tracking_list {
public:
	typedef typename std::list<T*>::const_iterator iter;
	
	change_tracking_list() {
		m_added_begin = current.begin();
	}
	
	~change_tracking_list() {
		iter i;
		for (i = current.begin(); i != current.end(); ++i) {
			delete *i;
		}
		clear_removed();
	}
	
	void add(T* v) {
		current.push_back(v);
		if (m_added_begin == current.end()) {
			m_added_begin--;
		}
	}
	
	void remove(T* v) {
		typename std::list<T*>::iterator i;
		if ((i = find(current.begin(), current.end(), v)) != current.end()) {
			if (i == m_added_begin) {
				m_added_begin++;
			}
			current.erase(i);
		}
		changed.remove(v);
		removed.push_back(v);
	}
	
	void change(T *v) {
		changed.push_back(v);
	}
	
	void clear_changes() {
		m_added_begin = current.end();
		changed.clear();
		clear_removed();
	}
	
	/*
	 This is kind of like the opposite of clear_changes, in that it
	 makes everything a new addition.
	*/
	void reset() {
		clear_removed();
		changed.clear();
		m_added_begin = current.begin();
	}
	
	void clear() {
		current.clear();
		clear_removed();
		changed.clear();
		m_added_begin = current.begin();
	}
		
	int size() const {
		return current.size();
	}
	
	iter curr_begin() const {
		return current.begin();
	}
	
	iter curr_end() const {
		return current.end();
	}
	
	iter added_begin() const {
		return m_added_begin;
	}
	
	iter added_end() const {
		return current.end();
	}
	
	iter removed_begin() const {
		return removed.begin();
	}
	
	iter removed_end() const {
		return removed.end();
	}
	
	iter changed_begin() const {
		return changed.begin();
	}
	
	iter changed_end() const {
		return changed.end();
	}
	
private:
	void clear_removed() {
		for (iter i = removed.begin(); i != removed.end(); ++i) {
			delete *i;
		}
		removed.clear();
	}
	
	std::list<T*> current;
	std::list<T*> removed;
	std::list<T*> changed;
	
	/*
	 Keeps track of the iterator pointing at the first new element
	 in the current list.
	*/
	iter m_added_begin;
};

class filter;

/*
 Every filter generates a list of filter values as a result, even if
 the list is empty or a singleton.
*/
typedef change_tracking_list<filter_val> filter_result;

/*
 A filter parameter set represents one complete input into a filter. It's
 just a list of pairs <parameter name, value>.
*/
typedef std::map<std::string, filter_val*> filter_param_set;

/*
 Each filter takes a number of input parameters. Each of those parameters
 is in the form of a result list. The derived classes of this abstract
 base class are responsible for combining those separate result lists into
 a single list of parameter sets. For example, the output parameter set
 could be the Cartesian product of all elements in each input result list.
 
 I'm assuming that this class owns the memory of the filters that are
 added to it.
*/
class filter_input : public change_tracking_list<filter_param_set> {
public:
	struct param_info {
		std::string name;
		filter *f;
		filter_result *res;
	};
	
	typedef std::vector<param_info> input_table;
	
	virtual ~filter_input();
	
	bool update();
	void add_param(std::string name, filter *f);

	virtual void combine(const input_table &inputs) = 0;

private:
	input_table input_info;
};

class null_filter_input : public filter_input {
public:
	void combine(const input_table &inputs) {}
};

/*
 Input class that just concatenates all separate result lists into
 a single parameter set list, with each parameter set being a single
 element.
*/
class concat_filter_input : public filter_input {
public:
	void combine(const input_table &inputs) {
		input_table::const_iterator i;
		for (i = inputs.begin(); i != inputs.end(); ++i) {
			filter_result::iter j;
			filter_param_set *p;
			filter_result *r = i->res;

			for (j = r->added_begin(); j != r->added_end(); ++j) {
				p = new filter_param_set();
				(*p)[i->name] = *j;
				val2params[*j] = p;
				add(p);
			}
			for (j = r->removed_begin(); j != r->removed_end(); ++j) {
				if (!map_pop(val2params, *j, p)) {
					assert(false);
				}
				remove(p);
			}
			for (j = r->changed_begin(); j != r->changed_end(); ++j) {
				p = val2params[*j];
				change(p);
			}
		}
	}

private:
	std::map<filter_val*, filter_param_set*> val2params;
};

/*
 Input class that takes the Cartesian product of all input result lists.
*/
class product_filter_input : public filter_input {
public:
	void combine(const input_table &inputs) {
		input_table::const_iterator i;
		for (i = inputs.begin(); i != inputs.end(); ++i) {
			filter_result::iter j;
			val2param_map::iterator k;
			param_set_list::iterator l;
			filter_result *r = i->res;
			
			for (j = r->removed_begin(); j != r->removed_end(); ++j) {
				k = val2params.find(*j);
				assert(k != val2params.end());
				for (l = k->second.begin(); l != k->second.end(); ++l) {
					remove(*l);
					erase_param_set(*l);
				}
			}
			for (j = r->changed_begin(); j != r->changed_end(); ++j) {
				k = val2params.find(*j);
				assert(k != val2params.end());
				for (l = k->second.begin(); l != k->second.end(); ++l) {
					change(*l);
				}
			}
		}
		gen_new_combinations(inputs);
	}
	
private:
	typedef std::list<filter_param_set*> param_set_list;
	typedef std::map<filter_val*, param_set_list > val2param_map;
	
	class product_gen {
	public:
		product_gen(const std::vector<filter_result::iter> &begin, const std::vector<filter_result::iter> &end)
		: begin(begin), curr(begin), end(end), i(0), first(true) {}
		
		bool next() {
			if (first) {
				for (int i = 0; i < begin.size(); ++i) {
					if (begin[i] == end[i]) {
						return false;
					}
				}
				first = false;
				return true;
			}
			
			while (true) {
				if (++curr[i] == end[i]) {
					if (i == curr.size() - 1) {
						return false;
					}
					curr[i] = begin[i];
				} else {
					return true;
				}
				if (++i >= curr.size()) {
					i = 0;
				}
			}
		}
		
		std::vector<filter_result::iter> curr;

	private:
		std::vector<filter_result::iter> begin;
		std::vector<filter_result::iter> end;
		int i;
		bool first;
	};
	
	/*
	 Generate all combinations of results that involve at least
	 one new result.  Do this by iterating over the result lists.
	 For the i^th result list, take the cartesian product of the
	 old results from lists 0..(i-1), the new results of list i,
	 and both old and new results from lists (i+1)..n.  This will
	 avoid generating duplicates.  I'm assuming that new results
	 are at the end of each result list.
	*/
	void gen_new_combinations(const input_table &inputs) {
		std::vector<filter_result::iter> begin, added_begin, end;
		std::vector<std::string> names;
		int i, j, k;
		input_table::const_iterator ti;
		for (ti = inputs.begin(); ti != inputs.end(); ++ti) {
			names.push_back(ti->name);
			begin.push_back(ti->res->curr_begin());
			end.push_back(ti->res->curr_end());
			added_begin.push_back(ti->res->added_begin());
		}
		for (i = 0; i < begin.size(); ++i) {
			std::vector<filter_result::iter> tbegin, tend;
			for (j = 0; j < begin.size(); ++j) {
				if (j < i) {
					tbegin.push_back(begin[j]);
					tend.push_back(added_begin[j]); // same as end of old results
				} else if (j == i) {
					tbegin.push_back(added_begin[j]);
					tend.push_back(end[j]);
				} else {
					tbegin.push_back(begin[j]);
					tend.push_back(end[j]);
				}
			}
			product_gen gen(tbegin, tend);
			while (gen.next()) {
				std::vector<filter_result::iter>::const_iterator ci;
				std::vector<std::string>::const_iterator ni;
				filter_param_set *p = new filter_param_set();
				for (ci = gen.curr.begin(), ni = names.begin(); ci != gen.curr.end(); ++ci, ++ni) {
					(*p)[*ni] = **ci;
					val2params[**ci].push_back(p);
				}
				add(p);
			}
		}
	}
	
	void erase_param_set(filter_param_set *s) {
		filter_param_set::const_iterator i;
		for (i = s->begin(); i != s->end(); ++i) {
			param_set_list &l = val2params[i->second];
			l.erase(std::find(l.begin(), l.end(), s));
		}
	}
	
	val2param_map val2params;
};

/*
 The filter is the basic query unit in SVS. Each filter takes a list of
 parameter sets generated by the filter_input class and produces a single
 result list. Soar can "mix-and-match" filters by plugging their outputs
 into inputs of other filters. This is done by specifying the desired
 filter plumbing on the SVS command link.
 
 Filter results are updated once every output phase. Updating a filter's
 result is recursive: the filter will first request an update on its
 input, which in turn requests updates on all filters feeding into the
 input. Filters should also try to cache results when possible to avoid
 unnecessary computation.
*/
class filter {
public:
	filter() {
		input = new null_filter_input();
	}
	
	filter(filter_input *in) : input(in) {
		if (input == NULL) {
			input = new null_filter_input();
		}
	}
	
	virtual ~filter() {
		delete input;
	}
	
	std::string get_error() {
		return errmsg;
	}
	
	bool is_error() {
		return !errmsg.empty();
	}
	
	void set_error(std::string msg) {
		errmsg = msg;
		result.clear();
	}
	
	void clear_error() {
		errmsg.clear();
	}
	
	void add_result(filter_val *v, filter_param_set *p) {
		result.add(v);
		result2params[v] = p;
	}
	
	bool get_result_params(filter_val *v, filter_param_set *&p) {
		return map_get(result2params, v, p);
	}
	
	void remove_result(filter_val *v) {
		result.remove(v);
		result2params.erase(v);
	}
	
	void change_result(filter_val *v) {
		result.change(v);
	}
	
	filter_result *get_result() {
		return &result;
	}
	
	bool update() {
		if (!input->update()) {
			set_error("Errors in input");
			result.clear();
			input->reset();
			return false;
		}
		
		if (!update_results()) {
			result.clear();
			input->reset();
			return false;
		}
		input->clear_changes();
		return true;
	}
	
	filter_input::iter added_input_begin() const {
		return input->added_begin();
	}
	
	filter_input::iter added_input_end() const {
		return input->added_end();
	}
	
	filter_input::iter removed_input_begin() const {
		return input->removed_begin();
	}
	
	filter_input::iter removed_input_end() const {
		return input->removed_end();
	}
	
	filter_input::iter changed_input_begin() const {
		return input->changed_begin();
	}
	
	filter_input::iter changed_input_end() const {
		return input->changed_end();
	}
	
private:
	virtual bool update_results() = 0;
	
	filter_input *input;
	filter_result result;
	std::string errmsg;
	std::map<filter_val*, filter_param_set*> result2params;
};

inline filter_input::~filter_input() {
	input_table::iterator i;
	for (i = input_info.begin(); i != input_info.end(); ++i) {
		delete i->f;
	}
}

inline bool filter_input::update() {
	input_table::iterator i;
	for (i = input_info.begin(); i != input_info.end(); ++i) {
		if (!i->f->update()) {
			return false;
		}
	}

	combine(input_info);

	for (i = input_info.begin(); i != input_info.end(); ++i) {
		i->res->clear_changes();
	}
	
	return true;
}

inline void filter_input::add_param(std::string name, filter *f) {
	param_info i;
	i.name = name;
	i.f = f;
	i.res = f->get_result();
	input_info.push_back(i);
}

/*
 This type of filter assumes a one-to-one mapping of results to input
 parameter sets. It's also assumed that each result is only dependent
 on one parameter set. This is in contrast to filters that perform some
 kind of quantification over its inputs; returning the closest object,
 for example.
*/
template <class T>
class map_filter : public filter {
public:
	map_filter(filter_input *input) : filter(input) {}
	virtual ~map_filter() {}
	
	/*
	 Compute the result from parameters. adding is true if this is
	 a new parameter set. If not, res will be the old result when
	 the function is called. In either case, res should be set to
	 the new result when the function terminates. An error is signaled
	 by returning false.
	*/
	virtual bool compute(filter_param_set *params, T &res, bool adding) = 0;
	
	/*
	 Some derived classes might allocate memory associated with each
	 result. They should override this function so they know when
	 to deallocate that memory.
	*/
	virtual void result_removed(const T &res) { }
	
	/*
	 Sometimes the function that maps from parameter set to result
	 is conditioned on things other than the parameter set, such as
	 the state of the scene graph. A derived class that implements
	 such a function should explicitly mark a result as needing to be
	 recomputed even when its associated parameter set doesn't change.
	*/
	void mark_stale(filter_param_set *s) {
		stale.push_back(s);
	}

	bool update_results() {
		filter_input::iter i;
		std::vector<filter_param_set*>::iterator j;
		
		for (i = added_input_begin(); i != added_input_end(); ++i) {
			T val;
			if (!compute(*i, val, true)) {
				return false;
			}
			filter_val_c<T> *fv = new filter_val_c<T>(val);
			add_result(fv, *i);
			io_map[*i] = fv;
		}
		for (i = removed_input_begin(); i != removed_input_end(); ++i) {
			filter_val_c<T>* r = io_map[*i];
			result_removed(r->get_value());
			remove_result(r);
			io_map.erase(*i);
		}
		for (i = changed_input_begin(); i != changed_input_end(); ++i) {
			if (!update_one(*i)) {
				return false;
			}
		}
		for (j = stale.begin(); j != stale.end(); ++j) {
			if (!update_one(*j)) {
				return false;
			}
		}
		stale.clear();
		return true;
	}
	
	void reset() {}

private:
	bool update_one(filter_param_set *params) {
		T val;
		filter_val_c<T> *fv = io_map[params];
		get_filter_val<T>(fv, val);
		if (!compute(params, val, false)) {
			return false;
		}
		set_filter_val<T>(fv, val);
		change_result(fv);
		return true;
	}
	
	std::map<filter_param_set*, filter_val_c<T>*> io_map;
	std::vector<filter_param_set*> stale;
};

/*
 Filters that don't take any inputs and always output the same result
*/
template <class T>
class const_filter : public filter {
public:
	const_filter(const T &single) : added(false) {
		v.push_back(single);
	}
	
	const_filter(const std::vector<T> &v) : filter(NULL), v(v), added(false) {}
	
	bool update_results() {
		if (!added) {
			typename std::vector<T>::const_iterator i;
			for (i = v.begin(); i != v.end(); ++i) {
				add_result(new filter_val_c<T>(*i), NULL);
			}
			added = true;
		}
		return true;
	}
		
private:
	std::vector<T> v;
	bool added;
};

template <typename T>
inline bool get_filter_param(filter *f, filter_param_set *params, const std::string &name, T &val) {
	filter_val *fv;
	std::stringstream ss;
	if (!map_get(*params, name, fv)) {
		if (f) {
			ss << "parameter \"" << name << "\" missing";
			f->set_error(ss.str());
		}
		return false;
	}
	if (!get_filter_val(fv, val)) {
		if (f) {
			ss << "parameter \"" << name << "\" has wrong type";
			f->set_error(ss.str());
		}
		return false;
	}
	return true;
}

#endif
