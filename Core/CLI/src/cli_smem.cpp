/////////////////////////////////////////////////////////////////
// smem command file.
//
// Author: Jonathan Voigt, voigtjr@gmail.com,
//         Nate Derbinsky, nlderbin@umich.edu
// Date  : 2009
//
/////////////////////////////////////////////////////////////////

#include "portability.h"

#include "cli_CommandLineInterface.h"
#include "cli_Commands.h"

#include "agent.h"
#include "lexer.h"
#include "misc.h"
#include "semantic_memory.h"
#include "smem_settings.h"
#include "smem_stats.h"
#include "smem_timers.h"
#include "slot.h"
#include "sml_Names.h"
#include "sml_AgentSML.h"
#include "soar_db.h"
#include "episodic_memory.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::DoSMem(const char pOp, const std::string* pAttr, const std::string* pVal)
{
    agent* thisAgent = m_pAgentSML->GetSoarAgent();
    std::ostringstream tempString;

    if (!pOp)
    {
        // Print SMem Settings
        PrintCLIMessage_Header("Semantic Memory Settings", 40);
        PrintCLIMessage_Item("learning:", thisAgent->SMem->smem_params->learning, 40);
        PrintCLIMessage_Section("Storage", 40);
        PrintCLIMessage_Item("database:", thisAgent->SMem->smem_params->database, 40);
        PrintCLIMessage_Item("append:", thisAgent->SMem->smem_params->append_db, 40);
        PrintCLIMessage_Item("path:", thisAgent->SMem->smem_params->path, 40);
        PrintCLIMessage_Item("lazy-commit:", thisAgent->SMem->smem_params->lazy_commit, 40);
        PrintCLIMessage_Section("Activation", 40);
        PrintCLIMessage_Item("activation-mode:", thisAgent->SMem->smem_params->activation_mode, 40);
        PrintCLIMessage_Item("activate-on-query:", thisAgent->SMem->smem_params->activate_on_query, 40);
        PrintCLIMessage_Item("base-decay:", thisAgent->SMem->smem_params->base_decay, 40);
        PrintCLIMessage_Item("base-update-policy:", thisAgent->SMem->smem_params->base_update, 40);
        PrintCLIMessage_Item("base-incremental-threshes:", thisAgent->SMem->smem_params->base_incremental_threshes, 40);
        PrintCLIMessage_Item("thresh:", thisAgent->SMem->smem_params->thresh, 40);
        PrintCLIMessage_Section("Performance", 40);
        PrintCLIMessage_Item("page-size:", thisAgent->SMem->smem_params->page_size, 40);
        PrintCLIMessage_Item("cache-size:", thisAgent->SMem->smem_params->cache_size, 40);
        PrintCLIMessage_Item("optimization:", thisAgent->SMem->smem_params->opt, 40);
        PrintCLIMessage_Item("timers:", thisAgent->SMem->smem_params->timers, 40);
        PrintCLIMessage_Section("Experimental", 40);
        PrintCLIMessage_Item("merge:", thisAgent->SMem->smem_params->merge, 40);
        PrintCLIMessage_Item("mirroring:", thisAgent->SMem->smem_params->mirroring, 40);
        PrintCLIMessage("");

        return true;
    }
    else if (pOp == 'a')
    {
        std::string* err = new std::string("");
        bool result = thisAgent->SMem->parse_chunks(pAttr->c_str(), &(err));

        if (!result)
        {
            SetError(*err);
        }
        else
        {
            PrintCLIMessage("Knowledge added to semantic memory.");
        }
        delete err;
        return result;
    }
    else if (pOp == 'b')
    {
        std::string err;
        bool result = thisAgent->SMem->backup_db(pAttr->c_str(), &(err));

        if (!result)
        {
            SetError("Error while backing up database: " + err);
        }
        else
        {
            tempString << "Semantic memory database backed up to " << pAttr->c_str();
            PrintCLIMessage(&tempString);
        }

        return result;
    }
    else if (pOp == 'e')
    {
        bool result = thisAgent->SMem->smem_params->learning->set_string("on");

        if (!result)
        {
            SetError("This parameter is protected while the semantic memory database is open.");
        }
        else
        {
            PrintCLIMessage("Semantic memory enabled.");
        }

        return result;
    }
    else if (pOp == 'd')
    {
        bool result = thisAgent->SMem->smem_params->learning->set_string("off");

        if (!result)
        {
            SetError("This parameter is protected while the semantic memory database is open.");
        }
        else
        {
            PrintCLIMessage("Semantic memory disabled.");
        }

        return result;
    }
    else if (pOp == 'g')
    {
        soar_module::param* my_param = thisAgent->SMem->smem_params->get(pAttr->c_str());
        if (!my_param)
        {
            return SetError("Invalid semantic memory parameter.  Use 'help smem' to see list of valid settings.");
        }

        PrintCLIMessage_Item("", my_param, 0);
        return true;
    }
    else if (pOp == 'h')
    {
        smem_lti_id lti_id = NIL;
        uint64_t depth = 1;
        bool history = true;
        thisAgent->SMem->attach();

        if (thisAgent->SMem->smem_db->get_status() != soar_module::connected)
        {
            return SetError("Semantic memory database not connected.");
        }

        if (pAttr)
        {
            soar::Lexeme lexeme = soar::Lexer::get_lexeme_from_string(thisAgent, pAttr->c_str());
            if (lexeme.type == IDENTIFIER_LEXEME)
            {
                lti_id = thisAgent->SMem->lti_get_id(lexeme.id_letter, lexeme.id_number);
            }
            if (lti_id == NIL)
            {
                return SetError("LTI not found");
            }
        }

        std::string viz;

        thisAgent->SMem->print_lti(lti_id, depth, &(viz), history);

        if (viz.empty())
        {
            return SetError("Could not find information on LTI.");
        }

        PrintCLIMessage_Header("Semantic Memory", 40);
        PrintCLIMessage(&viz);
        return true;
    }
    else if (pOp == 'i')
    {
        // Because of LTIs, re-initializing requires all other memories to be reinitialized.
        // epmem - close before working/production memories to get re-init benefits
        // smem - close before working/production memories to prevent id counter mess-ups
        // production memory (automatic init-soar clears working memory as a result)

        epmem_reinit_cmd(thisAgent);
        thisAgent->SMem->reinit_cmd();

        ExciseBitset options(0);
        options.set(EXCISE_ALL, true);
        DoExcise(options, 0);

        PrintCLIMessage("Semantic memory system re-initialized.");
        return true;
    }
    else if (pOp == 'p')
    {
        smem_lti_id lti_id = NIL;
        unsigned int depth = 1;

        thisAgent->SMem->attach();
        if (thisAgent->SMem->smem_db->get_status() != soar_module::connected)
        {
            return SetError("Semantic memory database not connected.");
        }

        if (pAttr)
        {
            const char* pAttr_c_str = pAttr->c_str();
            soar::Lexer lexer(thisAgent, pAttr_c_str);
            if (!lexer.get_lexeme()) return SetError("Value not found.");
            if (lexer.current_lexeme.type == AT_LEXEME)
            {
                if (!lexer.get_lexeme()) return SetError("Nothing found after @");
            }
            if (lexer.current_lexeme.type == IDENTIFIER_LEXEME)
            {
                if (thisAgent->SMem->smem_db->get_status() == soar_module::connected)
                {
                    lti_id = thisAgent->SMem->lti_get_id(lexer.current_lexeme.id_letter, lexer.current_lexeme.id_number);

                    if ((lti_id != NIL) && pVal)
                    {
                        from_c_string(depth, pVal->c_str());
                    }
                }
            }

            if (lti_id == NIL)
            {
                return SetError("LTI not found.");
            }
        }

        std::string viz;

        if (lti_id == NIL)
        {
            thisAgent->SMem->print_store(&(viz));
            if (!viz.empty())
            {
                PrintCLIMessage_Header("Semantic Memory", 40);
        }
        }
        else
        {
            thisAgent->SMem->print_lti(lti_id, depth, &(viz));
        }
        if (viz.empty())
        {
            return SetError("Semantic memory is empty.");
        }

        PrintCLIMessage(&viz);
        return true;
    }
    else if (pOp == 'q')
    {
        std::string* err = new std::string;
        std::string* retrieved = new std::string;
        uint64_t number_to_retrieve = 1;

        if (pVal)
        {
            from_c_string(number_to_retrieve, pVal->c_str());
        }

        bool result = thisAgent->SMem->parse_cues(pAttr->c_str(), &(err), &(retrieved), number_to_retrieve);

        if (!result)
        {
            SetError("Error while parsing query\n" + *err);
        }
        else
        {
            PrintCLIMessage(retrieved);
            PrintCLIMessage("SMem| Query complete.");
        }
        delete err;
        delete retrieved;
        return result;
    }
    else if (pOp == 'r')
    {
        std::string* err = new std::string;
        std::string* retrieved = new std::string;
        bool force = false;
        if (pVal)
        {
            force = (!strcmp(pVal->c_str(), "f") || (!strcmp(pVal->c_str(), "force")));
        }

        bool result = thisAgent->SMem->parse_remove(pAttr->c_str(), &(err), &(retrieved), force);

        if (!result)
        {
            SetError("Error while attempting removal.\n" + *err);
        }
        else
        {
            PrintCLIMessage(retrieved);
            PrintCLIMessage(err);
            PrintCLIMessage("SMem| Removal complete.");
        }
        delete err;
        delete retrieved;
        return result;
    }
    else if (pOp == 's')
    {
        soar_module::param* my_param = thisAgent->SMem->smem_params->get(pAttr->c_str());
        if (!my_param)
        {
            return SetError("Invalid SMem parameter.");
        }

        if (!my_param->validate_string(pVal->c_str()))
        {
            return SetError("Invalid setting for SMem parameter.");
        }

        smem_param_container::db_choices last_db_mode = thisAgent->SMem->smem_params->database->get_value();
        bool result = my_param->set_string(pVal->c_str());

        if (!result)
        {
            SetError("This parameter is protected while the semantic memory database is open.");
        }
        else
        {
            tempString << pAttr->c_str() << " = " << pVal->c_str();
            PrintCLIMessage(&tempString);
            if (thisAgent->SMem->smem_db->get_status() == soar_module::connected)
            {
                if (((!strcmp(pAttr->c_str(), "database")) && (thisAgent->SMem->smem_params->database->get_value() != last_db_mode)) ||
                        (!strcmp(pAttr->c_str(), "path")))
                {
                    PrintCLIMessage("To finalize database switch, issue an smem --init command.\n");
                }
            }
            if (!strcmp(pAttr->c_str(), "append"))
            {
                if (thisAgent->SMem->smem_params->append_db->get_value() == off)
                {
                    PrintCLIMessage("Warning: Since append mode is off, starting/reinitializing,\n"
                                    "         Soar will erase the semantic memory database.\n");
        }

            }
        }
        return result;
    }
    else if (pOp == 'S')
    {
        thisAgent->SMem->attach();
        if (!pAttr)
        {
            // Print SMem Settings
            PrintCLIMessage_Header("Semantic Memory Statistics", 40);
            PrintCLIMessage_Item("SQLite Version:", thisAgent->SMem->smem_stats->db_lib_version, 40);
            PrintCLIMessage_Item("Memory Usage:", thisAgent->SMem->smem_stats->mem_usage, 40);
            PrintCLIMessage_Item("Memory Highwater:", thisAgent->SMem->smem_stats->mem_high, 40);
            PrintCLIMessage_Item("Retrieves:", thisAgent->SMem->smem_stats->expansions, 40);
            PrintCLIMessage_Item("Queries:", thisAgent->SMem->smem_stats->cbr, 40);
            PrintCLIMessage_Item("Stores:", thisAgent->SMem->smem_stats->stores, 40);
            PrintCLIMessage_Item("Activation Updates:", thisAgent->SMem->smem_stats->act_updates, 40);
            PrintCLIMessage_Item("Mirrors:", thisAgent->SMem->smem_stats->mirrors, 40);
            PrintCLIMessage_Item("Nodes:", thisAgent->SMem->smem_stats->chunks, 40);
            PrintCLIMessage_Item("Edges:", thisAgent->SMem->smem_stats->slots, 40);
        }
        else
        {
            soar_module::statistic* my_stat = thisAgent->SMem->smem_stats->get(pAttr->c_str());
            if (!my_stat)
            {
                return SetError("Invalid statistic.");
            }

            PrintCLIMessage_Item("", my_stat, 0);
        }

        return true;
    }
    else if (pOp == 't')
    {
        if (!pAttr)
        {
            struct foo: public soar_module::accumulator< soar_module::timer* >
            {
                private:
                    bool raw;
                    cli::CommandLineInterface* this_cli;
                    std::ostringstream& m_Result;

                    foo& operator=(const foo&)
                    {
                        return *this;
                    }

                public:
                    foo(bool m_RawOutput, cli::CommandLineInterface* new_cli, std::ostringstream& m_Result): raw(m_RawOutput), this_cli(new_cli), m_Result(m_Result) {};

                    void operator()(soar_module::timer* t)
                    {
                        std::string output(t->get_name());
                        output += ":";
                        this_cli->PrintCLIMessage_Item(output.c_str(), t, 40);
                    }
            } bar(m_RawOutput, this, m_Result);

            PrintCLIMessage_Header("Semantic Memory Timers", 40);
            thisAgent->SMem->smem_timers->for_each(bar);
        }
        else
        {
            soar_module::timer* my_timer = thisAgent->SMem->smem_timers->get(pAttr->c_str());
            if (!my_timer)
            {
                return SetError("Invalid timer.");
            }

            PrintCLIMessage_Item("", my_timer, 0);
        }

        return true;
    }
    else if (pOp == 'v')
    {
        smem_lti_id lti_id = NIL;
        unsigned int depth = 1;

        // visualizing the store requires an open semantic database
        thisAgent->SMem->attach();

        if (pAttr)
        {
            soar::Lexeme lexeme = soar::Lexer::get_lexeme_from_string(thisAgent, pAttr->c_str());
            if (lexeme.type == IDENTIFIER_LEXEME)
            {
                if (thisAgent->SMem->smem_db->get_status() == soar_module::connected)
                {
                    lti_id = thisAgent->SMem->lti_get_id(lexeme.id_letter, lexeme.id_number);

                    if ((lti_id != NIL) && pVal)
                    {
                        from_c_string(depth, pVal->c_str());
                    }
                }
            }

            if (lti_id == NIL)
            {
                return SetError("Invalid long-term identifier.");
            }
        }

        std::string viz;

        if (lti_id == NIL)
        {
            thisAgent->SMem->visualize_store(&(viz));
        }
        else
        {
            thisAgent->SMem->visualize_lti(lti_id, depth, &(viz));
        }

        if (viz.empty())
        {
            return SetError("Nothing to visualize.");
        }
        PrintCLIMessage(&viz);

        return true;
    }
    else if (pOp == 'x')
    {
        std::string* err = new std::string("smem_export.soar");
        uint64_t lti_id = NIL;

        std::string export_text;
        bool result = thisAgent->SMem->export_smem(0, export_text, &(err));

        if (!result)
        {
            SetError(*err);
        }
        else
        {
            if (!DoCLog(LOG_NEW, err, 0, true))
            {
                return false;
            }

            if (!DoCLog(LOG_ADD, 0, &export_text, true))
            {
                return false;
            }

            if (!DoCLog(LOG_CLOSE, 0, 0, true))
            {
                return false;
            }

            PrintCLIMessage("Exported semantic memory to file.");
        }
        delete err;
        return result;
    }

    return SetError("Unknown option.");
}
