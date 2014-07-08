#include <string>
#include <utility>
#include <algorithm>
#include <assert.h>
#include "portability.h"
#include "soar_module.h"
#include "symtab.h"
#include "wmem.h"
#include "print.h"
#include "soar_interface.h"
#include "decide.h"

using namespace std;

common_syms::common_syms(soar_interface *si) : si(si) {
	svs    = si->make_sym("svs");
	cmd    = si->make_sym("command");
	scene  = si->make_sym("spatial-scene");
	child  = si->make_sym("child");
	result = si->make_sym("result");
	models = si->make_sym("models");
	id     = si->make_sym("id");
	status = si->make_sym("status");
}

common_syms::~common_syms() {
	si->del_sym(svs);
	si->del_sym(cmd);
	si->del_sym(scene);
	si->del_sym(child);
	si->del_sym(result);
	si->del_sym(models);
	si->del_sym(id);
	si->del_sym(status);
}

soar_interface::soar_interface(agent *a)
: thisAgent(a), cs(this)
{ }

soar_interface::~soar_interface() {
}


wme *soar_interface::make_id_wme(Symbol *id, const string &attr) {
	Symbol *attrsym = make_str_constant(thisAgent, attr.c_str());
	Symbol *valsym = make_new_identifier(thisAgent, attr[0], id->id->level);
	wme* w = soar_module::add_module_wme(thisAgent, id, attrsym, valsym);
	symbol_remove_ref(thisAgent, attrsym);
	symbol_remove_ref(thisAgent, valsym);
	return w;
}

wme *soar_interface::make_id_wme(Symbol *id, Symbol *attr) {
	char n;
	Symbol *val;

	if (attr->symbol_type != STR_CONSTANT_SYMBOL_TYPE ||
	    strlen(attr->sc->name) == 0)
	{
		n = 'a';
	} else {
		n = attr->sc->name[0];
	}

	val = make_new_identifier(thisAgent, n, id->id->level);
	wme* w = soar_module::add_module_wme(thisAgent, id, attr, val);
	symbol_remove_ref(thisAgent, val);
	return w;
}

void soar_interface::remove_wme(wme *w) {
	soar_module::remove_module_wme(thisAgent, w);
}

bool soar_interface::get_child_wmes(Symbol *id, wme_list &childs) {
	slot *s;
	wme *w;

	if (!is_identifier(id)) {
		return false;
	}

	childs.clear();
	for ( s=id->id->slots; s!=NULL; s=s->next ) {
		for ( w=s->wmes; w!=NULL; w=w->next ) {
			childs.push_back( w );
		}
	}

	return true;
}

bool soar_interface::find_child_wme(Symbol *id, const string &attr, wme *&w) {
	slot *s;
	wme *w1;
	string a;

	if (!is_identifier(id)) {
		return false;
	}

	for ( s=id->id->slots; s!=NULL; s=s->next ) {
		for ( w1=s->wmes; w1!=NULL; w1=w1->next ) {
			if (get_val(get_wme_attr(w1), a) && a == attr) {
				w = w1;
				return true;
			}
		}
	}

	return false;
}

wme *soar_interface::make_wme(Symbol *id, Symbol *attr, Symbol *val) {
	wme* w = soar_module::add_module_wme(thisAgent, id, attr, val);
	symbol_remove_ref(thisAgent, val);
	return w;
}

wme *soar_interface::make_wme(Symbol *id, const std::string &attr, Symbol *val) {
	wme* w;
	Symbol *attrsym = make_sym(attr);
	w = make_wme(id, attrsym, val);
	symbol_remove_ref(thisAgent, attrsym);

	return w;
}

void soar_interface::print(const string &msg) {
	print_string(thisAgent, msg.c_str());
}
