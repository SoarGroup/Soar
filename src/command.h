#ifndef COMMAND_H
#define COMMAND_H

#include <string>
#include "soar_interface.h"
#include "timer.h"
#include "cliproxy.h"

class svs_state;
class scene;
class filter;

class command : public cliproxy {
public:
	virtual std::string description() = 0;
	virtual bool early() = 0;
	
	bool update() {
		function_timer t(timers.get_or_add("update"));
		return update_sub();
	}
	
	command(svs_state *state, Symbol *root);
	virtual ~command();

	/* check if any substructure in the command changed */
	bool changed();
	
	/* get the value of a string wme */
	bool get_str_param(const std::string &name, std::string &val);
	
	void set_status(const std::string &s);
	
	Symbol *get_root() { return root; }
	
	svs_state *get_state() { return state; }
	
protected:
	virtual bool update_sub() = 0;
	
private:
	void parse_substructure(int &size, int &max_time);
	void proxy_get_children(std::map<std::string, cliproxy*> &c);
	
	svs_state      *state;
	soar_interface *si;
	Symbol         *root;
	wme            *status_wme;
	std::string     curr_status;
	int             subtree_size;
	int             prev_max_time;
	bool            first;
	
	timer_set       timers;
};

command *make_command(svs_state *state, wme *w);

/* Create a filter from a WM structure. Recursive. */
filter *parse_filter_spec(soar_interface *si, Symbol *root, scene *scn);

#endif
