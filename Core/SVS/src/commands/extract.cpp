/**********************************************************
 *
 * File: commands/extract.cpp
 * Contains:
 *  class extract_command
 *
 *  Soar Command to extract a filter on the world
 *    Name is either extract or extract_once
 *  Parameters:
 *    ^type <string> - type of the filter
 *    ...  - additional filter-specific parameters
 *
 *  Implementation of soar parsing in filter_table.cpp
 *    parse_filter_spec
 **********************************************************/
#include <iostream>
#include "command.h"
#include "filter.h"
#include "filter_table.h"
#include "svs.h"
#include "soar_interface.h"
#include "symbol.h"
#include "command_table.h"

class extract_command : public command, public filter_input::listener
{
    public:
        extract_command(svs_state* state, Symbol* root, bool once)
            : command(state, root), root(root), res_root(NULL), state(state), fltr(NULL), first(true), once(once)
        {
            si = state->get_svs()->get_soar_interface();
        }

        ~extract_command()
        {
            if (fltr)
            {
                delete fltr;
            }
        }

        std::string description()
        {
            return std::string("extract");
        }

        bool update_sub()
        {
            if (!once && !first && !svs::get_filter_dirty_bit())
            {
                // Don't update filter results if the dirty bit is false
                return true;
            }

            if (!res_root)
            {
                res_root = si->get_wme_val(si->make_id_wme(root, "result"));
            }

            if (changed())
            {
                clear_results();
                if (fltr)
                {
                    delete fltr;
                }

                fltr = parse_filter_spec(state->get_svs()->get_soar_interface(), root, state->get_scene());
                if (!fltr)
                {
                    set_status("incorrect filter syntax");
                    return false;
                }
                fltr->listen_for_input(this);
                first = true;
            }

            if (fltr && (!once || first))
            {
                if (!fltr->update())
                {
                    clear_results();
                    return false;
                }
                update_results();
                fltr->get_output()->clear_changes();
                first = false;
            }
            return true;
        }

        int command_type()
        {
            return SVS_READ_COMMAND;
        }

        void reset_results()
        {
            clear_results();
            filter_output* out = fltr->get_output();
            for (size_t i = 0, iend = out->num_current(); i < iend; ++i)
            {
                handle_output(out->get_current(i));
            }
            out->clear_changes();
        }

        void update_results()
        {
            filter_output* out = fltr->get_output();

            for (size_t i = out->first_added(), iend = out->num_current(); i < iend; ++i)
            {
                handle_output(out->get_current(i));
            }
            for (size_t i = 0, iend = out->num_removed(); i < iend; ++i)
            {
                filter_val* fv = out->get_removed(i);
                record r;
                if (!map_pop(records, fv, r))
                {
                    assert(false);
                }
                si->remove_wme(r.rec_wme);
            }
            for (size_t i = 0, iend = out->num_changed(); i < iend; ++i)
            {
                handle_output(out->get_changed(i));
            }
        }

        void clear_results()
        {
            record_map::iterator i, iend;
            for (i = records.begin(), iend = records.end(); i != iend; ++i)
            {
                si->remove_wme(i->second.rec_wme);
            }
            records.clear();
        }

    private:
        wme* make_filter_val_wme(Symbol* id, const std::string& attr, filter_val* v)
        {
            int iv;
            double fv;
            bool bv;
            Symbol* single_val = NULL;

            if (get_filter_val(v, iv))
            {
                single_val = si->make_sym(iv);
            }
            else if (get_filter_val(v, fv))
            {
                single_val = si->make_sym(fv);
            }
            else if (get_filter_val(v, bv))
            {
                single_val = si->make_sym(bv ? "true" : "false");
            }

            if (single_val != NULL)
            {
                return si->make_wme(id, attr, single_val);
            }

            std::map<std::string, std::string> rep;
            std::string def;
            v->get_rep(rep);
            if (map_get(rep, std::string(""), def))
            {
                single_val = si->make_sym(def);
                return si->make_wme(id, attr, single_val);
            }

            wme* w = si->make_id_wme(id, attr);
            Symbol* subid = si->get_wme_val(w);
            std::map<std::string, std::string>::const_iterator i, iend;
            for (i = rep.begin(), iend = rep.end(); i != iend; ++i)
            {
                double dval;
                if (parse_double(i->second, dval))
                {
                    si->make_wme(subid, i->first, dval);
                }
                else
                {
                    si->make_wme(subid, i->first, i->second);
                }
            }
            return w;
        }

