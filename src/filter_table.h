#ifndef FILTER_TABLE_H
#define FILTER_TABLE_H

#include <iostream>
#include <iterator>
#include <cassert>
#include <vector>
#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include "common.h"
#include "timer.h"
#include "soar_interface.h"
#include "relation.h"

class filter;
class filter_input;
class scene;
class sgnode;

struct filter_table_entry {
	std::string name;
	std::vector<std::string> parameters;
	bool ordered, allow_repeat;
	
	filter* (*create)(Symbol*, soar_interface*, scene*, filter_input*);
	bool    (*calc)(const scene*, const std::vector<const sgnode*> &);
	
	filter_table_entry() {
		create = NULL;
		calc = NULL;
		ordered = false;
		allow_repeat = false;
	}
};

class filter_table {
public:
	friend const filter_table &get_filter_table();

	void get_predicates(std::vector<std::string> &preds) const {
		std::map<std::string, filter_table_entry>::const_iterator i;
		for (i = t.begin(); i != t.end(); ++i) {
			preds.push_back(i->first);
		}
	}
	
	bool get_params(const std::string &pred, std::vector<std::string> &p) const {
		std::map<std::string, filter_table_entry>::const_iterator i = t.find(pred);
		if (i == t.end()) {
			return false;
		}
		p = i->second.parameters;
		return true;
	}
	
	filter* make_filter(const std::string &pred, Symbol *root, soar_interface *si, scene *scn, filter_input *input) const;
	
	/*
	 Returns a list of lists of atoms. Each atom is described by a
	 list of strings. The first string is the name of the predicate,
	 and the rest are the node names used as arguments.
	*/
	void get_all_atoms(scene *scn, std::vector<std::string> &atoms) const;
	
	void update_relations(const scene *scn, const std::vector<int> &dirty, int time, relation_table &rt) const;
	
	const timer_set &get_timers() const {
		return timers;
	}
	
private:
	filter_table();
	
	void add(const filter_table_entry &e) {
		assert(t.find(e.name) == t.end());
		t[e.name] = e;
	}
	
	std::map<std::string, filter_table_entry> t;
	
	mutable timer_set timers;
};

/* Get the singleton instance */
const filter_table &get_filter_table();

#endif
