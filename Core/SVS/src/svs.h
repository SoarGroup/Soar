#ifndef SVS_H
#define SVS_H

#include <vector>
#include <map>
#include <set>
#include "soar_interface.h"
#include "sgnode.h"
#include "common.h"
#include "model.h"
#include "timer.h"
#include "relation.h"
#include "svs_interface.h"
#include "cliproxy.h"

class command;
class scene;
class multi_model;
class drawer;
class logger_set;

/* working memory scene graph object - mediates between wmes and scene graph nodes */
class sgwme : public sgnode_listener
{
    public:
        sgwme(soar_interface* si, Symbol* ident, sgwme* parent, sgnode* node);
        ~sgwme();
        void node_update(sgnode* n, sgnode::change_type t, const std::string& update_info);
        Symbol* get_id()
        {
            return id;
        }
        sgnode* get_node()
        {
            return node;
        }
        std::map<sgwme*, wme*>* get_childs()
        {
            return &childs;
        }
    private:
        void add_child(sgnode* c);

        // These will create/maintain wmes on the child svs object
        // For properties added to the node
        void delete_property(const std::string& propertyName);
        void update_property(const std::string& propertyName);
        template <class WmeType>
        void set_property(const std::string& propertyName, const WmeType& value);

        sgwme*          parent;
        sgnode*         node;
        Symbol*         id;
        wme*            name_wme;
        soar_interface* soarint;

        std::map<sgwme*, wme*> childs;

        // AM: Puts the properties of the node onto the WM graph
        std::map<std::string, wme*> properties;

};


class svs;

/*
 A single output dimension is described by a name and minimum, maximum,
 default, and increment values.
*/
struct output_dim_spec
{
    std::string name;
    double min;
    double max;
    double def;
    double incr;
};

typedef std::vector<output_dim_spec> output_spec;

struct command_entry
{
    std::string id;
    command* cmd;
    wme* cmd_wme;
    command_entry(std::string id, command* cmd, wme* cmd_wme): id(id), cmd(cmd), cmd_wme(cmd_wme) {}
    bool operator < (const command_entry e) const
    {
        return id.compare(e.id) < 0;
    }
};
typedef std::set<command_entry> command_set;
typedef command_set::iterator command_set_it;
/*
 Each state in the state stack has its own SVS link, scene, models, etc.
*/
class svs_state : public cliproxy
{
    public:
        svs_state(svs* svsp, Symbol* state, soar_interface* soar, scene* scn);
        svs_state(Symbol* state, svs_state* parent);

        ~svs_state();

        void           process_cmds();
        void           update_cmd_results(bool early);
        void           update_scene_num();
        void           clear_scene();

        std::string    get_name() const
        {
            return name;
        }
        int            get_level() const
        {
            return level;
        }
        int            get_scene_num() const
        {
            return scene_num;
        }
        scene*         get_scene() const
        {
            return scn;
        }
        Symbol*        get_state()
        {
            return state;
        }
        svs*           get_svs()
        {
            return svsp;
        }
        multi_model*   get_model()
        {
            return mmdl;
        }
        void set_output(const rvec& out);
        bool get_output(rvec& out) const;
        const output_spec* get_output_spec() const
        {
            return outspec;
        }

        void update_models();

        /*
         Should only be called by svs::state_deletion_callback to save top-state scene
         during init.
        */
        void disown_scene();

    private:
        void init();
        void collect_cmds(Symbol* id, std::set<wme*>& all_cmds);
        void set_default_output();

        void proxy_get_children(std::map<std::string, cliproxy*>& c);
        void cli_out(const std::vector<std::string>& args, std::ostream& os);

        std::string     name;
        svs*            svsp;
        int             level;
        svs_state*      parent;
        scene*          scn;
        sgwme*          root;
        soar_interface* si;

        Symbol* state;
        Symbol* svs_link;
        Symbol* scene_link;
        Symbol* cmd_link;
        Symbol* model_link;

        int scene_num;
        wme* scene_num_wme;

        /* command changes per decision cycle */
        command_set curr_cmds;

        scene_sig                prev_sig;
        relation_table           prev_rels;
        rvec                     prev_pvals;
        multi_model*              mmdl;
        rvec                     next_out;
        const output_spec*       outspec;
        bool learn_models;
        bool test_models;

        timer_set timers;

        logger_set* loggers;
};


class svs : public svs_interface, public cliproxy
{
    public:
        svs(agent* a);
        ~svs();

        void state_creation_callback(Symbol* goal);
        void state_deletion_callback(Symbol* goal);
        void output_callback();
        void input_callback();
        void add_input(const std::string& in);
        std::string get_output() const;
        std::string svs_query(const std::string& query);
        bool add_model(const std::string& name, model* m);
        std::map<std::string, model*>* get_models()
        {
            return &models;
        }

        const output_spec* get_output_spec()
        {
            return &outspec;
        }
        soar_interface* get_soar_interface()
        {
            return si;
        }
        drawer* get_drawer() const
        {
            return draw;
        }
        logger_set* get_loggers()
        {
            return loggers;
        }

        bool do_cli_command(const std::vector<std::string>& args, std::string& output);

        static void mark_filter_dirty_bit() { svs::filter_dirty_bit = true; }
        static bool get_filter_dirty_bit() { return svs::filter_dirty_bit; }
        bool is_enabled() { return enabled; }
        void set_enabled(bool newSetting) { enabled = newSetting; }

    private:
        void proc_input(svs_state* s);
        int  parse_output_spec(const std::string& s);

        void proxy_get_children(std::map<std::string, cliproxy*>& c);
        void cli_connect_viewer(const std::vector<std::string>& args, std::ostream& os);
        void cli_disconnect_viewer(const std::vector<std::string>& args, std::ostream& os);
        void cli_use_models(const std::vector<std::string>& args, std::ostream& os);
        void cli_add_model(const std::vector<std::string>& args, std::ostream& os);

        soar_interface*           si;
        std::vector<svs_state*>   state_stack;
        std::vector<std::string>  env_inputs;
        std::string               env_output;
        output_spec               outspec;
        mutable drawer*           draw;
        bool                      use_models;
        bool                      record_movie;
        scene*                    scn_cache;      // temporarily holds top-state scene during init
        bool enabled;

        std::map<std::string, model*> models;

        timer_set timers;
        logger_set* loggers;

    public:
        static bool filter_dirty_bit;
};

#endif
