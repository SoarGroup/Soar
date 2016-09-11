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

#include "sml_Names.h"
#include "sml_AgentSML.h"

#include "agent.h"
#include "episodic_memory.h"
#include "lexer.h"
#include "misc.h"
#include "output_manager.h"
#include "semantic_memory.h"
#include "smem_settings.h"
#include "smem_stats.h"
#include "smem_timers.h"
#include "slot.h"
#include "soar_db.h"

using namespace cli;
using namespace sml;

bool CommandLineInterface::DoSMem(const char pOp, const std::string* pArg1, const std::string* pArg2, const std::string* pArg3)
{
    agent* thisAgent = m_pAgentSML->GetSoarAgent();
    std::ostringstream tempString;

    if (!pOp)
    {
        // Print SMem Settings
        PrintCLIMessage_Header("Semantic Memory Settings", 40);
        PrintCLIMessage_Item("enabled:", thisAgent->SMem->settings->learning, 40);
        PrintCLIMessage_Section("Storage", 40);
        PrintCLIMessage_Item("database:", thisAgent->SMem->settings->database, 40);
        PrintCLIMessage_Item("append:", thisAgent->SMem->settings->append_db, 40);
        PrintCLIMessage_Item("path:", thisAgent->SMem->settings->path, 40);
        PrintCLIMessage_Item("lazy-commit:", thisAgent->SMem->settings->lazy_commit, 40);
        PrintCLIMessage_Section("Activation", 40);
        PrintCLIMessage_Item("activation-mode:", thisAgent->SMem->settings->activation_mode, 40);
        PrintCLIMessage_Item("activate-on-query:", thisAgent->SMem->settings->activate_on_query, 40);
        PrintCLIMessage_Item("base-decay:", thisAgent->SMem->settings->base_decay, 40);
        PrintCLIMessage_Item("base-update-policy:", thisAgent->SMem->settings->base_update, 40);
        PrintCLIMessage_Item("base-incremental-threshes:", thisAgent->SMem->settings->base_incremental_threshes, 40);
        PrintCLIMessage_Item("thresh:", thisAgent->SMem->settings->thresh, 40);
        PrintCLIMessage_Section("Performance", 40);
        PrintCLIMessage_Item("page-size:", thisAgent->SMem->settings->page_size, 40);
        PrintCLIMessage_Item("cache-size:", thisAgent->SMem->settings->cache_size, 40);
        PrintCLIMessage_Item("optimization:", thisAgent->SMem->settings->opt, 40);
        PrintCLIMessage_Item("timers:", thisAgent->SMem->settings->timers, 40);
        PrintCLIMessage("");

        return true;
    }
    else if (pOp == 'a')
    {
        std::string* err = new std::string("");
        bool result = thisAgent->SMem->CLI_add(pArg1->c_str(), &(err));

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
        bool result = thisAgent->SMem->backup_db(pArg1->c_str(), &(err));

        if (!result)
        {
            SetError("Error while backing up database: " + err);
        }
        else
        {
            tempString << "Semantic memory database backed up to " << pArg1->c_str();
            PrintCLIMessage(&tempString);
        }

        return result;
    }
    else if (pOp == 'e')
    {
        bool result = thisAgent->SMem->settings->learning->set_string("on");

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
        bool result = thisAgent->SMem->settings->learning->set_string("off");

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
        soar_module::param* my_param = thisAgent->SMem->settings->get(pArg1->c_str());
        if (!my_param)
        {
            return SetError("Invalid semantic memory parameter.  Use 'help smem' to see list of valid settings.");
        }

        PrintCLIMessage_Item("", my_param, 0);
        return true;
    }
    else if (pOp == 'i')
    {
        /* Don't think we need clear out anything else any more */

        thisAgent->SMem->reinit();

        PrintCLIMessage("Semantic memory system re-initialized.");
        return true;
    }
    else if (pOp == 'h')
    {
        uint64_t lti_id = NIL;

        thisAgent->SMem->attach();
        if (!thisAgent->SMem->connected())
        {
            return SetError("Semantic memory database not connected.");
        }

        if (pArg1)
        {
            const char* pAttr_c_str = pArg1->c_str();
            soar::Lexer lexer(thisAgent, pAttr_c_str);
            if (!lexer.get_lexeme()) return SetError("Value not found.");
            if (lexer.current_lexeme.type == AT_LEXEME)
            {
                if (!lexer.get_lexeme()) return SetError("Nothing found after @");
            }
            if (lexer.current_lexeme.type == INT_CONSTANT_LEXEME)
            {
                if (thisAgent->SMem->connected())
                {
                    lti_id = thisAgent->SMem->lti_exists(lexer.current_lexeme.int_val);
                }
            }

            if (lti_id == NIL)
            {
                return SetError("LTI not found.");
            }
        }

        std::string viz;

        thisAgent->SMem->print_smem_object(lti_id, 1, &(viz));
        if (viz.empty())
        {
            return SetError("No history found.");
        }

        PrintCLIMessage(&viz);
        return true;
    }
    else if (pOp == 'q')
    {
        std::string* err = new std::string;
        std::string* retrieved = new std::string;
        uint64_t number_to_retrieve = 1;

        if (pArg2)
        {
            from_c_string(number_to_retrieve, pArg2->c_str());
        }

        bool result = thisAgent->SMem->CLI_query(pArg1->c_str(), &(err), &(retrieved), number_to_retrieve);

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
        if (pArg2)
        {
            force = (!strcmp(pArg2->c_str(), "f") || (!strcmp(pArg2->c_str(), "force")));
        }

        bool result = thisAgent->SMem->CLI_remove(pArg1->c_str(), &(err), &(retrieved), force);

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
        soar_module::param* my_param = thisAgent->SMem->settings->get(pArg1->c_str());
        if (!my_param)
        {
            return SetError("Invalid SMem parameter.");
        }

        if (!my_param->validate_string(pArg2->c_str()))
        {
            return SetError("Invalid setting for SMem parameter.");
        }

        smem_param_container::db_choices last_db_mode = thisAgent->SMem->settings->database->get_value();
        bool result = my_param->set_string(pArg2->c_str());

        if (!result)
        {
            SetError("This parameter is protected while the semantic memory database is open.");
        }
        else
        {
            tempString << pArg1->c_str() << " = " << pArg2->c_str();
            PrintCLIMessage(&tempString);
            if (thisAgent->SMem->connected())
            {
                if (((!strcmp(pArg1->c_str(), "database")) && (thisAgent->SMem->settings->database->get_value() != last_db_mode)) ||
                        (!strcmp(pArg1->c_str(), "path")))
                {
                    PrintCLIMessage("To finalize database switch, issue an smem --init command.\n");
                }
            }
            if (!strcmp(pArg1->c_str(), "append"))
            {
                if (thisAgent->SMem->settings->append_db->get_value() == off)
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
        if (!pArg1)
        {
            // Print SMem Settings
            PrintCLIMessage_Header("Semantic Memory Statistics", 40);
            PrintCLIMessage_Item("SQLite Version:", thisAgent->SMem->statistics->db_lib_version, 40);
            PrintCLIMessage_Item("Memory Usage:", thisAgent->SMem->statistics->mem_usage, 40);
            PrintCLIMessage_Item("Memory Highwater:", thisAgent->SMem->statistics->mem_high, 40);
            PrintCLIMessage_Item("Retrieves:", thisAgent->SMem->statistics->retrievals, 40);
            PrintCLIMessage_Item("Queries:", thisAgent->SMem->statistics->queries, 40);
            PrintCLIMessage_Item("Stores:", thisAgent->SMem->statistics->stores, 40);
            PrintCLIMessage_Item("Activation Updates:", thisAgent->SMem->statistics->act_updates, 40);
            PrintCLIMessage_Item("Nodes:", thisAgent->SMem->statistics->nodes, 40);
            PrintCLIMessage_Item("Edges:", thisAgent->SMem->statistics->edges, 40);
        }
        else
        {
            soar_module::statistic* my_stat = thisAgent->SMem->statistics->get(pArg1->c_str());
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
        if (!pArg1)
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
            thisAgent->SMem->timers->for_each(bar);
        }
        else
        {
            soar_module::timer* my_timer = thisAgent->SMem->timers->get(pArg1->c_str());
            if (!my_timer)
            {
                return SetError("Invalid timer.");
            }

            PrintCLIMessage_Item("", my_timer, 0);
        }

        return true;
    }
    else if (pOp == 'x')
    {
        std::string* err = new std::string("");
        uint64_t lti_id = NIL;

        if (pArg2)
        {
            thisAgent->SMem->attach();
            if (!thisAgent->SMem->connected())
            {
                return SetError("Semantic memory database not connected.");
            }

            const char* pAttr_c_str = pArg2->c_str();
            soar::Lexer lexer(thisAgent, pAttr_c_str);
            if (!lexer.get_lexeme()) return SetError("LTI not found.");
            if (lexer.current_lexeme.type == AT_LEXEME)
            {
                if (!lexer.get_lexeme()) return SetError("Nothing found after @");
            }
            if (lexer.current_lexeme.type == INT_CONSTANT_LEXEME)
            {
                if (thisAgent->SMem->connected())
                {
                    lti_id = thisAgent->SMem->lti_exists(lexer.current_lexeme.int_val);
                }
            }

            if (lti_id == NIL)
            {
                return SetError("LTI not found.");
            }
        }

        std::string export_text;
        bool result = thisAgent->SMem->export_smem(lti_id, export_text, &(err));

        if (!result)
        {
            SetError(*err);
        }
        else
        {
            if (!DoCLog(LOG_NEW, pArg1, 0, true))
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
