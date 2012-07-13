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
	virtual ~filter_val() {}
	virtual std::string get_string() = 0;
	virtual filter_val *clone() const = 0;
	virtual filter_val &operator=(const filter_val &rhs) = 0;
	virtual bool operator==(const filter_val &rhs) const = 0;
};

template <typename T>
class filter_val_c : public filter_val {  // c for concrete
public:
	filter_val_c(const T &v) : v(v) {}
	virtual ~filter_val_c() {}

	std::string get_string() {
		std::stringstream ss;
		ss << v;
		return ss.str();
	}
	
	filter_val *clone() const {
		return new filter_val_c<T>(v);
	}
	
	filter_val &operator=(const filter_val &rhs) {
		const filter_val_c<T> *c = dynamic_cast<const filter_val_c<T>*>(&rhs);
		assert(c);
		v = c->v;
		return *this;
	}
	
	bool operator==(const filter_val &rhs) const {
		const filter_val_c<T> *c = dynamic_cast<const filter_val_c<T>*>(&rhs);
		if (!c) {
			return false;
		}
		return v == c->v;
	}
	
	T get_value() const {
		return v;
	}
	
	void set_value(const T &n) {
		v = n;
	}
	
private:
	T v;
};

template <>
class filter_val_c <const sgnode*> : public filter_val {
public:
	filter_val_c(const sgnode *v) : v(v) {}
	virtual ~filter_val_c() {}
	
	std::string get_string() {
		return v->get_name();
	}
	
	filter_val *clone() const {
		return new filter_val_c<const sgnode *>(v);
	}
	
	filter_val &operator=(const filter_val &rhs) {
		const filter_val_c<const sgnode *> *c = dynamic_cast<const filter_val_c<const sgnode *>*>(&rhs);
		assert(c);
		v = c->v;
		return *this;
	}
	
	bool operator==(const filter_val &rhs) const {
		const filter_val_c<const sgnode *> *c = dynamic_cast<const filter_val_c<const sgnode *>*>(&rhs);
		if (!c) {
			return false;
		}
		return v == c->v;
	}
	
	const sgnode *get_value() const {
		return v;
	}
	
	void set_value(const sgnode *n) {
		v = n;
	}
	
private:
	const sgnode *v;
};

/*
 Convenience functions for getting filter results as specific values
 with error checking
 */
template <class T>
inline bool get_filter_val (const filter_val *fv, T &v) {
	const filter_val_c<T> *cast;
	
	if (!(cast = dynamic_cast<const filter_val_c<T>*>(fv))) {
		return false;
	}
	v = cast->get_value();
	return true;
}

/*
 Specialization for floats to allow getting floats, doubles, and ints
*/
template <>
inline bool get_filter_val<float>(const filter_val *fv, float &v) {
	const filter_val_c<double> *dfv;
	const filter_val_c<float> *ffv;
	const filter_val_c<int> *ifv;
	
	if (dfv = dynamic_cast<const filter_val_c<double>*>(fv)) {
		v = dfv->get_value();
		return true;
	}
	if (ffv = dynamic_cast<const filter_val_c<float>*>(fv)) {
		v = ffv->get_value();
		return true;
	}
	
	if (ifv = dynamic_cast<const filter_val_c<int>*>(fv)) {
		v = ifv->get_value();
		return true;
	}
	
	return false;
}

