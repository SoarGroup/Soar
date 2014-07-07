#ifndef SOAR_INTERFACE_H
#define SOAR_INTERFACE_H

#include <utility>
#include <list>
#include <sstream>
#include "portability.h"
#include "agent.h"
#include "symtab.h"
#include "wmem.h"

typedef tc_number tc_num;

typedef std::vector<wme*> wme_list;

class soar_interface;

class common_syms {
public:
	common_syms(soar_interface *si);
	~common_syms();
	
	Symbol *svs, *cmd, *scene, *child, *result, *models, *id, *status;

private:
	soar_interface *si;
};

class soar_interface {
public:
	soar_interface(agent *a);
	~soar_interface();

	Symbol      *make_sym(const std::string &val);
	Symbol      *make_sym(int val);
	Symbol      *make_sym(double val);
	void         del_sym(Symbol *s);
	
	wme         *make_id_wme(Symbol *id, const std::string &attr);
	wme         *make_id_wme(Symbol *id, Symbol *attr);
	
	wme         *make_wme(Symbol *id, Symbol *attr, Symbol *val);
	wme         *make_wme(Symbol *id, const std::string &attr, Symbol *val);
	
	template<class T>
	wme         *make_wme(Symbol *id, const std::string &attr, const T &val);
	
	template<class T>
	wme         *make_wme(Symbol *id, Symbol *attr, const T &val);
	
	void         remove_wme(wme *w);
	bool         get_child_wmes(Symbol *id, wme_list &childs);
	bool         find_child_wme(Symbol *id, const std::string &attr, wme *&w);

	bool         is_identifier(Symbol *sym);
	bool         is_string(Symbol *sym);
	bool         is_int(Symbol *sym);
	bool         is_float(Symbol *sym);
	bool         is_state(Symbol *sym);
	bool         is_top_state(Symbol *sym);
	
	bool         get_name(Symbol *sym, std::string &n);
	bool         get_val(Symbol *sym, std::string &v);
	bool         get_val(Symbol *sym, long &v);
	bool         get_val(Symbol *sym, double &v);
	
	template<class T>
	bool         get_const_attr(Symbol *id, const std::string &attr, T &val);
	
	Symbol      *get_wme_id(wme *w);
	Symbol      *get_wme_attr(wme *w);
	Symbol      *get_wme_val(wme *w);

	tc_num       new_tc_num();
	tc_num       get_tc_num(Symbol *s);
	void         set_tc_num(Symbol *s, tc_num n);
	
	int          get_timetag(wme *w);
	Symbol      *get_parent_state(Symbol *sym);
	common_syms &get_common_syms() { return cs; }
	
	void         print(const std::string &msg);
	
private:
	agent* thisAgent;
	common_syms cs;
};


inline Symbol *soar_interface::make_sym(const std::string &val) {
	return make_str_constant(thisAgent, val.c_str());
}

inline Symbol *soar_interface::make_sym(int val) {
	return make_int_constant(thisAgent, val);
}

inline Symbol *soar_interface::make_sym(double val) {
	return make_float_constant(thisAgent, val);
}

inline void soar_interface::del_sym(Symbol *s) {
	symbol_remove_ref(thisAgent, s);
}

template<class T>
wme *soar_interface::make_wme(Symbol *id, const std::string &attr, const T &val) {
	Symbol *valsym = make_sym(val);
	return make_wme(id, attr, valsym);
}

template<class T>
wme *soar_interface::make_wme(Symbol *id, Symbol *attr, const T &val) {
	Symbol *valsym = make_sym(val);
	return make_wme(id, attr, valsym);
}

inline bool soar_interface::is_identifier(Symbol *sym) {
	return sym->symbol_type == IDENTIFIER_SYMBOL_TYPE;
}

inline bool soar_interface::is_string(Symbol *sym) {
	return sym->symbol_type == STR_CONSTANT_SYMBOL_TYPE;
}

inline bool soar_interface::is_int(Symbol *sym) {
	return sym->symbol_type == INT_CONSTANT_SYMBOL_TYPE;
}

inline bool soar_interface::is_float(Symbol *sym) {
	return sym->symbol_type == FLOAT_CONSTANT_SYMBOL_TYPE;
}

inline bool soar_interface::is_state(Symbol *sym) {
	return is_identifier(sym) && sym->id->isa_goal;
}

inline bool soar_interface::is_top_state(Symbol *sym) {
	return is_state(sym) && (sym->id->higher_goal == NULL);
}

inline bool soar_interface::get_name(Symbol *sym, std::string &n) {
	std::stringstream ss;
	if (!is_identifier(sym))
		return false;
	ss << sym->id->name_letter << sym->id->name_number;
	n = ss.str();
	return true;
}

inline bool soar_interface::get_val(Symbol *sym, std::string &v) {
	if (is_string(sym)) {
		v = sym->sc->name;
		return true;
	}
	return false;
}

inline bool soar_interface::get_val(Symbol *sym, long &v) {
	if (is_int(sym)) {
		v = sym->ic->value;
		return true;
	}
	return false;
}

inline bool soar_interface::get_val(Symbol *sym, double &v) {
	if (is_float(sym)) {
		v = sym->fc->value;
		return true;
	}
	if (is_int(sym)) {
		v = static_cast<double>(sym->ic->value);
		return true;
	}
	return false;
}

inline Symbol *soar_interface::get_wme_id(wme *w) {
	return w->id;
}

inline Symbol *soar_interface::get_wme_attr(wme *w) {
	return w->attr;
}

inline Symbol *soar_interface::get_wme_val(wme *w) {
	return w->value;
}

inline tc_num soar_interface::new_tc_num() {
	return get_new_tc_number(thisAgent);
}

inline tc_num soar_interface::get_tc_num(Symbol *s) {
	return s->tc_num;
}

inline void soar_interface::set_tc_num(Symbol *s, tc_num n) {
	s->tc_num = n;
}

inline int soar_interface::get_timetag(wme *w) {
	return w->timetag;
}

inline Symbol *soar_interface::get_parent_state(Symbol *id) {
	return id->id->higher_goal;
}

template<class T>
bool soar_interface::get_const_attr(Symbol *id, const std::string &attr, T &val) {
	wme *w;
	if (!find_child_wme(id, attr, w)) {
		return false;
	}
	return get_val(get_wme_val(w), val);
}

#endif
