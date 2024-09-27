#ifndef SVS_H
#define SVS_H

#include <vector>
#include <map>
#include <set>
#include "soar_interface.h"
#include "sgnode.h"
#include "common.h"
#include "svs_interface.h"
#include "cliproxy.h"

class command;
class scene;
class drawer;

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

        // Functions dealing with maintaining tags on sgnodes
        void update_tag(const std::string& tag_name);
        void delete_tag(const std::string& tag_name);
        void set_tag(const std::string& tag_name, const std::string& tag_value);

        sgwme*          parent;
        sgnode*         node;
        Symbol*         id;
        wme*            id_wme;
        soar_interface* soarint;

        std::map<sgwme*, wme*> childs;

        std::map<std::string, wme*> tags;
};


class svs;

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
 Each state in the state stack has its own SVS link, scene, etc.
*/
class svs_state : public cliproxy
{
    public:
        svs_state(svs* svsp, Symbol* state, soar_interface* soar, scene* scn);
        svs_state(Symbol* state, svs_state* parent);

        ~svs_state();

        void           process_cmds();
        void           update_cmd_results(int command_type);
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

        /*
         Should only be called by svs::state_deletion_callback to save top-state scene
         during init.
        */
        void disown_scene();

    private:
        void init();
        void collect_cmds(Symbol* id, std::set<wme*>& all_cmds);

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

        int scene_num;
        wme* scene_num_wme;

        /* command changes per decision cycle */
        command_set curr_cmds;
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
        std::string svs_query(const std::string& query);

        soar_interface* get_soar_interface()
        {
            return si;
        }
        drawer* get_drawer() const
        {
            return draw;
        }

        bool do_cli_command(const std::vector<std::string>& args, std::string& output);

        bool is_enabled()
        {
            return enabled;
        }
        void set_enabled(bool is_enabled)
        {
            enabled = is_enabled;
        }

        bool is_enabled_in_substates()
        {
            return enabled_in_substates;
        }
        void set_enabled_in_substates(bool disable)
        {
            enabled_in_substates = disable;
        }

        std::string get_output() const
        {
            return "";
        }

        bool is_in_substate()
        {
            return state_stack.size() > 1;
        }

        // dirty bit is true only if there has been a new command
        //   from soar or from SendSVSInput
        //   (no need to recheck filters)
        static void mark_filter_dirty_bit()
        {
            svs::filter_dirty_bit = true;
        }

        static bool get_filter_dirty_bit()
        {
            return svs::filter_dirty_bit;
        }

    private:
        void proc_input(svs_state* s);

        void proxy_get_children(std::map<std::string, cliproxy*>& c);
        // For consistency with the rest of Soar, we allow scene names to be lower or upper case (s1 or S1)
        bool proxy_uppercase_paths()
        {
            return true;
        }
        void cli_connect_viewer(const std::vector<std::string>& args, std::ostream& os);
        void cli_disconnect_viewer(const std::vector<std::string>& args, std::ostream& os);

        soar_interface*           si;
        std::vector<svs_state*>   state_stack;
        std::vector<std::string>  env_inputs;
        std::string               env_output;
        mutable drawer*           draw;
        scene*                    scn_cache;      // temporarily holds top-state scene during init

        bool enabled;
        // when enabled is true but enabled_in_substates is false, only the top state is enabled
        bool enabled_in_substates = true;

        static bool filter_dirty_bit;
};

#endif