template <>
inline bool get_filter_val<double>(const filter_val *fv, double &v) {
	const filter_val_c<double> *dfv;
	const filter_val_c<float> *ffv;
	const filter_val_c<int> *ifv;
	
	if (dfv = dynamic_cast<const filter_val_c<double>*>(fv)) {
		v = dfv->get_value();
		return true;
	}
	if (ffv = dynamic_cast<const filter_val_c<float>*>(fv)) {
		v = ffv->get_value();
		return true;
	}
	
	if (ifv = dynamic_cast<const filter_val_c<int>*>(fv)) {
		v = ifv->get_value();
		return true;
	}
	
	return false;
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
	change_tracking_list() : m_added_begin(0) {}
	
	~change_tracking_list() {
		for (int i = 0; i < current.size(); ++i) {
			delete current[i];
		}
		clear_removed();
	}
	
	void add(T* v) {
		current.push_back(v);
	}
	
	void remove(T* v) {
		bool found = false;
		for (int i = 0; i < current.size(); ++i) {
			if (current[i] == v) {
				for (int j = i + 1; j < current.size(); ++j) {
					current[j - 1] = current[j];
				}
				current.pop_back();
				if (i < m_added_begin) {
					--m_added_begin;
				}
				found = true;
				break;
			}
		}
		assert(found);
		for (int i = 0; i < changed.size(); ++i) {
			if (changed[i] == v) {
				for (int j = i + 1; j < changed.size(); ++j) {
					changed[j - 1] = changed[j];
				}
				changed.pop_back();
			}
		}
		removed.push_back(v);
	}
	
	void change(T *v) {
		changed.push_back(v);
	}
	
	void clear_changes() {
		m_added_begin = current.size();
		changed.clear();
		clear_removed();
	}
	
	/*
	 This is kind of like the opposite of clear_changes, in that it
	 makes everything a new addition.
	*/
	void reset() {
		changed.clear();
		clear_removed();
		m_added_begin = 0;
	}
	
	void clear() {
		current.clear();
		changed.clear();
		clear_removed();
		m_added_begin = 0;
	}
		
	int num_current() const {
		return current.size();
	}
	
	int num_changed() const {
		return changed.size();
	}
	
	int num_removed() const {
		return removed.size();
	}
	
	T* get_current(int i) {
		return current[i];
	}
	
	const T* get_current(int i) const {
		return current[i];
	}
	
	T* get_changed(int i) {
		return changed[i];
	}
	
	const T* get_changed(int i) const {
		return changed[i];
	}
	
	T* get_removed(int i) {
		return removed[i];
	}
	
	const T* get_removed(int i) const {
		return removed[i];
	}

	int first_added() const {
		return m_added_begin;
	}
	
private:
	void clear_removed() {
		for (int i = 0; i < removed.size(); ++i) {
			delete removed[i];
		}
		removed.clear();
	}
	
	std::vector<T*> current;
	std::vector<T*> removed;
	std::vector<T*> changed;
	
	// Index of the first new element in the current list
	int m_added_begin;
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
	void combine(const input_table &inputs);

private:
	std::map<filter_val*, filter_param_set*> val2params;
};

/*
 Input class that takes the Cartesian product of all input result lists.
*/
class product_filter_input : public filter_input {
public:
	void combine(const input_table &inputs);
	
private:
	void gen_new_combinations(const input_table &inputs);
	void erase_param_set(filter_param_set *s);
	
	typedef std::list<filter_param_set*> param_set_list;
	typedef std::map<filter_val*, param_set_list > val2param_map;
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
	
	void add_result(filter_val *v, const filter_param_set *p) {
		result.add(v);
		result2params[v] = p;
	}
	
	bool get_result_params(filter_val *v, const filter_param_set *&p) {
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
	
	const filter_input *get_input() const {
		return input;
	}

private:
	virtual bool update_results() = 0;
	
	filter_input *input;
	filter_result result;
	std::string errmsg;
	std::map<filter_val*, const filter_param_set*> result2params;
};

/*
 This type of filter assumes a one-to-one mapping of results to input
 parameter sets. It's also assumed that each result is only dependent
 on one parameter set. This is in contrast to filters that perform some
 kind of quantification over its inputs; returning the closest object,
 for example.
*/
class map_filter : public filter {
public:
	map_filter(filter_input *input) : filter(input) {}
	
	/*
	 All created filter_vals are owned by the result list and cleaned
	 up there, so don't do it here.
	*/
	virtual ~map_filter() {}
	
