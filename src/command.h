#ifndef CMD_WATCHER_H
#define CMD_WATCHER_H

#include <string>
#include "soar_interface.h"
#include "filter.h"
#include "timer.h"

class svs_state;

class command {
public:
	virtual std::string description() = 0;
	virtual bool early() = 0;
	
	bool update() {
		function_timer t(timers.get(UPDATE_T));
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
	
	void cli_inspect(std::ostream &os) const {
		timers.report(os);
	}

protected:
	virtual bool update_sub() = 0;
	
private:
	void parse_substructure(int &size, int &max_time);
	
	svs_state      *state;
	soar_interface *si;
	Symbol         *root;
	wme            *status_wme;
	std::string     curr_status;
	int             subtree_size;
	int             prev_max_time;
	bool            first;
	
	enum Timers { UPDATE_T };
	timer_set       timers;
};

command *make_command(svs_state *state, wme *w);

/* Create a filter from a WM structure. Recursive. */
filter *parse_filter_spec(soar_interface *si, Symbol *root, scene *scn);

#endif