        bool sym_reps_filter_val(Symbol* s, const filter_val* fv)
        {
            long fiv, siv;
            double ffv, sfv;
            bool fbv;
            std::string str;

            if (get_filter_val(fv, fiv))
            {
                return (get_symbol_value(s, siv) && siv == fiv);
            }
            if (get_filter_val(fv, ffv))
            {
                return (get_symbol_value(s, sfv) && sfv == ffv);
            }
            if (get_filter_val(fv, fbv))
            {
                return (get_symbol_value(s, str) && ((fbv && str == "t") || (!fbv && str == "f")));
            }

            std::map<std::string, std::string> rep;
            std::string def;

            fv->get_rep(rep);
            if (map_get(rep, std::string(""), def))
            {
                get_symbol_value(s, str);
                return str == def;
            }

            /*
             The filter_val has a struct representation. For now, always treat the symbol
             as being different. In the future, maybe compare the substructure of an id
             and the map representation to see if they have the same attribute-value
             pairs.
            */
            return false;
        }

        wme* make_value_wme(filter_val* v, Symbol* root)
        {
            return make_filter_val_wme(root, "value", v);
        }

        void update_param_struct(const filter_params* p, Symbol* pid)
        {
            filter_params::const_iterator j;
            for (j = p->begin(); j != p->end(); ++j)
            {
                wme* pwme = NULL;
                if (!si->find_child_wme(pid, j->first, pwme) ||
                        !sym_reps_filter_val(si->get_wme_val(pwme), j->second))
                {
                    if (pwme)
                    {
                        si->remove_wme(pwme);
                    }
                    make_filter_val_wme(pid, j->first, j->second);
                }
            }
        }

        void make_record(filter_val* output)
        {
            record r;
            r.rec_wme = si->make_id_wme(res_root, "record");
            r.rec_id = si->get_wme_val(r.rec_wme);
            r.val_wme = make_value_wme(output, r.rec_id);
            r.params_wme = si->make_id_wme(r.rec_id, "params");
            fltr->get_output_params(output, r.params);
            if (r.params)
            {
                update_param_struct(r.params, si->get_wme_val(r.params_wme));
            }
            records[output] = r;
            output->reset_dirty();
        }

        void handle_output(filter_val* output)
        {
            record* r;
            if ((r = map_getp(records, output)))
            {
                if (output->is_dirty())
                {
                    si->remove_wme(r->val_wme);
                    r->val_wme = make_value_wme(output, r->rec_id);
                }
                output->reset_dirty();
            }
            else
            {
                make_record(output);
            }
        }


        void handle_ctlist_change(const filter_params* p)
        {
            record_map::iterator i;

            for (i = records.begin(); i != records.end(); ++i)
            {
                if (i->second.params == p)
                {
                    Symbol* pid = si->get_wme_val(i->second.params_wme);
                    update_param_struct(p, pid);
                    return;
                }
            }
        }

        Symbol*         root;
        Symbol*         res_root;
        Symbol*         pos_root;  // identifier for positive atoms
        Symbol*         neg_root;  // identifier for negative atoms
        svs_state*      state;
        soar_interface* si;
        filter*         fltr;
        bool            first, once;

        struct record
        {
            const filter_params* params;
            wme* rec_wme;
            wme* val_wme;
            wme* params_wme;
            Symbol* rec_id;
        };

        typedef std::map<filter_val*, record> record_map;
        record_map records;
};

command* _make_extract_command_(svs_state* state, Symbol* root)
{
    return new extract_command(state, root, false);
}

command* _make_extract_once_command_(svs_state* state, Symbol* root)
{
    return new extract_command(state, root, true);
}

command_table_entry* extract_command_entry()
{
    command_table_entry* e = new command_table_entry();
    e->name = "extract";
    e->description = "Continually extracts a filter";
    e->parameters["type"] = "Type of the filter to extract";
    e->parameters["other"] = "See specific filter for other parameters";
    e->create = &_make_extract_command_;
    return e;
}

command_table_entry* extract_once_command_entry()
{
    command_table_entry* e = new command_table_entry();
    e->name = "extract_once";
    e->description = "Extracts a filter once";
    e->parameters["type"] = "Type of the filter to extract";
    e->parameters["other"] = "See specific filter for other parameters";
    e->create = &_make_extract_once_command_;
    return e;
}