	/*
	 Compute the result from parameters. If called with a new
	 parameter set, res will be NULL, and the implementation should
	 set it to a new filter_val object (which will be owned by the
	 result list). Otherwise, res will point to a valid filter_val and
	 the implementation should change its value. If the value is
	 actually changed, the changed output argument should be set to
	 true. The implementation should return false if an error occurs.
	 */
	virtual bool compute(const filter_param_set *params, filter_val *&res, bool &changed) = 0;
	
	/*
	 Some derived classes might allocate memory associated with each
	 result. They should override this function so they know when
	 to deallocate that memory.
	*/
	virtual void result_removed(const filter_val *res) { }
	
	/*
	 Sometimes the function that maps from parameter set to result
	 is conditioned on things other than the parameter set, such as
	 the state of the scene graph. A derived class that implements
	 such a function should explicitly mark a result as needing to be
	 recomputed even when its associated parameter set doesn't change.
	*/
	void mark_stale(const filter_param_set *s) {
		stale.push_back(s);
	}

	bool update_results() {
		const filter_input* input = get_input();
		std::vector<const filter_param_set*>::iterator j;
		
		for (int i = input->first_added(); i < input->num_current(); ++i) {
			filter_val *v = NULL;
			bool changed = false;
			if (!compute(input->get_current(i), v, changed)) {
				return false;
			}
			add_result(v, input->get_current(i));
			io_map[input->get_current(i)] = v;
		}
		for (int i = 0; i < input->num_removed(); ++i) {
			io_map_t::iterator r = io_map.find(input->get_removed(i));
			assert(r != io_map.end());
			remove_result(r->second);
			result_removed(r->second);
			io_map.erase(r);
		}
		for (int i = 0; i < input->num_changed(); ++i) {
			if (!update_one(input->get_changed(i))) {
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
	bool update_one(const filter_param_set *params) {
		filter_val *v = io_map[params];
		bool changed = false;
		if (!compute(params, v, changed)) {
			return false;
		}
		if (changed) {
			change_result(v);
		}
		return true;
	}
	
	typedef std::map<const filter_param_set*, filter_val*> io_map_t;
	io_map_t io_map;
	std::vector<const filter_param_set*> stale;
};

/*
 User-defined filters should derive from this class so that they don't
 have to work with filter_val* directly. Assumes that the filter only
 returns one type of result.
*/
template <class T>
class typed_map_filter : public map_filter {
public:
	typed_map_filter(filter_input *input) : map_filter(input) {}
	virtual ~typed_map_filter() {}
	
	virtual bool compute(const filter_param_set *params, bool adding, T &res, bool &changed) = 0;
	virtual void result_removed(const T &res) { }
	
private:
	bool compute(const filter_param_set *params, filter_val *&res, bool &changed) {
		bool success;
		T val;
		if (res != NULL) {
			success = get_filter_val(res, val);
			assert(success);
		}
		success = compute(params, res == NULL, val, changed);
		if (!success) {
			return false;
		}
		if (!res) {
			res = new filter_val_c<T>(val);
		} else {
			success = set_filter_val(res, val);
			assert(success);
		}
		return true;
	}
	
	void result_removed(const filter_val *res) {
		T val;
		bool success = get_filter_val(res, val);
		assert(success);
		result_removed(val);
	}
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

/*
 Passes an arbitrary element in each input parameter set to the result
 list. This filter is intended to be used with concat_filter_input to
 implement a "combine" filter that multiplexes an arbitrary number of
 inputs into a single list.
*/
class passthru_filter : public map_filter {
public:
	passthru_filter(filter_input *input) : map_filter(input) {}
	
	bool compute(const filter_param_set *params, filter_val *&res, bool &changed) {
		if (params->empty()) {
			return false;
		}
		if (res == NULL) {
			res = params->begin()->second->clone();
			changed = true;
		} else {
			changed = (*res == *params->begin()->second);
			if (changed) {
				*res = *params->begin()->second;
			}
		}
		return true;
	}
};

template <typename T>
inline bool get_filter_param(filter *f, const filter_param_set *params, const std::string &name, T &val) {
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
