#ifndef SVS_H
#define SVS_H

#include <vector>
#include <map>
#include <set>
#include "soar_interface.h"
#include "sgnode.h"
#include "common.h"
#include "drawer.h"
#include "model.h"
#include "timer.h"
#include "relation.h"
#include "svs_interface.h"
#include "cliproxy.h"

class command;
class scene;
class multi_model;

/* working memory scene graph object - mediates between wmes and scene graph nodes */
class sgwme : public sgnode_listener {
public:
	sgwme(soar_interface *si, Symbol *ident, sgwme *parent, sgnode *node);
	~sgwme();
	void node_update(sgnode *n, sgnode::change_type t, int added_child);

private:
	void add_child(sgnode *c);
	
	sgwme*          parent;
	sgnode*         node;
	Symbol         *id;
	wme            *name_wme;
	soar_interface *soarint;

	std::map<sgwme*,wme*> childs;

};


class svs;

/*
 A single output dimension is described by a name and minimum, maximum,
 default, and increment values.
*/
struct output_dim_spec {
	std::string name;
	double min;
	double max;
	double def;
	double incr;
};

typedef std::vector<output_dim_spec> output_spec;

/*
 Each state in the state stack has its own SVS link, scene, models, etc.
*/
class svs_state : public cliproxy {
public:
	svs_state(svs *svsp, Symbol *state, soar_interface *soar);
	svs_state(Symbol *state, svs_state *parent);

	~svs_state();
	
	void           process_cmds();
	void           update_cmd_results(bool early);
	void           update_scene_num();
	void           clear_scene();
	
	int            get_level() const     { return level;     }
	int            get_scene_num() const { return scene_num; }
	scene         *get_scene() const     { return scn;       }
	Symbol        *get_state()           { return state;     }
	svs           *get_svs()             { return svsp;      }
	multi_model   *get_model()           { return mmdl;      }
	
	void set_output(const rvec &out);
	bool get_output(rvec &out) const;
	const output_spec *get_output_spec() const { return outspec; }
	
	void update_models();
	void refresh_view();

private:
	void init();
	void collect_cmds(Symbol* id, std::set<wme*>& all_cmds);
	void set_default_output();
	
	void proxy_get_children(std::map<std::string, cliproxy*> &c);
	void cli_relations(const std::vector<std::string> &args, std::ostream &os) const;
	void cli_props(const std::vector<std::string> &args, std::ostream &os);
	void cli_dist(const std::vector<std::string> &args, std::ostream &os) const;
	void cli_cmd(const std::vector<std::string> &args, std::ostream &os);
	void cli_out(const std::vector<std::string> &args, std::ostream &os);

	svs            *svsp;
	int             level;
	svs_state      *parent;
	scene          *scn;
	sgwme          *root;
	soar_interface *si;
	
	Symbol *state;
	Symbol *svs_link;
	Symbol *scene_link;
	Symbol *cmd_link;
	Symbol *model_link;

	int scene_num;
	wme *scene_num_wme;
	
	/* command changes per decision cycle */
	std::map<wme*, command*> curr_cmds;
	
	scene_sig                prev_sig;
	relation_table           prev_rels;
	rvec                     prev_pvals;
	multi_model              *mmdl;
	rvec                     next_out;
	const output_spec       *outspec;
	bool learn_models;
	bool test_models;
	
	timer_set timers;
};

class svs : public svs_interface, public cliproxy {
public:
	svs(agent *a);
	~svs();
	
	void state_creation_callback(Symbol *goal);
	void state_deletion_callback(Symbol *goal);
	void output_callback();
	void input_callback();
	void add_input(const std::string &in);
	std::string get_output() const;
	bool add_model(const std::string &name, model *m);
	std::map<std::string, model*> *get_models() { return &models; }

	soar_interface *get_soar_interface() { return si; }
	
	bool do_cli_command(const std::vector<std::string> &args, std::string &output);
	
	const output_spec *get_output_spec() { return &outspec; }
	
private:
	void proc_input(svs_state *s);
	int  parse_output_spec(const std::string &s);

	void proxy_get_children(std::map<std::string, cliproxy*> &c);
	void cli_log(const std::vector<std::string> &args, std::ostream &os);
	void cli_connect_viewer(const std::vector<std::string> &args, std::ostream &os);
	void cli_disconnect_viewer(const std::vector<std::string> &args, std::ostream &os);

	soar_interface           *si;
	std::vector<svs_state*>   state_stack;
	std::vector<std::string>  env_inputs;
	std::string               env_output;
	output_spec               outspec;
	bool                      learn;
	Symbol                   *model_root;
	
	std::map<std::string, model*> models;
	
	timer_set timers;
};

#endif
