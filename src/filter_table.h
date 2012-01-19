#ifndef FILTER_TABLE_H
#define FILTER_TABLE_H

#include <iostream>
#include <iterator>
#include <cassert>
#include <vector>
#include <map>
#include <string>
#include <fstream>

class filter;
class filter_input;
class scene;

struct filter_table_entry {
	std::string name;
	std::vector<std::string> parameters;
	filter* (*create)(scene*, filter_input*);
	bool    (*calc)(scene*, const std::vector<std::string> &);
	void    (*possible_args)(scene*, std::vector<std::vector<std::string> > &);
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
	
	filter* make_filter(const std::string &pred, scene *scn, filter_input *input) const {
		std::map<std::string, filter_table_entry>::const_iterator i = t.find(pred);
		if (i == t.end() || i->second.create == NULL) {
			return NULL;
		}
		return (*(i->second.create))(scn, input);
	}
	
	bool calc(const std::string &pred, scene *scn, const std::vector<std::string> &args, bool &res) const {
		std::map<std::string, filter_table_entry>::const_iterator i = t.find(pred);
		if (i == t.end() || i->second.calc == NULL) {
			return false;
		}
		res = (*(i->second.calc))(scn, args);
		return true;
	}
	
	bool possible_args(const std::string &pred, scene *scn, std::vector<std::vector<std::string> > &args) const {
		std::map<std::string, filter_table_entry>::const_iterator i = t.find(pred);
		if (i == t.end() || i->second.possible_args == NULL) {
			return false;
		}
		(*(i->second.possible_args))(scn, args);
		return true;
	}
		
	/*
	 Returns a list of lists of atoms. Each atom is described by a
	 list of strings. The first string is the name of the predicate,
	 and the rest are the node names used as arguments.
	*/
	void get_all_atoms(scene *scn, std::vector<std::vector<std::string> > &atoms) const {
		std::ofstream out("/tmp/atom_names");
		std::map<std::string, filter_table_entry>::const_iterator i;
		for(i = t.begin(); i != t.end(); ++i) {
			const filter_table_entry &e = i->second;
			if (e.possible_args != NULL && e.calc != NULL) {
				std::vector<std::vector<std::string> > args;
				std::vector<std::vector<std::string> >::iterator j;
				(*e.possible_args)(scn, args);
				for (j = args.begin(); j != args.end(); ++j) {
					j->insert(j->begin(), e.name);
					atoms.push_back(*j);
					std::copy(j->begin(), j->end(), std::ostream_iterator<std::string>(out, " "));
					out << std::endl;
				}
			}
		}
	}
	
	/*
	 Calculate the value of every predicate with every possible set
	 of arguments from the scene.
	*/
	void calc_all_atoms(scene *scn, std::vector<bool> &results) const {
		std::map<std::string, filter_table_entry>::const_iterator i;
		for(i = t.begin(); i != t.end(); ++i) {
			const filter_table_entry &e = i->second;
			if (e.possible_args != NULL && e.calc != NULL) {
				std::vector<std::vector<std::string> > args;
				std::vector<std::vector<std::string> >::const_iterator j;
				
				(*e.possible_args)(scn, args);
				for(j = args.begin(); j != args.end(); ++j) {
					results.push_back((*e.calc)(scn, *j));
				}
			}
		}
	}
	
private:
	filter_table();
	
	void add(const filter_table_entry &e) {
		assert(t.find(e.name) == t.end());
		t[e.name] = e;
	}
	
	std::map<std::string, filter_table_entry> t;
};

/* Get the singleton instance */
const filter_table &get_filter_table();

#endif
